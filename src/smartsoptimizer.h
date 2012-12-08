#ifndef SC_SMARTSOPTIMIZER_H
#define SC_SMARTSOPTIMIZER_H

#include <openbabel/parsmart.h>

namespace SC {

  class SmartsScores;

  /**
   * Optimize a SMARTS query.
   *
   * A binary expression's left expression is evaluated first. For an OR
   * expression it is best to place the highest scoring expression left
   * because once we have a match, the right expression is not evaluated.
   * For an AND expression the lowest scoring expression is placed left
   * because once the left expression does not match the right expression
   * is not evaluated.
   *
   * SMARTS matching starts with the first pattern atom. The atom with the lowest
   * score is used as first atom.
   *
   * Neighbor atoms are matched in the order in which they occur in the AtomSpec's
   * nbrs vector. The neighbors will be sorted by increasing score.
   */
  class SmartsOptimizer
  {
    public:
      enum Optimizations
      {
        NoOptimization = 0,
        DoubleNegationElim = 1,
        TrueElim = 2,
        FalseElim = 4,
        DuplicateElim = 8,
        NegationElim = 16,
        BinaryExpr1 = 32,
        BinaryExpr2 = 64,
        AtomFalseProp = 128,
        BondFalseProp = 256,
        ExprFactor = 512,
        AtomOrder = 1024,
        BondOrder = 2048,
        O0 = NoOptimization,
        O1 = DoubleNegationElim | TrueElim | FalseElim | DuplicateElim | NegationElim,
        O2 = O1 | BinaryExpr1 | BinaryExpr2 | AtomFalseProp | BondFalseProp,
        O3 = O2 | ExprFactor,
        O4 = O3 | AtomOrder,
        O5 = O4 | BondOrder
      };

      SmartsOptimizer(SmartsScores *scores) : m_scores(scores)
      {
      }

      void Optimize(OpenBabel::Pattern *pattern, int opts = O5);

    private:
      SmartsScores *m_scores;
  };

}

#endif
