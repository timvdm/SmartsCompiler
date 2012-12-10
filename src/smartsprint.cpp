#include "smartsprint.h"
#include "smartsscores.h"
#include "defines.h"
#include "util.h"

namespace SC {

  std::string GetSymbol(int atomicNumber)
  {
    const char *symbols[] = {
      "X", "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl",
      "Ar", "K", "Ca", "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As",
      "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In",
      "Sn", "Sb", "Te", "I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb",
      "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl",
      "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra", "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk",
      "Cf", "Es", "Fm", "Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn", "Uut",
      "Fl", "Uup", "Lv", "Uuh",
    };

    return symbols[atomicNumber];
  }

  std::string GetSmartsAtomExprType(int type)
  {
    switch (type) {
      case Smiley::OP_AndHi:
        return "And (high priority)";
      case Smiley::OP_AndLo:
        return "And (low priority)";
      case Smiley::OP_Or:
        return "Or";
      //case Smiley::AE_RECUR:
      //  return "Recursive";
      case Smiley::OP_Not:
        return "Not";
      case Smiley::AE_True:
        return "True";
      case Smiley::AE_False:
        return "False";
      case Smiley::AE_Aromatic:
        return "Aromatic";
      case Smiley::AE_Aliphatic:
        return "Aliphatic";
      case Smiley::AE_Cyclic:
        return "Cyclic";
      case Smiley::AE_Acyclic:
        return "Acyclic";
      case Smiley::AE_Isotope:
        return "Mass";
      case Smiley::AE_AtomicNumber:
        return "Element";
      case Smiley::AE_AromaticElement:
        return "Aromatic Element";
      case Smiley::AE_AliphaticElement:
        return "Aliphatic Element";
      case Smiley::AE_TotalH:
        return "Hydrogen Count";
      case Smiley::AE_Charge:
        return "Charge";
      case Smiley::AE_Connectivity:
        return "Connectivity (Implicit Valence)";
      case Smiley::AE_Degree:
        return "Degree (Valence)";
      case Smiley::AE_ImplicitH:
        return "Implicit Hydrogen Count";
      case Smiley::AE_RingMembership:
        return "Ring Membership Count";
      case Smiley::AE_RingSize:
        return "In Ring Size";
      case Smiley::AE_Valence:
        return "Valence";
      case Smiley::AE_Chirality:
        return "Chirality";
      //case Smiley::AE_HYB:
      //  return "Hybridization";
      case Smiley::AE_RingConnectivity:
        return "Ring Connectivity";
    }

    return "Unknown";
  }

  std::string GetSmartsBondExprType(int type)
  {
    switch (type) {
      case Smiley::OP_AndHi:
        return "And (high priority)";
      case Smiley::OP_AndLo:
        return "And (low priority)";
      case Smiley::OP_Or:
        return "Or";
      case Smiley::OP_Not:
        return "Not";
      case Smiley::BE_True:
        return "Any";
      //case Smiley::BE_DEFAULT:
      //  return "Implicit";
      case Smiley::BE_Single:
        return "Single";
      case Smiley::BE_Double:
        return "Double";
      case Smiley::BE_Triple:
        return "Triple";
      case Smiley::BE_Aromatic:
        return "Aromatic";
      case Smiley::BE_Ring:
        return "Ring";
      case Smiley::BE_Up:
        return "Up";
      case Smiley::BE_Down:
        return "Down";
    }

    return "Unknown";
  }

  void PrintSmartsAtomExprTree(SmartsAtomExpr *expr, int indent, SmartsScores *scores)
  {
    std::string prefix = "        ";
    for (int i = 0; i < indent; ++i)
      prefix += "    ";

    std::cout << prefix << "SmartsAtomExpr:" << std::endl;
    std::cout << prefix << "  Type: " << GetSmartsAtomExprType(expr->type) << std::endl;
//    std::cout << prefix << "  Score: " << scores->GetExprScore(expr) << std::endl;   FIXME FIXME FIXME

    switch (expr->type) {
      case Smiley::OP_AndHi:
        PrintSmartsAtomExprTree(expr->binary.lft, indent + 1, scores);
        PrintSmartsAtomExprTree(expr->binary.rgt, indent + 1, scores);
        break;
      case Smiley::OP_AndLo:
        PrintSmartsAtomExprTree(expr->binary.lft, indent + 1, scores);
        PrintSmartsAtomExprTree(expr->binary.rgt, indent + 1, scores);
        break;
      case Smiley::OP_Or:
        PrintSmartsAtomExprTree(expr->binary.lft, indent + 1, scores);
        PrintSmartsAtomExprTree(expr->binary.rgt, indent + 1, scores);
        break;
      //case Smiley::AE_RECUR:
        // FIXME
      //  break;
      case Smiley::OP_Not:
        PrintSmartsAtomExprTree(expr->unary.arg, indent + 1, scores);
        break;
      case Smiley::AE_True:
      case Smiley::AE_False:
      case Smiley::AE_Aromatic:
      case Smiley::AE_Aliphatic:
      case Smiley::AE_Cyclic:
      case Smiley::AE_Acyclic:
        break;
      case Smiley::AE_Isotope:
        std::cout << prefix << "  Isotope: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_AtomicNumber:
        std::cout << prefix << "  AtomicNumber: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_AromaticElement:
        std::cout << prefix << "  AtomicNumber: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_AliphaticElement:
        std::cout << prefix << "  AtomicNumber: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_TotalH:
        std::cout << prefix << "  HydrogenCount: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_Charge:
        std::cout << prefix << "  Charge: " << expr->leaf.value << std::endl;
      case Smiley::AE_Connectivity:
        std::cout << prefix << "  ImplicitValence: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_Degree:
        std::cout << prefix << "  Valence: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_ImplicitH:
        std::cout << prefix << "  ImplicitHydrogenCount: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_RingMembership:
        std::cout << prefix << "  RingMembershipCount: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_RingSize:
        std::cout << prefix << "  InRingSize: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_Valence:
        std::cout << prefix << "  Valence: " << expr->leaf.value << std::endl;
        break;
      case Smiley::AE_Chirality:
        break;
      //case Smiley::AE_HYB:
      //  std::cout << prefix << "  Hybridization: " << expr->leaf.value << std::endl;
      //  break;
      case Smiley::AE_RingConnectivity:
        std::cout << prefix << "  NumRingBonds: " << expr->leaf.value << std::endl;
        break;
    }
  }

  void PrintSmartsBondExprTree(SmartsBondExpr *expr, int indent, SmartsScores *scores)
  {
    std::string prefix = "        ";
    for (int i = 0; i < indent; ++i)
      prefix += "    ";

    std::cout << prefix << "SmartsBondExpr:" << std::endl;
    std::cout << prefix << "  Type: " << GetSmartsBondExprType(expr->type) << std::endl;
    //std::cout << prefix << "  Score: " << scores->GetExprScore(expr) << std::endl; FIXME FIXME FIXME

    switch (expr->type) {
      case Smiley::OP_AndHi:
        PrintSmartsBondExprTree(expr->binary.lft, indent + 1, scores);
        PrintSmartsBondExprTree(expr->binary.rgt, indent + 1, scores);
        break;
      case Smiley::OP_AndLo:
        PrintSmartsBondExprTree(expr->binary.lft, indent + 1, scores);
        PrintSmartsBondExprTree(expr->binary.rgt, indent + 1, scores);
        break;
      case Smiley::OP_Or:
        PrintSmartsBondExprTree(expr->binary.lft, indent + 1, scores);
        PrintSmartsBondExprTree(expr->binary.rgt, indent + 1, scores);
        break;
      case Smiley::OP_Not:
        PrintSmartsBondExprTree(expr->unary.arg, indent + 1, scores);
        break;
      default:
        break;
    }
  }

  void PrintSmartsAtomTree(SmartsAtom &as, SmartsScores *scores)
  {
    //std::cout << "    Part: " << as.part << std::endl;
    std::cout << "    Chiral: " << as.chiral << std::endl;
    std::cout << "    AtomClass: " << as.atomClass << std::endl;
    std::cout << "    Neighbors: ";
    //for (std::size_t i = 0; i < as.nbrs.size(); ++i)
    //  std::cout << as.nbrs[i] << " ";
    //std::cout << std::endl;
    std::cout << "    SmartsAtomExpression(s):" << std::endl;
    PrintSmartsAtomExprTree(as.expr, 0, scores);
  }

  void PrintSmartsBondTree(SmartsBond &bs, SmartsScores *scores)
  {
    std::cout << "    Source: " << bs.source << std::endl;
    std::cout << "    Target: " << bs.target << std::endl;
    std::cout << "    Grow: " << bs.grow << std::endl;
    std::cout << "    SmartsBondExpression(s):" << std::endl;
    PrintSmartsBondExprTree(bs.expr, 0, scores);
  }


  void PrintSmartsTree(Smarts *pattern, SmartsScores *scores)
  {
    std::cout << "NumAtoms: " << pattern->atoms.size() << std::endl;
    std::cout << "NumBonds: " << pattern->bonds.size() << std::endl;
    std::cout << "IsChiral: " << pattern->chiral << std::endl;
    //std::cout << "NumParts: " << pattern->parts << std::endl;
    //std::cout << "ExplicitH: " << pattern->hasExplicitH << std::endl;

    std::cout << "Atom Specification(s):" << std::endl;
    for (int i = 0; i < pattern->atoms.size(); ++i)
      PrintSmartsAtomTree(pattern->atoms[i], scores);
    std::cout << "Bond Specification(s):" << std::endl;
    for (int i = 0; i < pattern->bonds.size(); ++i)
      PrintSmartsBondTree(pattern->bonds[i], scores);
  }

  std::string SuppressImplicitAndHi(const std::string &expr)
  {
    std::string exprCopy(expr);
    std::size_t pos, last_pos = 0;
    while ((pos = exprCopy.find("&")) != std::string::npos) {
      char left = exprCopy[pos - 1];
      char right = exprCopy[pos + 1];
      bool suppressable = true;
      switch (left) {
        case 'A':
          suppressable = right != 's' && right != 'c';
        case 'B':
          suppressable = right != 'r';
        default:
          break;
      }
      switch (right) {
        case 'a':
          suppressable = left != 'B' && left != 'C' && left != 'N';
          break;
        case 'c':
          suppressable = left != 'S';
          break;
        case 'n':
          suppressable = left != 'C' && left != 'I' && left != 'S' && left != 'Z';
          break;
        case 'o':
          suppressable = left != 'C' && left != 'N' && left != 'P';
          break;
        case 's':
          suppressable = left != 'C' && left != 'N' && left != 'O';
          break;
        case 'p':
          suppressable = left != 'N';
        default:
          break;
      }

      if (suppressable)
        exprCopy.replace(pos, 1, "");

      last_pos = pos;
    }
    return exprCopy;
  }

  std::string GetExprString(const SmartsAtomExpr *expr, bool implicitAnd)
  {
    std::string lft, rgt;
    std::stringstream ss;
    int value;

    switch (expr->type) {
      case Smiley::OP_AndHi:
        lft = GetExprString(expr->binary.lft);
        rgt = GetExprString(expr->binary.rgt);
        return implicitAnd ? SuppressImplicitAndHi(lft + "&" + rgt) : lft + "&" + rgt;
      case Smiley::OP_AndLo:
        lft = GetExprString(expr->binary.lft);
        rgt = GetExprString(expr->binary.rgt);
        return lft + ";" + rgt;
      case Smiley::OP_Or:
        lft = GetExprString(expr->binary.lft);
        rgt = GetExprString(expr->binary.rgt);
        return lft + "," + rgt;
      //case Smiley::AE_RECUR:
        // FIXME
      //  return "";
      case Smiley::OP_Not:
        lft = GetExprString(expr->unary.arg);
        return "!" + lft;
      case Smiley::AE_True:
        return "*";
      case Smiley::AE_False:
        return "!*";
      case Smiley::AE_Aromatic:
        return "a";
      case Smiley::AE_Aliphatic:
        return "A";
      case Smiley::AE_Cyclic:
        return "R";
      case Smiley::AE_Acyclic:
        return "R0";
      case Smiley::AE_Isotope:
        return make_string(expr->leaf.value);
      case Smiley::AE_AtomicNumber:
        switch (expr->leaf.value) {
          case 1:
          case 6:
          case 7:
          case 8:
          case 15:
          case 16:
          case 34:
            return make_string("#", expr->leaf.value);
          default:
            return GetSymbol(expr->leaf.value);
        }
      case Smiley::AE_AromaticElement:
        switch (expr->leaf.value) {
          case 6:
            return "c";
          case 7:
            return "n";
          case 8:
            return "o";
          case 15:
            return "p";
          case 16:
            return "s";
          case 34:
            return "se";
        }
      case Smiley::AE_AliphaticElement:
        return GetSymbol(expr->leaf.value);
      case Smiley::AE_TotalH:
        return expr->leaf.value == 1 ? "H" : make_string("H", expr->leaf.value);
      case Smiley::AE_Charge:
        if (expr->leaf.value > 0)
          return expr->leaf.value == 1 ? "+" : make_string("+", expr->leaf.value);
        return expr->leaf.value == -1 ? "-" : make_string(expr->leaf.value);
      case Smiley::AE_Connectivity:
        return expr->leaf.value == 1 ? "X" : make_string("X", expr->leaf.value);
      case Smiley::AE_Degree:
        return expr->leaf.value == 1 ? "D" : make_string("D", expr->leaf.value);
      case Smiley::AE_ImplicitH:
        return expr->leaf.value == 1 ? "h" : make_string("h", expr->leaf.value);
      case Smiley::AE_RingMembership:
        return make_string("R", expr->leaf.value);
      case Smiley::AE_RingSize:
        return expr->leaf.value == 1 ? "r" : make_string("r", expr->leaf.value);
      case Smiley::AE_Valence:
        return expr->leaf.value == 1 ? "v" : make_string("v", expr->leaf.value);
      case Smiley::AE_Chirality:
        return "";
      //case Smiley::AE_HYB:
      //  return expr->leaf.value == 1 ? "^" : make_string("^", expr->leaf.value);
      case Smiley::AE_RingConnectivity:
        return expr->leaf.value == 1 ? "x" : make_string("x", expr->leaf.value);
    }

    return "<?>";
  }
 
  template<typename Expr>
  std::string GetBinaryExprString(const std::vector<Expr*> &expr, const std::string &op, bool implicitAnd)
  {
    std::string str;
    for (std::size_t i = 0; i < expr.size(); ++i) {
      if (i)
        str += op;
      str += GetExprString(expr[i]);
    }
    return implicitAnd ? SuppressImplicitAndHi(str) : str;
  }

  template std::string GetBinaryExprString<SmartsAtomExpr>(const std::vector<SmartsAtomExpr*> &expr, const std::string &op, bool implicitAnd = true);

  std::string GetExprString(const SmartsBondExpr *expr, bool implicitAnd)
  {
    std::string lft, rgt;

    switch (expr->type) {
      case Smiley::OP_AndHi:
        lft = GetExprString(expr->binary.lft);
        rgt = GetExprString(expr->binary.rgt);
        return implicitAnd ? lft + rgt : lft + "&" + rgt;
      case Smiley::OP_AndLo:
        lft = GetExprString(expr->binary.lft);
        rgt = GetExprString(expr->binary.rgt);
        return lft + ";" + rgt;
      case Smiley::OP_Or:
        lft = GetExprString(expr->binary.lft);
        rgt = GetExprString(expr->binary.rgt);
        return lft + "," + rgt;
      case Smiley::OP_Not:
        lft = GetExprString(expr->unary.arg);
        return "!" + lft;
      //case Smiley::BE_DEFAULT:
      //  return "";
      case Smiley::BE_True:
        return "~";
      case Smiley::BE_Single:
        return "-";
      case Smiley::BE_Double:
        return "=";
      case Smiley::BE_Triple:
        return "#";
      case Smiley::BE_Aromatic:
        return ":";
      case Smiley::BE_Ring:
        return "@";
    }

    return "<?>";
  }

  void PrintSmarts(Smarts *pattern, SmartsScores *scores)
  {
    std::cout << "Expressions:" << std::endl;
    //for (int i = 0; i < pattern->atoms.size(); ++i)
    //  std::cout << GetExprString(pattern->atoms[i].expr) << "   score: " << /*scores->GetExprScore(pattern->atoms[i].expr)*/ << std::endl;
    for (int i = 0; i < pattern->bonds.size(); ++i) {
      std::string result = GetExprString(pattern->bonds[i].expr);
      //if (result.size())
      //  std::cout << result << "   score: " << /*scores->GetExprScore(pattern->bonds[i].expr)*/ << std::endl;
    }
    if (pattern->bonds.size()) {
      std::cout << "Bonds:" << std::endl;
      for (int i = 0; i < pattern->bonds.size(); ++i)
        std::cout << pattern->bonds[i].source << "\t" << pattern->bonds[i].target << std::endl;
    }
  }

  void PrintEnvironmentScores(Smarts *pattern, SmartsScores *scores)
  {
    std::cout << "Atom Expressions:" << std::endl;
    //for (int i = 0; i < pattern->atoms.size(); ++i)
    //  std::cout << /*scores->GetExprEnvironmentScore(pattern, pattern->atoms[i].expr, 1000)*/ << std::endl;
  }

}
