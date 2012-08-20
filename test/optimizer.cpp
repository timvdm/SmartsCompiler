#include "../src/smartsoptimizer.h"
#include "../src/smartsscores.h"
#include "../src/smartsmatcher.h"
#include "../src/smartsprint.h"

#include "test.h"

using namespace SC;

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
  SC_ASSERT(TestAtomExprOptimization("[!!!C]", "!C", SmartsOptimizer::DoubleNegationElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[!!!!C]", "C", SmartsOptimizer::DoubleNegationElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[!!!!!C]", "!C", SmartsOptimizer::DoubleNegationElim, scores));

  SC_ASSERT(TestAtomExprOptimization("[C*]", "C", SmartsOptimizer::TrueElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[*C]", "C", SmartsOptimizer::TrueElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[C,*]", "*", SmartsOptimizer::TrueElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[*,C]", "*", SmartsOptimizer::TrueElim, scores));

  SC_ASSERT(TestAtomExprOptimization("[C!*]", "!*", SmartsOptimizer::NegationElim | SmartsOptimizer::FalseElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[!*C]", "!*", SmartsOptimizer::NegationElim | SmartsOptimizer::FalseElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[!*,C]", "C", SmartsOptimizer::NegationElim | SmartsOptimizer::FalseElim, scores));

  SC_ASSERT(TestAtomExprOptimization("[CC]", "C", SmartsOptimizer::DuplicateElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[CCCCCC]", "C", SmartsOptimizer::DuplicateElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[C,C]", "C", SmartsOptimizer::DuplicateElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[C,C,C,C,C,C]", "C", SmartsOptimizer::DuplicateElim, scores));

  SC_ASSERT(TestAtomExprOptimization("[!*]", "!*", SmartsOptimizer::NegationElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[!!*]", "*", SmartsOptimizer::NegationElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[!A]", "a", SmartsOptimizer::NegationElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[!a]", "A", SmartsOptimizer::NegationElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[!R]", "R0", SmartsOptimizer::NegationElim, scores));
  SC_ASSERT(TestAtomExprOptimization("[!R0]", "R", SmartsOptimizer::NegationElim, scores));

  // complex duplicate elimination
  SC_ASSERT(TestAtomExprOptimization("[CRHCRHCRHCRHCRH]", "CRH", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[C,R,H,C,R,H,C,R,H,C,R,H,C,R,H]", "C,R,H", SmartsOptimizer::BinaryExpr1, scores));
  // aliphatic/aromatic contraction
  SC_ASSERT(TestAtomExprOptimization("[#6A]", "C", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[A#6]", "C", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[#6a]", "c", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[a#6]", "c", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[CA]", "C", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[AC]", "C", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[ca]", "c", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[ac]", "c", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[C,A]", "A", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[A,C]", "A", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[c,a]", "a", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[a,c]", "a", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[A,a]", "*", SmartsOptimizer::BinaryExpr1, scores));
  // redundancy elimination
  SC_ASSERT(TestAtomExprOptimization("[C!N!S!P]", "C", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[X2!X3]", "X2", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[D2!D3]", "D2", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[2!3]", "2", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[C!#8]", "C", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[C!o]", "C", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[c!#8]", "c", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[c!O]", "c", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[aR]", "a", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[R2R]", "R2", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[Rr6]", "r6", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[Rx3]", "x3", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[R,R2]", "R", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[R,r6]", "R", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[R,x2]", "R", SmartsOptimizer::BinaryExpr1, scores));

  SC_ASSERT(TestAtomExprOptimization("[H#6&13]", "13#6H", SmartsOptimizer::BinaryExpr1, scores));
  SC_ASSERT(TestAtomExprOptimization("[C,S,B,Br,F,I,Cl]", "B,C,F,S,Cl,Br,I", SmartsOptimizer::BinaryExpr1, scores));

  SC_ASSERT(TestAtomExprOptimization("[O,C]", "C,O", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[X3,X2]", "X2,X3", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[2,1]", "1,2", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[H3,H2]", "H2,H3", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[h2,h1]", "h,h2", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[R3,R2]", "R2,R3", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[R2,R]", "R,R2", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[-3,-2]", "-2,-3", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[-2,-1]", "-,-2", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[+1,-2]", "-2,+", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[#7,#2]", "He,#7", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[C13]", "13C", SmartsOptimizer::BinaryExpr2, scores));
  SC_ASSERT(TestAtomExprOptimization("[#6,#7,#8;a]", "a;#6,#7,#8", SmartsOptimizer::BinaryExpr2, scores));

  SC_ASSERT(TestAtomExprOptimization("", "", SmartsOptimizer::ExprFactor, scores));
}
