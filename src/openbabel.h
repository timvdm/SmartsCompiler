#ifndef SC_OPENBABEL_H
#define SC_OPENBABEL_H

#include "toolkit.h"
#include "smartspattern.h"
#include <openbabel/mol.h>
#include <openbabel/obiter.h>

namespace SC {

  class OpenBabelToolkit : public Toolkit
  {
    public:
      std::string AtomType(enum SmartsCodeGenerator::Language lang);
      std::string BondType(enum SmartsCodeGenerator::Language lang);
      std::string AtomArgType(enum SmartsCodeGenerator::Language lang);
      std::string BondArgType(enum SmartsCodeGenerator::Language lang);
      std::string AromaticAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string AliphaticAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string CyclicAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string AcyclicAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string MassAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string ElementAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string AliphaticElementAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string AromaticElementAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string HydrogenCountAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string ChargeAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string ConnectAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string DegreeAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string ImplicitAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string NumRingsAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string RingSizeAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string ValenceAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string HybAtomTemplate(enum SmartsCodeGenerator::Language lang);
      std::string RingConnectAtomTemplate(enum SmartsCodeGenerator::Language lang);

      bool IsSwitchable(int atomExprType);
      std::string GetSwitchExpr(enum SmartsCodeGenerator::Language lang, int atomExprType);
      std::string GetSwitchPredicate(enum SmartsCodeGenerator::Language lang, int atomExprType);

      std::string DefaultBondTemplate(enum SmartsCodeGenerator::Language lang);
      std::string SingleBondTemplate(enum SmartsCodeGenerator::Language lang);
      std::string DoubleBondTemplate(enum SmartsCodeGenerator::Language lang);
      std::string TripleBondTemplate(enum SmartsCodeGenerator::Language lang);
      std::string AromaticBondTemplate(enum SmartsCodeGenerator::Language lang);
      std::string RingBondTemplate(enum SmartsCodeGenerator::Language lang);
  };

  /**
   * Iterators
   */
  template<typename OBIter> struct obiter_traits;
  template<> struct obiter_traits<OpenBabel::OBMolAtomIter>
  {
    typedef OpenBabel::OBMol container_type;
    typedef OpenBabel::OBAtom value_type;
  };
  template<> struct obiter_traits<OpenBabel::OBMolBondIter> 
  {
    typedef OpenBabel::OBMol container_type;
    typedef OpenBabel::OBBond value_type;
  };
  template<> struct obiter_traits<OpenBabel::OBAtomAtomIter>
  {
    typedef OpenBabel::OBAtom container_type;
    typedef OpenBabel::OBAtom value_type;
  };
  template<> struct obiter_traits<OpenBabel::OBAtomBondIter>
  {
    typedef OpenBabel::OBAtom container_type;
    typedef OpenBabel::OBBond value_type;
  };

  template<typename OBIterType>
  class OBIter
  {
      OBIterType m_iter;
    public:
      typedef std::forward_iterator_tag iterator_category;
      typedef typename obiter_traits<OBIterType>::value_type value_type;
      typedef ptrdiff_t difference_type;
      typedef value_type* pointer;
      typedef value_type& reference;

      OBIter() {}
      OBIter(typename obiter_traits<OBIterType>::container_type &c) : m_iter(c) {}
      OBIter(typename obiter_traits<OBIterType>::container_type *c) : m_iter(c) {}
      OBIter(const OBIterType &i) : m_iter(i) {}
      ~OBIter() {}

      OBIter& operator=(const OBIter &iter) { m_iter = iter; }
      operator bool() const { return m_iter(); }
      OBIter& operator++() { ++m_iter; return *this; }
      OBIter  operator++(int) { OBIter tmp(*this); ++m_iter; tmp; }
      pointer operator->() const { return m_iter.operator->(); }
      pointer operator*() const { return m_iter.operator->(); }
  };

  template<>
  struct molecule_traits<OpenBabel::OBMol>
  {
    typedef OpenBabel::OBAtomIterator mol_atom_iterator_type;
    typedef OpenBabel::OBBondIterator mol_bond_iterator_type;
    typedef OBIter<OpenBabel::OBAtomAtomIter> atom_atom_iterator_type;
    typedef OpenBabel::OBBondIterator atom_bond_iterator_type;
  };

  template<>
  OpenBabel::OBAtomIterator GetBeginAtoms<OpenBabel::OBMol, OpenBabel::OBAtomIterator>(OpenBabel::OBMol &mol) 
  {
    return mol.BeginAtoms();
  }
  template<>
  OpenBabel::OBAtomIterator GetEndAtoms<OpenBabel::OBMol, OpenBabel::OBAtomIterator>(OpenBabel::OBMol &mol)
  {
    return mol.EndAtoms();
  }
  template<>
  OpenBabel::OBBondIterator GetBeginBonds<OpenBabel::OBMol, OpenBabel::OBBondIterator>(OpenBabel::OBMol &mol) 
  {
    return mol.BeginBonds();
  }
  template<>
  OpenBabel::OBBondIterator GetEndBonds<OpenBabel::OBMol, OpenBabel::OBBondIterator>(OpenBabel::OBMol &mol)
  {
    return mol.EndBonds();
  }
  template<>
  OBIter<OpenBabel::OBAtomAtomIter> GetBeginAtoms<OpenBabel::OBAtom, OBIter<OpenBabel::OBAtomAtomIter> >(OpenBabel::OBAtom &atom) 
  {
    return OBIter<OpenBabel::OBAtomAtomIter>(atom);
  }
  template<>
  OBIter<OpenBabel::OBAtomAtomIter> GetEndAtoms<OpenBabel::OBAtom, OBIter<OpenBabel::OBAtomAtomIter> >(OpenBabel::OBAtom &atom)
  {
    return OBIter<OpenBabel::OBAtomAtomIter>(atom);
  }
  template<>
  OpenBabel::OBBondIterator GetBeginBonds<OpenBabel::OBAtom, OpenBabel::OBBondIterator>(OpenBabel::OBAtom &atom) 
  {
    return atom.BeginBonds();
  }
  template<>
  OpenBabel::OBBondIterator GetEndBonds<OpenBabel::OBAtom, OpenBabel::OBBondIterator>(OpenBabel::OBAtom &atom)
  {
    return atom.EndBonds();
  }

  /**
   * CallEvalExpr
   */
  template<>
  bool SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>::CallEvalAtomExpr(int index, OpenBabel::OBAtom *atom) const
  {
    return EvalAtomExpr(index, atom);
  }

  template<>
  bool SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>::CallEvalBondExpr(int index, OpenBabel::OBBond *bond) const
  {
    return EvalBondExpr(index, bond);
  }

  /**
   * OpenBabel Atom
   */
  class OpenBabelAtom
  {
    public:
      OpenBabelAtom(OpenBabel::OBAtom *atom) : m_atom(atom)
      {
      }

      bool isAromatic() const
      {
        return m_atom->IsAromatic();
      }

      bool isAliphatic() const
      {
        return !isAromatic();
      }

      bool isCyclic() const
      {
        return m_atom->IsInRing();
      }

      bool isAcyclic() const
      {
        return !m_atom->IsInRing();
      }

      int element() const
      {
        return m_atom->GetAtomicNum();
      }

      int mass() const
      {
        return m_atom->GetIsotope();
      }

      int degree() const
      {
        return m_atom->GetValence();
      }

      int valence() const
      {
        return m_atom->KBOSum() - (m_atom->GetSpinMultiplicity() ? m_atom->GetSpinMultiplicity() - 1 : 0);
      }

      int connectivity() const
      {
        return m_atom->GetImplicitValence();
      }

      int totalHydrogens() const
      {
        return m_atom->ExplicitHydrogenCount() + m_atom->ImplicitHydrogenCount();
      }

      int implicitHydrogens() const
      {
        return m_atom->ImplicitHydrogenCount();
      }

      int ringMembership() const
      {
        return m_atom->MemberOfRingCount();
      }

      bool isInRingSize(int size) const
      {
        return m_atom->IsInRingSize(size);
      }

      int ringConnectivity() const
      {
        return m_atom->CountRingBonds();
      }

      int charge() const
      {
        return m_atom->GetFormalCharge();
      }

      int atomClass() const
      {
        return 0;
      }

    private:
      OpenBabel::OBAtom *m_atom;
  };

  /**
   * OpenBabel Bond
   */
  class OpenBabelBond
  {
    public:
      OpenBabelBond(OpenBabel::OBBond *bond) : m_bond(bond)
      {
      }

      bool isAromatic() const
      {
        return m_bond->IsAromatic();
      }

      int order() const
      {
        return m_bond->GetBO();
      }

    private:
      OpenBabel::OBBond *m_bond;
  };

  typedef SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond> OpenBabelSmartsPattern;

}

#endif
