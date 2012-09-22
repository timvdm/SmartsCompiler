#include "smartsprint.h"
#include "smartsscores.h"
#include "defines.h"
#include "util.h"

namespace SC {

  using namespace OpenBabel;

  std::string GetAtomExprType(int type)
  {
    switch (type) {
      case AE_ANDHI:
        return "And (high priority)";
      case AE_ANDLO:
        return "And (low priority)";
      case AE_OR:
        return "Or";
      case AE_RECUR:
        return "Recursive";
      case AE_NOT:
        return "Not";
      case AE_TRUE:
        return "True";
      case AE_FALSE:
        return "False";
      case AE_AROMATIC:
        return "Aromatic";
      case AE_ALIPHATIC:
        return "Aliphatic";
      case AE_CYCLIC:
        return "Cyclic";
      case AE_ACYCLIC:
        return "Acyclic";
      case AE_MASS:
        return "Mass";
      case AE_ELEM:
        return "Element";
      case AE_AROMELEM:
        return "Aromatic Element";
      case AE_ALIPHELEM:
        return "Aliphatic Element";
      case AE_HCOUNT:
        return "Hydrogen Count";
      case AE_CHARGE:
        return "Charge";
      case AE_CONNECT:
        return "Connectivity (Implicit Valence)";
      case AE_DEGREE:
        return "Degree (Valence)";
      case AE_IMPLICIT:
        return "Implicit Hydrogen Count";
      case AE_RINGS:
        return "Ring Membership Count";
      case AE_SIZE:
        return "In Ring Size";
      case AE_VALENCE:
        return "Valence";
      case AE_CHIRAL:
        return "Chirality";
      case AE_HYB:
        return "Hybridization";
      case AE_RINGCONNECT:
        return "Ring Connectivity";
    }
  }

  std::string GetBondExprType(int type)
  {
    switch (type) {
      case BE_ANDHI:
        return "And (high priority)";
      case BE_ANDLO:
        return "And (low priority)";
      case BE_OR:
        return "Or";
      case BE_NOT:
        return "Not";
      case BE_ANY:
        return "Any";
      case BE_DEFAULT:
        return "Implicit";
      case BE_SINGLE:
        return "Single";
      case BE_DOUBLE:
        return "Double";
      case BE_TRIPLE:
        return "Triple";
      case BE_AROM:
        return "Aromatic";
      case BE_RING:
        return "Ring";
      case BE_UP:
        return "Up";
      case BE_DOWN:
        return "Down";
    }
  }

  void PrintAtomExprTree(AtomExpr *expr, int indent, SmartsScores *scores)
  {
    std::string prefix = "        ";
    for (int i = 0; i < indent; ++i)
      prefix += "    ";

    std::cout << prefix << "AtomExpr:" << std::endl;
    std::cout << prefix << "  Type: " << GetAtomExprType(expr->type) << std::endl;
    std::cout << prefix << "  Score: " << scores->GetExprScore(expr) << std::endl;

    switch (expr->type) {
      case AE_ANDHI:
        PrintAtomExprTree(expr->bin.lft, indent + 1, scores);
        PrintAtomExprTree(expr->bin.rgt, indent + 1, scores);
        break;
      case AE_ANDLO:
        PrintAtomExprTree(expr->bin.lft, indent + 1, scores);
        PrintAtomExprTree(expr->bin.rgt, indent + 1, scores);
        break;
      case AE_OR:
        PrintAtomExprTree(expr->bin.lft, indent + 1, scores);
        PrintAtomExprTree(expr->bin.rgt, indent + 1, scores);
        break;
      case AE_RECUR:
        // FIXME
        break;
      case AE_NOT:
        PrintAtomExprTree(expr->mon.arg, indent + 1, scores);
        break;
      case AE_TRUE:
      case AE_FALSE:
      case AE_AROMATIC:
      case AE_ALIPHATIC:
      case AE_CYCLIC:
      case AE_ACYCLIC:
        break;
      case AE_MASS:
        std::cout << prefix << "  Isotope: " << expr->leaf.value << std::endl;
        break;
      case AE_ELEM:
        std::cout << prefix << "  AtomicNumber: " << expr->leaf.value << std::endl;
        break;
      case AE_AROMELEM:
        std::cout << prefix << "  AtomicNumber: " << expr->leaf.value << std::endl;
        break;
      case AE_ALIPHELEM:
        std::cout << prefix << "  AtomicNumber: " << expr->leaf.value << std::endl;
        break;
      case AE_HCOUNT:
        std::cout << prefix << "  HydrogenCount: " << expr->leaf.value << std::endl;
        break;
      case AE_CHARGE:
        std::cout << prefix << "  Charge: " << expr->leaf.value << std::endl;
      case AE_CONNECT:
        std::cout << prefix << "  ImplicitValence: " << expr->leaf.value << std::endl;
        break;
      case AE_DEGREE:
        std::cout << prefix << "  Valence: " << expr->leaf.value << std::endl;
        break;
      case AE_IMPLICIT:
        std::cout << prefix << "  ImplicitHydrogenCount: " << expr->leaf.value << std::endl;
        break;
      case AE_RINGS:
        std::cout << prefix << "  RingMembershipCount: " << expr->leaf.value << std::endl;
        break;
      case AE_SIZE:
        std::cout << prefix << "  InRingSize: " << expr->leaf.value << std::endl;
        break;
      case AE_VALENCE:
        std::cout << prefix << "  Valence: " << expr->leaf.value << std::endl;
        break;
      case AE_CHIRAL:
        break;
      case AE_HYB:
        std::cout << prefix << "  Hybridization: " << expr->leaf.value << std::endl;
        break;
      case AE_RINGCONNECT:
        std::cout << prefix << "  NumRingBonds: " << expr->leaf.value << std::endl;
        break;
    }
  }

  void PrintBondExprTree(BondExpr *expr, int indent, SmartsScores *scores)
  {
    std::string prefix = "        ";
    for (int i = 0; i < indent; ++i)
      prefix += "    ";

    std::cout << prefix << "BondExpr:" << std::endl;
    std::cout << prefix << "  Type: " << GetBondExprType(expr->type) << std::endl;
    std::cout << prefix << "  Score: " << scores->GetExprScore(expr) << std::endl;

    switch (expr->type) {
      case BE_ANDHI:
        PrintBondExprTree(expr->bin.lft, indent + 1, scores);
        PrintBondExprTree(expr->bin.rgt, indent + 1, scores);
        break;
      case BE_ANDLO:
        PrintBondExprTree(expr->bin.lft, indent + 1, scores);
        PrintBondExprTree(expr->bin.rgt, indent + 1, scores);
        break;
      case BE_OR:
        PrintBondExprTree(expr->bin.lft, indent + 1, scores);
        PrintBondExprTree(expr->bin.rgt, indent + 1, scores);
        break;
      case BE_NOT:
        PrintBondExprTree(expr->mon.arg, indent + 1, scores);
        break;
      case BE_ANY:
      case BE_SINGLE:
      case BE_DOUBLE:
      case BE_TRIPLE:
      case BE_AROM:
      case BE_RING:
      case BE_UP:
      case BE_DOWN:
        break;
    }
  }

  void PrintAtomSpecTree(AtomSpec &as, SmartsScores *scores)
  {
    std::cout << "    Part: " << as.part << std::endl;
    std::cout << "    ChiralFlag: " << as.chiral_flag << std::endl;
    std::cout << "    VectorBinding: " << as.vb << std::endl;
    std::cout << "    Neighbors: ";
    for (std::size_t i = 0; i < as.nbrs.size(); ++i)
      std::cout << as.nbrs[i] << " ";
    std::cout << std::endl;
    std::cout << "    AtomExpression(s):" << std::endl;
    PrintAtomExprTree(as.expr, 0, scores);
  }

  void PrintBondSpecTree(BondSpec &bs, SmartsScores *scores)
  {
    std::cout << "    Src: " << bs.src << std::endl;
    std::cout << "    Dst: " << bs.dst << std::endl;
    std::cout << "    Grow: " << bs.grow << std::endl;
    std::cout << "    BondExpression(s):" << std::endl;
    PrintBondExprTree(bs.expr, 0, scores);
  }


  void PrintPatternTree(Pattern *pattern, SmartsScores *scores)
  {
    std::cout << "NumAtoms: " << pattern->acount << std::endl;
    std::cout << "NumBonds: " << pattern->bcount << std::endl;
    std::cout << "IsChiral: " << pattern->ischiral << std::endl;
    std::cout << "NumParts: " << pattern->parts << std::endl;
    std::cout << "ExplicitH: " << pattern->hasExplicitH << std::endl;

    std::cout << "Atom Specification(s):" << std::endl;
    for (int i = 0; i < pattern->acount; ++i)
      PrintAtomSpecTree(pattern->atom[i], scores);
    std::cout << "Bond Specification(s):" << std::endl;
    for (int i = 0; i < pattern->bcount; ++i)
      PrintBondSpecTree(pattern->bond[i], scores);
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

  std::string GetExprString(const AtomExpr *expr, bool implicitAnd)
  {
    std::string lft, rgt;
    std::stringstream ss;
    int value;

    switch (expr->type) {
      case AE_ANDHI:
        lft = GetExprString(expr->bin.lft);
        rgt = GetExprString(expr->bin.rgt);
        return implicitAnd ? SuppressImplicitAndHi(lft + "&" + rgt) : lft + "&" + rgt;
      case AE_ANDLO:
        lft = GetExprString(expr->bin.lft);
        rgt = GetExprString(expr->bin.rgt);
        return lft + ";" + rgt;
      case AE_OR:
        lft = GetExprString(expr->bin.lft);
        rgt = GetExprString(expr->bin.rgt);
        return lft + "," + rgt;
      case AE_RECUR:
        // FIXME
        return "";
      case AE_NOT:
        lft = GetExprString(expr->mon.arg);
        return "!" + lft;
      case AE_TRUE:
        return "*";
      case AE_FALSE:
        return "!*";
      case AE_AROMATIC:
        return "a";
      case AE_ALIPHATIC:
        return "A";
      case AE_CYCLIC:
        return "R";
      case AE_ACYCLIC:
        return "R0";
      case AE_MASS:
        return make_string(expr->leaf.value);
      case AE_ELEM:
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
            return etab.GetSymbol(expr->leaf.value);
        }
      case AE_AROMELEM:
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
      case AE_ALIPHELEM:
        return etab.GetSymbol(expr->leaf.value);
      case AE_HCOUNT:
        return expr->leaf.value == 1 ? "H" : make_string("H", expr->leaf.value);
      case AE_CHARGE:
        if (expr->leaf.value > 0)
          return expr->leaf.value == 1 ? "+" : make_string("+", expr->leaf.value);
        return expr->leaf.value == -1 ? "-" : make_string(expr->leaf.value);
      case AE_CONNECT:
        return expr->leaf.value == 1 ? "X" : make_string("X", expr->leaf.value);
      case AE_DEGREE:
        return expr->leaf.value == 1 ? "D" : make_string("D", expr->leaf.value);
      case AE_IMPLICIT:
        return expr->leaf.value == 1 ? "h" : make_string("h", expr->leaf.value);
      case AE_RINGS:
        return make_string("R", expr->leaf.value);
      case AE_SIZE:
        return expr->leaf.value == 1 ? "r" : make_string("r", expr->leaf.value);
      case AE_VALENCE:
        return expr->leaf.value == 1 ? "v" : make_string("v", expr->leaf.value);
      case AE_CHIRAL:
        return "";
      case AE_HYB:
        return expr->leaf.value == 1 ? "^" : make_string("^", expr->leaf.value);
      case AE_RINGCONNECT:
        return expr->leaf.value == 1 ? "x" : make_string("x", expr->leaf.value);
    }
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

  template std::string GetBinaryExprString<AtomExpr>(const std::vector<AtomExpr*> &expr, const std::string &op, bool implicitAnd = true);

  std::string GetExprString(const BondExpr *expr, bool implicitAnd)
  {
    std::string lft, rgt;

    switch (expr->type) {
      case BE_ANDHI:
        lft = GetExprString(expr->bin.lft);
        rgt = GetExprString(expr->bin.rgt);
        return implicitAnd ? lft + rgt : lft + "&" + rgt;
      case BE_ANDLO:
        lft = GetExprString(expr->bin.lft);
        rgt = GetExprString(expr->bin.rgt);
        return lft + ";" + rgt;
      case BE_OR:
        lft = GetExprString(expr->bin.lft);
        rgt = GetExprString(expr->bin.rgt);
        return lft + "," + rgt;
      case BE_NOT:
        lft = GetExprString(expr->mon.arg);
        return "!" + lft;
      case BE_DEFAULT:
        return "";
      case BE_ANY:
        return "~";
      case BE_SINGLE:
        return "-";
      case BE_DOUBLE:
        return "=";
      case BE_TRIPLE:
        return "#";
      case BE_AROM:
        return ":";
      case BE_RING:
        return "@";
    }
  }

  void PrintPattern(Pattern *pattern, SmartsScores *scores)
  {
    std::cout << "Expressions:" << std::endl;
    for (int i = 0; i < pattern->acount; ++i)
      std::cout << GetExprString(pattern->atom[i].expr) << "   score: " << scores->GetExprScore(pattern->atom[i].expr) << std::endl;
    for (int i = 0; i < pattern->bcount; ++i) {
      std::string result = GetExprString(pattern->bond[i].expr);
      if (result.size())
        std::cout << result << "   score: " << scores->GetExprScore(pattern->bond[i].expr) << std::endl;
    }
    if (pattern->bcount) {
      std::cout << "Bonds:" << std::endl;
      for (int i = 0; i < pattern->bcount; ++i)
        std::cout << pattern->bond[i].src << "\t" << pattern->bond[i].dst << std::endl;
    }
  }

  void PrintEnvironmentScores(Pattern *pattern, SmartsScores *scores)
  {
    std::cout << "Atom Expressions:" << std::endl;
    for (int i = 0; i < pattern->acount; ++i)
      std::cout << scores->GetExprEnvironmentScore(pattern, pattern->atom[i].expr, 1000) << std::endl;
  }

}
