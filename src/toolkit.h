#ifndef SC_TOOLKIT_H
#define SC_TOOLKIT_H

#include "smartscompiler.h"

namespace SC {

  class Toolkit
  {
    public:
      virtual ~Toolkit()
      {
      }

      virtual std::string AtomType(enum SmartsCompiler::Language lang) = 0;
      virtual std::string BondType(enum SmartsCompiler::Language lang) = 0;

      virtual std::string AtomArgType(enum SmartsCompiler::Language lang) = 0;
      virtual std::string BondArgType(enum SmartsCompiler::Language lang) = 0;

      virtual std::string AromaticAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string AliphaticAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string CyclicAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string AcyclicAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string MassAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string ElementAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string AliphaticElementAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string AromaticElementAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string HydrogenCountAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string ChargeAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string ConnectAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string DegreeAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string ImplicitAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string NumRingsAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string RingSizeAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string ValenceAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string HybAtomTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string RingConnectAtomTemplate(enum SmartsCompiler::Language lang) = 0;

      virtual bool IsSwitchable(int atomExprType)
      {
        return false;
      }
      virtual std::string GetSwitchExpr(enum SmartsCompiler::Language lang, int atomExprType)
      {
        return "";
      }
      virtual std::string GetSwitchPredicate(enum SmartsCompiler::Language lang, int atomExprType)
      {
        return "";
      }

      virtual std::string DefaultBondTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string SingleBondTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string DoubleBondTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string TripleBondTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string AromaticBondTemplate(enum SmartsCompiler::Language lang) = 0;
      virtual std::string RingBondTemplate(enum SmartsCompiler::Language lang) = 0;
  };

  template<typename MolType>
  struct molecule_traits
  {
    typedef typename MolType::mol_atom_iterator_type mol_atom_iterator_type;
    typedef typename MolType::mol_bond_iterator_type mol_bond_iterator_type;
    typedef typename MolType::atom_atom_iterator_type atom_atom_iterator_type;
    typedef typename MolType::atom_bond_iterator_type atom_bond_iterator_type;
  };

  template<typename MolType, typename MolAtomIterType>
  MolAtomIterType GetBeginAtoms(MolType &mol);
  template<typename MolType, typename MolAtomIterType>
  MolAtomIterType GetEndAtoms(MolType &mol);
  template<typename MolType, typename MolBondIterType>
  MolBondIterType GetBeginBonds(MolType &mol);
  template<typename MolType, typename MolBondIterType>
  MolBondIterType GetEndBonds(MolType &mol);
  template<typename AtomType, typename AtomAtomIterType>
  AtomAtomIterType GetBeginAtoms(AtomType &mol);
  template<typename AtomType, typename AtomAtomIterType>
  AtomAtomIterType GetEndAtoms(AtomType &mol);
  template<typename AtomType, typename AtomBondIterType>
  AtomBondIterType GetBeginBonds(AtomType &mol);
  template<typename AtomType, typename AtomBondIterType>
  AtomBondIterType GetEndBonds(AtomType &mol);

}

#endif
