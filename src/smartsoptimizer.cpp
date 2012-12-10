#include "smartsoptimizer.h"
#include "smartsscores.h"
#include "smartsprint.h"
#include "pattern.h"
#include "util.h"

namespace SC {

  /**
   * Double negation.
   *
   * not(not(x)) = x
   *
   * [!!C] -> C
   */
  template<typename Expr>
  Expr* DoubleNegation(Expr *expr)
  {
    if (IsNot(expr)) {
      if (IsNot(expr->unary.arg)) {
        Expr *tmp = expr->unary.arg->unary.arg;
        delete expr->unary.arg;
        delete expr;
        expr = DoubleNegation(tmp);
        return expr;
      }
    } else if (!IsLeaf(expr)) {
      expr->binary.lft = DoubleNegation(expr->binary.lft);
      expr->binary.rgt = DoubleNegation(expr->binary.rgt);      
    }
    return expr;
  }

  /**
   * True elimination.
   *
   * and(x, true) = x
   * or(x, true) = true
   *
   * [C&*] -> [C]
   * [C,*] -> [*]
   */
  template<typename Expr>
  Expr* TrueElimination(Expr *expr)
  {
    if (SameType<Expr, SmartsBondExpr>::result)
      return expr;

    if (expr->type == Smiley::OP_AndHi || expr->type == Smiley::OP_AndLo) {
      if (expr->binary.lft->type == Smiley::AE_True) {
        Expr *tmp = expr->binary.rgt;
        DeleteExpr(expr->binary.lft);
        delete expr;
        tmp = TrueElimination(tmp);
        return tmp;
      }
      if (expr->binary.rgt->type == Smiley::AE_True) {
        Expr *tmp = expr->binary.lft;
        DeleteExpr(expr->binary.rgt);
        delete expr;
        tmp = TrueElimination(tmp);
        return tmp;
      }
      expr->binary.lft = TrueElimination(expr->binary.lft);
      expr->binary.rgt = TrueElimination(expr->binary.rgt);
    }
    if (expr->type == Smiley::OP_Or) {
      if (expr->binary.lft->type == Smiley::AE_True) {
        Expr *tmp = expr->binary.lft;
        DeleteExpr(expr->binary.rgt);
        delete expr;
        tmp = TrueElimination(tmp);
        return tmp;
      }
      if (expr->binary.rgt->type == Smiley::AE_True) {
        Expr *tmp = expr->binary.rgt;
        DeleteExpr(expr->binary.lft);
        delete expr;
        tmp = TrueElimination(tmp);
        return tmp;
      }
      expr->binary.lft = TrueElimination(expr->binary.lft);
      expr->binary.rgt = TrueElimination(expr->binary.rgt);
    }
    return expr;
  }

  /**
   * False elimination.
   *
   * and(x, false) = false
   * or(x, false) = x
   *
   * [C&!*] -> [!*]
   * [C,!*] -> [C]
   */
  template<typename Expr>
  Expr* FalseElimination(Expr *expr)
  {
    if (SameType<Expr, SmartsBondExpr>::result)
      return expr;

    if (IsAnd(expr)) {
      if (expr->binary.lft->type == Smiley::AE_False) {
        Expr *tmp = expr->binary.lft;
        DeleteExpr(expr->binary.rgt);
        delete expr;
        tmp = FalseElimination(tmp);
        return tmp;
      }
      if (expr->binary.rgt->type == Smiley::AE_False) {
        Expr *tmp = expr->binary.rgt;
        DeleteExpr(expr->binary.lft);
        delete expr;
        tmp = FalseElimination(tmp);
        return tmp;
      }
      expr->binary.lft = FalseElimination(expr->binary.lft);
      expr->binary.rgt = FalseElimination(expr->binary.rgt);
    }
    if (IsOr(expr)) {
      if (expr->binary.lft->type == Smiley::AE_False) {
        Expr *tmp = expr->binary.rgt;
        DeleteExpr(expr->binary.lft);
        delete expr;
        tmp = FalseElimination(tmp);
        return tmp;
      }
      if (expr->binary.rgt->type == Smiley::AE_False) {
        Expr *tmp = expr->binary.lft;
        DeleteExpr(expr->binary.rgt);
        delete expr;
        tmp = FalseElimination(tmp);
        return tmp;
      }
      expr->binary.lft = FalseElimination(expr->binary.lft);
      expr->binary.rgt = FalseElimination(expr->binary.rgt);
    }
    return expr;
  }

  /**
   * Duplicate elimination.
   *
   * and(x, x) - > x
   * or(x, x) -> x
   */
  template<typename Expr>
  Expr* DuplicateElimination(Expr *expr)
  {
    if (IsAnd(expr) || IsOr(expr)) {
      if (IsDuplicate(expr->binary.lft, expr->binary.rgt)) {
        Expr *tmp = expr->binary.lft;
        DeleteExpr(expr->binary.rgt);
        delete expr;
        tmp = DuplicateElimination(tmp);
        return tmp;
      }

      expr->binary.lft = DuplicateElimination(expr->binary.lft);
      expr->binary.rgt = DuplicateElimination(expr->binary.rgt);
    }
    return expr;
  }

  /**
   * Negation.
   *
   * !true -> false
   * !false -> true
   * !a -> A
   * !A -> a
   * !R -> R0
   * !R0 -> R
   */
  template<typename Expr>
  Expr* Negation(Expr *expr)
  {
    if (SameType<Expr, SmartsBondExpr>::result)
      return expr;

    if (IsBinary(expr)) {
      expr->binary.lft = Negation(expr->binary.lft);
      expr->binary.rgt = Negation(expr->binary.rgt);
    }
    if (IsNot(expr)) {
      if (IsLeaf(expr->unary.arg) && !IsValued(expr->unary.arg)) {
        Expr *tmp = expr->unary.arg;
        delete expr;
        switch (tmp->type) {
          case Smiley::AE_True:
            tmp->type = Smiley::AE_False;
            return tmp;
          case Smiley::AE_False:
            tmp->type = Smiley::AE_True;
            return tmp;
          case Smiley::AE_Aromatic:
            tmp->type = Smiley::AE_Aliphatic;
            return tmp;
          case Smiley::AE_Aliphatic:
            tmp->type = Smiley::AE_Aromatic;
            return tmp;
          case Smiley::AE_Cyclic:
            tmp->type = Smiley::AE_Acyclic;
            return tmp;
          case Smiley::AE_Acyclic:
            tmp->type = Smiley::AE_Cyclic;
            return tmp;        
        }
      }
      expr->unary.arg = Negation(expr->unary.arg);
    }
    return expr;
  }

  template<typename Expr>
  void RemoveExpressions(std::vector<std::size_t> remove, std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    std::sort(remove.begin(), remove.end());
    remove.resize(std::unique(remove.begin(), remove.end()) - remove.begin());
    while (!remove.empty()) {
      delete same.back();
      same.pop_back();
      std::size_t index = remove.back();
      DeleteExpr(other[index]);
      remove.pop_back();
      other.erase(other.begin() + index);
    }
  }

  template<typename Expr>
  void DuplicateElimination(std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    std::vector<std::size_t> duplicates;
    for (std::size_t i = 0; i < other.size(); ++i) {
      for (std::size_t j = i + 1; j < other.size(); ++j) {
        if (IsDuplicate(other[i], other[j]))
          duplicates.push_back(j);
        else
          break;
      }
    }
    RemoveExpressions(duplicates, same, other);
  }

  template<typename Expr>
  bool IsAromAliphAndExpansion(Expr *expr1, Expr *expr2)
  {
    if (expr1 == expr2)
      return false;
    if (expr1->type == Smiley::AE_AtomicNumber && expr2->type == Smiley::AE_Aliphatic) {
      expr1->type = Smiley::AE_AliphaticElement;
      return true;
    }
    if (expr1->type == Smiley::AE_AtomicNumber && expr2->type == Smiley::AE_Aromatic) {
      expr1->type = Smiley::AE_AromaticElement;
      return true;
    }
    if (expr1->type == Smiley::AE_AromaticElement && expr2->type == Smiley::AE_Aromatic)
      return true;
    if (expr1->type == Smiley::AE_AliphaticElement && expr2->type == Smiley::AE_Aliphatic)
      return true;
    return false;
  }

  template<typename Expr>
  void AromAliphAndContraction(std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    if (SameType<Expr, SmartsBondExpr>::result)
      return;

    std::vector<std::size_t> remove;
    for (std::size_t i = 0; i < other.size(); ++i)
      for (std::size_t j = 0; j < other.size(); ++j)
        if (IsAromAliphAndExpansion(other[i], other[j]))
          remove.push_back(j);

    RemoveExpressions(remove, same, other);
  }

  template<typename Expr>
  bool IsAromAliphOrExpansion(Expr *expr1, Expr *expr2)
  {
    if (expr1 == expr2)
      return false;
    if (expr1->type == Smiley::AE_Aliphatic && expr2->type == Smiley::AE_AliphaticElement)
      return true;
    if (expr1->type == Smiley::AE_Aromatic && expr2->type == Smiley::AE_AromaticElement)
      return true;
    if (expr1->type == Smiley::AE_Aliphatic && expr2->type == Smiley::AE_Aromatic) {
      expr1->type = Smiley::AE_True;
      return true;
    }
    return false;
  }

  template<typename Expr>
  void AromAliphOrContraction(std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    if (SameType<Expr, SmartsBondExpr>::result)
      return;

    std::vector<std::size_t> remove;
    for (std::size_t i = 0; i < other.size(); ++i)
      for (std::size_t j = 0; j < other.size(); ++j)
        if (IsAromAliphOrExpansion(other[i], other[j]))
          remove.push_back(j);

    RemoveExpressions(remove, same, other);
  }

  bool IsAndRedundant(SmartsAtomExpr *expr1, SmartsAtomExpr *expr2)
  {
    if (expr1 == expr2)
      return false;
    // e.g. [C!N] [X2!X3] [R3!R2] ...
    if (IsUnique(expr1) && expr2->type == Smiley::OP_Not && expr1->type == expr2->unary.arg->type)
      return expr1->leaf.value != expr2->unary.arg->leaf.value;
    // e.g. [F!#6] = F
    if (expr1->type == Smiley::AE_AliphaticElement && expr2->type == Smiley::OP_Not && expr2->unary.arg->type == Smiley::AE_AtomicNumber)
      return true;
    // e.g. [F!n] = F
    if (expr1->type == Smiley::AE_AliphaticElement && expr2->type == Smiley::OP_Not && expr2->unary.arg->type == Smiley::AE_AromaticElement)
      return true;
    // e.g. [n!#8] = n
    if (expr1->type == Smiley::AE_AromaticElement && expr2->type == Smiley::OP_Not && expr2->unary.arg->type == Smiley::AE_AtomicNumber)
      return true;
    // e.g. [o!C] = o
    if (expr1->type == Smiley::AE_AromaticElement && expr2->type == Smiley::OP_Not && expr2->unary.arg->type == Smiley::AE_AliphaticElement)
      return true;
    // e.g. [#6!N] = [#6]
    if (expr1->type == Smiley::AE_AtomicNumber && expr2->type == Smiley::OP_Not && expr2->unary.arg->type == Smiley::AE_AliphaticElement)
      return true;
    // e.g. [#7!o] = [#7]
    if (expr1->type == Smiley::AE_AtomicNumber && expr2->type == Smiley::OP_Not && expr2->unary.arg->type == Smiley::AE_AromaticElement)
      return true;
    // e.g. [aR] = [a]
    if (expr1->type == Smiley::AE_Aromatic && expr2->type == Smiley::AE_Cyclic)
      return true;
    // e.g. [RR2] = [R2]
    if (expr1->type == Smiley::AE_RingMembership && expr2->type == Smiley::AE_Cyclic)
      return true;
    // e.g. [Rr6] = [r6]
    if (expr1->type == Smiley::AE_RingSize && expr2->type == Smiley::AE_Cyclic)
      return true;
    // e.g. [Rx3] = [x3]
    if (expr1->type == Smiley::AE_RingConnectivity && expr2->type == Smiley::AE_Cyclic)
      return true;
    return false;
  }

  bool IsAndRedundant(SmartsBondExpr *expr1, SmartsBondExpr *expr2)
  {
    if (expr1 == expr2)
      return false;
    // e.g. C~=C = C=C
    //      C~@C = C@C
    if ((IsBondOrderExpr(expr1) || expr1->type == Smiley::BE_Ring) && expr2->type == Smiley::BE_True)
      return true;
    // e.g. C-!=C C-!#C C-!:C
    if (IsBondOrderExpr(expr1) && expr2->type == Smiley::OP_Not && IsBondOrderExpr(expr2->unary.arg) && !IsSameType(expr1, expr2->unary.arg))
      return true;
    return false;
  }

  template<typename Expr>
  void RedundantAndElimination(std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    std::vector<std::size_t> remove;
    for (std::size_t i = 0; i < other.size(); ++i)
      for (std::size_t j = 0; j < other.size(); ++j)
        if (IsAndRedundant(other[i], other[j]))
          remove.push_back(j);

    RemoveExpressions(remove, same, other);
  }

  bool IsOrRedundant(SmartsAtomExpr *expr1, SmartsAtomExpr *expr2)
  {
    if (expr1 == expr2)
      return false;
    // e.g. [R,R2] [R,r6] [R,x2]
    if (expr1->type == Smiley::AE_Cyclic && (expr2->type == Smiley::AE_RingMembership || expr2->type == Smiley::AE_RingSize || expr2->type == Smiley::AE_RingConnectivity))
      return true;
    return false;
  }

  bool IsOrRedundant(SmartsBondExpr *expr1, SmartsBondExpr *expr2)
  {
    if (expr1 == expr2)
      return false;
    // e.g. C~,=C = C~C
    //      C~,@C = C~C
    if (expr1->type == Smiley::BE_True && (IsBondOrderExpr(expr2) || expr2->type == Smiley::BE_Ring))
      return true;
    return false;
  }

  template<typename Expr>
  void RedundantOrElimination(std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    std::vector<std::size_t> remove;
    for (std::size_t i = 0; i < other.size(); ++i)
      for (std::size_t j = 0; j < other.size(); ++j)
        if (IsOrRedundant(other[i], other[j]))
          remove.push_back(j);

    RemoveExpressions(remove, same, other);
  }

  /**
   * Swap left & right between same type binary expressions.
   */
  template<typename Expr>
  Expr* OptimizeBinaryExpr1(Expr *expr, SmartsScores *scores)
  {
    if (IsBinary(expr)) {
      std::vector<Expr*> same, other;
      FindSameBinaryExpr(expr, same, other);
      //assert(same.size() == other.size() || same.size() + 1 == other.size());
      // sort the other expressions by score
      scores->Sort(other, IsAnd(expr));
      /*
         for (std::size_t i = 0; i < other.size(); ++i)
         std::cout << GetExprString(other[i]) << " ";
         std::cout << std::endl;
         */
      // eliminate duplicates
      DuplicateElimination(same, other);
      // aromatic/aliphatic contraction
      if (IsAnd(expr)) {
        AromAliphAndContraction(same, other);        
        RedundantAndElimination(same, other);
      } else {
        AromAliphOrContraction(same, other);
        RedundantOrElimination(same, other);
      }
      if (same.empty())
        return other[0];
      if (same.size() == 1) {
        expr->binary.lft = other[0];
        expr->binary.rgt = other[1];
        expr->binary.lft = OptimizeBinaryExpr1(expr->binary.lft, scores);
        expr->binary.rgt = OptimizeBinaryExpr1(expr->binary.rgt, scores);
        return expr;
      }
      //assert(same.size() == other.size() || same.size() + 1 == other.size());

      // make a chain from all same expressions to the right
      for (std::size_t i = 1; i < same.size(); ++i)
        same[i - 1]->binary.rgt = same[i];
      // assign sorted other expressions to left children
      for (std::size_t i = 0; i < same.size(); ++i) {
        same[i]->binary.lft = other[i];
        if (i + 1 == same.size() && same.size() + 1 == other.size())
          same[i]->binary.rgt = other[i + 1];        
      }
      // optimize other
      for (std::size_t i = 0; i < other.size(); ++i)
        other[i] = OptimizeBinaryExpr1(other[i], scores);
    }

    if (IsUnary(expr))
      expr->unary.arg = OptimizeBinaryExpr1(expr->unary.arg, scores);

    return expr;
  }

  /**
   * Swap binary node children based on score.
   */
  template<typename Expr>
  void OptimizeBinaryExpr2(Expr *expr, SmartsScores *scores)
  {
    if (IsBinary(expr)) {
      double lftScore = scores->GetExprScore(expr->binary.lft);
      double rgtScore = scores->GetExprScore(expr->binary.rgt);

      std::vector<Expr*> children(2);
      children[0] = expr->binary.lft;
      children[1] = expr->binary.rgt;
      scores->Sort(children, IsAnd(expr));
      expr->binary.lft = children[0];
      expr->binary.rgt = children[1];

      if (!IsLeaf(expr->binary.lft))
        OptimizeBinaryExpr2(expr->binary.lft, scores);
      if (!IsLeaf(expr->binary.rgt))
        OptimizeBinaryExpr2(expr->binary.rgt, scores);
    }

    if (IsUnary(expr))
      OptimizeBinaryExpr2(expr->unary.arg, scores);
  }

  template<typename Expr>
  Expr* CreateExprTree(int type, std::vector<Expr*> &binaries, const std::vector<Expr*> &children)
  {
    if (children.empty())
      return 0;
    if (children.size() == 1)
      return children[0];
    Expr *root = 0;
    Expr *append = 0;
    for (std::size_t i = 0; i < children.size(); ++i) {
      if (i + 1 < children.size()) {
        // set up the next binary expression
        Expr *binary = binaries.back();
        binaries.pop_back();
        binary->type = type;
        if (append)
          append->binary.rgt = binary;
        else
          root = append = binary;
        // assign the left 
        binary->binary.lft = children[i];
        append = binary;
      } else {
        // assign last children expression to right
        append->binary.rgt = children[i];            
      }
    }
    return root; 
  }

  template<typename Expr>
  void ExprFactorization(Expr *expr)
  {
    if (IsBinary(expr)) {
      // find same type binary expressions
      std::vector<Expr*> same, other, opposite, not_opposite;
      FindSameBinaryExpr(expr, same, other);

      //std::cout << "same.size = " << same.size() << std::endl;
      //std::cout << "other.size = " << other.size() << std::endl;

      // find opposite type binary expressions in other
      for (std::size_t i = 0; i < other.size(); ++i)
        if ((IsAnd(expr) && IsOr(other[i])) || (IsOr(expr) && IsAnd(other[i])))
          opposite.push_back(other[i]);
        else
          not_opposite.push_back(other[i]);

      //std::cout << "opposite.size = " << opposite.size() << std::endl;
      //std::cout << "not_opposite.size = " << not_opposite.size() << std::endl;

      if (opposite.size() < 2) {
        for (std::size_t i = 0; i < other.size(); ++i)
          ExprFactorization(other[i]);
        return;        
      }

      // find others for opposite
      std::vector<std::vector<Expr*> > opposite_same(opposite.size()), opposite_other(opposite.size());
      for (std::size_t i = 0; i < opposite.size(); ++i) {
        FindSameBinaryExpr(opposite[i], opposite_same[i], opposite_other[i]);
        //std::cout << "opposite_same[" << i << "].size = " << opposite_same[i].size() << std::endl;
        //std::cout << "opposite_other[" << i << "].size = " << opposite_other[i].size() << std::endl;
      }

      // find the intersection of opposite's others
      std::vector<Expr*> intersection, duplicates;
      for (std::size_t i = 0; i < opposite_other[0].size(); ++i) {
        int found = 0;
        int duplicates_size = duplicates.size();
        for (std::size_t j = 1; j < opposite_other.size(); ++j)
          for (std::size_t k = 0; k < opposite_other[j].size(); ++k)
            if (IsDuplicate(opposite_other[0][i], opposite_other[j][k])) {
              duplicates.push_back(opposite_other[j][k]);
              ++found;
              break;
            }

        if (found == opposite_other.size() - 1)
          intersection.push_back(opposite_other[0][i]);
        else
          duplicates.resize(duplicates_size);
      }

      //std::cout << "intersection.size = " << intersection.size() << std::endl;
      //std::cout << "duplicates.size = " << duplicates.size() << std::endl;

      if (intersection.empty()) {
        for (std::size_t i = 0; i < opposite.size(); ++i)
          ExprFactorization(opposite[i]);
        return;
      }

      // find the non intersecting
      std::vector<std::vector<Expr*> > not_intersection(opposite_other.size());
      for (std::size_t i = 0; i < opposite_other[0].size(); ++i)
        if (std::find(intersection.begin(), intersection.end(), opposite_other[0][i]) == intersection.end())
          not_intersection[0].push_back(opposite_other[0][i]);
      for (std::size_t i = 1; i < opposite_other.size(); ++i)
        for (std::size_t j = 0; j < opposite_other[i].size(); ++j)
          if (std::find(duplicates.begin(), duplicates.end(), opposite_other[i][j]) == duplicates.end())
            not_intersection[i].push_back(opposite_other[i][j]);

      //for (std::size_t i = 0; i < not_intersection.size(); ++i)
      //std::cout << "not_intersection[" << i << "].size = " << not_intersection[i].size() << std::endl;

      // gather all binary expressions except for root expr
      std::vector<Expr*> binaries;
      binaries.insert(binaries.end(), same.begin() + 1, same.end());
      for (std::size_t i = 0; i < opposite_same.size(); ++i)
        binaries.insert(binaries.end(), opposite_same[i].begin(), opposite_same[i].end());
      for (std::size_t i = 0; i < opposite_same.size(); ++i)
        same.insert(same.end(), opposite_same[i].begin(), opposite_same[i].end());

      //std::cout << "binaries.size = " << binaries.size() << std::endl;

      // create the factorized tree
      Expr *factorized_tree = CreateExprTree(GetOppositeBinaryExprType(expr), binaries, intersection);

      //std::cout << "factorized_tree = " << GetExprString(factorized_tree) << std::endl;

      // create not_intersection trees
      std::vector<Expr*> not_intersection_trees;
      for (std::size_t i = 0; i < not_intersection.size(); ++i) {
        Expr *tree = CreateExprTree(GetOppositeBinaryExprType(expr), binaries, not_intersection[i]);
        if (tree) {
          not_intersection_trees.push_back(tree);
          //std::cout << "not_intersection_trees[" << not_intersection_trees.size() - 1 << "] = " << GetExprString(tree) << std::endl;
        }
      }

      // create not_intersection tree
      int type = expr->type;
      if (IsAnd(expr))
        type = SameType<Expr, SmartsAtomExpr>::result ? Smiley::OP_AndLo : Smiley::OP_AndLo;
      Expr *not_intersection_tree = CreateExprTree(type, binaries, not_intersection_trees);
      //if (not_intersection_tree)
      //std::cout << "not_intersection_tree = " << GetExprString(not_intersection_tree) << std::endl;

      // create not_opposite tree
      Expr *not_opposite_tree = CreateExprTree(type, binaries, not_opposite);
      //if (not_opposite_tree)
      //std::cout << "not_opposite_tree = " << GetExprString(not_opposite_tree) << std::endl;

      // assign trees to expr
      if (not_intersection_tree && not_opposite_tree) {
        Expr *binary = binaries.back();
        binaries.pop_back();
        binary->type = GetOppositeBinaryExprType(expr);
        binary->binary.lft = factorized_tree;
        binary->binary.rgt = not_intersection_tree;
        expr->binary.lft = binary;
        expr->binary.rgt = not_opposite_tree;
      } else if (not_intersection_tree) {
        expr->type = GetOppositeBinaryExprType(expr);
        expr->binary.lft = factorized_tree;
        expr->binary.rgt = not_intersection_tree;
      } else if (not_opposite_tree) {
        expr->type = GetOppositeBinaryExprType(expr);
        expr->binary.lft = factorized_tree;
        expr->binary.rgt = not_opposite_tree;
      } else {
        expr->binary.lft = factorized_tree;
        // hack... :)
        Expr *tmp = duplicates.back();
        duplicates.pop_back();
        if (IsNot(tmp))
          delete tmp->unary.arg;
        tmp->type = SameType<Expr, SmartsAtomExpr>::result ? Smiley::AE_True : Smiley::BE_True;
        expr->binary.rgt = tmp;
      }

      //std::cout << "expr = " << GetExprString(expr) << std::endl;

      // delete duplicates
      for (std::size_t i = 0; i < duplicates.size(); ++i)
        DeleteExpr(duplicates[i]);
      // delete remaining binary expressions
      for (std::size_t i = 0; i < binaries.size(); ++i)
        delete binaries[i];      
    }

    if (IsUnary(expr))
      ExprFactorization(expr->unary.arg);
  }

  template<typename Expr>
  Expr* OptimizeExpr(Expr *expr, int opts, SmartsScores *scores)
  {
    int last_count, count = CountExpr(expr);
    do {
      last_count = count;
      if (opts & SmartsOptimizer::DoubleNegationElim)
        expr = DoubleNegation(expr);
      if (opts & SmartsOptimizer::TrueElim)
        expr = TrueElimination(expr);
      if (opts & SmartsOptimizer::FalseElim)
        expr = FalseElimination(expr);
      if (opts & SmartsOptimizer::DuplicateElim)
        expr = DuplicateElimination(expr);
      if (opts & SmartsOptimizer::NegationElim)
        expr = Negation(expr);
      if (opts & SmartsOptimizer::BinaryExpr1)
        expr = OptimizeBinaryExpr1(expr, scores);
      if (opts & SmartsOptimizer::BinaryExpr2)
        OptimizeBinaryExpr2(expr, scores);
      if (opts & SmartsOptimizer::ExprFactor)
        ExprFactorization(expr);

    } while ((count = CountExpr(expr)) < last_count);
    return expr;
  }

  void FalsePropagation(Smarts *pattern)
  {
    // delete the expressions
    for (int i = 0; i < pattern->atoms.size(); ++i)
      DeleteExpr(pattern->atoms[i].expr);
    for (int i = 0; i < pattern->bonds.size(); ++i)
      DeleteExpr(pattern->bonds[i].expr);
    // reset counts
    pattern->atoms.clear();
    pattern->bonds.clear();
    // create single atom false spec
    pattern->atoms.resize(1);
    //pattern->atoms[0].part = 0;
    pattern->atoms[0].chiral = 0;
    pattern->atoms[0].atomClass = 0;
    pattern->atoms[0].expr = new SmartsAtomExpr(Smiley::AE_False);
  }

  void AtomFalsePropagation(Smarts *pattern)
  {
    for (int i = 0; i < pattern->atoms.size(); ++i)
      if (pattern->atoms[i].expr->type == Smiley::AE_False) {
        FalsePropagation(pattern);
        return;
      }
  }

  void BondFalsePropagation(Smarts *pattern)
  {
    for (int i = 0; i < pattern->bonds.size(); ++i)
      if (pattern->bonds[i].expr->type == Smiley::OP_Not)
        if (pattern->bonds[i].expr->unary.arg->type == Smiley::BE_True) {
          FalsePropagation(pattern);
          return;
        }
  }

  bool IsOpposite(SmartsAtomExpr *expr1, SmartsAtomExpr *expr2)
  {
    if (expr2->type == Smiley::OP_Not && expr1->type == expr2->unary.arg->type) {
      if (IsValued(expr1)) {
        if (IsUnique(expr1))
          return expr1->leaf.value == expr2->unary.arg->leaf.value;
        return false;
      }
      return true;      
    }
    if (expr1->type == Smiley::OP_Not && expr2->type == expr1->unary.arg->type) {
      if (IsValued(expr2)) {
        if (IsUnique(expr2))
          return expr2->leaf.value == expr1->unary.arg->leaf.value;
        return false;
      }
      return true;      
    }

    switch (expr1->type) {
      case Smiley::AE_True:
        return expr2->type == Smiley::AE_False;
      case Smiley::AE_False:
        return expr2->type == Smiley::AE_False;
      case Smiley::AE_Aromatic:
        return expr2->type == Smiley::AE_Aliphatic;
      case Smiley::AE_Aliphatic:
        return expr2->type == Smiley::AE_Aromatic;
      case Smiley::AE_Cyclic:
        return expr2->type == Smiley::AE_Acyclic;
      case Smiley::AE_Acyclic:
        return expr2->type == Smiley::AE_Cyclic;
      default:
        return false;
    }
  }

  template<typename Expr>
  void PrintConflictError(Expr *root, Expr *expr1, Expr *expr2)
  {
    std::cerr << "SMARTS Error: Conflict in expression '" << GetExprString(root) << "', '" 
      << GetExprString(expr1) << "' and '" << GetExprString(expr2) 
      << "' can't be true at the same time." << std::endl;
  }

  bool IsConflicting(SmartsAtomExpr *root, SmartsAtomExpr *expr1, SmartsAtomExpr *expr2)
  {
    // e.g. [Cc] [Ss]
    if (AreTypes(expr1, expr2, Smiley::AE_AromaticElement, Smiley::AE_AliphaticElement))
      return true;
    // e.g. [C&a]
    if (AreTypes(expr1, expr2, Smiley::AE_AliphaticElement, Smiley::AE_Aromatic))
      return true;
    // e.g. [cA]
    if (AreTypes(expr1, expr2, Smiley::AE_AromaticElement, Smiley::AE_Aliphatic))
      return true;

    // e.g. [!**] [Aa] [R0R] [C!C]
    if (IsOpposite(expr1, expr2))
      return true;

    // e.g. [R0a]
    if (AreTypes(expr1, expr2, Smiley::AE_Acyclic, Smiley::AE_Aromatic))
      return true;
    // e.g. [R0r5]
    if (AreTypes(expr1, expr2, Smiley::AE_Acyclic, Smiley::AE_RingSize))
      return true;
    // e.g. [R0R2]
    if (AreTypes(expr1, expr2, Smiley::AE_Acyclic, Smiley::AE_RingMembership))
      return true;
    // e.g. [R0x3]
    if (AreTypes(expr1, expr2, Smiley::AE_Acyclic, Smiley::AE_RingConnectivity))
      return true;

    if (!IsSameType(expr1, expr2))
      return false;

    // e.g. [#6#7] [X2X3] [H2H3] [^2^3] [CN] but not [r5r6]
    if (IsUnique(expr1) && expr1->leaf.value != expr2->leaf.value)
      return true;

    return false;
  }

  bool IsConflicting(SmartsBondExpr *root, SmartsBondExpr *expr1, SmartsBondExpr *expr2)
  {
    // e.g. -!- =!= @!@
    if (expr2->type == Smiley::OP_Not && expr1->type == expr2->unary.arg->type)
      return true;
    if (expr1->type == Smiley::OP_Not && expr2->type == expr1->unary.arg->type)
      return true;
    // e.g. -= -# -:
    if (AreTypes(expr1, expr2, Smiley::BE_Single, Smiley::BE_Double) || AreTypes(expr1, expr2, Smiley::BE_Single, Smiley::BE_Triple) ||
        AreTypes(expr1, expr2, Smiley::BE_Single, Smiley::BE_Aromatic)) 
      return true;
    // e.g. =# =:
    if (AreTypes(expr1, expr2, Smiley::BE_Double, Smiley::BE_Triple) || AreTypes(expr1, expr2, Smiley::BE_Double, Smiley::BE_Aromatic))
      return true;
    // e.g. #:
    if (AreTypes(expr1, expr2, Smiley::BE_Triple, Smiley::BE_Aromatic))
      return true;

    return false;
  }

  template<typename Expr>
  void ErrorDetection(Expr *root, Expr *expr)
  {
    if (IsBinary(expr)) {
      if (IsAnd(expr)) {
        std::vector<Expr*> same, other;
        FindSameBinaryExpr(expr, same, other);

        for (std::size_t i = 0; i < other.size(); ++i)
          for (std::size_t j = i + 1; j < other.size(); ++j)
            if (IsConflicting(root, other[i], other[j]))
              PrintConflictError(root, other[i], other[j]);


        for (std::size_t i = 0; i < other.size(); ++i)
          ErrorDetection(root, other[i]);
      }

      ErrorDetection(root, expr->binary.lft);
      ErrorDetection(root, expr->binary.rgt);
    }

    if (IsUnary(expr))
      ErrorDetection(root, expr->unary.arg);
  }

  void SmartsOptimizer::Optimize(Smarts *pattern, int opts)
  {
    // Optimize atom expressions
    for (int i = 0; i < pattern->atoms.size(); ++i)
      pattern->atoms[i].expr = OptimizeExpr(pattern->atoms[i].expr, opts, m_scores);
    // Optimize bond expressions
    for (int i = 0; i < pattern->bonds.size(); ++i)
      pattern->bonds[i].expr = OptimizeExpr(pattern->bonds[i].expr, opts, m_scores);

    if (opts & AtomFalseProp)
      AtomFalsePropagation(pattern);
    if (opts & BondFalseProp)
      BondFalsePropagation(pattern);

    // Keep track of original indices & neighbors
    /*
       std::map<SmartsAtom*, int> specToIndex;
       std::vector<std::vector<SmartsAtom*> > nbrs(pattern->atoms.size());
       for (int i = 0; i < pattern->atoms.size(); ++i) {
       specToIndex[&pattern->atoms[i]] = i;
       for (int j = 0; j < pattern->atoms[i].nbrs.size(); ++j)
       nbrs[i].push_back(&pattern->atoms[pattern->atoms[i].nbrs[j]]);
    // Sort SmartsAtom's neighbors
    std::sort(nbrs[i].begin(), nbrs[i].end(), OptimizeSmartsAtomSort(m_scores));
    }
    */

    // Sort the pattern's SmartsAtoms
    if (opts & AtomOrder) {
      std::vector<SmartsAtom*> specs;
      for (int i = 0; i < pattern->atoms.size(); ++i)
        specs.push_back(&pattern->atoms[i]);
      m_scores->Sort(specs);
    }

    // Update neighbor indices
    /*
       for (int i = 0; i < pattern->atoms.size(); ++i) {
       SmartsAtom &spec = pattern->atoms[i];
       int index = specToIndex[&spec];
       spec.nbrs.clear();
       for (int j = 0; j < nbrs[index].size(); ++j)
       spec.nbrs.push_back(nbrs[index][j] - pattern->atom);
       }
       */

    // atom expression error detection
    for (int i = 0; i < pattern->atoms.size(); ++i)
      ErrorDetection(pattern->atoms[i].expr, pattern->atoms[i].expr);
    // bond expression error detection
    for (int i = 0; i < pattern->bonds.size(); ++i)
      ErrorDetection(pattern->bonds[i].expr, pattern->bonds[i].expr);
  }

}
