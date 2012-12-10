#include "openbabel.h"
#include "smartsmatcher.h"
#include "defines.h"
#include "smiley.h"

namespace SC {

  std::string OpenBabelToolkit::AtomType(enum SmartsCodeGenerator::Language lang)
  {
    return "OBAtom";
  }

  std::string OpenBabelToolkit::BondType(enum SmartsCodeGenerator::Language lang)
  {
    return "OBBond";
  }

  std::string OpenBabelToolkit::AtomArgType(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "OBAtom*";
      case SmartsCodeGenerator::Python:
        return "";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::BondArgType(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "OBBond*";
      case SmartsCodeGenerator::Python:
        return "";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AromaticAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->IsAromatic()";
      case SmartsCodeGenerator::Python:
        return "atom.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AliphaticAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "!atom->IsAromatic()";
      case SmartsCodeGenerator::Python:
        return "not atom.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::CyclicAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->IsInRing()";
      case SmartsCodeGenerator::Python:
        return "atom.IsInRing()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AcyclicAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "!atom->IsInRing()";
      case SmartsCodeGenerator::Python:
        return "not atom.IsInRing()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::MassAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->GetIsotope() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.GetIsotope() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ElementAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->GetAtomicNum() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.GetAtomicNum() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AliphaticElementAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->GetAtomicNum() == $value && !atom->IsAromatic()";
      case SmartsCodeGenerator::Python:
        return "atom.GetAtomicNum() == $value and not atom.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AromaticElementAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->GetAtomicNum() == $value && atom->IsAromatic()";
      case SmartsCodeGenerator::Python:
        return "atom.GetAtomicNum() == $value and atom.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::HydrogenCountAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->ExplicitHydrogenCount() + atom->ImplicitHydrogenCount() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.ExplicitHydrogenCount() + atom.ImplicitHydrogenCount() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ChargeAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->GetFormalCharge() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.GetFormalCharge() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ConnectAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->GetImplicitValence() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.GetImplicitValence() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::DegreeAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->GetValence() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.GetValence() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ImplicitAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->ImplicitHydrogenCount() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.ImplicitHydrogenCount() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::NumRingsAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->MemberOfRingCount() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.MemberOfRingCount() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::RingSizeAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->IsInRingSize($value)";
      case SmartsCodeGenerator::Python:
        return "atom.IsInRingSize($value)";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::ValenceAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->KBOSum() - (atom->GetSpinMultiplicity() ? atom->GetSpinMultiplicity() - 1 : 0) == $value";
      case SmartsCodeGenerator::Python:
        return "atom.KBOSum() - (atom.GetSpinMultiplicity() - 1 if atom.GetSpinMultiplicity() else 0) == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::HybAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->GetHyb() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.GetHyb() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::RingConnectAtomTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "atom->CountRingBonds() == $value";
      case SmartsCodeGenerator::Python:
        return "atom.CountRingBonds() == $value";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::DefaultBondTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "bond->GetBO() == 1 || bond->IsAromatic()";
      case SmartsCodeGenerator::Python:
        return "bond.GetBO() == 1 or bond.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::SingleBondTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "bond->GetBO() == 1 && !bond->IsAromatic()";
      case SmartsCodeGenerator::Python:
        return "bond.GetBO() == 1 and not bond.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::DoubleBondTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "bond->GetBO() == 2 && !bond->IsAromatic()";
      case SmartsCodeGenerator::Python:
        return "bond.GetBO() == 2 and not bond.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::TripleBondTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "bond->GetBO() == 3";
      case SmartsCodeGenerator::Python:
        return "bond.GetBO() == 3";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::AromaticBondTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "bond->IsAromatic()";
      case SmartsCodeGenerator::Python:
        return "bond.IsAromatic()";
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::RingBondTemplate(enum SmartsCodeGenerator::Language lang)
  {
    switch (lang) {
      case SmartsCodeGenerator::Cpp:
        return "bond->IsInRing()";
      case SmartsCodeGenerator::Python:
        return "bond.IsInRing()";
      default:
        return "";
    }
  }

  bool OpenBabelToolkit::IsSwitchable(int atomExprType)
  {
    switch (atomExprType) {
      case Smiley::AE_Isotope:
      case Smiley::AE_AtomicNumber:
      case Smiley::AE_AromaticElement:
      case Smiley::AE_AliphaticElement:
      case Smiley::AE_TotalH:
      case Smiley::AE_Charge:
      case Smiley::AE_Connectivity:
      case Smiley::AE_Degree:
      case Smiley::AE_ImplicitH:
      case Smiley::AE_RingMembership:
      case Smiley::AE_Valence:
      //case Smiley::AE_HYB:
      case Smiley::AE_RingConnectivity:
        return true;
      default:
        return false;
    }
  }

  std::string OpenBabelToolkit::GetSwitchExpr(enum SmartsCodeGenerator::Language lang, int atomExprType)
  {
    std::string expr;
    switch (atomExprType) {
      case Smiley::AE_Isotope:
        expr = MassAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_AtomicNumber:
        expr = ElementAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_AromaticElement:
        expr = AromaticElementAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_AliphaticElement:
        expr = AliphaticElementAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_TotalH:
        expr = HydrogenCountAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_Charge:
        expr = ChargeAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_Connectivity:
        expr = ConnectAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_Degree:
        expr = DegreeAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_ImplicitH:
        expr = ImplicitAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_RingMembership:
        expr = NumRingsAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      case Smiley::AE_Valence:
        expr = ValenceAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      //case Smiley::AE_HYB:
      //  expr = HybAtomTemplate(lang);
      //  return expr.substr(0, expr.find(" "));
      case Smiley::AE_RingConnectivity:
        expr = RingConnectAtomTemplate(lang);
        return expr.substr(0, expr.find(" "));
      default:
        return "";
    }
  }

  std::string OpenBabelToolkit::GetSwitchPredicate(enum SmartsCodeGenerator::Language lang, int atomExprType)
  {
    std::string expr;
    switch (atomExprType) {
      case Smiley::AE_AromaticElement:
        switch (lang) {
          case SmartsCodeGenerator::Cpp:
            return "atom->IsAromatic()";
          case SmartsCodeGenerator::Python:
            return "atom.IsAromatic()";
          default:
            return "";
        }
      case Smiley::AE_AliphaticElement:
        switch (lang) {
          case SmartsCodeGenerator::Cpp:
            return "!atom->IsAromatic()";
          case SmartsCodeGenerator::Python:
            return "not atom.IsAromatic()";
          default:
            return "";
        }
      default:
        return "";
    }
  }

  template struct SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>;

}
