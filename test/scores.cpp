#include "../src/smartsscores.h"
#include "../src/smarts.h"
#include "../src/smartsprint.h"

#include "test.h"

using namespace SC;

bool TestAtomScoreSort(const std::string &expr, const std::string &correct, SmartsScores &scores, bool increasing)
{
  if (expr.empty())
    return true;

  std::cout << "Test: " << expr << " -> " << correct << "" << std::endl;

  Smarts *pattern = parse(expr);

  std::vector<SmartsAtomExpr*> atoms;
  for (int i = 0; i < pattern->atoms.size(); ++i)
    atoms.push_back(pattern->atoms[i].expr);

  scores.Sort(atoms, increasing);

  std::string result;
  for (int i = 0; i < pattern->atoms.size(); ++i)
    result += GetExprString(atoms[i]);

  COMPARE(result, correct);

  delete pattern;

  return result == correct;
}

int main()
{
  PrettySmartsScores scores;

  ASSERT(TestAtomScoreSort("CO", "CO", scores, true));
  ASSERT(TestAtomScoreSort("OC", "CO", scores, true));
}
