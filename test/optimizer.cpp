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

  Smarts *pattern = parse(expr);

  REQUIRE(pattern->atoms.size() == 1);

  SmartsOptimizer optimizer(&scores);
  optimizer.Optimize(pattern, opt);

  std::string result = GetExprString(pattern->atoms[0].expr);
  COMPARE(result, correct);

  delete pattern;

  return result == correct;
}

int main()
{
  PrettySmartsScores scores;

  ASSERT(TestAtomExprOptimization("[!!C]", "C", SmartsOptimizer::DoubleNegationElim, scores));
  ASSERT(TestAtomExprOptimization("[!!!C]", "!C", SmartsOptimizer::DoubleNegationElim, scores));
  ASSERT(TestAtomExprOptimization("[!!!!C]", "C", SmartsOptimizer::DoubleNegationElim, scores));
  ASSERT(TestAtomExprOptimization("[!!!!!C]", "!C", SmartsOptimizer::DoubleNegationElim, scores));

  ASSERT(TestAtomExprOptimization("[C*]", "C", SmartsOptimizer::TrueElim, scores));
  ASSERT(TestAtomExprOptimization("[*C]", "C", SmartsOptimizer::TrueElim, scores));
  ASSERT(TestAtomExprOptimization("[C,*]", "*", SmartsOptimizer::TrueElim, scores));
  ASSERT(TestAtomExprOptimization("[*,C]", "*", SmartsOptimizer::TrueElim, scores));

  ASSERT(TestAtomExprOptimization("[C!*]", "!*", SmartsOptimizer::NegationElim | SmartsOptimizer::FalseElim, scores));
  ASSERT(TestAtomExprOptimization("[!*C]", "!*", SmartsOptimizer::NegationElim | SmartsOptimizer::FalseElim, scores));
  ASSERT(TestAtomExprOptimization("[!*,C]", "C", SmartsOptimizer::NegationElim | SmartsOptimizer::FalseElim, scores));

  ASSERT(TestAtomExprOptimization("[CC]", "C", SmartsOptimizer::DuplicateElim, scores));
  ASSERT(TestAtomExprOptimization("[CCCCCC]", "C", SmartsOptimizer::DuplicateElim, scores));
  ASSERT(TestAtomExprOptimization("[C,C]", "C", SmartsOptimizer::DuplicateElim, scores));
  ASSERT(TestAtomExprOptimization("[C,C,C,C,C,C]", "C", SmartsOptimizer::DuplicateElim, scores));

  ASSERT(TestAtomExprOptimization("[!*]", "!*", SmartsOptimizer::NegationElim, scores));
  ASSERT(TestAtomExprOptimization("[!!*]", "*", SmartsOptimizer::NegationElim, scores));
  ASSERT(TestAtomExprOptimization("[!A]", "a", SmartsOptimizer::NegationElim, scores));
  ASSERT(TestAtomExprOptimization("[!a]", "A", SmartsOptimizer::NegationElim, scores));
  ASSERT(TestAtomExprOptimization("[!R]", "R0", SmartsOptimizer::NegationElim, scores));
  ASSERT(TestAtomExprOptimization("[!R0]", "R", SmartsOptimizer::NegationElim, scores));

  // complex duplicate elimination
  ASSERT(TestAtomExprOptimization("[CRHCRHCRHCRHCRH]", "CRH", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[C,R,H,C,R,H,C,R,H,C,R,H,C,R,H]", "C,R,H", SmartsOptimizer::BinaryExpr1, scores));
  // aliphatic/aromatic contraction
  ASSERT(TestAtomExprOptimization("[#6A]", "C", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[A#6]", "C", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[#6a]", "c", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[a#6]", "c", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[CA]", "C", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[AC]", "C", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[ca]", "c", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[ac]", "c", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[C,A]", "A", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[A,C]", "A", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[c,a]", "a", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[a,c]", "a", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[A,a]", "*", SmartsOptimizer::BinaryExpr1, scores));
  // redundancy elimination
  ASSERT(TestAtomExprOptimization("[C!N!S!P]", "C", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[X2!X3]", "X2", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[D2!D3]", "D2", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[2!3]", "2", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[C!#8]", "C", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[C!o]", "C", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[c!#8]", "c", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[c!O]", "c", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[aR]", "a", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[R2R]", "R2", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[Rr6]", "r6", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[Rx3]", "x3", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[R,R2]", "R", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[R,r6]", "R", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[R,x2]", "R", SmartsOptimizer::BinaryExpr1, scores));

  ASSERT(TestAtomExprOptimization("[H#6&13]", "13#6H", SmartsOptimizer::BinaryExpr1, scores));
  ASSERT(TestAtomExprOptimization("[C,S,B,Br,F,I,Cl]", "B,C,F,S,Cl,Br,I", SmartsOptimizer::BinaryExpr1, scores));

  ASSERT(TestAtomExprOptimization("[O,C]", "C,O", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[X3,X2]", "X2,X3", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[2,1]", "1,2", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[H3,H2]", "H2,H3", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[h2,h1]", "h,h2", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[R3,R2]", "R2,R3", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[R2,R]", "R,R2", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[-3,-2]", "-2,-3", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[-2,-1]", "-,-2", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[+1,-2]", "-2,+", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[#7,#2]", "He,#7", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[C13]", "13C", SmartsOptimizer::BinaryExpr2, scores));
  ASSERT(TestAtomExprOptimization("[#6,#7,#8;a]", "a;#6,#7,#8", SmartsOptimizer::BinaryExpr2, scores));

  ASSERT(TestAtomExprOptimization("", "", SmartsOptimizer::ExprFactor, scores));
}
