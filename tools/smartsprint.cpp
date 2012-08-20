#include "../src/smartsprint.h"
#include "../src/smartsscores.h"
#include "../src/smartsoptimizer.h"
#include "../src/smartscompiler.h"
#include "../src/smartsmatcher.h"

#include "args.h"

using namespace SC;

int main(int argc, char**argv)
{
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " [-tree] [-On] <SMARTS>" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -tree      Print SMARTS expression tree" << std::endl;
    std::cerr << "  -pretty    Print pretty SMARTS expression" << std::endl;
    PrintOptimizationOptions();
    return 1;
  }

  ListSmartsScores list_scores("emolecules-2012-03-01.smarts_scores");
  PrettySmartsScores pretty_scores;

  ParseArgs args(argc, argv, ParseArgs::Args("-tree", "-pretty"), ParseArgs::Args("SMARTS"));
  //std::cout << args.GetArgInt("-O") << std::endl;

  SmartsScores *scores = args.IsArg("-pretty") ? static_cast<SmartsScores*>(&pretty_scores) : static_cast<SmartsScores*>(&list_scores);

  OpenBabelSmartsMatcher matcher;
  matcher.Init(args.GetArgString("SMARTS"));

  OpenBabel::Pattern *pattern = matcher.GetPattern();

  int opt = GetOptimizationFlags(args);
  if (opt) {
    SmartsOptimizer optimizer(scores);
    optimizer.Optimize(pattern, opt);
  }

  if (args.IsArg("-tree"))
    PrintPatternTree(pattern, scores);
  else {
    PrintPattern(pattern, scores);
    PrintEnvironmentScores(pattern, scores);
  }

}
