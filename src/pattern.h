#include "defines.h"
#include "util.h"
#include "smarts.h"

namespace SC {

  template<typename Expr>
  bool IsLeaf(Expr *expr)
  {
    if (SameType<SmartsAtomExpr, Expr>::result)
      switch (expr->type) {
        case Smiley::OP_AndHi:
        case Smiley::OP_AndLo:
        case Smiley::OP_Or:
        case Smiley::OP_Not:
        //case AE_RECUR:
          return false;
        default:
          return true;      
      }
    switch (expr->type) {
      case Smiley::OP_AndHi:
      case Smiley::OP_AndLo:
      case Smiley::OP_Or:
      case Smiley::OP_Not:
        return false;
      default:
        return true;      
    }
  }

  template<typename Expr>
  bool IsNot(Expr *expr)
  {
    if (SameType<SmartsAtomExpr, Expr>::result)
      return expr->type == Smiley::OP_Not;
    return expr->type == Smiley::OP_Not;
  }

  template<typename Expr>
  bool IsNegatedLeaf(Expr *expr)
  {
    return IsNot(expr) && IsLeaf(expr->unary.arg);
  }

  template<typename Expr>
  bool IsAnd(Expr *expr)
  {
    if (SameType<SmartsAtomExpr, Expr>::result)
      return expr->type == Smiley::OP_AndHi || expr->type == Smiley::OP_AndLo;
    return expr->type == Smiley::OP_AndHi || expr->type == Smiley::OP_AndLo;
  }

  template<typename Expr>
  bool IsOr(Expr *expr)
  {
    if (SameType<SmartsAtomExpr, Expr>::result)
      return expr->type == Smiley::OP_Or;
    return expr->type == Smiley::OP_Or;
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
        if (SameType<SmartsAtomExpr, Expr>::result)
          return Smiley::OP_Or;
        return Smiley::OP_Or;
    }
    if (SameType<SmartsAtomExpr, Expr>::result)
      return Smiley::OP_AndLo;
    return Smiley::OP_AndLo;
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
    if (SameType<SmartsAtomExpr, Expr>::result)
      switch (expr->type) {
        case Smiley::AE_Isotope:
        case Smiley::AE_AtomicNumber:
        case Smiley::AE_AromaticElement:
        case Smiley::AE_AliphaticElement:
        case Smiley::AE_TotalH:
        case Smiley::AE_Charge:
        case Smiley::AE_Connectivity:
        case Smiley::AE_Degree:
        case Smiley::AE_ImplicitH:
        case Smiley::AE_RingMembership:
        case Smiley::AE_RingSize:
        case Smiley::AE_Valence:
        //case AE_HYB:
        case Smiley::AE_RingConnectivity:
        case Smiley::AE_Chirality:
          return true;
        default:
          return false;
      }
    return false;
  }

  template<typename Expr>
  bool IsUnique(Expr *expr)
  {
    if (SameType<SmartsAtomExpr, Expr>::result)
      switch (expr->type) {
        case Smiley::AE_Isotope:
        case Smiley::AE_AtomicNumber:
        case Smiley::AE_AromaticElement:
        case Smiley::AE_AliphaticElement:
        case Smiley::AE_TotalH:
        case Smiley::AE_Charge:
        case Smiley::AE_Connectivity:
        case Smiley::AE_Degree:
        case Smiley::AE_ImplicitH:
        case Smiley::AE_RingMembership:
        case Smiley::AE_Valence:
        case Smiley::AE_Chirality:
        //case AE_HYB:
        case Smiley::AE_RingConnectivity:
          return true;
        default:
          return false;
      }
    return false;
  }

  template<typename Expr>
  bool IsBondOrderExpr(Expr *expr)
  {
    if (SameType<SmartsBondExpr, Expr>::result)
      switch (expr->type) {
        case Smiley::BE_Single:
        case Smiley::BE_Double:
        case Smiley::BE_Triple:
        case BE_QUAD:
        case Smiley::BE_Aromatic:
          return true;
        default:
          return false;
      }
    return false;
  }

  inline bool IsDuplicate(SmartsAtomExpr *left, SmartsAtomExpr *right)
  {
    if (left->type != right->type)
      return false;
    if (IsNot(left)) {
      left = left->unary.arg;
      right = right->unary.arg;
    }
    if (IsLeaf(left)) {
      if (IsValued(left))
        return left->leaf.value == right->leaf.value;
      return true;
    }
    return false;
  }

  inline bool IsDuplicate(SmartsBondExpr *left, SmartsBondExpr *right)
  {
    if (left->type != right->type)
      return false;
    if (IsNot(left)) {
      left = left->unary.arg;
      right = right->unary.arg;
    }
    if (IsLeaf(left))
      return true;
    return false;
  }

  template<typename Expr>
  void DeleteExpr(Expr *expr)
  {
    if (IsUnary(expr))
      DeleteExpr(expr->unary.arg);
    if (IsBinary(expr)) {
      DeleteExpr(expr->binary.lft);
      DeleteExpr(expr->binary.rgt);
    }
    delete expr;
  }

  template<typename Expr>
  int CountExpr(Expr *expr)
  {
    int size = 1;
    if (IsUnary(expr))
      size += CountExpr(expr->unary.arg);
    if (IsBinary(expr)) {
      size += CountExpr(expr->binary.lft);
      size += CountExpr(expr->binary.rgt);
    }
    return size;
  }

  template<typename Expr>
  void FindSameBinaryExpr(Expr *expr, std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    same.push_back(expr);
    if (IsSameType(expr, expr->binary.lft))
      FindSameBinaryExpr(expr->binary.lft, same, other);
    else
      other.push_back(expr->binary.lft);
    if (IsSameType(expr, expr->binary.rgt))
      FindSameBinaryExpr(expr->binary.rgt, same, other);
    else
      other.push_back(expr->binary.rgt);
  }
 
}
