#ifndef SC_SMARTSPATTERN_H
#define SC_SMARTSPATTERN_H

#include "util.h"

#include <vector>

#ifdef HAVE_PYTHON
#include <Python.h>
#endif

namespace SC {

  enum SmartsExprType
  {
    // null type
    SE_NullType,
    AE_NullType = SE_NullType,
    BE_NullType = SE_NullType,
    // unary & binary operations
    SE_AndHigh,
    AE_AndHigh = SE_AndHigh,
    BE_AndHigh = SE_AndHigh,
    SE_AndLow,
    AE_AndLow = SE_AndLow,
    BE_AndLow = SE_AndLow,
    SE_Or,
    AE_Or = SE_Or,
    BE_Or = SE_Or,
    SE_Not,
    AE_Not = SE_Not,
    BE_Not = SE_Not,
    AE_Recursive,
    // true & false
    SE_True,
    AE_True = SE_True, // *
    BE_True = SE_True, // ~
    SE_False,
    AE_False = SE_False, // !*
    BE_False = SE_False, // !~
    // atom expressions
    AE_Aromatic, // a
    AE_Aliphatic, // A
    AE_Cyclic, // R
    AE_Acyclic, // R0
    AE_Isotope, // <n>
    AE_Element, // #<n> (symbols for not organic subset)
    AE_AromaticElement, // c n o ...
    AE_AliphaticElement, // C N O
    AE_HydrogenCount, // H<n> (default: H = H1)
    AE_ImplicitHydrogenCount, // h<n> (default: h >= h1)
    AE_Charge, // -<n> +<n> (default: - = -1 + = +1)
    AE_Connectivity, // X<n> // (default: X = X1)
    AE_Degree, // D<n> (default: D = D1)
    AE_Valence, // v<n> (default: v = v1)
    AE_NumRings, // R<n> (no default)
    AE_RingConnectivity, // x<n> (default: x = x1)
    AE_RingSize, // r<n> (default: r = R)
    AE_Hybridization, // ^ (default: ^ = ^1)
    AE_Chiral, // @/@@ (anticlockwise/clockwise, default class), @<c><n> (no default), @<c><n>? (no default)
    // bond expressions
    BE_Default, // implicit CC cc
    BE_Single, // -
    BE_Double, // =
    BE_Triple, // #
    BE_Quadruple, // $
    BE_Aromatic, // :
    BE_Ring, // @
    BE_Up, // '\'
    BE_Down, // /
    BE_UpUnspecified, // /?
    BE_DownUnspecified // \?
  };

  typedef struct SmartsAtomExpr_ {
    enum SmartsExprType type;
    union {
      struct { int value; } leaf;
      struct { void *expr; } recursive;
      struct { SmartsAtomExpr_ *arg; } unary;
      struct { SmartsAtomExpr_ *left, *right; } binary;
    };
  } SmartsAtomExpr;

  typedef struct SmartsBondExpr_ {
    enum SmartsExprType type;
    union {
      struct { int value; } leaf;
      struct { SmartsBondExpr_ *arg; } unary;
      struct { SmartsBondExpr_ *left, *right; } binary;
    };
  } SmartsBondExpr;

  template<typename Expr>
  struct SmartsBase { Expr *expr; };
  template<>
  struct SmartsBase<EmptyType> {};

  /**
   * Empty type optimization. An empty base class is allowed to have size 0
   * while this is not allowed for a data member which is at least size 1. The
   * size 1 data member could also be padded taking up 8 bytes on a 64 bit
   * platform in the worst case.
   */
  template<typename Expr>
  struct SmartsAtom : public SmartsBase<Expr>
  {
    int fragment;
    int atomclass;
    int chiral;
  };

  struct SmartsBond
  {
    SmartsBond() : src(-1), dst(-1), grow(0)
    {
    }

    SmartsBond(int src_, int dst_, int grow_) : src(src_), dst(dst_), grow(grow_)
    {
    }

    int src, dst;
    bool grow;
  };
 
  template<typename AtomType, typename BondType>
  struct SmartsPattern
  {
    bool CallEvalAtomExpr(int index, AtomType *atom) const;
    bool CallEvalBondExpr(int index, BondType *bond) const;


    int numAtoms;
    std::vector<SmartsBond> bonds;
    bool ischiral;

    bool (*EvalAtomExpr)(int, AtomType*);
    bool (*EvalBondExpr)(int, BondType*);
  };

#if defined(HAVE_PYTHON) || defined(SWIG)
  struct PythonSmartsPattern
  {
    template<typename AtomType>
    bool CallEvalAtomExpr(int index, AtomType *atom) const;
    template<typename BondType>
    bool CallEvalBondExpr(int index, BondType *bond) const;

    int numAtoms;
    std::vector<SmartsBond> bonds;
    bool ischiral;

    PyObject *EvalAtomExpr;
    PyObject *EvalBondExpr;
  };
#endif
 
}

#endif
