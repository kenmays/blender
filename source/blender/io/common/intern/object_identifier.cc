/* SPDX-FileCopyrightText: 2019 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "IO_abstract_hierarchy_iterator.h"

#include "BLI_assert.h"

#include "BKE_duplilist.hh"

namespace blender::io {

ObjectIdentifier::ObjectIdentifier(Object *object,
                                   Object *duplicated_by,
                                   const PersistentID &persistent_id)
    : object(object), duplicated_by(duplicated_by), persistent_id(persistent_id)
{
  /* Class invariants:
   *   If duplicated_by is null, persistent_id must be default.
   *   If duplicated_by is not null, persistent_id must not be default. */
  BLI_assert(duplicated_by == nullptr ? persistent_id == PersistentID() :
                                        !(persistent_id == PersistentID()));
}

ObjectIdentifier ObjectIdentifier::for_real_object(Object *object)
{
  return ObjectIdentifier(object, nullptr, PersistentID());
}

ObjectIdentifier ObjectIdentifier::for_hierarchy_context(const HierarchyContext *context)
{
  if (context == nullptr) {
    return for_graph_root();
  }
  if (context->duplicator != nullptr) {
    return ObjectIdentifier(context->object, context->duplicator, context->persistent_id);
  }
  return for_real_object(context->object);
}

ObjectIdentifier ObjectIdentifier::for_duplicated_object(const DupliObject *dupli_object,
                                                         Object *duplicated_by)
{
  return ObjectIdentifier(dupli_object->ob, duplicated_by, PersistentID(dupli_object));
}

ObjectIdentifier ObjectIdentifier::for_graph_root()
{
  return ObjectIdentifier(nullptr, nullptr, PersistentID());
}

bool ObjectIdentifier::is_root() const
{
  return object == nullptr;
}

bool operator==(const ObjectIdentifier &obj_ident_a, const ObjectIdentifier &obj_ident_b)
{
  if (obj_ident_a.object != obj_ident_b.object) {
    return false;
  }
  if (obj_ident_a.duplicated_by != obj_ident_b.duplicated_by) {
    return false;
  }

  /* Return early if we know the expensive persistent_id check won't be necessary. */
  if (obj_ident_a.duplicated_by == nullptr) {
    return true;
  }

  /* Same object, both are duplicated, use the persistent IDs to determine equality. */
  return obj_ident_a.persistent_id == obj_ident_b.persistent_id;
}

}  // namespace blender::io
