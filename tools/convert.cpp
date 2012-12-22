#include "../src/molecule.h"

#include "args.h"

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>

using namespace SC;

template<typename Matcher>
void run(const std::string &smarts, const std::string &filename)
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
    if (molCount >= 25000)
      break;

    if (matcher.match(&mol))
      ++hits;
  }

  std::cout << matcher.name() << ": " << hits << "/" << molCount << std::endl;
}


int main(int argc, char**argv)
{
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <in_file> <out_file>" << std::endl;
    return 0;
  }

  ParseArgs args(argc, argv, ParseArgs::Args(), ParseArgs::Args("in_file", "out_file"));
  std::string inFile = args.GetArgString("in_file");
  std::string outFile = args.GetArgString("out_file");


  std::ifstream ifs(inFile.c_str());
  std::ofstream ofs(outFile.c_str());

  OpenBabel::OBMol mol;
  OpenBabel::OBConversion conv(&ifs);
  conv.SetInFormat(conv.FormatFromExt(inFile));

  while (conv.Read(&mol))
    writeMolecule(ofs, &mol);

}
