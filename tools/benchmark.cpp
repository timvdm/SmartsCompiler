#include "../src/smartsmatcher.h"
#include "../src/smartsscores.h"
#include "../src/smartsoptimizer.h"
#include "../src/smartscodegenerator.h"
#include "../src/smartsprint.h"
#include "../src/molecule.h"

#include "args.h"

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#include <openbabel/parsmart.h>

using namespace SC;

class OBMatcher
{
  public:
    OBMatcher(const std::string &smarts)
    {
      m_smarts.Init(smarts);
    }

    bool match(OpenBabel::OBMol *mol)
    {
      return m_smarts.Match(*mol, true);
    }

    std::string name() const
    {
      return "OpenBabel matcher";
    }

  private:
    OpenBabel::OBSmartsPattern m_smarts;
};

class SCMatcher
{
  public:
    SCMatcher(const std::string &smarts)
    {
      m_smarts = parse(smarts);
      //SmartsOptimizer optimizer(scores);
      //optimizer.Optimize(s);
    }

    ~SCMatcher()
    {
      delete m_smarts;
    }

    bool match(OpenBabel::OBMol *mol)
    {
      return SC::match(mol, m_smarts);
    }

    std::string name() const
    {
      return "SmartsCompiler matcher";
    }


  private:
    Smarts *m_smarts;
};

class SCMatcher2
{
  public:
    SCMatcher2(const std::string &smarts)
    {
      m_smarts = parse(smarts);
      //SmartsOptimizer optimizer(scores);
      //optimizer.Optimize(s);
    }

    ~SCMatcher2()
    {
      delete m_smarts;
    }

    bool match(Molecule *mol)
    {
      return SC::match(mol, m_smarts);
    }

    std::string name() const
    {
      return "SmartsCompiler matcher (*.scm file)";
    }


  private:
    Smarts *m_smarts;
};

template<typename Matcher>
void run_ob(const std::string &smarts, const std::string &filename)
{
  std::ifstream ifs(filename.c_str());

  OpenBabel::OBMol mol;
  OpenBabel::OBConversion conv(&ifs);
  conv.SetInFormat(conv.FormatFromExt(filename));

  Matcher matcher(smarts);

  int molCount = 0;
  int hits = 0;
  while (conv.Read(&mol)) {
    ++molCount;
    if ((molCount % 1000) == 0)
      std::cout << "  molecule # " << molCount << std::endl;
    //if (molCount >= 25000)
    //  break;

    if (matcher.match(&mol))
      ++hits;
  }

  std::cout << matcher.name() << ": " << hits << "/" << molCount << std::endl;
}

template<typename Matcher>
void run_sc(const std::string &smarts, const std::string &filename)
{
  std::ifstream ifs(filename.c_str());

  Matcher matcher(smarts);

  Molecule mol;

  int molCount = 0;
  int hits = 0;
  while (readMolecule(ifs, mol)) {
    ++molCount;
    if ((molCount % 1000) == 0)
      std::cout << "  molecule # " << molCount << std::endl;
    //if (molCount >= 3)
    //  break;

    if (matcher.match(&mol))
      ++hits;
  }

  std::cout << matcher.name() << ": " << hits << "/" << molCount << std::endl;
}

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

  ParseArgs args(argc, argv, ParseArgs::Args("-anti", "-ob", "-scores(file)"), ParseArgs::Args("smarts_file", "molecule_file"));
  SmartsScores *scores = args.IsArg("-scores") ? static_cast<SmartsScores*>(new ListSmartsScores(args.GetArgString("-scores", 0))) : static_cast<SmartsScores*>(new PrettySmartsScores);
  bool anti = args.IsArg("-anti");
  bool ob = args.IsArg("-ob");

  std::string smartsFile = args.GetArgString("smarts_file");
  std::string molFile = args.GetArgString("molecule_file");

  std::ifstream ifs(smartsFile.c_str());

  bool scmFile = molFile.substr(molFile.size() - 4, 4) == ".scm";
  if (scmFile)
    std::cout << "Using *.scm file..." << std::endl;
 
  int smartsCount = 0;
  std::string line;
  while (std::getline(ifs, line)) {
    ++smartsCount;
    std::string smarts = line.substr(0, line.find(" "));
    std::cout << "SMARTS #" << smartsCount << ": " << smarts << std::endl;

    if (scmFile) {
      run_sc<SCMatcher2>(smarts, molFile);
    } else {
      if (ob)
        run_ob<OBMatcher>(smarts, molFile);
      else
        run_ob<SCMatcher>(smarts, molFile);
    }
  }

}
