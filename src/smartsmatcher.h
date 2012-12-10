#ifndef SC_SMARTSMATCHER_H
#define SC_SMARTSMATCHER_H

#include <openbabel/mol.h>
#include <openbabel/parsmart.h>

namespace SC {

  struct PythonSmartsPattern;

  typedef std::vector<int> SingleVectorMapping;
  typedef std::vector<std::vector<int> > VectorMappingList;

  struct NoMapping
  {
    enum { single = true };
    bool match;
  };

  struct CountMapping
  {
    enum { single = false };
    int count;
  };

  struct SingleMapping
  {
    enum { single = true };
    std::vector<int> map;
  };

  struct MappingList
  {
    enum { single = false };
    std::vector<std::vector<int> > maps;
  };

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

  template<typename MolAtomIterType = OpenBabel::OBAtomIterator, typename MolBondIterType = OpenBabel::OBBondIterator,
           typename AtomAtomIterType = OpenBabel::OBAtomAtomIter, typename AtomBondIterType = OpenBabel::OBBondIterator>
  class SmartsMatcher
  {
    protected:
      template<typename MoleculeType, typename SmartsPatternType, typename MappingType>
      void FastSingleMatch(MoleculeType &mol, SmartsPatternType *pattern, MappingType &mapping);

    public:
      SmartsMatcher() {}
      virtual ~SmartsMatcher() {}

      template<typename MoleculeType, typename SmartsPatternType, typename MappingType>
      bool Match(MoleculeType &mol, SmartsPatternType *pat, MappingType &mapping);
  };

  class OpenBabelSmartsMatcher : public OpenBabel::OBSmartsPattern
  {
    public:
      OpenBabelSmartsMatcher() : OpenBabel::OBSmartsPattern()
      {
      }

      OpenBabel::Pattern* GetPattern()
      {
        return _pat;
      }
  };

}

#endif
