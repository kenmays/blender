/* SPDX-FileCopyrightText: 2013 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup edobj
 */

#include "DNA_object_types.h"
#include "DNA_scene_types.h"
#include "DNA_view3d_types.h"

#include "BKE_context.hh"

#include "BLI_math_matrix.h"
#include "BLI_math_rotation.h"
#include "BLI_math_vector.h"

#include "RNA_access.hh"
#include "RNA_define.hh"

#include "WM_api.hh"
#include "WM_types.hh"

#include "ED_object.hh"
#include "ED_transverts.hh"

#include "object_intern.hh"

namespace blender::ed::object {

static void object_warp_calc_view_matrix(float r_mat_view[4][4],
                                         float r_center_view[3],
                                         Object *obedit,
                                         const float viewmat[4][4],
                                         const float center[3],
                                         const float offset_angle)
{
  float mat_offset[4][4];
  float viewmat_roll[4][4];

  /* apply the rotation offset by rolling the view */
  axis_angle_to_mat4_single(mat_offset, 'Z', offset_angle);
  mul_m4_m4m4(viewmat_roll, mat_offset, viewmat);

  /* apply the view and the object matrix */
  mul_m4_m4m4(r_mat_view, viewmat_roll, obedit->object_to_world().ptr());

  /* get the view-space cursor */
  mul_v3_m4v3(r_center_view, viewmat_roll, center);
}

static void object_warp_transverts_minmax_x(TransVertStore *tvs,
                                            const float mat_view[4][4],
                                            const float center_view[3],
                                            float *r_min,
                                            float *r_max)
{
  /* no need to apply translation and cursor offset for every vertex, delay this */
  const float x_ofs = (mat_view[3][0] - center_view[0]);
  float min = FLT_MAX, max = -FLT_MAX;

  TransVert *tv = tvs->transverts;
  for (int i = 0; i < tvs->transverts_tot; i++, tv++) {
    float val;

    /* Convert object-space to view-space. */
    val = dot_m4_v3_row_x(mat_view, tv->loc);

    min = min_ff(min, val);
    max = max_ff(max, val);
  }

  *r_min = min + x_ofs;
  *r_max = max + x_ofs;
}

static void object_warp_transverts(TransVertStore *tvs,
                                   const float mat_view[4][4],
                                   const float center_view[3],
                                   const float angle_,
                                   const float min,
                                   const float max)
{
  TransVert *tv;
  const float angle = -angle_;
/* cache vars for tiny speedup */
#if 1
  const float range = max - min;
  const float range_inv = 1.0f / range;
  const float min_ofs = min + (0.5f * range);
#endif

  float dir_min[2], dir_max[2];
  float imat_view[4][4];

  invert_m4_m4(imat_view, mat_view);

  /* calculate the direction vectors outside min/max range */
  {
    const float phi = angle * 0.5f;

    dir_max[0] = cosf(phi);
    dir_max[1] = sinf(phi);

    dir_min[0] = -dir_max[0];
    dir_min[1] = dir_max[1];
  }

  tv = tvs->transverts;
  for (int i = 0; i < tvs->transverts_tot; i++, tv++) {
    float co[3], co_add[2];
    float val, phi;

    /* Convert object-space to view-space. */
    mul_v3_m4v3(co, mat_view, tv->loc);
    sub_v2_v2(co, center_view);

    val = co[0];
    /* is overwritten later anyway */
    // co[0] = 0.0f;

    if (val < min) {
      mul_v2_v2fl(co_add, dir_min, min - val);
      val = min;
    }
    else if (val > max) {
      mul_v2_v2fl(co_add, dir_max, val - max);
      val = max;
    }
    else {
      zero_v2(co_add);
    }

/* map from x axis to (-0.5 - 0.5) */
#if 0
    val = ((val - min) / (max - min)) - 0.5f;
#else
    val = (val - min_ofs) * range_inv;
#endif

    /* convert the x axis into a rotation */
    phi = val * angle;

    co[0] = -sinf(phi) * co[1];
    co[1] = cosf(phi) * co[1];

    add_v2_v2(co, co_add);

    /* Convert view-space to object-space. */
    add_v2_v2(co, center_view);
    mul_v3_m4v3(tv->loc, imat_view, co);
  }
}

static wmOperatorStatus object_warp_verts_exec(bContext *C, wmOperator *op)
{
  const float warp_angle = RNA_float_get(op->ptr, "warp_angle");
  const float offset_angle = RNA_float_get(op->ptr, "offset_angle");

  TransVertStore tvs = {nullptr};
  Object *obedit = CTX_data_edit_object(C);

  /* typically from 'rv3d' and 3d cursor */
  float viewmat[4][4];
  float center[3];

  /* 'viewmat' relative vars */
  float mat_view[4][4];
  float center_view[3];

  float min, max;

  if (shape_key_report_if_locked(obedit, op->reports)) {
    return OPERATOR_CANCELLED;
  }

  ED_transverts_create_from_obedit(&tvs, obedit, TM_ALL_JOINTS | TM_SKIP_HANDLES);
  if (tvs.transverts == nullptr) {
    return OPERATOR_CANCELLED;
  }

  /* Get view-matrix. */
  {
    PropertyRNA *prop_viewmat = RNA_struct_find_property(op->ptr, "viewmat");
    if (RNA_property_is_set(op->ptr, prop_viewmat)) {
      RNA_property_float_get_array(op->ptr, prop_viewmat, (float *)viewmat);
    }
    else {
      RegionView3D *rv3d = CTX_wm_region_view3d(C);

      if (rv3d) {
        copy_m4_m4(viewmat, rv3d->viewmat);
      }
      else {
        unit_m4(viewmat);
      }

      RNA_property_float_set_array(op->ptr, prop_viewmat, (float *)viewmat);
    }
  }

  /* get center */
  {
    PropertyRNA *prop_center = RNA_struct_find_property(op->ptr, "center");
    if (RNA_property_is_set(op->ptr, prop_center)) {
      RNA_property_float_get_array(op->ptr, prop_center, center);
    }
    else {
      const Scene *scene = CTX_data_scene(C);
      copy_v3_v3(center, scene->cursor.location);

      RNA_property_float_set_array(op->ptr, prop_center, center);
    }
  }

  object_warp_calc_view_matrix(mat_view, center_view, obedit, viewmat, center, offset_angle);

  /* get minmax */
  {
    PropertyRNA *prop_min = RNA_struct_find_property(op->ptr, "min");
    PropertyRNA *prop_max = RNA_struct_find_property(op->ptr, "max");

    if (RNA_property_is_set(op->ptr, prop_min) || RNA_property_is_set(op->ptr, prop_max)) {
      min = RNA_property_float_get(op->ptr, prop_min);
      max = RNA_property_float_get(op->ptr, prop_max);
    }
    else {
      /* handy to set the bounds of the mesh */
      object_warp_transverts_minmax_x(&tvs, mat_view, center_view, &min, &max);

      RNA_property_float_set(op->ptr, prop_min, min);
      RNA_property_float_set(op->ptr, prop_max, max);
    }

    if (min > max) {
      std::swap(min, max);
    }
  }

  if (min != max) {
    object_warp_transverts(&tvs, mat_view, center_view, warp_angle, min, max);
  }

  ED_transverts_update_obedit(&tvs, obedit);
  ED_transverts_free(&tvs);

  WM_event_add_notifier(C, NC_OBJECT | ND_DRAW, obedit);

  return OPERATOR_FINISHED;
}

void TRANSFORM_OT_vertex_warp(wmOperatorType *ot)
{
  PropertyRNA *prop;

  /* identifiers */
  ot->name = "Warp";
  ot->description = "Warp vertices around the cursor";
  ot->idname = "TRANSFORM_OT_vertex_warp";

  /* API callbacks. */
  ot->exec = object_warp_verts_exec;
  ot->poll = ED_transverts_poll;

  /* flags */
  ot->flag = OPTYPE_REGISTER | OPTYPE_UNDO;

  /* props */
  prop = RNA_def_float(ot->srna,
                       "warp_angle",
                       DEG2RADF(360.0f),
                       -FLT_MAX,
                       FLT_MAX,
                       "Warp Angle",
                       "Amount to warp about the cursor",
                       DEG2RADF(-360.0f),
                       DEG2RADF(360.0f));
  RNA_def_property_subtype(prop, PROP_ANGLE);

  prop = RNA_def_float(ot->srna,
                       "offset_angle",
                       DEG2RADF(0.0f),
                       -FLT_MAX,
                       FLT_MAX,
                       "Offset Angle",
                       "Angle to use as the basis for warping",
                       DEG2RADF(-360.0f),
                       DEG2RADF(360.0f));
  RNA_def_property_subtype(prop, PROP_ANGLE);

  prop = RNA_def_float(ot->srna, "min", -1.0f, -FLT_MAX, FLT_MAX, "Min", "", -100.0, 100.0);
  prop = RNA_def_float(ot->srna, "max", 1.0f, -FLT_MAX, FLT_MAX, "Max", "", -100.0, 100.0);

  /* hidden props */
  prop = RNA_def_float_matrix(
      ot->srna, "viewmat", 4, 4, nullptr, 0.0f, 0.0f, "Matrix", "", 0.0f, 0.0f);
  RNA_def_property_flag(prop, PROP_HIDDEN | PROP_SKIP_SAVE);

  prop = RNA_def_float_vector_xyz(
      ot->srna, "center", 3, nullptr, -FLT_MAX, FLT_MAX, "Center", "", -FLT_MAX, FLT_MAX);
  RNA_def_property_flag(prop, PROP_HIDDEN | PROP_SKIP_SAVE);
}

}  // namespace blender::ed::object
