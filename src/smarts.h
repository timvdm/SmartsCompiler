#ifndef SC_SMARTS_H
#define SC_SMARTS_H

#include "smiley.h"

namespace SC {

  struct SmartsAtomExpr
  {
    SmartsAtomExpr(int typ) : type(typ) {}
    int type;
    union {
      struct {
        int value;
      } leaf;
      struct {
        void *recursive;
      } recursive;
      struct {
        SmartsAtomExpr *arg;
      } unary;
      struct {
        SmartsAtomExpr *lft;
        SmartsAtomExpr *rgt;
      } binary;
    };
  };

  struct SmartsBondExpr
  {
    SmartsBondExpr(int typ) : type(typ) {}
    int type;
    struct {
      SmartsBondExpr *arg;
    } unary;
    struct {
      SmartsBondExpr *lft;
      SmartsBondExpr *rgt;
    } binary;
  };

  template<typename Expr>
  void freeExpr(Expr *expr)
  {
    switch (expr->type) {
      case Smiley::OP_Not:
        freeExpr(expr->unary.arg);
        break;
      case Smiley::OP_AndHi:
      case Smiley::OP_AndLo:
      case Smiley::OP_Or:
        freeExpr(expr->binary.lft);
        freeExpr(expr->binary.rgt);
        break;
      default:
        break;
    }

    delete expr;
  }

  struct SmartsBond
  {
    SmartsBond() : expr(0), source(0), target(0), grow(false)
    {
    }
    
    SmartsBond(int src, int trg, bool grw = false)
        : expr(0), source(src), target(trg), grow(grw)
    {
    }

    int other(int index) const
    {
      return index == source ? target : source;
    }

    SmartsBondExpr *expr;
    int index;
    int source;
    int target;
    bool grow;
  };

  struct SmartsAtom
  {
    SmartsAtom() : expr(0), atomClass(0), chiral(false)
    {
    }
    
    SmartsAtom(SmartsAtomExpr *expr_, int ac, bool chrl)
        : expr(expr_), atomClass(ac), chiral(chrl)
    {
    }

    int degree() const
    {
      return bonds.size();
    }

    const SmartsBond& bond(int index) const
    {
      return *bonds[index];
    }

    SmartsAtomExpr *expr;
    std::vector<const SmartsBond*> bonds;
    int index;
    int atomClass;
    bool chiral;
  };

  struct Smarts
  {
    typedef const SmartsAtom& atom_type;
    typedef const SmartsBond& bond_type;

    Smarts() : chiral(false) {}

    ~Smarts()
    {
      for (std::size_t i = 0; i < atoms.size(); ++i)
        if (atoms[i].expr)
          freeExpr(atoms[i].expr);
      for (std::size_t i = 0; i < bonds.size(); ++i)
        if (bonds[i].expr)
          freeExpr(bonds[i].expr);
    }

    void clear()
    {
      atoms.clear();
      bonds.clear();
      chiral = false;
    }

    int numAtoms() const
    {
      return atoms.size();
    }

    const SmartsAtom& atom(int index) const
    {
      return atoms[index];
    }

    int numBonds() const
    {
      return bonds.size();
    }

    const SmartsBond& bond(int index) const
    {
      return bonds[index];
    }

    template<typename AtomType>
    bool matchAtom(const SmartsAtom &smartsAtom, const AtomType &atom) const
    {
      return matchAtomExpr(smartsAtom.expr, atom);
    }
    
    template<typename AtomType>
    bool matchAtomExpr(const SmartsAtomExpr *expr, const AtomType &atom) const
    {
      switch (expr->type) {
        case Smiley::OP_Not:
          return !matchAtomExpr(expr->unary.arg, atom);
        case Smiley::OP_AndHi:
        case Smiley::OP_AndLo:
          return matchAtomExpr(expr->binary.lft, atom) && matchAtomExpr(expr->binary.rgt, atom);
        case Smiley::OP_Or:
          return matchAtomExpr(expr->binary.lft, atom) || matchAtomExpr(expr->binary.rgt, atom);
        case Smiley::AE_True:
          return true;
        case Smiley::AE_False:
          return false;
        case Smiley::AE_Aromatic:
          return atom.isAromatic();
        case Smiley::AE_Aliphatic:
          return atom.isAliphatic();
        case Smiley::AE_Cyclic:
          return atom.isCyclic();
        case Smiley::AE_Acyclic:
          return atom.isAcyclic();
        case Smiley::AE_Isotope:
          return atom.mass() == expr->leaf.value;
        case Smiley::AE_AtomicNumber:
          return atom.element() == expr->leaf.value;
        case Smiley::AE_AromaticElement:
          return atom.isAromatic() && atom.element() == expr->leaf.value;
        case Smiley::AE_AliphaticElement:
          return atom.isAliphatic() && atom.element() == expr->leaf.value;
        case Smiley::AE_Degree:
          return atom.degree() == expr->leaf.value;
        case Smiley::AE_Valence:
          return atom.valence() == expr->leaf.value;
        case Smiley::AE_Connectivity:
          return atom.connectivity() == expr->leaf.value;
        case Smiley::AE_TotalH:
          return atom.totalHydrogens() == expr->leaf.value;
        case Smiley::AE_ImplicitH:
          return atom.implicitHydrogens() == expr->leaf.value;
        case Smiley::AE_RingMembership:
          return atom.ringMembership() == expr->leaf.value;
        case Smiley::AE_RingSize:
          return atom.isInRingSize(expr->leaf.value);
        case Smiley::AE_RingConnectivity:
          return atom.ringConnectivity() == expr->leaf.value;
        case Smiley::AE_Charge:
          return atom.charge() == expr->leaf.value;
        case Smiley::AE_AtomClass:
          return atom.atomClass() == expr->leaf.value;
        default:
          return true;
      }
    }

    template<typename BondType>
    bool matchBond(const SmartsBond &smartsBond, const BondType &bond) const
    {
      return matchBondExpr(smartsBond.expr, bond);
    }
    
    template<typename BondType>
    bool matchBondExpr(const SmartsBondExpr *expr, const BondType &bond) const
    {
      switch (expr->type) {
        case Smiley::OP_Not:
          return !matchBondExpr(expr->unary.arg, bond);
        case Smiley::OP_AndHi:
        case Smiley::OP_AndLo:
          return matchBondExpr(expr->binary.lft, bond) && matchBondExpr(expr->binary.rgt, bond);
        case Smiley::OP_Or:
          return matchBondExpr(expr->binary.lft, bond) || matchBondExpr(expr->binary.rgt, bond);
        case Smiley::BE_True:
          return true;
        case Smiley::BE_False:
          return false;
        case Smiley::BE_Single:
          return bond.order() == 1 && !bond.isAromatic();
        case Smiley::BE_Double:
          return bond.order() == 2 && !bond.isAromatic();
        case Smiley::BE_Triple:
          return bond.order() == 3;
        case Smiley::BE_Quadriple:
          return bond.order() == 4;
        case Smiley::BE_Aromatic:
          return bond.isAromatic();
        case Smiley::BE_Ring:
          return bond.isCyclic();
        default:
          return true;
      }
    }

    std::vector<SmartsAtom> atoms;
    std::vector<SmartsBond> bonds;
    bool chiral;
  };

  template<typename SmartsType>
  struct smarts_traits
  {
    typedef typename SmartsType::atom_type atom_type;
    typedef typename SmartsType::bond_type bond_type;
  };

  /**
   * Parse a SMARTS string and create an expression trees.
   *
   * @param smarts The SMARTS string to parse.
   *
   * @return Smarts object containing expression trees.
   */
  Smarts* parse(const std::string &smarts);

  /**
   * Write a SMARTS string.
   *
   * @param smarts Smarts object with expression trees.
   *
   * @return The SMARTS string.
   */
  std::string write(const Smarts *smarts);

}

#endif
