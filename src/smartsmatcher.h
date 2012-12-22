#ifndef SC_SMARTSMATCHER_H
#define SC_SMARTSMATCHER_H

#include <vector>

namespace SC {

  typedef std::vector<int> SingleVectorMapping;
  typedef std::vector<std::vector<int> > VectorMappingList;

  /**
   * NoMapping
   */
  struct NoMapping
  {
    enum { single = true };

    NoMapping() : match(false)
    {
    }

    bool match;
  };

  /**
   * CountMapping
   */
  struct CountMapping
  {
    enum { single = false };

    CountMapping() : count(0)
    {
    }

    int count;
  };

  /**
   * SingleMapping
   */
  struct SingleMapping
  {
    enum { single = true };
    std::vector<int> map;
  };

  /**
   * MappingList
   */
  struct MappingList
  {
    enum { single = false };
    std::vector<std::vector<int> > maps;
  };

  /**
   * Match a SMARTS against a molecule.
   */
  template<typename MoleculeType, typename SmartsType, typename MappingType>
  bool match(MoleculeType *mol, SmartsType *smarts, MappingType &mapping);

  /**
   * @overload
   */
  template<typename MoleculeType, typename SmartsType>
  bool match(MoleculeType *mol, SmartsType *smarts)
  {
    NoMapping mapping;
    return match(mol, smarts, mapping);
  }

}

#endif
