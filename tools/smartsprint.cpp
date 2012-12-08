#include "../src/smartsprint.h"
#include "../src/smartsscores.h"
#include "../src/smartsoptimizer.h"
#include "../src/smartscodegenerator.h"
#include "../src/smartsmatcher.h"

#include "args.h"

std::vector<OpenBabel::AtomExpr*> expressions;

#include "../src/defines.h"
void memory(OpenBabel::AtomExpr *expr)
{
  std::cout << "expr: " << expr << std::endl;
  expressions.push_back(expr);
  if (expr->type == AE_ANDHI || expr->type == AE_ANDLO) {
    memory(expr->bin.lft);
    memory(expr->bin.rgt);
  }
}

void analyse(OpenBabel::AtomExpr *expr)
{
  std::cout << "read(expr.type) @ " << expr << std::cout;
  if (expr->type == AE_ANDHI) {

  
  }
}

using namespace SC;

int main(int argc, char**argv)
{
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " [options] <SMARTS>" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -tree                Print SMARTS expression tree" << std::endl;
    std::cerr << "  -scores <file>       Scores file (default is pretty scores)" << std::endl;
    PrintOptimizationOptions();
    return 1;
  }

  ParseArgs args(argc, argv, ParseArgs::Args("-tree", "-scores(file)"), ParseArgs::Args("SMARTS"));
  SmartsScores *scores = args.IsArg("-scores") ? static_cast<SmartsScores*>(new ListSmartsScores(args.GetArgString("-scores", 0))) : static_cast<SmartsScores*>(new PrettySmartsScores);

  OpenBabelSmartsMatcher matcher;
  matcher.Init(args.GetArgString("SMARTS"));

  OpenBabel::Pattern *pattern = matcher.GetPattern();
  std::cout << pattern->acount << std::endl;
  memory(pattern->atom[0].expr);

  std::sort(expressions.begin(), expressions.end());
  for (std::size_t i = 1; i < expressions.size(); ++i)
    std::cout << "delta: " << expressions[i] - expressions[i - 1] - sizeof(OpenBabel::AtomExpr) << std::endl;

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
