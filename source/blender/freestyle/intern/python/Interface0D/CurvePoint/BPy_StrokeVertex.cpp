/* SPDX-FileCopyrightText: 2004-2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup freestyle
 */

#include "BPy_StrokeVertex.h"

#include "../../BPy_Convert.h"
#include "../../BPy_Freestyle.h"
#include "../../BPy_StrokeAttribute.h"
#include "../../Interface0D/BPy_SVertex.h"

using namespace Freestyle;

///////////////////////////////////////////////////////////////////////////////////////////

//------------------------INSTANCE METHODS ----------------------------------

PyDoc_STRVAR(
    /* Wrap. */
    StrokeVertex_doc,
    "Class hierarchy: :class:`Interface0D` > :class:`CurvePoint` > :class:`StrokeVertex`\n"
    "\n"
    "Class to define a stroke vertex.\n"
    "\n"
    ".. method:: __init__()\n"
    "            __init__(brother)\n"
    "            __init__(first_vertex, second_vertex, t3d)\n"
    "            __init__(point)\n"
    "            __init__(svertex)\n"
    "            __init__(svertex, attribute)\n"
    "\n"
    "   Builds a :class:`StrokeVertex` using the default constructor,\n"
    "   copy constructor, from 2 :class:`StrokeVertex` and an interpolation parameter,\n"
    "   from a CurvePoint, from a SVertex, or a :class:`SVertex`"
    "   and a :class:`StrokeAttribute` object.\n"
    "\n"
    "   :arg brother: A StrokeVertex object.\n"
    "   :type brother: :class:`StrokeVertex`\n"
    "   :arg first_vertex: The first StrokeVertex.\n"
    "   :type first_vertex: :class:`StrokeVertex`\n"
    "   :arg second_vertex: The second StrokeVertex.\n"
    "   :type second_vertex: :class:`StrokeVertex`\n"
    "   :arg t3d: An interpolation parameter.\n"
    "   :type t3d: float\n"
    "   :arg point: A CurvePoint object.\n"
    "   :type point: :class:`CurvePoint`\n"
    "   :arg svertex: An SVertex object.\n"
    "   :type svertex: :class:`SVertex`\n"
    "   :arg svertex: An SVertex object.\n"
    "   :type svertex: :class:`SVertex`\n"
    "   :arg attribute: A StrokeAttribute object.\n"
    "   :type attribute: :class:`StrokeAttribute`");

static int StrokeVertex_init(BPy_StrokeVertex *self, PyObject *args, PyObject *kwds)
{
  static const char *kwlist_1[] = {"brother", nullptr};
  static const char *kwlist_2[] = {"first_vertex", "second_vertex", "t3d", nullptr};
  static const char *kwlist_3[] = {"point", nullptr};
  static const char *kwlist_4[] = {"svertex", "attribute", nullptr};
  PyObject *obj1 = nullptr, *obj2 = nullptr;
  float t3d;

  if (PyArg_ParseTupleAndKeywords(args, kwds, "|O!", (char **)kwlist_1, &StrokeVertex_Type, &obj1))
  {
    if (!obj1) {
      self->sv = new StrokeVertex();
    }
    else {
      if (!((BPy_StrokeVertex *)obj1)->sv) {
        PyErr_SetString(PyExc_TypeError, "argument 1 is an invalid StrokeVertex object");
        return -1;
      }
      self->sv = new StrokeVertex(*(((BPy_StrokeVertex *)obj1)->sv));
    }
  }
  else if ((void)PyErr_Clear(),
           PyArg_ParseTupleAndKeywords(args,
                                       kwds,
                                       "O!O!f",
                                       (char **)kwlist_2,
                                       &StrokeVertex_Type,
                                       &obj1,
                                       &StrokeVertex_Type,
                                       &obj2,
                                       &t3d))
  {
    StrokeVertex *sv1 = ((BPy_StrokeVertex *)obj1)->sv;
    StrokeVertex *sv2 = ((BPy_StrokeVertex *)obj2)->sv;
    if (!sv1 || (sv1->A() == nullptr && sv1->B() == nullptr)) {
      PyErr_SetString(PyExc_TypeError, "argument 1 is an invalid StrokeVertex object");
      return -1;
    }
    if (!sv2 || (sv2->A() == nullptr && sv2->B() == nullptr)) {
      PyErr_SetString(PyExc_TypeError, "argument 2 is an invalid StrokeVertex object");
      return -1;
    }
    self->sv = new StrokeVertex(sv1, sv2, t3d);
  }
  else if ((void)PyErr_Clear(),
           PyArg_ParseTupleAndKeywords(
               args, kwds, "O!", (char **)kwlist_3, &CurvePoint_Type, &obj1))
  {
    CurvePoint *cp = ((BPy_CurvePoint *)obj1)->cp;
    if (!cp || cp->A() == nullptr || cp->B() == nullptr) {
      PyErr_SetString(PyExc_TypeError, "argument 1 is an invalid CurvePoint object");
      return -1;
    }
    self->sv = new StrokeVertex(cp);
  }
  else if ((void)PyErr_Clear(),
           (void)(obj2 = nullptr),
           PyArg_ParseTupleAndKeywords(args,
                                       kwds,
                                       "O!|O!",
                                       (char **)kwlist_4,
                                       &SVertex_Type,
                                       &obj1,
                                       &StrokeAttribute_Type,
                                       &obj2))
  {
    if (!obj2) {
      self->sv = new StrokeVertex(((BPy_SVertex *)obj1)->sv);
    }
    else {
      self->sv = new StrokeVertex(((BPy_SVertex *)obj1)->sv, *(((BPy_StrokeAttribute *)obj2)->sa));
    }
  }
  else {
    PyErr_SetString(PyExc_TypeError, "invalid argument(s)");
    return -1;
  }
  self->py_cp.cp = self->sv;
  self->py_cp.py_if0D.if0D = self->sv;
  self->py_cp.py_if0D.borrowed = false;
  return 0;
}

// real     operator[] (const int i) const
// real &   operator[] (const int i)

/*----------------------mathutils callbacks ----------------------------*/

static int StrokeVertex_mathutils_check(BaseMathObject *bmo)
{
  if (!BPy_StrokeVertex_Check(bmo->cb_user)) {
    return -1;
  }
  return 0;
}

static int StrokeVertex_mathutils_get(BaseMathObject *bmo, int /*subtype*/)
{
  BPy_StrokeVertex *self = (BPy_StrokeVertex *)bmo->cb_user;
  bmo->data[0] = float(self->sv->x());
  bmo->data[1] = float(self->sv->y());
  return 0;
}

static int StrokeVertex_mathutils_set(BaseMathObject *bmo, int /*subtype*/)
{
  BPy_StrokeVertex *self = (BPy_StrokeVertex *)bmo->cb_user;
  self->sv->setX((real)bmo->data[0]);
  self->sv->setY((real)bmo->data[1]);
  return 0;
}

static int StrokeVertex_mathutils_get_index(BaseMathObject *bmo, int /*subtype*/, int index)
{
  BPy_StrokeVertex *self = (BPy_StrokeVertex *)bmo->cb_user;
  switch (index) {
    case 0:
      bmo->data[0] = float(self->sv->x());
      break;
    case 1:
      bmo->data[1] = float(self->sv->y());
      break;
    default:
      return -1;
  }
  return 0;
}

static int StrokeVertex_mathutils_set_index(BaseMathObject *bmo, int /*subtype*/, int index)
{
  BPy_StrokeVertex *self = (BPy_StrokeVertex *)bmo->cb_user;
  switch (index) {
    case 0:
      self->sv->setX((real)bmo->data[0]);
      break;
    case 1:
      self->sv->setY((real)bmo->data[1]);
      break;
    default:
      return -1;
  }
  return 0;
}

static Mathutils_Callback StrokeVertex_mathutils_cb = {
    StrokeVertex_mathutils_check,
    StrokeVertex_mathutils_get,
    StrokeVertex_mathutils_set,
    StrokeVertex_mathutils_get_index,
    StrokeVertex_mathutils_set_index,
};

static uchar StrokeVertex_mathutils_cb_index = -1;

void StrokeVertex_mathutils_register_callback()
{
  StrokeVertex_mathutils_cb_index = Mathutils_RegisterCallback(&StrokeVertex_mathutils_cb);
}

/*----------------------StrokeVertex get/setters ----------------------------*/

PyDoc_STRVAR(
    /* Wrap. */
    StrokeVertex_attribute_doc,
    "StrokeAttribute for this StrokeVertex.\n"
    "\n"
    ":type: :class:`StrokeAttribute`");

static PyObject *StrokeVertex_attribute_get(BPy_StrokeVertex *self, void * /*closure*/)
{
  return BPy_StrokeAttribute_from_StrokeAttribute(self->sv->attribute());
}

static int StrokeVertex_attribute_set(BPy_StrokeVertex *self, PyObject *value, void * /*closure*/)
{
  if (!BPy_StrokeAttribute_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "value must be a StrokeAttribute object");
    return -1;
  }
  self->sv->setAttribute(*(((BPy_StrokeAttribute *)value)->sa));
  return 0;
}

PyDoc_STRVAR(
    /* Wrap. */
    StrokeVertex_curvilinear_abscissa_doc,
    "Curvilinear abscissa of this StrokeVertex in the Stroke.\n"
    "\n"
    ":type: float");

static PyObject *StrokeVertex_curvilinear_abscissa_get(BPy_StrokeVertex *self, void * /*closure*/)
{
  return PyFloat_FromDouble(self->sv->curvilinearAbscissa());
}

static int StrokeVertex_curvilinear_abscissa_set(BPy_StrokeVertex *self,
                                                 PyObject *value,
                                                 void * /*closure*/)
{
  float scalar;
  if ((scalar = PyFloat_AsDouble(value)) == -1.0f && PyErr_Occurred()) {
    /* parsed item not a number */
    PyErr_SetString(PyExc_TypeError, "value must be a number");
    return -1;
  }
  self->sv->setCurvilinearAbscissa(scalar);
  return 0;
}

PyDoc_STRVAR(
    /* Wrap. */
    StrokeVertex_point_doc,
    "2D point coordinates.\n"
    "\n"
    ":type: :class:`mathutils.Vector`");

static PyObject *StrokeVertex_point_get(BPy_StrokeVertex *self, void * /*closure*/)
{
  return Vector_CreatePyObject_cb((PyObject *)self, 2, StrokeVertex_mathutils_cb_index, 0);
}

static int StrokeVertex_point_set(BPy_StrokeVertex *self, PyObject *value, void * /*closure*/)
{
  float v[2];
  if (mathutils_array_parse(v, 2, 2, value, "value must be a 2-dimensional vector") == -1) {
    return -1;
  }
  self->sv->setX(v[0]);
  self->sv->setY(v[1]);
  return 0;
}

PyDoc_STRVAR(
    /* Wrap. */
    StrokeVertex_stroke_length_doc,
    "Stroke length (it is only a value retained by the StrokeVertex,\n"
    "and it won't change the real stroke length).\n"
    "\n"
    ":type: float");

static PyObject *StrokeVertex_stroke_length_get(BPy_StrokeVertex *self, void * /*closure*/)
{
  return PyFloat_FromDouble(self->sv->strokeLength());
}

static int StrokeVertex_stroke_length_set(BPy_StrokeVertex *self,
                                          PyObject *value,
                                          void * /*closure*/)
{
  float scalar;
  if ((scalar = PyFloat_AsDouble(value)) == -1.0f && PyErr_Occurred()) {
    /* parsed item not a number */
    PyErr_SetString(PyExc_TypeError, "value must be a number");
    return -1;
  }
  self->sv->setStrokeLength(scalar);
  return 0;
}

PyDoc_STRVAR(
    /* Wrap. */
    StrokeVertex_u_doc,
    "Curvilinear abscissa of this StrokeVertex in the Stroke.\n"
    "\n"
    ":type: float");

static PyObject *StrokeVertex_u_get(BPy_StrokeVertex *self, void * /*closure*/)
{
  return PyFloat_FromDouble(self->sv->u());
}

static PyGetSetDef BPy_StrokeVertex_getseters[] = {
    {"attribute",
     (getter)StrokeVertex_attribute_get,
     (setter)StrokeVertex_attribute_set,
     StrokeVertex_attribute_doc,
     nullptr},
    {"curvilinear_abscissa",
     (getter)StrokeVertex_curvilinear_abscissa_get,
     (setter)StrokeVertex_curvilinear_abscissa_set,
     StrokeVertex_curvilinear_abscissa_doc,
     nullptr},
    {"point",
     (getter)StrokeVertex_point_get,
     (setter)StrokeVertex_point_set,
     StrokeVertex_point_doc,
     nullptr},
    {"stroke_length",
     (getter)StrokeVertex_stroke_length_get,
     (setter)StrokeVertex_stroke_length_set,
     StrokeVertex_stroke_length_doc,
     nullptr},
    {"u", (getter)StrokeVertex_u_get, (setter) nullptr, StrokeVertex_u_doc, nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} /* Sentinel */
};

/*-----------------------BPy_StrokeVertex type definition ------------------------------*/

PyTypeObject StrokeVertex_Type = {
    /*ob_base*/ PyVarObject_HEAD_INIT(nullptr, 0)
    /*tp_name*/ "StrokeVertex",
    /*tp_basicsize*/ sizeof(BPy_StrokeVertex),
    /*tp_itemsize*/ 0,
    /*tp_dealloc*/ nullptr,
    /*tp_vectorcall_offset*/ 0,
    /*tp_getattr*/ nullptr,
    /*tp_setattr*/ nullptr,
    /*tp_as_async*/ nullptr,
    /*tp_repr*/ nullptr,
    /*tp_as_number*/ nullptr,
    /*tp_as_sequence*/ nullptr,
    /*tp_as_mapping*/ nullptr,
    /*tp_hash*/ nullptr,
    /*tp_call*/ nullptr,
    /*tp_str*/ nullptr,
    /*tp_getattro*/ nullptr,
    /*tp_setattro*/ nullptr,
    /*tp_as_buffer*/ nullptr,
    /*tp_flags*/ Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    /*tp_doc*/ StrokeVertex_doc,
    /*tp_traverse*/ nullptr,
    /*tp_clear*/ nullptr,
    /*tp_richcompare*/ nullptr,
    /*tp_weaklistoffset*/ 0,
    /*tp_iter*/ nullptr,
    /*tp_iternext*/ nullptr,
    /*tp_methods*/ nullptr,
    /*tp_members*/ nullptr,
    /*tp_getset*/ BPy_StrokeVertex_getseters,
    /*tp_base*/ &CurvePoint_Type,
    /*tp_dict*/ nullptr,
    /*tp_descr_get*/ nullptr,
    /*tp_descr_set*/ nullptr,
    /*tp_dictoffset*/ 0,
    /*tp_init*/ (initproc)StrokeVertex_init,
    /*tp_alloc*/ nullptr,
    /*tp_new*/ nullptr,
};

///////////////////////////////////////////////////////////////////////////////////////////
