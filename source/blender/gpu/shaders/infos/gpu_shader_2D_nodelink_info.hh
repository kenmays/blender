/* SPDX-FileCopyrightText: 2022 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup gpu
 */

#include "gpu_shader_create_info.hh"

GPU_SHADER_INTERFACE_INFO(nodelink_iface, "")
    .smooth(Type::VEC4, "finalColor")
    .smooth(Type::VEC2, "lineUV")
    .flat(Type::FLOAT, "lineLength")
    .flat(Type::FLOAT, "lineThickness")
    .flat(Type::FLOAT, "dashLength")
    .flat(Type::FLOAT, "dashFactor")
    .flat(Type::INT, "hasBackLink")
    .flat(Type::FLOAT, "dashAlpha")
    .flat(Type::INT, "isMainLine")
    .flat(Type::FLOAT, "aspect");

GPU_SHADER_CREATE_INFO(gpu_shader_2D_nodelink)
    .vertex_in(0, Type::VEC2, "uv")
    .vertex_in(1, Type::VEC2, "pos")
    .vertex_in(2, Type::VEC2, "expand")
    .vertex_out(nodelink_iface)
    .fragment_out(0, Type::VEC4, "fragColor")
    .uniform_buf(0, "NodeLinkData", "node_link_data", Frequency::PASS)
    .push_constant(Type::MAT4, "ModelViewProjectionMatrix")
    .vertex_source("gpu_shader_2D_nodelink_vert.glsl")
    .fragment_source("gpu_shader_2D_nodelink_frag.glsl")
    .typedef_source("GPU_shader_shared.hh")
    .do_static_compilation(true);

GPU_SHADER_CREATE_INFO(gpu_shader_2D_nodelink_inst)
    .vertex_in(0, Type::VEC2, "uv")
    .vertex_in(1, Type::VEC2, "pos")
    .vertex_in(2, Type::VEC2, "expand")
    .vertex_in(3, Type::VEC2, "P0")
    .vertex_in(4, Type::VEC2, "P1")
    .vertex_in(5, Type::VEC2, "P2")
    .vertex_in(6, Type::VEC2, "P3")
    .vertex_in(7, Type::UVEC4, "colid_doarrow")
    .vertex_in(8, Type::VEC4, "start_color")
    .vertex_in(9, Type::VEC4, "end_color")
    .vertex_in(10, Type::UVEC2, "domuted")
    .vertex_in(11, Type::FLOAT, "dim_factor")
    .vertex_in(12, Type::FLOAT, "thickness")
    .vertex_in(13, Type::VEC3, "dash_params")
    .vertex_in(14, Type::INT, "has_back_link")
    .vertex_out(nodelink_iface)
    .fragment_out(0, Type::VEC4, "fragColor")
    .uniform_buf(0, "NodeLinkInstanceData", "node_link_data", Frequency::PASS)
    .push_constant(Type::MAT4, "ModelViewProjectionMatrix")
    .vertex_source("gpu_shader_2D_nodelink_vert.glsl")
    .fragment_source("gpu_shader_2D_nodelink_frag.glsl")
    .typedef_source("GPU_shader_shared.hh")
    .define("USE_INSTANCE")
    .do_static_compilation(true);
