# SPDX-FileCopyrightText: 2011 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

set(INC
  ../include
  ../../makesrna

  # RNA_prototypes.hh
  ${CMAKE_BINARY_DIR}/source/blender/makesrna
)

set(INC_SYS
)

set(SRC
  clip_buttons.cc
  clip_dopesheet_draw.cc
  clip_dopesheet_ops.cc
  clip_draw.cc
  clip_editor.cc
  clip_graph_draw.cc
  clip_graph_ops.cc
  clip_ops.cc
  clip_utils.cc
  space_clip.cc
  tracking_ops.cc
  tracking_ops_detect.cc
  tracking_ops_orient.cc
  tracking_ops_plane.cc
  tracking_ops_solve.cc
  tracking_ops_stabilize.cc
  tracking_ops_track.cc
  tracking_ops_utils.cc
  tracking_select.cc

  clip_intern.hh
  tracking_ops_intern.hh
)

set(LIB
  PRIVATE bf::blenfont
  PRIVATE bf::blenkernel
  PRIVATE bf::blenlib
  PRIVATE bf::blenloader
  PRIVATE bf::blentranslation
  PRIVATE bf::depsgraph
  PRIVATE bf::dna
  PRIVATE bf::gpu
  PRIVATE bf::imbuf
  PRIVATE bf::imbuf::movie
  PRIVATE bf::intern::guardedalloc
  PRIVATE bf::windowmanager
)

blender_add_lib(bf_editor_space_clip "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

# RNA_prototypes.hh
add_dependencies(bf_editor_space_clip bf_rna)
