#include "openbabel.h"
#include "defines.h"

namespace SC {

  std::string OpenBabelToolkit::AtomArgType(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "OBAtom*";
      case SmartsCompiler::Python:
        return "";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::BondArgType(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "OBBond*";
      case SmartsCompiler::Python:
        return "";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AromaticAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->IsAtomatic()";
      case SmartsCompiler::Python:
        return "atom.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AliphaticAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "!atom->IsAtomatic()";
      case SmartsCompiler::Python:
        return "not atom.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::CyclicAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->IsInRing()";
      case SmartsCompiler::Python:
        return "atom.IsInRing()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AcyclicAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "!atom->IsInRing()";
      case SmartsCompiler::Python:
        return "not atom.IsInRing()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::MassAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->GetIsotope() == $value";
      case SmartsCompiler::Python:
        return "atom.GetIsotope() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ElementAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->GetAtomicNum() == $value";
      case SmartsCompiler::Python:
        return "atom.GetAtomicNum() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AliphaticElementAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->GetAtomicNum() == $value && !atom->IsAromatic()";
      case SmartsCompiler::Python:
        return "atom.GetAtomicNum() == $value and not atom.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AromaticElementAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->GetAtomicNum() == $value && atom->IsAromatic()";
      case SmartsCompiler::Python:
        return "atom.GetAtomicNum() == $value and atom.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::HydrogenCountAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->ExplicitHydrogenCount() + atom->ImplicitHydrogenCount() == $value";
      case SmartsCompiler::Python:
        return "atom.ExplicitHydrogenCount() + atom.ImplicitHydrogenCount() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ChargeAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->GetFormalCharge() == $value";
      case SmartsCompiler::Python:
        return "atom.GetFormalCharge() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ConnectAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->GetImplicitValence() == $value";
      case SmartsCompiler::Python:
        return "atom.GetImplicitValence() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::DegreeAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->GetValence() == $value";
      case SmartsCompiler::Python:
        return "atom.GetValence() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ImplicitAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->ImplicitHydrogenCount() == $value";
      case SmartsCompiler::Python:
        return "atom.ImplicitHydrogenCount() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::NumRingsAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->MemberOfRingCount() == $value";
      case SmartsCompiler::Python:
        return "atom.MemberOfRingCount() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::RingSizeAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->IsInRingSize($value)";
      case SmartsCompiler::Python:
        return "atom.IsInRingSize($value)";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ValenceAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->KBOSum() - (atom->GetSpinMultiplicity() ? atom->GetSpinMultiplicity() - 1 : 0) == $value";
      case SmartsCompiler::Python:
        return "atom.KBOSum() - (atom.GetSpinMultiplicity() - 1 if atom.GetSpinMultiplicity() else 0) == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::HybAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->GetHyb() == $value";
      case SmartsCompiler::Python:
        return "atom.GetHyb() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::RingConnectAtomTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "atom->CountRingBonds() == $value";
      case SmartsCompiler::Python:
        return "atom.CountRingBonds() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::DefaultBondTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "bond->GetBO() == 1 || bond->IsAromatic()";
      case SmartsCompiler::Python:
        return "bond.GetBO() == 1 or bond.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::SingleBondTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "bond->GetBO() == 1 && !bond->IsAromatic()";
      case SmartsCompiler::Python:
        return "bond.GetBO() == 1 and not bond.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::DoubleBondTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "bond->GetBO() == 2 && !bond->IsAromatic()";
      case SmartsCompiler::Python:
        return "bond.GetBO() == 2 and not bond.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::TripleBondTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "bond->GetBO() == 3";
      case SmartsCompiler::Python:
        return "bond.GetBO() == 3";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AromaticBondTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "bond->IsAromatic()";
      case SmartsCompiler::Python:
        return "bond.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::RingBondTemplate(enum SmartsCompiler::Language lang)
  {
    switch (lang) {
      case SmartsCompiler::Cpp:
        return "bond->IsInRing()";
      case SmartsCompiler::Python:
        return "bond.IsInRing()";
      default:
        return "";
    }
  }

  bool OpenBabelToolkit::IsSwitchable(int atomExprType)
  {
    switch (atomExprType) {
      case AE_MASS:
      case AE_ELEM:
      case AE_AROMELEM:
      case AE_ALIPHELEM:
      case AE_HCOUNT:
      case AE_CHARGE:
      case AE_CONNECT:
      case AE_DEGREE:
      case AE_IMPLICIT:
      case AE_RINGS:
      case AE_VALENCE:
      case AE_HYB:
      case AE_RINGCONNECT:
        return true;
      default:
        return false;
    }
  }

  std::string OpenBabelToolkit::GetSwitchExpr(enum SmartsCompiler::Language lang, int atomExprType)
  {
    std::string expr;
    switch (atomExprType) {
      case AE_MASS:
        expr = MassAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_ELEM:
        expr = ElementAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_AROMELEM:
        expr = AromaticElementAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_ALIPHELEM:
        expr = AliphaticElementAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_HCOUNT:
        expr = HydrogenCountAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_CHARGE:
        expr = ChargeAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_CONNECT:
        expr = ConnectAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_DEGREE:
        expr = DegreeAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_IMPLICIT:
        expr = ImplicitAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_RINGS:
        expr = NumRingsAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_VALENCE:
        expr = ValenceAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_HYB:
        expr = HybAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case AE_RINGCONNECT:
        expr = RingConnectAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::GetSwitchPredicate(enum SmartsCompiler::Language lang, int atomExprType)
  {
    std::string expr;
    switch (atomExprType) {
      case AE_AROMELEM:
        switch (lang) {
          case SmartsCompiler::Cpp:
            return "atom->IsAromatic()";
          case SmartsCompiler::Python:
            return "atom.IsAromatic()";
          default:
            return "";
        }
      case AE_ALIPHELEM:
        switch (lang) {
          case SmartsCompiler::Cpp:
            return "!atom->IsAromatic()";
          case SmartsCompiler::Python:
            return "not atom.IsAromatic()";
          default:
            return "";
        }
      default:
        return "";
    }
  }

}
