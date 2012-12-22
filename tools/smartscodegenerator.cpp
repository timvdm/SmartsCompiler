#include "../src/smartscodegenerator.h"
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
  std::cerr << "  -no-inline           No function inlining" << std::endl;
  std::cerr << "  -no-switch           No switch functions" << std::endl;
  std::cerr << "  -opt-function-names  Optimize function names (f1 f2 ...)" << std::endl;
  PrintOptimizationOptions();
  return 1;
}

int main(int argc, char**argv)
{
  ParseArgs args(argc, argv, ParseArgs::Args("-c++", "-python", "-module(name)", "-scores(file)", "-no-inline",
      "-no-switch", "-no-match", "-opt-function-names"), ParseArgs::Args("smarts_file", "output_code_file"));
  if (!args.IsValid())
    return PrintUsage(argv[0]);

  std::string smarts_file = args.GetArgString("smarts_file");
  std::string output_code_file = args.GetArgString("output_code_file");

  int opt = GetOptimizationFlags(args);
  enum SmartsCodeGenerator::Language lang = SmartsCodeGenerator::Cpp;
  
  if (args.IsArg("-python"))
    lang = SmartsCodeGenerator::Python;

  std::string module;
  if (args.IsArg("-module"))
    module = args.GetArgString("-module", 0);
  else
    module = smarts_file.substr(0, smarts_file.find("."));
  
  SmartsScores *scores = args.IsArg("-scores") ? static_cast<SmartsScores*>(new ListSmartsScores(args.GetArgString("-scores", 0))) : static_cast<SmartsScores*>(new PrettySmartsScores);


  OpenBabelToolkit toolkit;
  SmartsCodeGenerator compiler(&toolkit, lang);
  SmartsOptimizer optimizer(scores);

  std::ofstream ofs(output_code_file.c_str());

  compiler.StartSmartsModule(module, args.IsArg("-no-inline"), args.IsArg("-no-switch"), args.IsArg("-no-match"), args.IsArg("-opt-function-names"));
  
  std::ifstream ifs(smarts_file.c_str());
  std::string line;
  while (std::getline(ifs, line)) {
    strip(line);
    if (line.size() && line[0] == '#')
      continue;
    std::string function;
    std::size_t pos;
    bool nomap = false, count = false, atom = false;
    while ((pos = line.rfind(" ")) != std::string::npos) {
      std::string option = line.substr(pos + 1);
      if (option == "nomap")
        nomap = true;
      else if (option == "count")
        count = true;
      else if (option == "atom")
        atom = true;
      else
        function = option;

      line.resize(pos);
      strip(line);    
    }
    if (function.size() && function[0] == '#')
      function = "";

    Smarts *smarts = parse(line);

    if (atom && smarts->numAtoms() > 1) {
      std::cerr << "Warning: The 'atom' option is set for SMARTS " << line << " which is not a single atom SMARTS, ignoring option." << std::endl;
      atom = false;
    }
    optimizer.Optimize(smarts, opt);
    PrintSmarts(smarts, scores);
    compiler.GeneratePatternCode(line, smarts, function, nomap, count, atom);
  }
  
  compiler.StopSmartsModule(ofs);

  delete scores;
}
