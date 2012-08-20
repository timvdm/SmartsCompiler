#ifndef SC_SWIG_H
#define SC_SWIG_H

#include "swigpyrun.h"

namespace SC {

#define SWIG_as_voidptr(a) const_cast< void * >(static_cast< const void * >(a)) 

  static swig_type_info swig_types[3] = {
    {"_p_OpenBabel__OBMol", "std::vector< OpenBabel::OBMol >::value_type *|OpenBabel::OBMol *", 0, 0, (void*)0, 0},
    {"_p_OpenBabel__OBAtom", "OpenBabel::OBNodeBase *|OpenBabel::OBAtom *", 0, 0, (void*)0, 0},
    {"_p_OpenBabel__OBBond", "std::vector< OpenBabel::OBBond >::value_type *|OpenBabel::OBBond *|OpenBabel::OBEdgeBase *", 0, 0, (void*)0, 0}
  };

#define _swigt__p_OpenBabel__OBMol swig_types[0]
#define _swigt__p_OpenBabel__OBAtom swig_types[1]
#define _swigt__p_OpenBabel__OBBond swig_types[2]

  /* PySwigObject */
  typedef struct {
    PyObject_HEAD
      void *ptr;
    swig_type_info *ty;
    int own;
    PyObject *next;
  } PySwigObject;


}

#endif
