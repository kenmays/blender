/* SPDX-FileCopyrightText: 2005 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "node_shader_util.hh"

namespace blender::nodes::node_shader_volume_absorption_cc {

static void node_declare(NodeDeclarationBuilder &b)
{
  b.add_input<decl::Color>("Color").default_value({0.8f, 0.8f, 0.8f, 1.0f});
#define SOCK_COLOR_ID 0
  b.add_input<decl::Float>("Density").default_value(1.0f).min(0.0f).max(1000.0f);
#define SOCK_DENSITY_ID 1
  b.add_input<decl::Float>("Weight").available(false);
  b.add_output<decl::Shader>("Volume").translation_context(BLT_I18NCONTEXT_ID_ID);
}

static int node_shader_gpu_volume_absorption(GPUMaterial *mat,
                                             bNode *node,
                                             bNodeExecData * /*execdata*/,
                                             GPUNodeStack *in,
                                             GPUNodeStack *out)
{
  if (node_socket_not_zero(in[SOCK_DENSITY_ID]) && node_socket_not_white(in[SOCK_COLOR_ID])) {
    GPU_material_flag_set(mat, GPU_MATFLAG_VOLUME_ABSORPTION);
  }
  return GPU_stack_link(mat, node, "node_volume_absorption", in, out);
}

#undef SOCK_COLOR_ID
#undef SOCK_DENSITY_ID

}  // namespace blender::nodes::node_shader_volume_absorption_cc

/* node type definition */
void register_node_type_sh_volume_absorption()
{
  namespace file_ns = blender::nodes::node_shader_volume_absorption_cc;

  static blender::bke::bNodeType ntype;

  sh_node_type_base(&ntype, "ShaderNodeVolumeAbsorption", SH_NODE_VOLUME_ABSORPTION);
  ntype.ui_name = "Volume Absorption";
  ntype.ui_description = "Absorb light as it passes through the volume";
  ntype.enum_name_legacy = "VOLUME_ABSORPTION";
  ntype.nclass = NODE_CLASS_SHADER;
  ntype.declare = file_ns::node_declare;
  ntype.gpu_fn = file_ns::node_shader_gpu_volume_absorption;

  blender::bke::node_register_type(ntype);
}
