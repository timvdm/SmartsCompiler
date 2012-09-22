#ifndef SC_OPENBABEL_H
#define SC_OPENBABEL_H

#include "toolkit.h"
#include "smartspattern.h"
#include <openbabel/mol.h>
#include <openbabel/obiter.h>

namespace SC {

  struct MockAtom
  {
    void SetProperties(AtomExpr *expr, bool negated = false)
    {
      int sign = negated ? -1 : 1;
      switch (expr->type) {
        case AE_NOT:
          SetProperties(expr->mon.arg, true);
          break;
        case AE_AROMATIC:
          aromatic = sign;
          break;
        case AE_ALIPHATIC:
          aliphatic = sign;
          break;
        case AE_CYCLIC:
          cyclic = sign;
          break;
        case AE_ACYCLIC:
          acyclic = sign;
          break;
        case AE_DEGREE:
          degree.push_back(sign * expr->leaf.value);
          break;
        case AE_HCOUNT:
          totalH.push_back(sign * expr->leaf.value);
          break;
        case AE_IMPLICIT:
          implicitH.push_back(sign * expr->leaf.value);
          break;
        case AE_RINGS:
          numrings.push_back(sign * expr->leaf.value);
          break;
        case AE_SIZE:
          ringsize.push_back(sign * expr->leaf.value);
          break;
        case AE_VALENCE:
          valence.push_back(sign * expr->leaf.value);
          break;
        case AE_CONNECT:
          connect.push_back(sign * expr->leaf.value);
          break;
        case AE_RINGCONNECT:
          ringconnect.push_back(sign * expr->leaf.value);
          break;
        case AE_CHARGE:
          if (expr->leaf.value < 0)
            negcharge.push_back(sign * -expr->leaf.value);
          else
            poscharge.push_back(sign * expr->leaf.value);
          break;
        case AE_ELEM:
          elem.push_back(sign * expr->leaf.value);
          break;
        case AE_ALIPHELEM:
          aliphelem.push_back(sign * expr->leaf.value);
          break;
        case AE_AROMELEM:
          aromelem.push_back(sign * expr->leaf.value);
          break;
        case AE_MASS:
          isotope.push_back(sign * expr->leaf.value);
          break;
        default:
          break;
      }
    }
    
    MockAtom(const std::string &smarts)
    {
      OpenBabelSmartsMatcher matcher;
      sp.Init(smarts);
      Pattern *pat = matcher.GetPattern();
      for (int i = 0; i < pat->acount; ++i)
        SetProperties(pat->atom[i]->expr);
    }

    int aromatic;
    bool aliphatic;
    bool cyclic;
    bool acyclic;
    std::vector<int> degree;
    std::vector<int> totalH;
    std::vector<int> implicitH;
    std::vector<int> numrings;
    std::vector<int> ringsize;
    std::vector<int> valence;
    std::vector<int> connect;
    std::vector<int> ringconnect;
    std::vector<int> poscharge;
    std::vector<int> negcharge;
    std::vector<int> elem;
    std::vector<int> aromelem;
    std::vector<int> aliphelem;
    std::vector<int> isotope;
  };


  class MockToolkit : public Toolkit
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

  /**
   * Iterators
   */
  /*
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
  */

  /**
   * CallEvalExpr
   */
  /*
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

#ifdef HAVE_PYTHON
  template<>
  bool PythonSmartsPattern::CallEvalAtomExpr<OpenBabel::OBAtom>(int index, OpenBabel::OBAtom *atom) const
  {
    //std::cout << "PyCallable_Check: " << PyCallable_Check(EvalAtomExpr) << std::endl;
    //std::cout << "PyFunction_Check: " << PyFunction_Check(EvalAtomExpr) << std::endl;
    PyObject *result = PyEval_CallFunction(EvalAtomExpr, "ii", index, atom->GetIdx());
    //std::cout << "PyBool_Check: " << PyBool_Check(result) << std::endl;
    PyErr_Print();
    return result == Py_True;
  }

  template<>
  bool PythonSmartsPattern::CallEvalBondExpr<OpenBabel::OBBond>(int index, OpenBabel::OBBond *bond) const
  {
    PyObject *result = PyEval_CallFunction(EvalBondExpr, "ii", index, bond->GetIdx());
    PyErr_Print();
    return result == Py_True;
  }
#endif

  typedef SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond> OpenBabelSmartsPattern;
  */

}

#endif
