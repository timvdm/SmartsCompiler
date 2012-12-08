#ifndef SC_SMARTSCOMPILER_H
#define SC_SMARTSCOMPILER_H

#include <openbabel/parsmart.h>

namespace SC {

  class Toolkit;

  struct SmartsCodeGeneratorPrivate;

  class SmartsCodeGenerator
  {
    public:
      enum Language
      {
        Cpp,
        Python
      };

      enum ArgType
      {
        AtomArg,
        BondArg
      };

      SmartsCodeGenerator(Toolkit *toolkit, enum Language language = Cpp);
      ~SmartsCodeGenerator();

      void StartSmartsModule(const std::string &name,bool noInline = false, 
          bool noSwitch = false, bool noMatch = false, bool optimizeFunctioNames = false);
      void GeneratePatternCode(const std::string &smarts, OpenBabel::Pattern *pattern,
          const std::string &function = std::string(), bool nomap = false, 
          bool count = false, bool atom = false);
      void StopSmartsModule(std::ostream &os);

    private:
      SmartsCodeGeneratorPrivate * const d;
  };

}

#endif
