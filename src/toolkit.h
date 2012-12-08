#ifndef SC_TOOLKIT_H
#define SC_TOOLKIT_H

#include "smartscodegenerator.h"

namespace SC {

  class Toolkit
  {
    public:
      virtual ~Toolkit()
      {
      }

      virtual std::string AtomType(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string BondType(enum SmartsCodeGenerator::Language lang) = 0;

      virtual std::string AtomArgType(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string BondArgType(enum SmartsCodeGenerator::Language lang) = 0;

      virtual std::string AromaticAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string AliphaticAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string CyclicAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string AcyclicAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string MassAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string ElementAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string AliphaticElementAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string AromaticElementAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string HydrogenCountAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string ChargeAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string ConnectAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string DegreeAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string ImplicitAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string NumRingsAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string RingSizeAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string ValenceAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string HybAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string RingConnectAtomTemplate(enum SmartsCodeGenerator::Language lang) = 0;

      virtual bool IsSwitchable(int atomExprType)
      {
        return false;
      }
      virtual std::string GetSwitchExpr(enum SmartsCodeGenerator::Language lang, int atomExprType)
      {
        return "";
      }
      virtual std::string GetSwitchPredicate(enum SmartsCodeGenerator::Language lang, int atomExprType)
      {
        return "";
      }

      virtual std::string DefaultBondTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string SingleBondTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string DoubleBondTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string TripleBondTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string AromaticBondTemplate(enum SmartsCodeGenerator::Language lang) = 0;
      virtual std::string RingBondTemplate(enum SmartsCodeGenerator::Language lang) = 0;
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
