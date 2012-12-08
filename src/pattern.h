#include "defines.h"
#include "util.h"

#include <openbabel/parsmart.h>

namespace SC {

  using namespace OpenBabel;

  template<typename Expr>
  bool IsLeaf(Expr *expr)
  {
    if (SameType<AtomExpr, Expr>::result)
      switch (expr->type) {
        case AE_ANDHI:
        case AE_ANDLO:
        case AE_OR:
        case AE_NOT:
        case AE_RECUR:
          return false;
        default:
          return true;      
      }
    switch (expr->type) {
      case BE_ANDHI:
      case BE_ANDLO:
      case BE_OR:
      case BE_NOT:
        return false;
      default:
        return true;      
    }
  }

  template<typename Expr>
  bool IsNot(Expr *expr)
  {
    if (SameType<AtomExpr, Expr>::result)
      return expr->type == AE_NOT;
    return expr->type == BE_NOT;
  }

  template<typename Expr>
  bool IsNegatedLeaf(Expr *expr)
  {
    return IsNot(expr) && IsLeaf(expr->mon.arg);
  }

  template<typename Expr>
  bool IsAnd(Expr *expr)
  {
    if (SameType<AtomExpr, Expr>::result)
      return expr->type == AE_ANDHI || expr->type == AE_ANDLO;
    return expr->type == BE_ANDHI || expr->type == BE_ANDLO;
  }

  template<typename Expr>
  bool IsOr(Expr *expr)
  {
    if (SameType<AtomExpr, Expr>::result)
      return expr->type == AE_OR;
    return expr->type == BE_OR;
  }

  template<typename Expr>
  bool IsUnary(Expr *expr)
  {
    return IsNot(expr);
  }

  template<typename Expr>
  bool IsBinary(Expr *expr)
  {
    return IsAnd(expr) || IsOr(expr);
  }

  template<typename Expr>
  int GetOppositeBinaryExprType(Expr *expr)
  {
    if (IsAnd(expr)) {
        if (SameType<AtomExpr, Expr>::result)
          return AE_OR;
        return BE_OR;
    }
    if (SameType<AtomExpr, Expr>::result)
      return AE_ANDLO;
    return BE_ANDLO;
  }

  template<typename Expr>
  bool IsSameType(Expr *left, Expr *right)
  {
    if (IsAnd(left) && IsAnd(right))
      return true;
    return left->type == right->type;
  }
  
  template<typename Expr>
  bool AreTypes(Expr *left, Expr *right, int type1, int type2)
  {
    return (left->type == type1 && right->type == type2) || (left->type == type2 && right->type == type1);
  }

  template<typename Expr>
  bool IsValued(Expr *expr)
  {
    if (SameType<AtomExpr, Expr>::result)
      switch (expr->type) {
        case AE_MASS:
        case AE_ELEM:
        case AE_AROMELEM:
        case AE_ALIPHELEM:
        case AE_HCOUNT:
        case AE_CHARGE:
        case AE_CONNECT:
        case AE_DEGREE:
        case AE_IMPLICIT:
        case AE_RINGS:
        case AE_SIZE:
        case AE_VALENCE:
        case AE_HYB:
        case AE_RINGCONNECT:
        case AE_CHIRAL:
          return true;
        default:
          return false;
      }
    return false;
  }

  template<typename Expr>
  bool IsUnique(Expr *expr)
  {
    if (SameType<AtomExpr, Expr>::result)
      switch (expr->type) {
        case AE_MASS:
        case AE_ELEM:
        case AE_AROMELEM:
        case AE_ALIPHELEM:
        case AE_HCOUNT:
        case AE_CHARGE:
        case AE_CONNECT:
        case AE_DEGREE:
        case AE_IMPLICIT:
        case AE_RINGS:
        case AE_VALENCE:
        case AE_CHIRAL:
        case AE_HYB:
        case AE_RINGCONNECT:
          return true;
        default:
          return false;
      }
    return false;
  }

  template<typename Expr>
  bool IsBondOrderExpr(Expr *expr)
  {
    if (SameType<BondExpr, Expr>::result)
      switch (expr->type) {
        case BE_SINGLE:
        case BE_DOUBLE:
        case BE_TRIPLE:
        case BE_QUAD:
        case BE_AROM:
          return true;
        default:
          return false;
      }
    return false;
  }

  inline bool IsDuplicate(AtomExpr *left, AtomExpr *right)
  {
    if (left->type != right->type)
      return false;
    if (IsNot(left)) {
      left = left->mon.arg;
      right = right->mon.arg;
    }
    if (IsLeaf(left)) {
      if (IsValued(left))
        return left->leaf.value == right->leaf.value;
      return true;
    }
    return false;
  }

  inline bool IsDuplicate(BondExpr *left, BondExpr *right)
  {
    if (left->type != right->type)
      return false;
    if (IsNot(left)) {
      left = left->mon.arg;
      right = right->mon.arg;
    }
    if (IsLeaf(left))
      return true;
    return false;
  }

  template<typename Expr>
  void DeleteExpr(Expr *expr)
  {
    if (IsUnary(expr))
      DeleteExpr(expr->mon.arg);
    if (IsBinary(expr)) {
      DeleteExpr(expr->bin.lft);
      DeleteExpr(expr->bin.rgt);
    }
    delete expr;
  }

  template<typename Expr>
  int CountExpr(Expr *expr)
  {
    int size = 1;
    if (IsUnary(expr))
      size += CountExpr(expr->mon.arg);
    if (IsBinary(expr)) {
      size += CountExpr(expr->bin.lft);
      size += CountExpr(expr->bin.rgt);
    }
    return size;
  }

  template<typename Expr>
  void FindSameBinaryExpr(Expr *expr, std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    same.push_back(expr);
    if (IsSameType(expr, expr->bin.lft))
      FindSameBinaryExpr(expr->bin.lft, same, other);
    else
      other.push_back(expr->bin.lft);
    if (IsSameType(expr, expr->bin.rgt))
      FindSameBinaryExpr(expr->bin.rgt, same, other);
    else
      other.push_back(expr->bin.rgt);
  }
 
}
