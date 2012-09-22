#include "openbabel.h"
#include "smartsmatcher.h"
#include "defines.h"

namespace SC {

  std::string OpenBabelToolkit::AtomArgType(enum SmartsCompiler::Language lang)
  {
    return "MockAtom*";
  }

  std::string OpenBabelToolkit::BondArgType(enum SmartsCompiler::Language lang)
  {
    return "MockBond*";
  }

  std::string OpenBabelToolkit::AromaticAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "atom->aromatic";
  }

  std::string OpenBabelToolkit::AliphaticAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "atom->aliphatic";
  }

  std::string OpenBabelToolkit::CyclicAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "atom->cyclic";
  }

  std::string OpenBabelToolkit::AcyclicAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "atom->acyclic";
  }

  std::string OpenBabelToolkit::MassAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->isotope.begin(), atom->isotope.end(), $value) != atom.isotope.end()";
  }

  std::string OpenBabelToolkit::ElementAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->elem.begin(), atom->elem.end(), $value) != atom.elem.end()";
  }

  std::string OpenBabelToolkit::AliphaticElementAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->aliphelem.begin(), atom->aliphelem.end(), $value) != atom.aliphelem.end()";
  }

  std::string OpenBabelToolkit::AromaticElementAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->aromelem.begin(), atom->aromelem.end(), $value) != atom.aromelem.end()";
  }

  std::string OpenBabelToolkit::HydrogenCountAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->totalH.begin(), atom->totalH.end(), $value) != atom.totalH.end()";
  }

  std::string OpenBabelToolkit::ChargeAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->charge.begin(), atom->charge.end(), $value) != atom.charge.end()";
  }

  std::string OpenBabelToolkit::ConnectAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->connect.begin(), atom->connect.end(), $value) != atom.connect.end()";
  }

  std::string OpenBabelToolkit::DegreeAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->degree.begin(), atom->degree.end(), $value) != atom.degree.end()";
  }

  std::string OpenBabelToolkit::ImplicitAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->implicit.begin(), atom->implicit.end(), $value) != atom.implicit.end()";
  }

  std::string OpenBabelToolkit::NumRingsAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->numrings.begin(), atom->numrings.end(), $value) != atom.numrings.end()";
  }

  std::string OpenBabelToolkit::RingSizeAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->ringsize.begin(), atom->ringsize.end(), $value) != atom.ringsize.end()";
  }

  std::string OpenBabelToolkit::ValenceAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->valence.begin(), atom->valence.end(), $value) != atom.valence.end()";
  }

  std::string OpenBabelToolkit::HybAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->hyb.begin(), atom->hyb.end(), $value) != atom.hyb.end()";
  }

  std::string OpenBabelToolkit::RingConnectAtomTemplate(enum SmartsCompiler::Language lang)
  {
    return "std::find(atom->ringconnect.begin(), atom->ringconnect.end(), $value) != atom.ringconnect.end()";
  }

  std::string OpenBabelToolkit::DefaultBondTemplate(enum SmartsCompiler::Language lang)
  {
    return "true";
  }

  std::string OpenBabelToolkit::SingleBondTemplate(enum SmartsCompiler::Language lang)
  {
    return "true";
  }

  std::string OpenBabelToolkit::DoubleBondTemplate(enum SmartsCompiler::Language lang)
  {
    return "true";
  }

  std::string OpenBabelToolkit::TripleBondTemplate(enum SmartsCompiler::Language lang)
  {
    return "true";
  }

  std::string OpenBabelToolkit::AromaticBondTemplate(enum SmartsCompiler::Language lang)
  {
    return "true";
  }

  std::string OpenBabelToolkit::RingBondTemplate(enum SmartsCompiler::Language lang)
  {
    return "true";
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

#ifdef HAVE_PYTHON  
  template bool PythonSmartsPattern::CallEvalAtomExpr<OpenBabel::OBAtom>(int index, OpenBabel::OBAtom *atom) const;
  template bool PythonSmartsPattern::CallEvalBondExpr<OpenBabel::OBBond>(int index, OpenBabel::OBBond *atom) const;
#endif
  
  template struct SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>;

}
