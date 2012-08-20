#include "../src/smartsscores.h"
#include "../src/smartsmatcher.h"
#include "../src/smartsprint.h"

#include "test.h"

using namespace SC;

bool TestAtomScoreSort(const std::string &expr, const std::string &correct, SmartsScores &scores, bool increasing)
{
  if (expr.empty())
    return true;

  std::cout << "Test: " << expr << " -> " << correct << "" << std::endl;

  OpenBabelSmartsMatcher matcher;
  matcher.Init(expr);
  OpenBabel::Pattern *pattern = matcher.GetPattern();

  std::vector<OpenBabel::AtomExpr*> atoms;
  for (int i = 0; i < pattern->acount; ++i)
    atoms.push_back(pattern->atom[i].expr);

  scores.Sort(atoms, increasing);

  std::string result;
  for (int i = 0; i < pattern->acount; ++i)
    result += GetExprString(atoms[i]);

  SC_COMPARE(result, correct);

  return result == correct;
}

int main()
{
  PrettySmartsScores scores;

  SC_ASSERT(TestAtomScoreSort("CO", "CO", scores, true));
  SC_ASSERT(TestAtomScoreSort("OC", "CO", scores, true));
}
