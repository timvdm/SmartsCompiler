#include "../src/smartsmatcher.h"
#include "../src/smartsscores.h"
#include "../src/smartsoptimizer.h"
#include "../src/smartscompiler.h"
#include "../src/smartsprint.h"

#include <openbabel/obconversion.h>

using namespace SC;

ListSmartsScores scores("emolecules-2012-03-01.smarts_scores");


int main(int argc, char**argv)
{
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [-Ox] <smarts_file> <molecule_file>" << std::endl;
    return 0;
  }

  bool anti = std::string(argv[1]) == "-anti" ? true : false;
  std::string smartsFile = anti ? argv[2] : argv[1];
  std::string molFile = anti ? argv[3] : argv[2];

  std::cout << "anti: " << anti << std::endl;
  std::cout << "smarts file: " << smartsFile << std::endl;
  std::cout << "molecule file: " << molFile << std::endl;

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

    SmartsOptimizer optimizer(&scores);
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
