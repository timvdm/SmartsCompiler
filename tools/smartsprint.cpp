#include "../src/smartsprint.h"
#include "../src/smartsscores.h"
#include "../src/smartsoptimizer.h"
#include "../src/smartscodegenerator.h"
#include "../src/smartsmatcher.h"

#include "args.h"

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


  Smarts *smarts = parse(args.GetArgString("SMARTS"));

  int opt = GetOptimizationFlags(args);
  if (opt) {
    SmartsOptimizer optimizer(scores);
    optimizer.Optimize(smarts, opt);
  }

  if (args.IsArg("-tree"))
    PrintSmartsTree(smarts, scores);
  else {
    PrintSmarts(smarts, scores);
    PrintEnvironmentScores(smarts, scores);
  }

  delete smarts;
}
