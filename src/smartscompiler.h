#ifndef SC_SMARTSCOMPILER_H
#define SC_SMARTSCOMPILER_H

#include <openbabel/parsmart.h>

namespace SC {

  class Toolkit;

  struct SmartsCompilerPrivate;

  class SmartsCompiler
  {
    public:
      enum Language
      {
        Cpp,
        Python,
        Java,
      };

      enum ArgType
      {
        AtomArg,
        BondArg
      };

      SmartsCompiler(Toolkit *toolkit, enum Language language = Cpp);
      ~SmartsCompiler();

      void StartSmartsModule(const std::string &name, std::ostream &os);
      void GeneratePatternCode(std::ostream &os, const std::string &smarts, OpenBabel::Pattern *pattern);
      void StopSmartsModule(std::ostream &os);

    private:
      SmartsCompilerPrivate * const d;
  };

}

#endif
