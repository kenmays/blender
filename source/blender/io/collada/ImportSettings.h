/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup collada
 */

#pragma once

struct ImportSettings {
  bool import_units;
  bool custom_normals;
  bool find_chains;
  bool auto_connect;
  bool fix_orientation;
  int min_chain_length;
  char *filepath;
  bool keep_bind_info;
};
