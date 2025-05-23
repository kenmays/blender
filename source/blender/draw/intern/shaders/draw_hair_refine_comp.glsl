/* SPDX-FileCopyrightText: 2021-2022 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "draw_hair_refine_info.hh"

#include "draw_curves_lib.glsl"

COMPUTE_SHADER_CREATE_INFO(draw_hair_refine_compute)

void main()
{
  float interp_time = 0.0f;
  float4 data0, data1, data2, data3;
  hair_get_interp_attrs(data0, data1, data2, data3, interp_time);

  float4 weights = hair_get_weights_cardinal(interp_time);
  float4 result = hair_interp_data(data0, data1, data2, data3, weights);

  uint index = uint(hair_get_id() * hairStrandsRes) + gl_GlobalInvocationID.y;
  posTime[index] = result;
}
