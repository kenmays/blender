/* SPDX-FileCopyrightText: 2005 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup gpu
 *
 * Generate shader code from the intermediate node graph.
 */

#pragma once

#include "GPU_material.hh"
#include "GPU_shader.hh"

struct GPUNodeGraph;

struct GPUPass;

/* Pass */

GPUPass *GPU_generate_pass(GPUMaterial *material,
                           GPUNodeGraph *graph,
                           eGPUMaterialEngine engine,
                           GPUCodegenCallbackFn finalize_source_cb,
                           void *thunk,
                           bool optimize_graph);
GPUShader *GPU_pass_shader_get(GPUPass *pass);
bool GPU_pass_compile(GPUPass *pass, const char *shname);
void GPU_pass_acquire(GPUPass *pass);
void GPU_pass_release(GPUPass *pass);
bool GPU_pass_should_optimize(GPUPass *pass);

/* Custom pass compilation. */

GPUShaderCreateInfo *GPU_pass_begin_compilation(GPUPass *pass, const char *shname);
bool GPU_pass_finalize_compilation(GPUPass *pass, GPUShader *shader);

void GPU_pass_begin_async_compilation(GPUPass *pass, const char *shname);
/** NOTE: Unlike the non-async version, this one returns true when compilation has finalized,
 * regardless if it succeeded or not.
 * To check for success, see if `GPU_pass_shader_get() != nullptr`. */
bool GPU_pass_async_compilation_try_finalize(GPUPass *pass);

/* Module */

void gpu_codegen_init();
void gpu_codegen_exit();
