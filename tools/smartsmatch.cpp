#include "../src/smartsmatcher.h"
#include "../src/smartsscores.h"
#include "../src/smartsoptimizer.h"
#include "../src/smartscompiler.h"
#include "../src/smartsprint.h"

#include "args.h"

#include <openbabel/obconversion.h>

using namespace SC;

int main(int argc, char**argv)
{
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [options] <smarts_file> <molecule_file>" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -anti                Anti-optimize SMARTS" << std::endl;
    std::cerr << "  -scores <file>       Scores file (default is pretty scores)" << std::endl;
    PrintOptimizationOptions();
    return 0;
  }

  ParseArgs args(argc, argv, ParseArgs::Args("-anti", "-anti", "-scores(file)"), ParseArgs::Args("smarts_file", "molecule_file"));
  SmartsScores *scores = args.IsArg("-scores") ? static_cast<SmartsScores*>(new ListSmartsScores(args.GetArgString("-scores", 0))) : static_cast<SmartsScores*>(new PrettySmartsScores);
  bool anti = args.IsArg("-anti");

  std::string smartsFile = args.GetArgString("smarts_file");
  std::string molFile = args.GetArgString("mol_file");

  std::ifstream ifs(smartsFile.c_str());
 
  OpenBabel::OBMol mol;
  OpenBabel::OBConversion conv;
  conv.SetInFormat(conv.FormatFromExt(molFile));

  int smartsCount = 0;
  std::string line;
  while (std::getline(ifs, line)) {
    ++smartsCount;
    std::cout << "SMARTS # " << smartsCount << std::endl;

    std::string smarts = line.substr(0, line.find(" "));
    
    OpenBabelSmartsMatcher matcher;
    if (!matcher.Init(smarts))
      continue;
    OpenBabel::Pattern *pattern = matcher.GetPattern();

    SmartsOptimizer optimizer(scores);
    optimizer.Optimize(pattern);

    std::ifstream mol_ifs(molFile.c_str());
    conv.SetInStream(&mol_ifs);
    int molCount = 0;
    while (conv.Read(&mol)) {
      ++molCount;
      if ((molCount % 1000) == 0)
        std::cout << "  molecule # " << molCount << std::endl;
      if (molCount >= 25000)
        break;

      matcher.Match(mol);    
    }
  }
}
