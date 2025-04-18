/* SPDX-FileCopyrightText: 2024 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup overlay
 *
 * Draws object axes extra display.
 */

#pragma once

#include "overlay_base.hh"

namespace blender::draw::overlay {

/**
 * Displays extra object axes.
 * It is toggled by Object Panel > Viewport Display > Axes.
 * Also visible if Options > Affect Only > Origins is enabled.
 */
class Axes : Overlay {
  using EmptyInstanceBuf = ShapeInstanceBuf<ExtraInstanceData>;

 private:
  const SelectionType selection_type_;

  PassSimple ps_ = {"Axes"};

  EmptyInstanceBuf axes_buf = {selection_type_, "object_axes"};
  EmptyInstanceBuf xform_origins_buf = {selection_type_, "xform_origins"};

 public:
  Axes(const SelectionType selection_type) : selection_type_{selection_type} {};

  void begin_sync(Resources & /*res*/, const State &state) final
  {
    enabled_ = state.is_space_v3d();

    ps_.init();
    axes_buf.clear();
    xform_origins_buf.clear();
  }

  void object_sync(Manager & /*manager*/,
                   const ObjectRef &ob_ref,
                   Resources &res,
                   const State &state) final
  {
    if (!enabled_) {
      return;
    }

    Object *ob = ob_ref.object;
    if (is_from_dupli_or_set(ob)) {
      return;
    }

    const bool use_display_axis = (ob->dtx & OB_AXIS) != 0;
    const bool use_xform_origins_axis = state.ctx_mode == CTX_MODE_OBJECT &&
                                        (state.scene->toolsettings->transform_flag &
                                         SCE_XFORM_DATA_ORIGIN) &&
                                        (ob->base_flag & BASE_SELECTED);
    if (!use_display_axis && !use_xform_origins_axis) {
      return;
    }

    ExtraInstanceData data(ob->object_to_world(), res.object_wire_color(ob_ref, state), 1.0f);
    if (use_xform_origins_axis) {
      data.color_ = float4(0.15f, 0.15f, 0.15f, 0.7f);
      xform_origins_buf.append(data, select::SelectMap::select_invalid_id());
    }
    else {
      axes_buf.append(data, res.select_id(ob_ref));
    }
  }

  void end_sync(Resources &res, const State &state) final
  {
    if (!enabled_) {
      return;
    }
    DRWState state_common = DRW_STATE_WRITE_COLOR | DRW_STATE_WRITE_DEPTH;
    ps_.state_set(state_common | DRW_STATE_DEPTH_LESS_EQUAL, state.clipping_plane_count);
    ps_.shader_set(res.shaders->extra_shape.get());
    ps_.bind_ubo(OVERLAY_GLOBALS_SLOT, &res.globals_buf);
    ps_.bind_ubo(DRW_CLIPPING_UBO_SLOT, &res.clip_planes_buf);
    res.select_bind(ps_);

    axes_buf.end_sync(ps_, res.shapes.arrows.get());

    PassSimple::Sub &xform_origins_ps = ps_.sub("XForm Origins");
    xform_origins_ps.state_set(state_common | DRW_STATE_DEPTH_ALWAYS, state.clipping_plane_count);

    xform_origins_buf.end_sync(xform_origins_ps, res.shapes.arrows.get());
  }

  void draw_line(Framebuffer &framebuffer, Manager &manager, View &view) final
  {
    if (!enabled_) {
      return;
    }

    GPU_framebuffer_bind(framebuffer);
    manager.submit(ps_, view);
  }
};

}  // namespace blender::draw::overlay
