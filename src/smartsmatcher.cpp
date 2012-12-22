#include "smartsmatcher.h"
#include "smartspattern.h"
#include "defines.h"
#include "util.h"

#include "openbabel.h"
#include "molecule.h"

#include <openbabel/mol.h>
#include <openbabel/stereo/stereo.h>
#include <openbabel/stereo/tetrahedral.h>

namespace SC {

  /**
   * Clear mapping implementations.
   */
  template<typename MappingType>
  inline void ClearMapping(MappingType &mapping)
  {
    mapping.clear();
  }
  template<>
  inline void ClearMapping<NoMapping>(NoMapping &mapping) 
  {
    mapping.match = false;
  }
  template<>
  inline void ClearMapping<SingleMapping>(SingleMapping &mapping) 
  {
    mapping.map.clear();
  }
  template<>
  inline void ClearMapping<CountMapping>(CountMapping &mapping)
  {
    mapping.count = 0;
  }
  template<>
  inline void ClearMapping<MappingList>(MappingList &mapping)
  {
    mapping.maps.clear();
  }

  /**
   * Add mapping implementations.
   */
  inline void AddMapping(SingleVectorMapping &mapping, std::vector<int> &map)
  {
    mapping.swap(map);
  }
  inline void AddMapping(VectorMappingList &mapping, std::vector<int> &map)
  {
    mapping.push_back(map);
  }
  inline void AddMapping(NoMapping &mapping, std::vector<int> &map) 
  {
    mapping.match = true;
  }
  inline void AddMapping(SingleMapping &mapping, std::vector<int> &map) 
  {
    mapping.map.swap(map);
  }
  inline void AddMapping(CountMapping &mapping, std::vector<int> &map)
  {
    mapping.count++;
  }
  inline void AddMapping(MappingList &mapping, std::vector<int> &map)
  {
    mapping.maps.push_back(map);
  }

  /**
   * Empty mapping implementations.
   */
  template<typename MappingType>
  inline bool EmptyMapping(MappingType &mapping)
  {
    return mapping.empty();
  }
  template<>
  inline bool EmptyMapping<NoMapping>(NoMapping &mapping) 
  {
    return !mapping.match;
  }
  template<>
  inline bool EmptyMapping<SingleMapping>(SingleMapping &mapping) 
  {
    return mapping.map.empty();
  }
  template<>
  inline bool EmptyMapping<CountMapping>(CountMapping &mapping)
  {
    return mapping.count == 0;
  }
  template<>
  inline bool EmptyMapping<MappingList>(MappingList &mapping)
  {
    return mapping.maps.empty();
  }

  /**
   * DoSingleMapping
   */
  template<typename MappingType>
  struct DoSingleMapping
  {
    enum { result = MappingType::single };
  };
  template<>
  struct DoSingleMapping<SingleVectorMapping>
  {
    enum { result = true };
  };
  template<>
  struct DoSingleMapping<VectorMappingList>
  {
    enum { result = false };
  };


  template<typename MoleculeType, typename SmartsType, typename MappingType>
  class SmartsMatcherImpl
  {
    public:
      typedef typename smarts_traits<SmartsType>::atom_type SmartsAtomType;
      typedef typename smarts_traits<SmartsType>::bond_type SmartsBondType;

      typedef typename molecule_traits<MoleculeType>::atom_arg_type AtomArgType;
      typedef typename molecule_traits<MoleculeType>::mol_atom_iterator_type MolAtomIter;
      typedef typename molecule_traits<MoleculeType>::atom_bond_iterator_type AtomBondIter;
      
      typedef typename molecule_traits<MoleculeType>::atom_wrapper_type AtomWrapperType;
      typedef typename molecule_traits<MoleculeType>::bond_wrapper_type BondWrapperType;

      SmartsMatcherImpl(MoleculeType *mol, SmartsType *smarts)
      {
        m_mol = mol;
        m_smarts = smarts;
        m_map.resize(smarts->numAtoms(), -1);
        m_bonds.resize(smarts->numBonds(), false);
      }

      void match(MappingType &mapping, SmartsAtomType smartsAtom, AtomArgType atom, int prevAtom = -1)
      {
        // if the atom properties don't match, there is nothing to do
        if (!m_smarts->matchAtom(smartsAtom, AtomWrapperType(atom)))
          return;

        m_map[smartsAtom.index] = GetAtomIndex(m_mol, atom);
        //std::cout << smartsAtom.index << " -> " << GetAtomIndex(m_mol, atom) << std::endl;

        // check for mapping
        if (std::find(m_map.begin(), m_map.end(), -1) == m_map.end()) {
          // ring closures
          bool ringClosuresMatch = true;
          for (std::size_t i = 0; i < m_bonds.size(); ++i) {
            SmartsBondType smartsBond = m_smarts->bond(i);
            if (!m_bonds[smartsBond.index]) {
              ringClosuresMatch = false;

              MolAtomIter source = GetBeginAtoms<MoleculeType*, MolAtomIter>(m_mol);
              std::advance(source, m_map[smartsBond.source]);
              MolAtomIter target = GetBeginAtoms<MoleculeType*, MolAtomIter>(m_mol);
              std::advance(target, m_map[smartsBond.target]);

              //std::cout << "Ring Closure: " << (*source)->GetIndex() << "-" << (*target)->GetIndex() << std::endl;

              AtomBondIter bond = GetBeginBonds<MoleculeType*, AtomArgType, AtomBondIter>(m_mol, *source);
              AtomBondIter bonds_end = GetEndBonds<MoleculeType*, AtomArgType, AtomBondIter>(m_mol, *source);
              for (; bond != bonds_end; ++bond)
                if (GetAtomIndex(m_mol, GetOtherAtom(m_mol, *bond, *source)) == GetAtomIndex(m_mol, *target)) {
                  //std::cout << "..." << std::endl;
                  if (m_smarts->matchBond(smartsBond, BondWrapperType(*bond))) {
                    ringClosuresMatch = true;
                    break;
                  }
                }

              if (!ringClosuresMatch)
                break;
            }
          }

          if (ringClosuresMatch) {
            //std::cout << "found mapping..." << std::endl;
            AddMapping(mapping, m_map);
          }

          m_map[smartsAtom.index] = -1;
          return;
        }

        for (int i = 0; i < smartsAtom.degree(); ++i) {
          SmartsBondType smartsBond = smartsAtom.bond(i);

          if (m_bonds[smartsBond.index])
            continue;
          

          SmartsAtomType nbrSmartsAtom = m_smarts->atom(smartsBond.other(smartsAtom.index));

          if (m_map[nbrSmartsAtom.index] != -1)
            continue;

          AtomBondIter bond = GetBeginBonds<MoleculeType*, AtomArgType, AtomBondIter>(m_mol, atom);
          AtomBondIter bonds_end = GetEndBonds<MoleculeType*, AtomArgType, AtomBondIter>(m_mol, atom);
          for (; bond != bonds_end; ++bond) {
            if (!m_smarts->matchBond(smartsBond, BondWrapperType(*bond)))
              continue;

            AtomArgType nbrAtom = GetOtherAtom(m_mol, *bond, atom);
            if (GetAtomIndex(m_mol, nbrAtom) == prevAtom)
              continue;

            m_bonds[smartsBond.index] = true;
            match(mapping, nbrSmartsAtom, nbrAtom, GetAtomIndex(m_mol, atom));

            if (DoSingleMapping<MappingType>::result && !EmptyMapping(mapping))
              return;


            m_bonds[smartsBond.index] = false;
          }
          
        }

        m_map[smartsAtom.index] = -1;
      }

      void match(MappingType &mapping)
      {
        if (m_smarts->atoms.empty())
          return;

        // try to match each atom in the molecule against the first atom
        // epxression in the SMARTS 
        MolAtomIter atom = GetBeginAtoms<MoleculeType*, MolAtomIter>(m_mol);
        MolAtomIter atoms_end = GetEndAtoms<MoleculeType*, MolAtomIter>(m_mol);
        for (; atom != atoms_end; ++atom) {
          match(mapping, m_smarts->atom(0), *atom);

          if (DoSingleMapping<MappingType>::result && !EmptyMapping(mapping))
            return;
        }
      }

    private:
      MoleculeType *m_mol;
      SmartsType *m_smarts;
      std::vector<int>  m_map;
      std::vector<bool> m_bonds;
  };

  template<typename MoleculeType, typename SmartsType, typename MappingType>
  bool match(MoleculeType *mol, SmartsType *smarts, MappingType &mapping)
  {
    ClearMapping(mapping);

    if (!smarts || smarts->numAtoms() == 0)
      return false;

    SmartsMatcherImpl<MoleculeType, SmartsType, MappingType> ssm(mol, smarts);
    ssm.match(mapping);

    return !EmptyMapping(mapping);
  }

  template bool match<Molecule, Smarts, SingleVectorMapping>(Molecule *mol, Smarts *smarts, SingleVectorMapping &mapping);
  template bool match<Molecule, Smarts, VectorMappingList>(Molecule *mol, Smarts *smarts, VectorMappingList &mapping);
  template bool match<Molecule, Smarts, NoMapping>(Molecule *mol, Smarts *smarts, NoMapping &mapping);
  template bool match<Molecule, Smarts, SingleMapping>(Molecule *mol, Smarts *smarts, SingleMapping &mapping);
  template bool match<Molecule, Smarts, CountMapping>(Molecule *mol, Smarts *smarts, CountMapping &mapping);
  template bool match<Molecule, Smarts, MappingList>(Molecule *mol, Smarts *smarts, MappingList &mapping);


  // OpenBabel
  template bool match<OpenBabel::OBMol, Smarts, SingleVectorMapping>(OpenBabel::OBMol *mol, Smarts *smarts, SingleVectorMapping &mapping);
  template bool match<OpenBabel::OBMol, Smarts, VectorMappingList>(OpenBabel::OBMol *mol, Smarts *smarts, VectorMappingList &mapping);
  template bool match<OpenBabel::OBMol, Smarts, NoMapping>(OpenBabel::OBMol *mol, Smarts *smarts, NoMapping &mapping);
  template bool match<OpenBabel::OBMol, Smarts, SingleMapping>(OpenBabel::OBMol *mol, Smarts *smarts, SingleMapping &mapping);
  template bool match<OpenBabel::OBMol, Smarts, CountMapping>(OpenBabel::OBMol *mol, Smarts *smarts, CountMapping &mapping);
  template bool match<OpenBabel::OBMol, Smarts, MappingList>(OpenBabel::OBMol *mol, Smarts *smarts, MappingList &mapping);

}
