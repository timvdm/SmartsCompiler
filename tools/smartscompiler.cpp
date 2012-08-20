#include "../src/smartscompiler.h"
#include "../src/smartsscores.h"
#include "../src/smartsoptimizer.h"
#include "../src/openbabel.h"
#include "../src/smartsprint.h"
#include "../src/smartsmatcher.h"

#include "args.h"

using namespace SC;


int PrintUsage(const char *exe)
{
  std::cerr << "Usage: " << exe << " [options] <smarts_file> <output_code_file>" << std::endl;
  std::cerr << "Options:" << std::endl;
  std::cerr << "  -module <name>       Module name (default is smarts_file w/o extension)" << std::endl;
  std::cerr << "  -c++                 Generate C++ code (default)" << std::endl;
  std::cerr << "  -python              Generate python code" << std::endl;
  std::cerr << "  -scores <file>       Scores file (default is pretty scores)" << std::endl;
  PrintOptimizationOptions();
  return 1;
}

int main(int argc, char**argv)
{
  ParseArgs args(argc, argv, ParseArgs::Args("-c++", "-python", "-module(name)", "-scores(file)"), ParseArgs::Args("smarts_file", "output_code_file"));
  if (!args.IsValid())
    return PrintUsage(argv[0]);

  std::string smarts_file = args.GetArgString("smarts_file");
  std::string output_code_file = args.GetArgString("output_code_file");

  int opt = GetOptimizationFlags(args);
  enum SmartsCompiler::Language lang = SmartsCompiler::Cpp;
  
  if (args.IsArg("-python"))
    lang = SmartsCompiler::Python;

  std::string module;
  if (args.IsArg("-module"))
    module = args.GetArgString("-module", 0);
  else
    module = smarts_file.substr(0, smarts_file.find("."));
  
  SmartsScores *scores = args.IsArg("-scores") ? static_cast<SmartsScores*>(new ListSmartsScores(args.GetArgString("-scores", 0))) : static_cast<SmartsScores*>(new PrettySmartsScores);


  OpenBabelToolkit toolkit;
  SmartsCompiler compiler(&toolkit, lang);
  SmartsOptimizer optimizer(scores);

  std::ofstream ofs(output_code_file.c_str());
  compiler.StartSmartsModule(module, ofs);
  
  std::ifstream ifs(smarts_file.c_str());
  std::string line;
  while (std::getline(ifs, line)) {
    OpenBabelSmartsMatcher matcher;
    matcher.Init(line);
    OpenBabel::Pattern *pattern = matcher.GetPattern();
    optimizer.Optimize(pattern, opt);
    PrintPattern(pattern, scores);
    compiler.GeneratePatternCode(ofs, line, pattern);
  }
  
  compiler.StopSmartsModule(ofs);

  delete scores;
}
