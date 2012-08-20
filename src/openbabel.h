#ifndef SC_OPENBABEL_H
#define SC_OPENBABEL_H

#include "toolkit.h"

namespace SC {

  class OpenBabelToolkit : public Toolkit
  {
    public:
      std::string AtomArgType(enum SmartsCompiler::Language lang);
      std::string BondArgType(enum SmartsCompiler::Language lang);
      std::string AromaticAtomTemplate(enum SmartsCompiler::Language lang);
      std::string AliphaticAtomTemplate(enum SmartsCompiler::Language lang);
      std::string CyclicAtomTemplate(enum SmartsCompiler::Language lang);
      std::string AcyclicAtomTemplate(enum SmartsCompiler::Language lang);
      std::string MassAtomTemplate(enum SmartsCompiler::Language lang);
      std::string ElementAtomTemplate(enum SmartsCompiler::Language lang);
      std::string AliphaticElementAtomTemplate(enum SmartsCompiler::Language lang);
      std::string AromaticElementAtomTemplate(enum SmartsCompiler::Language lang);
      std::string HydrogenCountAtomTemplate(enum SmartsCompiler::Language lang);
      std::string ChargeAtomTemplate(enum SmartsCompiler::Language lang);
      std::string ConnectAtomTemplate(enum SmartsCompiler::Language lang);
      std::string DegreeAtomTemplate(enum SmartsCompiler::Language lang);
      std::string ImplicitAtomTemplate(enum SmartsCompiler::Language lang);
      std::string NumRingsAtomTemplate(enum SmartsCompiler::Language lang);
      std::string RingSizeAtomTemplate(enum SmartsCompiler::Language lang);
      std::string ValenceAtomTemplate(enum SmartsCompiler::Language lang);
      std::string HybAtomTemplate(enum SmartsCompiler::Language lang);
      std::string RingConnectAtomTemplate(enum SmartsCompiler::Language lang);

      bool IsSwitchable(int atomExprType);
      std::string GetSwitchExpr(enum SmartsCompiler::Language lang, int atomExprType);
      std::string GetSwitchPredicate(enum SmartsCompiler::Language lang, int atomExprType);

      std::string DefaultBondTemplate(enum SmartsCompiler::Language lang);
      std::string SingleBondTemplate(enum SmartsCompiler::Language lang);
      std::string DoubleBondTemplate(enum SmartsCompiler::Language lang);
      std::string TripleBondTemplate(enum SmartsCompiler::Language lang);
      std::string AromaticBondTemplate(enum SmartsCompiler::Language lang);
      std::string RingBondTemplate(enum SmartsCompiler::Language lang);
  };

}

#endif
