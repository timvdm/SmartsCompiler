#include "../src/smartsoptimizer.h"
#include "../src/smartsscores.h"
#include "../src/smartsmatcher.h"
#include "../src/smartsprint.h"

#include "test.h"

#include "../src/smartsmatcher.h"

using namespace SC;
using namespace OpenBabel;

class MockAtom
{
    void SetProperties(AtomExpr *expr, bool negated = false)
    {
      int sign = negated ? -1 : 1;
      switch (expr->type) {
        case AE_NOT:
          SetProperties(expr->mon.arg, true);
          break;
        case AE_AROMATIC:
          m_aromatic = sign;
          break;
        case AE_ALIPHATIC:
          m_aliphatic = sign;
          break;
        case AE_CYCLIC:
          m_cyclic = sign;
          break;
        case AE_ACYCLIC:
          m_acyclic = sign;
          break;
        case AE_DEGREE:
          m_degree.push_back(sign * expr->leaf.value);
          break;
        case AE_HCOUNT:
          m_totalH.push_back(sign * expr->leaf.value);
          break;
        case AE_IMPLICIT:
          m_implicitH.push_back(sign * expr->leaf.value);
          break;
        case AE_RINGS:
          m_numrings.push_back(sign * expr->leaf.value);
          break;
        case AE_SIZE:
          m_ringsize.push_back(sign * expr->leaf.value);
          break;
        case AE_VALENCE:
          m_valence.push_back(sign * expr->leaf.value);
          break;
        case AE_CONNECT:
          m_connect.push_back(sign * expr->leaf.value);
          break;
        case AE_RINGCONNECT:
          m_ringconnect.push_back(sign * expr->leaf.value);
          break;
        case AE_CHARGE:
          if (expr->leaf.value < 0)
            m_negcharge.push_back(sign * -expr->leaf.value);
          else
            m_poscharge.push_back(sign * expr->leaf.value);
          break;
        case AE_ELEM:
          m_elem.push_back(sign * expr->leaf.value);
          break;
        case AE_ALIPHELEM:
          m_aliphelem.push_back(sign * expr->leaf.value);
          break;
        case AE_AROMELEM:
          m_aromelem.push_back(sign * expr->leaf.value);
          break;
        case AE_MASS:
          m_isotope.push_back(sign * expr->leaf.value);
          break;
        default:
          break;
      }
    }
  public:
    MockAtom(const std::string &smarts)
    {
      OpenBabelSmartsMatcher matcher;
      sp.Init(smarts);
      Pattern *pat = matcher.GetPattern();
      for (int i = 0; i < pat->acount; ++i)
        SetProperties(pat->atom[i]->expr);
    }

  private:
    int m_aromatic;
    bool m_aliphatic;
    bool m_cyclic;
    bool m_acyclic;
    std::vector<int> m_degree;
    std::vector<int> m_totalH;
    std::vector<int> m_implicitH;
    std::vector<int> m_numrings;
    std::vector<int> m_ringsize;
    std::vector<int> m_valence;
    std::vector<int> m_connect;
    std::vector<int> m_ringconnect;
    std::vector<int> m_poscharge;
    std::vector<int> m_negcharge;
    std::vector<int> m_elem;
    std::vector<int> m_aromelem;
    std::vector<int> m_aliphelem;
    std::vector<int> m_isotope;
};

bool TestAtomExprOptimization(const std::string &expr, const std::string &correct, int opt, SmartsScores &scores)
{
  if (expr.empty())
    return true;

  std::cout << "Test: " << expr << " -> [" << correct << "]" << std::endl;

  OpenBabelSmartsMatcher matcher;
  matcher.Init(expr);
  OpenBabel::Pattern *pattern = matcher.GetPattern();

  SC_REQUIRE(pattern->acount == 1);

  SmartsOptimizer optimizer(&scores);
  optimizer.Optimize(pattern, opt);

  std::string result = GetExprString(pattern->atom[0].expr);
  SC_COMPARE(result, correct);

  return result == correct;
}

int main()
{
  PrettySmartsScores scores;

  SC_ASSERT(TestAtomExprOptimization("[!!C]", "C", SmartsOptimizer::DoubleNegationElim, scores));
}
