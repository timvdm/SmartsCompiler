#include "smartsmatcher.h"
#include "smartspattern.h"
#include "defines.h"
#include "util.h"

#include <openbabel/stereo/stereo.h>
#include <openbabel/stereo/tetrahedral.h>

namespace SC {

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

  template<typename MappingType>
  inline bool EmptyMapping(MappingType &mapping)
  {
    return mapping.empty();
  }
  template<>
  inline bool EmptyMapping<NoMapping>(NoMapping &mapping) 
  {
    return mapping.match;
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

  using namespace OpenBabel;
  
  const int SmartsImplicitRef = -9999; // Used as a placeholder when recording atom nbrs for chiral atoms

  //! \class SSMatch parsmart.h <openbabel/parsmart.h>
  //! \brief Internal class: performs fast, exhaustive matching used to find
  //! just a single match in match() using recursion and explicit stack handling.
  template<typename PatternType, typename MappingType, typename MolAtomIterType,
           typename MolBondIterType, typename AtomAtomIterType, typename AtomBondIterType>
  class SSMatch
  {
    public:
      SSMatch(OBMol &mol, PatternType *pattern)
      {
        m_mol = &mol;
        m_pat = pattern;
        m_map.resize(pattern->numAtoms);

        if (!mol.Empty()) {
          m_uatoms = new bool [mol.NumAtoms() + 1];
          memset((char*)m_uatoms, '\0', sizeof(bool) * (mol.NumAtoms() + 1));
        } else
          m_uatoms = (bool*)NULL;
      }

      ~SSMatch()
      {
        if (m_uatoms)
          delete [] m_uatoms;
      }

      void Match(MappingType &mapping, int bidx = -1);
    private:
      bool *m_uatoms;
      OBMol *m_mol;
      PatternType *m_pat;
      std::vector<int>  m_map;
  };

  template<typename PatternType, typename MappingType, typename MolAtomIterType,
           typename MolBondIterType, typename AtomAtomIterType, typename AtomBondIterType>
  void SSMatch<PatternType, MappingType, MolAtomIterType, MolBondIterType,
               AtomAtomIterType, AtomBondIterType>::Match(MappingType &mapping, int bidx)
  {
    SmartsMatcher<MolAtomIterType, MolBondIterType, AtomAtomIterType, AtomBondIterType> matcher;
    if (bidx == -1) {
      OBAtom *atom;
      std::vector<OBAtom*>::iterator i;
      for (atom = m_mol->BeginAtom(i); atom; atom = m_mol->NextAtom(i))
        if (m_pat->CallEvalAtomExpr(0, atom)) {
          m_map[0] = atom->GetIdx();
          m_uatoms[atom->GetIdx()] = true;
          Match(mapping, 0);
          m_map[0] = 0;
          m_uatoms[atom->GetIdx()] = false;
        }
      return;
    }

    //if (bidx == m_pat->bcount) { //save full match here
    if (bidx == m_pat->bonds.size()) { //save full match here
      AddMapping(mapping, m_map);
      return;
    }

    if (m_pat->bonds[bidx].grow) { //match the next bond
      int src = m_pat->bonds[bidx].src;
      int dst = m_pat->bonds[bidx].dst;

      if (m_map[src] <= 0 || m_map[src] > (signed)m_mol->NumAtoms())
        return;

      OBAtom *atom, *nbr;
      std::vector<OBBond*>::iterator i;

      atom = m_mol->GetAtom(m_map[src]);
      for (nbr = atom->BeginNbrAtom(i); nbr; nbr = atom->NextNbrAtom(i))
        if (!m_uatoms[nbr->GetIdx()] && m_pat->CallEvalAtomExpr(dst, nbr) &&
            m_pat->CallEvalBondExpr(bidx, ((OBBond*) *i))) {
          m_map[dst] = nbr->GetIdx();
          m_uatoms[nbr->GetIdx()] = true;
          Match(mapping, bidx + 1);
          m_uatoms[nbr->GetIdx()] = false;
          m_map[dst] = 0;
        }
    } else { //just check bond here
      OBBond *bond = m_mol->GetBond(m_map[m_pat->bonds[bidx].src], m_map[m_pat->bonds[bidx].dst]);
      if (bond && m_pat->CallEvalBondExpr(bidx, bond))
        Match(mapping, bidx + 1);
    }
  }

  /*
  void SmartsMatcher::SetupAtomMatchTable(std::vector<std::vector<bool> > &ttab,
  const SmartsPattern *pat, OBMol &mol)
  {
  int i;

  ttab.resize(pat->acount);
  for (i = 0;i < pat->acount;++i)
  ttab[i].resize(mol.NumAtoms()+1);

  OBAtom *atom;
  std::vector<OBAtom*>::iterator j;
  for (i = 0;i < pat->acount;++i)
  for (atom = mol.BeginAtom(j);atom;atom = mol.NextAtom(j))
  if (EvalAtomExpr(pat->atom[0].expr,atom))
  ttab[i][atom->GetIdx()] = true;
  }
  */

  template<typename MolAtomIterType, typename MolBondIterType, typename AtomAtomIterType, typename AtomBondIterType>
  template<typename MoleculeType, typename PatternType, typename MappingType>
  void SmartsMatcher<MolAtomIterType, MolBondIterType, AtomAtomIterType, AtomBondIterType>::FastSingleMatch(MoleculeType &mol, PatternType *pat, MappingType &mapping)
  {












    OBAtom *atom,*a1,*nbr;
    std::vector<OBAtom*>::iterator i;

    OBBitVec bv(mol.NumAtoms()+1);
    std::vector<int> map;
    //map.resize(pat->acount);
    map.resize(pat->numAtoms);
    std::vector<std::vector<OBBond*>::iterator> vi;
    std::vector<bool> vif;

    //if (pat->bcount) {
    if (pat->bonds.size()) {
      //vif.resize(pat->bcount);
      vif.resize(pat->bonds.size());
      //vi.resize(pat->bcount);
      vi.resize(pat->bonds.size());
    }

    int bcount;
    for (atom = mol.BeginAtom(i); atom; atom=mol.NextAtom(i)) {
      if (!pat->CallEvalAtomExpr(0, atom))
        continue;

      map[0] = atom->GetIdx();
      if (pat->bonds.size())
        vif[0] = false;
      bv.Clear();
      bv.SetBitOn(atom->GetIdx());

      for (bcount = 0; bcount >= 0;) {
        //***entire pattern matched***
        if (bcount == pat->bonds.size()) { //save full match here
          AddMapping(mapping, map);
          bcount--;
          return; //found a single match
        }

        //***match the next bond***
        if (!pat->bonds[bcount].grow) { //just check bond here
          if (!vif[bcount]) {
            OBBond *bond = mol.GetBond(map[pat->bonds[bcount].src], map[pat->bonds[bcount].dst]);
            if (bond && pat->CallEvalBondExpr(bcount, bond)) {
              vif[bcount++] = true;
              if (bcount < pat->bonds.size())
                vif[bcount] = false;
            } else
              bcount--;
          } else //bond must have already been visited - backtrack
            bcount--;
        } else { //need to map atom and check bond
          a1 = mol.GetAtom(map[pat->bonds[bcount].src]);

          if (!vif[bcount]) //figure out which nbr atom we are mapping
            nbr = a1->BeginNbrAtom(vi[bcount]);
          else {
            bv.SetBitOff(map[pat->bonds[bcount].dst]);
            nbr = a1->NextNbrAtom(vi[bcount]);
          }

          for (; nbr; nbr = a1->NextNbrAtom(vi[bcount]))
            if (!bv[nbr->GetIdx()])
              if (pat->CallEvalAtomExpr(pat->bonds[bcount].dst, nbr)
                  && pat->CallEvalBondExpr(bcount, (OBBond *)*(vi[bcount]))) {
                bv.SetBitOn(nbr->GetIdx());
                map[pat->bonds[bcount].dst] = nbr->GetIdx();
                vif[bcount] = true;
                bcount++;
                if (bcount < pat->bonds.size())
                  vif[bcount] = false;
                break;
              }

          if (!nbr)//no match - time to backtrack
            bcount--;
        }
      } // for bcount
    } // for atom
  }


  template<typename MolAtomIterType, typename MolBondIterType, typename AtomAtomIterType, typename AtomBondIterType>
  template<typename MoleculeType, typename PatternType, typename MappingType>
  bool SmartsMatcher<MolAtomIterType, MolBondIterType, AtomAtomIterType, AtomBondIterType>::Match(MoleculeType &mol, PatternType *pat, MappingType &mapping)
  {
    ClearMapping(mapping);
    if (!pat || pat->numAtoms == 0)
      return(false);//shouldn't ever happen

    if (DoSingleMapping<MappingType>::result && !pat->ischiral) {
      // perform a fast single match (only works for non-chiral SMARTS)
      FastSingleMatch(mol, pat, mapping);
    } else {
      // perform normal match (chirality ignored and checked below)
      SSMatch<PatternType, MappingType, MolAtomIterType, MolBondIterType, AtomAtomIterType, AtomBondIterType> ssm(mol, pat);
      ssm.Match(mapping);
    }

    return !EmptyMapping(mapping);
  }

  template bool SmartsMatcher<>::Match<OpenBabel::OBMol, SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>, SingleVectorMapping>(OpenBabel::OBMol &mol,
      SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond> *pat, SingleVectorMapping &mapping);
  template bool SmartsMatcher<>::Match<OpenBabel::OBMol, SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>, VectorMappingList>(OpenBabel::OBMol &mol,
      SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond> *pat, VectorMappingList &mapping);
  template bool SmartsMatcher<>::Match<OpenBabel::OBMol, SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>, NoMapping>(OpenBabel::OBMol &mol,
      SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond> *pat, NoMapping &mapping);
  template bool SmartsMatcher<>::Match<OpenBabel::OBMol, SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>, SingleMapping>(OpenBabel::OBMol &mol,
      SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond> *pat, SingleMapping &mapping);
  template bool SmartsMatcher<>::Match<OpenBabel::OBMol, SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>, CountMapping>(OpenBabel::OBMol &mol,
      SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond> *pat, CountMapping &mapping);
  template bool SmartsMatcher<>::Match<OpenBabel::OBMol, SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond>, MappingList>(OpenBabel::OBMol &mol,
      SmartsPattern<OpenBabel::OBAtom, OpenBabel::OBBond> *pat, MappingList &mapping);


}
