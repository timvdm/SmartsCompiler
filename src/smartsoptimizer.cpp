#include "smartsoptimizer.h"
#include "smartsscores.h"
#include "smartsprint.h"
#include "pattern.h"
#include "util.h"

namespace SC {

  using namespace OpenBabel;

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
      if (IsNot(expr->mon.arg)) {
        Expr *tmp = expr->mon.arg->mon.arg;
        delete expr->mon.arg;
        delete expr;
        expr = DoubleNegation(tmp);
        return expr;
      }
    } else if (!IsLeaf(expr)) {
      expr->bin.lft = DoubleNegation(expr->bin.lft);
      expr->bin.rgt = DoubleNegation(expr->bin.rgt);      
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
    if (SameType<Expr, BondExpr>::result)
      return expr;

    if (expr->type == AE_ANDHI || expr->type == AE_ANDLO) {
      if (expr->bin.lft->type == AE_TRUE) {
        Expr *tmp = expr->bin.rgt;
        DeleteExpr(expr->bin.lft);
        delete expr;
        tmp = TrueElimination(tmp);
        return tmp;
      }
      if (expr->bin.rgt->type == AE_TRUE) {
        Expr *tmp = expr->bin.lft;
        DeleteExpr(expr->bin.rgt);
        delete expr;
        tmp = TrueElimination(tmp);
        return tmp;
      }
      expr->bin.lft = TrueElimination(expr->bin.lft);
      expr->bin.rgt = TrueElimination(expr->bin.rgt);
    }
    if (expr->type == AE_OR) {
      if (expr->bin.lft->type == AE_TRUE) {
        Expr *tmp = expr->bin.lft;
        DeleteExpr(expr->bin.rgt);
        delete expr;
        tmp = TrueElimination(tmp);
        return tmp;
      }
      if (expr->bin.rgt->type == AE_TRUE) {
        Expr *tmp = expr->bin.rgt;
        DeleteExpr(expr->bin.lft);
        delete expr;
        tmp = TrueElimination(tmp);
        return tmp;
      }
      expr->bin.lft = TrueElimination(expr->bin.lft);
      expr->bin.rgt = TrueElimination(expr->bin.rgt);
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
    if (SameType<Expr, BondExpr>::result)
      return expr;

    if (IsAnd(expr)) {
      if (expr->bin.lft->type == AE_FALSE) {
        Expr *tmp = expr->bin.lft;
        DeleteExpr(expr->bin.rgt);
        delete expr;
        tmp = FalseElimination(tmp);
        return tmp;
      }
      if (expr->bin.rgt->type == AE_FALSE) {
        Expr *tmp = expr->bin.rgt;
        DeleteExpr(expr->bin.lft);
        delete expr;
        tmp = FalseElimination(tmp);
        return tmp;
      }
      expr->bin.lft = FalseElimination(expr->bin.lft);
      expr->bin.rgt = FalseElimination(expr->bin.rgt);
    }
    if (IsOr(expr)) {
      if (expr->bin.lft->type == AE_FALSE) {
        Expr *tmp = expr->bin.rgt;
        DeleteExpr(expr->bin.lft);
        delete expr;
        tmp = FalseElimination(tmp);
        return tmp;
      }
      if (expr->bin.rgt->type == AE_FALSE) {
        Expr *tmp = expr->bin.lft;
        DeleteExpr(expr->bin.rgt);
        delete expr;
        tmp = FalseElimination(tmp);
        return tmp;
      }
      expr->bin.lft = FalseElimination(expr->bin.lft);
      expr->bin.rgt = FalseElimination(expr->bin.rgt);
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
      if (IsDuplicate(expr->bin.lft, expr->bin.rgt)) {
        Expr *tmp = expr->bin.lft;
        DeleteExpr(expr->bin.rgt);
        delete expr;
        tmp = DuplicateElimination(tmp);
        return tmp;
      }

      expr->bin.lft = DuplicateElimination(expr->bin.lft);
      expr->bin.rgt = DuplicateElimination(expr->bin.rgt);
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
    if (SameType<Expr, BondExpr>::result)
      return expr;

    if (IsBinary(expr)) {
      expr->bin.lft = Negation(expr->bin.lft);
      expr->bin.rgt = Negation(expr->bin.rgt);
    }
    if (IsNot(expr)) {
      if (IsLeaf(expr->mon.arg) && !IsValued(expr->mon.arg)) {
        Expr *tmp = expr->mon.arg;
        delete expr;
        switch (tmp->type) {
          case AE_TRUE:
            tmp->type = AE_FALSE;
            return tmp;
          case AE_FALSE:
            tmp->type = AE_TRUE;
            return tmp;
          case AE_AROMATIC:
            tmp->type = AE_ALIPHATIC;
            return tmp;
          case AE_ALIPHATIC:
            tmp->type = AE_AROMATIC;
            return tmp;
          case AE_CYCLIC:
            tmp->type = AE_ACYCLIC;
            return tmp;
          case AE_ACYCLIC:
            tmp->type = AE_CYCLIC;
            return tmp;        
        }
      }
      expr->mon.arg = Negation(expr->mon.arg);
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
    if (expr1->type == AE_ELEM && expr2->type == AE_ALIPHATIC) {
      expr1->type = AE_ALIPHELEM;
      return true;
    }
    if (expr1->type == AE_ELEM && expr2->type == AE_AROMATIC) {
      expr1->type = AE_AROMELEM;
      return true;
    }
    if (expr1->type == AE_AROMELEM && expr2->type == AE_AROMATIC)
      return true;
    if (expr1->type == AE_ALIPHELEM && expr2->type == AE_ALIPHATIC)
      return true;
    return false;
  }

  template<typename Expr>
  void AromAliphAndContraction(std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    if (SameType<Expr, BondExpr>::result)
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
    if (expr1->type == AE_ALIPHATIC && expr2->type == AE_ALIPHELEM)
      return true;
    if (expr1->type == AE_AROMATIC && expr2->type == AE_AROMELEM)
      return true;
    if (expr1->type == AE_ALIPHATIC && expr2->type == AE_AROMATIC) {
      expr1->type = AE_TRUE;
      return true;
    }
    return false;
  }

  template<typename Expr>
  void AromAliphOrContraction(std::vector<Expr*> &same, std::vector<Expr*> &other)
  {
    if (SameType<Expr, BondExpr>::result)
      return;

    std::vector<std::size_t> remove;
    for (std::size_t i = 0; i < other.size(); ++i)
      for (std::size_t j = 0; j < other.size(); ++j)
        if (IsAromAliphOrExpansion(other[i], other[j]))
          remove.push_back(j);

    RemoveExpressions(remove, same, other);
  }

  bool IsAndRedundant(AtomExpr *expr1, AtomExpr *expr2)
  {
    if (expr1 == expr2)
      return false;
    // e.g. [C!N] [X2!X3] [R3!R2] ...
    if (IsUnique(expr1) && expr2->type == AE_NOT && expr1->type == expr2->mon.arg->type)
      return expr1->leaf.value != expr2->mon.arg->leaf.value;
    // e.g. [F!#6] = F
    if (expr1->type == AE_ALIPHELEM && expr2->type == AE_NOT && expr2->mon.arg->type == AE_ELEM)
      return true;
    // e.g. [F!n] = F
    if (expr1->type == AE_ALIPHELEM && expr2->type == AE_NOT && expr2->mon.arg->type == AE_AROMELEM)
      return true;
    // e.g. [n!#8] = n
    if (expr1->type == AE_AROMELEM && expr2->type == AE_NOT && expr2->mon.arg->type == AE_ELEM)
      return true;
    // e.g. [o!C] = o
    if (expr1->type == AE_AROMELEM && expr2->type == AE_NOT && expr2->mon.arg->type == AE_ALIPHELEM)
      return true;
    // e.g. [#6!N] = [#6]
    if (expr1->type == AE_ELEM && expr2->type == AE_NOT && expr2->mon.arg->type == AE_ALIPHELEM)
      return true;
    // e.g. [#7!o] = [#7]
    if (expr1->type == AE_ELEM && expr2->type == AE_NOT && expr2->mon.arg->type == AE_AROMELEM)
      return true;
    // e.g. [aR] = [a]
    if (expr1->type == AE_AROMATIC && expr2->type == AE_CYCLIC)
      return true;
    // e.g. [RR2] = [R2]
    if (expr1->type == AE_RINGS && expr2->type == AE_CYCLIC)
      return true;
    // e.g. [Rr6] = [r6]
    if (expr1->type == AE_SIZE && expr2->type == AE_CYCLIC)
      return true;
    // e.g. [Rx3] = [x3]
    if (expr1->type == AE_RINGCONNECT && expr2->type == AE_CYCLIC)
      return true;
    return false;
  }

  bool IsAndRedundant(BondExpr *expr1, BondExpr *expr2)
  {
    if (expr1 == expr2)
      return false;
    // e.g. C~=C = C=C
    //      C~@C = C@C
    if ((IsBondOrderExpr(expr1) || expr1->type == BE_RING) && expr2->type == BE_ANY)
      return true;
    // e.g. C-!=C C-!#C C-!:C
    if (IsBondOrderExpr(expr1) && expr2->type == BE_NOT && IsBondOrderExpr(expr2->mon.arg) && !IsSameType(expr1, expr2->mon.arg))
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

  bool IsOrRedundant(AtomExpr *expr1, AtomExpr *expr2)
  {
    if (expr1 == expr2)
      return false;
    // e.g. [R,R2] [R,r6] [R,x2]
    if (expr1->type == AE_CYCLIC && (expr2->type == AE_RINGS || expr2->type == AE_SIZE || expr2->type == AE_RINGCONNECT))
      return true;
    return false;
  }

  bool IsOrRedundant(BondExpr *expr1, BondExpr *expr2)
  {
    if (expr1 == expr2)
      return false;
    // e.g. C~,=C = C~C
    //      C~,@C = C~C
    if (expr1->type == BE_ANY && (IsBondOrderExpr(expr2) || expr2->type == BE_RING))
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
        expr->bin.lft = other[0];
        expr->bin.rgt = other[1];
        expr->bin.lft = OptimizeBinaryExpr1(expr->bin.lft, scores);
        expr->bin.rgt = OptimizeBinaryExpr1(expr->bin.rgt, scores);
        return expr;
      }
      //assert(same.size() == other.size() || same.size() + 1 == other.size());

      // make a chain from all same expressions to the right
      for (std::size_t i = 1; i < same.size(); ++i)
        same[i - 1]->bin.rgt = same[i];
      // assign sorted other expressions to left children
      for (std::size_t i = 0; i < same.size(); ++i) {
        same[i]->bin.lft = other[i];
        if (i + 1 == same.size() && same.size() + 1 == other.size())
          same[i]->bin.rgt = other[i + 1];        
      }
      // optimize other
      for (std::size_t i = 0; i < other.size(); ++i)
        other[i] = OptimizeBinaryExpr1(other[i], scores);
    }

    if (IsUnary(expr))
      expr->mon.arg = OptimizeBinaryExpr1(expr->mon.arg, scores);

    return expr;
  }

  /**
   * Swap binary node children based on score.
   */
  template<typename Expr>
  void OptimizeBinaryExpr2(Expr *expr, SmartsScores *scores)
  {
    if (IsBinary(expr)) {
      double lftScore = scores->GetExprScore(expr->bin.lft);
      double rgtScore = scores->GetExprScore(expr->bin.rgt);

      std::vector<Expr*> children(2);
      children[0] = expr->bin.lft;
      children[1] = expr->bin.rgt;
      scores->Sort(children, IsAnd(expr));
      expr->bin.lft = children[0];
      expr->bin.rgt = children[1];

      if (!IsLeaf(expr->bin.lft))
        OptimizeBinaryExpr2(expr->bin.lft, scores);
      if (!IsLeaf(expr->bin.rgt))
        OptimizeBinaryExpr2(expr->bin.rgt, scores);
    }

    if (IsUnary(expr))
      OptimizeBinaryExpr2(expr->mon.arg, scores);
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
          append->bin.rgt = binary;
        else
          root = append = binary;
        // assign the left 
        binary->bin.lft = children[i];
        append = binary;
      } else {
        // assign last children expression to right
        append->bin.rgt = children[i];            
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
        type = SameType<Expr, AtomExpr>::result ? AE_ANDLO : BE_ANDLO;
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
        binary->bin.lft = factorized_tree;
        binary->bin.rgt = not_intersection_tree;
        expr->bin.lft = binary;
        expr->bin.rgt = not_opposite_tree;
      } else if (not_intersection_tree) {
        expr->type = GetOppositeBinaryExprType(expr);
        expr->bin.lft = factorized_tree;
        expr->bin.rgt = not_intersection_tree;
      } else if (not_opposite_tree) {
        expr->type = GetOppositeBinaryExprType(expr);
        expr->bin.lft = factorized_tree;
        expr->bin.rgt = not_opposite_tree;
      } else {
        expr->bin.lft = factorized_tree;
        // hack... :)
        Expr *tmp = duplicates.back();
        duplicates.pop_back();
        if (IsNot(tmp))
          delete tmp->mon.arg;
        tmp->type = SameType<Expr, AtomExpr>::result ? AE_TRUE : BE_ANY;
        expr->bin.rgt = tmp;
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
      ExprFactorization(expr->mon.arg);
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

  void FalsePropagation(Pattern *pattern)
  {
    // delete the expressions
    for (int i = 0; i < pattern->acount; ++i)
      DeleteExpr(pattern->atom[i].expr);
    for (int i = 0; i < pattern->bcount; ++i)
      DeleteExpr(pattern->bond[i].expr);
    // reset counts
    pattern->acount = 1;
    pattern->bcount = 0;
    // delete specs
    delete [] pattern->atom;
    delete [] pattern->bond;
    // create single atom false spec
    pattern->atom = new AtomSpec[1];
    pattern->atom[0].part = 0;
    pattern->atom[0].chiral_flag = 0;
    pattern->atom[0].vb = 0;
    pattern->atom[0].expr = new AtomExpr;
    pattern->atom[0].expr->type = AE_FALSE;
    pattern->bond = 0;
  }

  void AtomFalsePropagation(Pattern *pattern)
  {
    for (int i = 0; i < pattern->acount; ++i)
      if (pattern->atom[i].expr->type == AE_FALSE) {
        FalsePropagation(pattern);
        return;
      }
  }

  void BondFalsePropagation(Pattern *pattern)
  {
    for (int i = 0; i < pattern->bcount; ++i)
      if (pattern->bond[i].expr->type == BE_NOT)
        if (pattern->bond[i].expr->mon.arg->type == BE_ANY) {
          FalsePropagation(pattern);
          return;
        }
  }

  bool IsOpposite(AtomExpr *expr1, AtomExpr *expr2)
  {
    if (expr2->type == AE_NOT && expr1->type == expr2->mon.arg->type) {
      if (IsValued(expr1)) {
        if (IsUnique(expr1))
          return expr1->leaf.value == expr2->mon.arg->leaf.value;
        return false;
      }
      return true;      
    }
    if (expr1->type == AE_NOT && expr2->type == expr1->mon.arg->type) {
      if (IsValued(expr2)) {
        if (IsUnique(expr2))
          return expr2->leaf.value == expr1->mon.arg->leaf.value;
        return false;
      }
      return true;      
    }

    switch (expr1->type) {
      case AE_TRUE:
        return expr2->type == AE_FALSE;
      case AE_FALSE:
        return expr2->type == AE_FALSE;
      case AE_AROMATIC:
        return expr2->type == AE_ALIPHATIC;
      case AE_ALIPHATIC:
        return expr2->type == AE_AROMATIC;
      case AE_CYCLIC:
        return expr2->type == AE_ACYCLIC;
      case AE_ACYCLIC:
        return expr2->type == AE_CYCLIC;
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

  bool IsConflicting(AtomExpr *root, AtomExpr *expr1, AtomExpr *expr2)
  {
    // e.g. [Cc] [Ss]
    if (AreTypes(expr1, expr2, AE_AROMELEM, AE_ALIPHELEM))
      return true;
    // e.g. [C&a]
    if (AreTypes(expr1, expr2, AE_ALIPHELEM, AE_AROMATIC))
      return true;
    // e.g. [cA]
    if (AreTypes(expr1, expr2, AE_AROMELEM, AE_ALIPHATIC))
      return true;

    // e.g. [!**] [Aa] [R0R] [C!C]
    if (IsOpposite(expr1, expr2))
      return true;

    // e.g. [R0a]
    if (AreTypes(expr1, expr2, AE_ACYCLIC, AE_AROMATIC))
      return true;
    // e.g. [R0r5]
    if (AreTypes(expr1, expr2, AE_ACYCLIC, AE_SIZE))
      return true;
    // e.g. [R0R2]
    if (AreTypes(expr1, expr2, AE_ACYCLIC, AE_RINGS))
      return true;
    // e.g. [R0x3]
    if (AreTypes(expr1, expr2, AE_ACYCLIC, AE_RINGCONNECT))
      return true;

    if (!IsSameType(expr1, expr2))
      return false;

    // e.g. [#6#7] [X2X3] [H2H3] [^2^3] [CN] but not [r5r6]
    if (IsUnique(expr1) && expr1->leaf.value != expr2->leaf.value)
      return true;

    return false;
  }

  bool IsConflicting(BondExpr *root, BondExpr *expr1, BondExpr *expr2)
  {
    // e.g. -!- =!= @!@
    if (expr2->type == BE_NOT && expr1->type == expr2->mon.arg->type)
      return true;
    if (expr1->type == BE_NOT && expr2->type == expr1->mon.arg->type)
      return true;
    // e.g. -= -# -:
    if (AreTypes(expr1, expr2, BE_SINGLE, BE_DOUBLE) || AreTypes(expr1, expr2, BE_SINGLE, BE_TRIPLE) ||
        AreTypes(expr1, expr2, BE_SINGLE, BE_AROM)) 
      return true;
    // e.g. =# =:
    if (AreTypes(expr1, expr2, BE_DOUBLE, BE_TRIPLE) || AreTypes(expr1, expr2, BE_DOUBLE, BE_AROM))
      return true;
    // e.g. #:
    if (AreTypes(expr1, expr2, BE_TRIPLE, BE_AROM))
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

      ErrorDetection(root, expr->bin.lft);
      ErrorDetection(root, expr->bin.rgt);
    }

    if (IsUnary(expr))
      ErrorDetection(root, expr->mon.arg);
  }

  void SmartsOptimizer::Optimize(Pattern *pattern, int opts)
  {
    // Optimize atom expressions
    for (int i = 0; i < pattern->acount; ++i)
      pattern->atom[i].expr = OptimizeExpr(pattern->atom[i].expr, opts, m_scores);
    // Optimize bond expressions
    for (int i = 0; i < pattern->bcount; ++i)
      pattern->bond[i].expr = OptimizeExpr(pattern->bond[i].expr, opts, m_scores);

    if (opts & AtomFalseProp)
      AtomFalsePropagation(pattern);
    if (opts & BondFalseProp)
      BondFalsePropagation(pattern);

    // Keep track of original indices & neighbors
    /*
       std::map<AtomSpec*, int> specToIndex;
       std::vector<std::vector<AtomSpec*> > nbrs(pattern->acount);
       for (int i = 0; i < pattern->acount; ++i) {
       specToIndex[&pattern->atom[i]] = i;
       for (int j = 0; j < pattern->atom[i].nbrs.size(); ++j)
       nbrs[i].push_back(&pattern->atom[pattern->atom[i].nbrs[j]]);
    // Sort AtomSpec's neighbors
    std::sort(nbrs[i].begin(), nbrs[i].end(), OptimizeAtomSpecSort(m_scores));
    }
    */

    // Sort the pattern's AtomSpecs
    if (opts & AtomOrder) {
      std::vector<AtomSpec*> specs;
      for (int i = 0; i < pattern->acount; ++i)
        specs.push_back(&pattern->atom[i]);
      m_scores->Sort(specs);
    }

    // Update neighbor indices
    /*
       for (int i = 0; i < pattern->acount; ++i) {
       AtomSpec &spec = pattern->atom[i];
       int index = specToIndex[&spec];
       spec.nbrs.clear();
       for (int j = 0; j < nbrs[index].size(); ++j)
       spec.nbrs.push_back(nbrs[index][j] - pattern->atom);
       }
       */

    // atom expression error detection
    for (int i = 0; i < pattern->acount; ++i)
      ErrorDetection(pattern->atom[i].expr, pattern->atom[i].expr);
    // bond expression error detection
    for (int i = 0; i < pattern->bcount; ++i)
      ErrorDetection(pattern->bond[i].expr, pattern->bond[i].expr);
  }

}
