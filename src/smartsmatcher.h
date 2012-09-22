#ifndef SC_SMARTSMATCHER_H
#define SC_SMARTSMATCHER_H

#include <openbabel/mol.h>
#include <openbabel/parsmart.h>

#ifdef HAVE_PYTHON
#include <Python.h>
#endif

namespace SC {

  struct PythonSmartsPattern;

  typedef std::vector<int> SingleVectorMapping;
  typedef std::vector<std::vector<int> > VectorMappingList;

  struct NoMapping
  {
    enum { single = false };
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

  //! \class OBSmartsMatcher parsmart.h <openbabel/parsmart.h>
  //! \brief Internal class: performs matching; a wrapper around previous
  //! C matching code to make it thread safe.
  template<typename MolAtomIterType = OpenBabel::OBAtomIterator, typename MolBondIterType = OpenBabel::OBBondIterator,
           typename AtomAtomIterType = OpenBabel::OBAtomAtomIter, typename AtomBondIterType = OpenBabel::OBBondIterator>
  class SmartsMatcher
  {
    protected:
      //recursive smarts cache
      //std::vector<std::pair<const SmartsPattern*, std::vector<bool> > > RSCACHE;
      // list of fragment patterns (e.g., (*).(*)
      //std::vector<const OpenBabel::Pattern*> Fragments;
      //void SetupAtomMatchTable(std::vector<std::vector<bool> > &ttab,
      //    const OpenBabel::Pattern *pat, OpenBabel::OBMol &mol);
      template<typename SmartsPatternType, typename MappingType>
      void FastSingleMatch(OpenBabel::OBMol &mol, SmartsPatternType *pattern, MappingType &mapping);

    public:
      SmartsMatcher() {}
      virtual ~SmartsMatcher() {}

#ifndef SWIG
      template<typename SmartsPatternType, typename MappingType>
      bool Match(OpenBabel::OBMol &mol, SmartsPatternType *pat, MappingType &mapping);
#endif
#if defined(HAVE_PYTHON) || defined(SWIG)
      bool Match(PyObject *mol, PythonSmartsPattern *pat, PyObject *mapping);
#endif
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

#ifdef HAVE_PYTHON
  template<typename MappingType>
  bool Match(const std::string &pythonFileOrModuleName, OpenBabel::OBMol &mol, const std::string &smarts, MappingType &mapping);
#endif

}

#endif
