#ifndef SC_SMARTSPRINT_H
#define SC_SMARTSPRINT_H

#include <openbabel/parsmart.h>

namespace SC {

  class SmartsScores;

  void PrintPattern(OpenBabel::Pattern *pattern, SmartsScores *scores);
  void PrintEnvironmentScores(OpenBabel::Pattern *pattern, SmartsScores *scores);
  void PrintPatternTree(OpenBabel::Pattern *pattern, SmartsScores *scores);

  std::string GetExprString(const OpenBabel::AtomExpr *expr, bool implicitAnd = true);
  std::string GetExprString(const OpenBabel::BondExpr *expr, bool implicitAnd = true);

  void PrintAtomExprTree(OpenBabel::AtomExpr *expr, int indent, SmartsScores *scores);
  void PrintBondExprTree(OpenBabel::BondExpr *expr, int indent, SmartsScores *scores);

  void PrintAtomSpecTree(OpenBabel::AtomSpec &spec, SmartsScores *scores);
  void PrintBondSpecTree(OpenBabel::BondSpec &spec, SmartsScores *scores);

}

#endif
