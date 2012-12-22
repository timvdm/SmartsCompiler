#ifndef SC_SMARTSPRINT_H
#define SC_SMARTSPRINT_H

#include "smarts.h"

namespace SC {

  // forward declaration
  class SmartsScores;

  void PrintSmarts(Smarts *smarts, SmartsScores *scores);
  void PrintEnvironmentScores(Smarts *smarts, SmartsScores *scores);
  void PrintSmartsTree(Smarts *smarts, SmartsScores *scores);

  std::string GetExprString(const SmartsAtomExpr *expr, bool implicitAnd = true);
  std::string GetExprString(const SmartsBondExpr *expr, bool implicitAnd = true);
 
  template<typename Expr> 
  std::string GetBinaryExprString(const std::vector<Expr*> &expr, const std::string &op, bool implicitAnd = true);


  void PrintAtomExprTree(SmartsAtomExpr *expr, int indent, SmartsScores *scores);
  void PrintBondExprTree(SmartsBondExpr *expr, int indent, SmartsScores *scores);

  void PrintAtomSpecTree(SmartsAtom &atom, SmartsScores *scores);
  void PrintBondSpecTree(SmartsBond &bond, SmartsScores *scores);
  
  std::string SuppressImplicitAndHi(const std::string &expr);

}

#endif
