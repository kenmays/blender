/* SPDX-FileCopyrightText: 2010-2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup collada
 */

#include <sstream>

#include "COLLADABUUtils.h"
#include "COLLADASWPrimitves.h"
#include "COLLADASWSource.h"
#include "COLLADASWVertices.h"

#include "GeometryExporter.h"

#include "DNA_key_types.h"
#include "DNA_meshdata_types.h"

#include "BLI_math_vector_types.hh"
#include "BLI_string.h"

#include "BKE_attribute.hh"
#include "BKE_customdata.hh"
#include "BKE_key.hh"
#include "BKE_lib_id.hh"
#include "BKE_material.hh"
#include "BKE_mesh.hh"

#include "collada_internal.h"
#include "collada_utils.h"

using blender::float3;
using blender::Span;

void GeometryExporter::exportGeom()
{
  Scene *sce = blender_context.get_scene();
  openLibrary();

  GeometryFunctor gf;
  gf.forEachMeshObjectInExportSet<GeometryExporter>(
      sce, *this, this->export_settings.get_export_set());

  closeLibrary();
}

void GeometryExporter::operator()(Object *ob)
{
  bool use_instantiation = this->export_settings.get_use_object_instantiation();
  Mesh *mesh = bc_get_mesh_copy(blender_context,
                                ob,
                                this->export_settings.get_export_mesh_type(),
                                this->export_settings.get_apply_modifiers(),
                                this->export_settings.get_triangulate());

  std::string geom_id = get_geometry_id(ob, use_instantiation);
  std::vector<Normal> nor;
  std::vector<BCPolygonNormalsIndices> norind;

  /* Skip if linked geometry was already exported from another reference */
  if (use_instantiation && exportedGeometry.find(geom_id) != exportedGeometry.end()) {
    return;
  }

  std::string geom_name = (use_instantiation) ? id_name(ob->data) : id_name(ob);
  geom_name = encode_xml(geom_name);

  exportedGeometry.insert(geom_id);

  bool has_color = CustomData_has_layer(&mesh->fdata_legacy, CD_MCOL);

  create_normals(nor, norind, mesh);

  /* openMesh(geoId, geoName, meshId) */
  openMesh(geom_id, geom_name);

  /* writes <source> for vertex coords */
  createVertsSource(geom_id, mesh);

  /* writes <source> for normal coords */
  createNormalsSource(geom_id, mesh, nor);

  bool has_uvs = CustomData_has_layer(&mesh->corner_data, CD_PROP_FLOAT2);

  /* writes <source> for uv coords if mesh has uv coords */
  if (has_uvs) {
    createTexcoordsSource(geom_id, mesh);
  }

  if (has_color) {
    createVertexColorSource(geom_id, mesh);
  }
  /* <vertices> */

  COLLADASW::Vertices verts(mSW);
  verts.setId(getIdBySemantics(geom_id, COLLADASW::InputSemantic::VERTEX));
  COLLADASW::InputList &input_list = verts.getInputList();
  COLLADASW::Input input(COLLADASW::InputSemantic::POSITION,
                         getUrlBySemantics(geom_id, COLLADASW::InputSemantic::POSITION));
  input_list.push_back(input);
  verts.add();

  createLooseEdgeList(ob, mesh, geom_id);

  /* Only create poly-lists if number of faces > 0. */
  if (mesh->totface_legacy > 0) {
    /* XXX slow */
    if (ob->totcol) {
      for (int a = 0; a < ob->totcol; a++) {
        create_mesh_primitive_list(a, has_uvs, has_color, ob, mesh, geom_id, norind);
      }
    }
    else {
      create_mesh_primitive_list(0, has_uvs, has_color, ob, mesh, geom_id, norind);
    }
  }

  closeMesh();

  closeGeometry();

  if (this->export_settings.get_include_shapekeys()) {
    Key *key = BKE_key_from_object(ob);
    if (key) {
      blender::MutableSpan<float3> positions = mesh->vert_positions_for_write();
      KeyBlock *kb = (KeyBlock *)key->block.first;
      /* skip the basis */
      kb = kb->next;
      for (; kb; kb = kb->next) {
        BKE_keyblock_convert_to_mesh(kb, positions);
        export_key_mesh(ob, mesh, kb);
      }
    }
  }

  BKE_id_free(nullptr, mesh);
}

void GeometryExporter::export_key_mesh(Object *ob, Mesh *mesh, KeyBlock *kb)
{
  std::string geom_id = get_geometry_id(ob, false) + "_morph_" + translate_id(kb->name);
  std::vector<Normal> nor;
  std::vector<BCPolygonNormalsIndices> norind;

  if (exportedGeometry.find(geom_id) != exportedGeometry.end()) {
    return;
  }

  std::string geom_name = kb->name;

  exportedGeometry.insert(geom_id);

  bool has_color = CustomData_has_layer(&mesh->fdata_legacy, CD_MCOL);

  create_normals(nor, norind, mesh);

  // openMesh(geoId, geoName, meshId)
  openMesh(geom_id, geom_name);

  /* writes <source> for vertex coords */
  createVertsSource(geom_id, mesh);

  /* writes <source> for normal coords */
  createNormalsSource(geom_id, mesh, nor);

  bool has_uvs = CustomData_has_layer(&mesh->corner_data, CD_PROP_FLOAT2);

  /* writes <source> for uv coords if mesh has uv coords */
  if (has_uvs) {
    createTexcoordsSource(geom_id, mesh);
  }

  if (has_color) {
    createVertexColorSource(geom_id, mesh);
  }

  /* <vertices> */

  COLLADASW::Vertices verts(mSW);
  verts.setId(getIdBySemantics(geom_id, COLLADASW::InputSemantic::VERTEX));
  COLLADASW::InputList &input_list = verts.getInputList();
  COLLADASW::Input input(COLLADASW::InputSemantic::POSITION,
                         getUrlBySemantics(geom_id, COLLADASW::InputSemantic::POSITION));
  input_list.push_back(input);
  verts.add();

  // createLooseEdgeList(ob, mesh, geom_id, norind);

  /* XXX slow */
  if (ob->totcol) {
    for (int a = 0; a < ob->totcol; a++) {
      create_mesh_primitive_list(a, has_uvs, has_color, ob, mesh, geom_id, norind);
    }
  }
  else {
    create_mesh_primitive_list(0, has_uvs, has_color, ob, mesh, geom_id, norind);
  }

  closeMesh();

  closeGeometry();
}

void GeometryExporter::createLooseEdgeList(Object *ob, Mesh *mesh, std::string &geom_id)
{
  using namespace blender;
  const Span<int2> edges = mesh->edges();
  int edges_in_linelist = 0;
  std::vector<uint> edge_list;
  int index;

  /* Find all loose edges in Mesh
   * and save vertex indices in edge_list */
  const bke::LooseEdgeCache &loose_edges = mesh->loose_edges();
  if (loose_edges.count > 0) {
    for (const int64_t i : edges.index_range()) {
      if (loose_edges.is_loose_bits[i]) {
        const int2 &edge = edges[i];
        edges_in_linelist += 1;
        edge_list.push_back(edge[0]);
        edge_list.push_back(edge[1]);
      }
    }
  }

  if (edges_in_linelist > 0) {
    /* Create the list of loose edges */
    COLLADASW::Lines lines(mSW);

    lines.setCount(edges_in_linelist);

    COLLADASW::InputList &til = lines.getInputList();

    /* creates <input> in <lines> for vertices */
    COLLADASW::Input input1(COLLADASW::InputSemantic::VERTEX,
                            getUrlBySemantics(geom_id, COLLADASW::InputSemantic::VERTEX),
                            0);
    til.push_back(input1);

    lines.prepareToAppendValues();

    for (index = 0; index < edges_in_linelist; index++) {
      lines.appendValues(edge_list[2 * index + 1]);
      lines.appendValues(edge_list[2 * index]);
    }
    lines.finish();
  }
}

static void prepareToAppendValues(bool is_triangulated,
                                  COLLADASW::PrimitivesBase &primitive_list,
                                  std::vector<ulong> &vcount_list)
{
  /* performs the actual writing */
  if (is_triangulated) {
    ((COLLADASW::Triangles &)primitive_list).prepareToAppendValues();
  }
  else {
    /* sets <vcount> */
    primitive_list.setVCountList(vcount_list);
    ((COLLADASW::Polylist &)primitive_list).prepareToAppendValues();
  }
}

static void finish_and_delete_primitive_List(bool is_triangulated,
                                             COLLADASW::PrimitivesBase *primitive_list)
{
  if (is_triangulated) {
    ((COLLADASW::Triangles *)primitive_list)->finish();
  }
  else {
    ((COLLADASW::Polylist *)primitive_list)->finish();
  }
  delete primitive_list;
}

static COLLADASW::PrimitivesBase *create_primitive_list(bool is_triangulated,
                                                        COLLADASW::StreamWriter *mSW)
{
  COLLADASW::PrimitivesBase *primitive_list;

  if (is_triangulated) {
    primitive_list = new COLLADASW::Triangles(mSW);
  }
  else {
    primitive_list = new COLLADASW::Polylist(mSW);
  }
  return primitive_list;
}

static bool collect_vertex_counts_per_poly(Mesh *mesh,
                                           int material_index,
                                           std::vector<ulong> &vcount_list)
{
  using namespace blender;
  const blender::OffsetIndices faces = mesh->faces();
  const blender::bke::AttributeAccessor attributes = mesh->attributes();
  const blender::VArray<int> material_indices = *attributes.lookup_or_default<int>(
      "material_index", bke::AttrDomain::Face, 0);
  bool is_triangulated = true;

  /* Expecting that the material index is always 0 if the mesh has no materials assigned */
  for (const int i : faces.index_range()) {
    if (material_indices[i] == material_index) {
      const int vertex_count = faces[i].size();
      vcount_list.push_back(vertex_count);
      if (vertex_count != 3) {
        is_triangulated = false;
      }
    }
  }
  return is_triangulated;
}

std::string GeometryExporter::makeVertexColorSourceId(std::string &geom_id, const char *layer_name)
{
  std::string result = getIdBySemantics(geom_id, COLLADASW::InputSemantic::COLOR) + "-" +
                       layer_name;
  return result;
}

void GeometryExporter::create_mesh_primitive_list(short material_index,
                                                  bool has_uvs,
                                                  bool has_color,
                                                  Object *ob,
                                                  Mesh *mesh,
                                                  std::string &geom_id,
                                                  std::vector<BCPolygonNormalsIndices> &norind)
{
  using namespace blender;
  const blender::OffsetIndices faces = mesh->faces();
  const Span<int> corner_verts = mesh->corner_verts();

  std::vector<ulong> vcount_list;

  bool is_triangulated = collect_vertex_counts_per_poly(mesh, material_index, vcount_list);
  int polygon_count = vcount_list.size();

  /* no faces using this material */
  if (polygon_count == 0) {
    fprintf(
        stderr, "%s: material with index %d is not used.\n", id_name(ob).c_str(), material_index);
    return;
  }

  Material *ma = ob->totcol ? BKE_object_material_get(ob, material_index + 1) : nullptr;
  COLLADASW::PrimitivesBase *primitive_list = create_primitive_list(is_triangulated, mSW);

  /* sets count attribute in `<polylist>`. */
  primitive_list->setCount(polygon_count);

  /* sets material name */
  if (ma) {
    std::string material_id = get_material_id(ma);
    std::ostringstream ostr;
    ostr << translate_id(material_id);
    primitive_list->setMaterial(ostr.str());
  }

  COLLADASW::Input vertex_input(COLLADASW::InputSemantic::VERTEX,
                                getUrlBySemantics(geom_id, COLLADASW::InputSemantic::VERTEX),
                                0);
  COLLADASW::Input normals_input(COLLADASW::InputSemantic::NORMAL,
                                 getUrlBySemantics(geom_id, COLLADASW::InputSemantic::NORMAL),
                                 1);

  COLLADASW::InputList &til = primitive_list->getInputList();
  til.push_back(vertex_input);
  til.push_back(normals_input);

  /* if mesh has uv coords writes <input> for TEXCOORD */
  int num_layers = CustomData_number_of_layers(&mesh->corner_data, CD_PROP_FLOAT2);
  int active_uv = CustomData_get_active_layer(&mesh->corner_data, CD_PROP_FLOAT2);
  for (int i = 0; i < num_layers; i++) {
    if (!this->export_settings.get_active_uv_only() || i == active_uv) {

      // char *name = CustomData_get_layer_name(&mesh->ldata, CD_PROP_FLOAT2, i);
      COLLADASW::Input texcoord_input(
          COLLADASW::InputSemantic::TEXCOORD,
          makeUrl(makeTexcoordSourceId(geom_id, i, this->export_settings.get_active_uv_only())),
          2, /* this is only until we have optimized UV sets */
          this->export_settings.get_active_uv_only() ? 0 : i /* set (0,1,2,...) */
      );
      til.push_back(texcoord_input);
    }
  }

  int totlayer_mcol = CustomData_number_of_layers(&mesh->corner_data, CD_PROP_BYTE_COLOR);
  if (totlayer_mcol > 0) {
    int map_index = 0;

    for (int a = 0; a < totlayer_mcol; a++) {
      const char *layer_name = bc_CustomData_get_layer_name(
          &mesh->corner_data, CD_PROP_BYTE_COLOR, a);
      COLLADASW::Input input4(COLLADASW::InputSemantic::COLOR,
                              makeUrl(makeVertexColorSourceId(geom_id, layer_name)),
                              (has_uvs) ? 3 : 2, /* all color layers have same index order */
                              map_index          /* set number equals color map index */
      );
      til.push_back(input4);
      map_index++;
    }
  }

  /* performs the actual writing */
  prepareToAppendValues(is_triangulated, *primitive_list, vcount_list);

  const blender::bke::AttributeAccessor attributes = mesh->attributes();
  const blender::VArray<int> material_indices = *attributes.lookup_or_default<int>(
      "material_index", bke::AttrDomain::Face, 0);

  /* <p> */
  int texindex = 0;
  for (const int i : faces.index_range()) {
    const blender::IndexRange poly = faces[i];
    int loop_count = poly.size();

    if (material_indices[i] == material_index) {
      BCPolygonNormalsIndices normal_indices = norind[i];

      for (int j = 0; j < loop_count; j++) {
        const int vert = corner_verts[poly[j]];
        primitive_list->appendValues(vert);
        primitive_list->appendValues(normal_indices[j]);
        if (has_uvs) {
          primitive_list->appendValues(texindex + j);
        }

        if (has_color) {
          primitive_list->appendValues(texindex + j);
        }
      }
    }

    texindex += loop_count;
  }

  finish_and_delete_primitive_List(is_triangulated, primitive_list);
}

void GeometryExporter::createVertsSource(std::string geom_id, Mesh *mesh)
{
  const Span<float3> positions = mesh->vert_positions();

  COLLADASW::FloatSourceF source(mSW);
  source.setId(getIdBySemantics(geom_id, COLLADASW::InputSemantic::POSITION));
  source.setArrayId(getIdBySemantics(geom_id, COLLADASW::InputSemantic::POSITION) +
                    ARRAY_ID_SUFFIX);
  source.setAccessorCount(positions.size());
  source.setAccessorStride(3);

  COLLADASW::SourceBase::ParameterNameList &param = source.getParameterNameList();
  param.emplace_back("X");
  param.emplace_back("Y");
  param.emplace_back("Z");
  /* main function, it creates <source id = "">, <float_array id = ""
   * count = ""> */
  source.prepareToAppendValues();
  /* appends data to <float_array> */
  for (const int i : positions.index_range()) {
    Vector co;
    if (export_settings.get_apply_global_orientation()) {
      float co_c[3];
      copy_v3_v3(co_c, positions[i]);
      bc_add_global_transform(co, co_c, export_settings.get_global_transform());
    }
    else {
      copy_v3_v3(co, positions[i]);
    }
    source.appendValues(co[0], co[1], co[2]);
  }

  source.finish();
}

void GeometryExporter::createVertexColorSource(std::string geom_id, Mesh *mesh)
{
  /* Find number of vertex color layers */
  int totlayer_mcol = CustomData_number_of_layers(&mesh->corner_data, CD_PROP_BYTE_COLOR);
  if (totlayer_mcol == 0) {
    return;
  }

  int map_index = 0;
  for (int a = 0; a < totlayer_mcol; a++) {

    map_index++;
    const MLoopCol *mloopcol = (const MLoopCol *)CustomData_get_layer_n(
        &mesh->corner_data, CD_PROP_BYTE_COLOR, a);

    COLLADASW::FloatSourceF source(mSW);

    const char *layer_name = bc_CustomData_get_layer_name(
        &mesh->corner_data, CD_PROP_BYTE_COLOR, a);
    std::string layer_id = makeVertexColorSourceId(geom_id, layer_name);
    source.setId(layer_id);

    source.setNodeName(layer_name);

    source.setArrayId(layer_id + ARRAY_ID_SUFFIX);
    source.setAccessorCount(mesh->corners_num);
    source.setAccessorStride(4);

    COLLADASW::SourceBase::ParameterNameList &param = source.getParameterNameList();
    param.emplace_back("R");
    param.emplace_back("G");
    param.emplace_back("B");
    param.emplace_back("A");

    source.prepareToAppendValues();

    const blender::OffsetIndices faces = mesh->faces();
    for (const int i : faces.index_range()) {
      for (const int corner : faces[i]) {
        const MLoopCol *mlc = &mloopcol[corner];
        source.appendValues(mlc->r / 255.0f, mlc->g / 255.0f, mlc->b / 255.0f, mlc->a / 255.0f);
      }
    }

    source.finish();
  }
}

std::string GeometryExporter::makeTexcoordSourceId(std::string &geom_id,
                                                   int layer_index,
                                                   bool is_single_layer)
{
  char suffix[20];
  if (is_single_layer) {
    suffix[0] = '\0';
  }
  else {
    SNPRINTF(suffix, "-%d", layer_index);
  }
  return getIdBySemantics(geom_id, COLLADASW::InputSemantic::TEXCOORD) + suffix;
}

void GeometryExporter::createTexcoordsSource(std::string geom_id, Mesh *mesh)
{
  int totuv = mesh->corners_num;
  const blender::OffsetIndices faces = mesh->faces();

  int num_layers = CustomData_number_of_layers(&mesh->corner_data, CD_PROP_FLOAT2);

  /* write <source> for each layer
   * each <source> will get id like meshName + "map-channel-1" */
  int active_uv_index = CustomData_get_active_layer_index(&mesh->corner_data, CD_PROP_FLOAT2);
  for (int a = 0; a < num_layers; a++) {
    int layer_index = CustomData_get_layer_index_n(&mesh->corner_data, CD_PROP_FLOAT2, a);
    if (!this->export_settings.get_active_uv_only() || layer_index == active_uv_index) {
      const blender::float2 *uv_map = static_cast<const blender::float2 *>(
          CustomData_get_layer_n(&mesh->corner_data, CD_PROP_FLOAT2, a));

      COLLADASW::FloatSourceF source(mSW);
      std::string layer_id = makeTexcoordSourceId(
          geom_id, a, this->export_settings.get_active_uv_only());
      source.setId(layer_id);
      source.setArrayId(layer_id + ARRAY_ID_SUFFIX);

      source.setAccessorCount(totuv);
      source.setAccessorStride(2);
      COLLADASW::SourceBase::ParameterNameList &param = source.getParameterNameList();
      param.emplace_back("S");
      param.emplace_back("T");

      source.prepareToAppendValues();

      for (const int i : faces.index_range()) {
        for (const int corner : faces[i]) {
          source.appendValues(uv_map[corner][0], uv_map[corner][1]);
        }
      }

      source.finish();
    }
  }
}

bool operator<(const Normal &a, const Normal &b)
{
  /* Only needed to sort normal vectors and find() them later in a map. */
  return a.x < b.x || (a.x == b.x && (a.y < b.y || (a.y == b.y && a.z < b.z)));
}

void GeometryExporter::createNormalsSource(std::string geom_id,
                                           Mesh *mesh,
                                           std::vector<Normal> &nor)
{
  COLLADASW::FloatSourceF source(mSW);
  source.setId(getIdBySemantics(geom_id, COLLADASW::InputSemantic::NORMAL));
  source.setArrayId(getIdBySemantics(geom_id, COLLADASW::InputSemantic::NORMAL) + ARRAY_ID_SUFFIX);
  source.setAccessorCount(ulong(nor.size()));
  source.setAccessorStride(3);
  COLLADASW::SourceBase::ParameterNameList &param = source.getParameterNameList();
  param.emplace_back("X");
  param.emplace_back("Y");
  param.emplace_back("Z");

  source.prepareToAppendValues();

  std::vector<Normal>::iterator it;
  for (it = nor.begin(); it != nor.end(); it++) {
    Normal &n = *it;

    Vector no{n.x, n.y, n.z};
    if (export_settings.get_apply_global_orientation()) {
      bc_add_global_transform(no, export_settings.get_global_transform());
    }
    source.appendValues(no[0], no[1], no[2]);
  }

  source.finish();
}

void GeometryExporter::create_normals(std::vector<Normal> &normals,
                                      std::vector<BCPolygonNormalsIndices> &polygons_normals,
                                      Mesh *mesh)
{
  using namespace blender;
  std::map<Normal, uint> shared_normal_indices;
  int last_normal_index = -1;

  const Span<float3> positions = mesh->vert_positions();
  const Span<float3> vert_normals = mesh->vert_normals();
  const blender::OffsetIndices faces = mesh->faces();
  const Span<int> corner_verts = mesh->corner_verts();

  const bke::AttributeAccessor attributes = mesh->attributes();
  const VArray<bool> sharp_faces = *attributes.lookup_or_default<bool>(
      "sharp_face", bke::AttrDomain::Face, false);

  blender::Span<blender::float3> corner_normals;
  if (mesh->normals_domain() == blender::bke::MeshNormalDomain::Corner) {
    corner_normals = mesh->corner_normals();
  }

  for (const int face_index : faces.index_range()) {
    const IndexRange face = faces[face_index];
    bool use_vert_normals = !corner_normals.is_empty() || !sharp_faces[face_index];

    if (!use_vert_normals) {
      /* For flat faces use face normal as vertex normal: */

      const float3 vector = blender::bke::mesh::face_normal_calc(positions,
                                                                 corner_verts.slice(face));

      Normal n = {vector[0], vector[1], vector[2]};
      normals.push_back(n);
      last_normal_index++;
    }

    BCPolygonNormalsIndices poly_indices;
    for (const int corner : face) {
      if (use_vert_normals) {
        float normalized[3];

        if (!corner_normals.is_empty()) {
          normalize_v3_v3(normalized, corner_normals[corner]);
        }
        else {
          copy_v3_v3(normalized, vert_normals[corner_verts[corner]]);
          normalize_v3(normalized);
        }
        Normal n = {normalized[0], normalized[1], normalized[2]};

        if (shared_normal_indices.find(n) != shared_normal_indices.end()) {
          poly_indices.add_index(shared_normal_indices[n]);
        }
        else {
          last_normal_index++;
          poly_indices.add_index(last_normal_index);
          shared_normal_indices[n] = last_normal_index;
          normals.push_back(n);
        }
      }
      else {
        poly_indices.add_index(last_normal_index);
      }
    }

    polygons_normals.push_back(poly_indices);
  }
}

std::string GeometryExporter::getIdBySemantics(std::string geom_id,
                                               COLLADASW::InputSemantic::Semantics type,
                                               std::string other_suffix)
{
  return geom_id + getSuffixBySemantic(type) + other_suffix;
}

COLLADASW::URI GeometryExporter::getUrlBySemantics(std::string geom_id,
                                                   COLLADASW::InputSemantic::Semantics type,
                                                   std::string other_suffix)
{

  std::string id(getIdBySemantics(geom_id, type, other_suffix));
  return COLLADASW::URI(COLLADABU::Utils::EMPTY_STRING, id);
}

COLLADASW::URI GeometryExporter::makeUrl(std::string id)
{
  return COLLADASW::URI(COLLADABU::Utils::EMPTY_STRING, id);
}
