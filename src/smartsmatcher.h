#ifndef SC_SMARTSMATCHER_H
#define SC_SMARTSMATCHER_H

#include <openbabel/mol.h>
#include <openbabel/parsmart.h>

#ifdef HAVE_PYTHON
#include <Python.h>
#endif

namespace SC {

  struct SmartsPattern;
  struct PythonSmartsPattern;

  //! \class OBSmartsMatcher parsmart.h <openbabel/parsmart.h>
  //! \brief Internal class: performs matching; a wrapper around previous
  //! C matching code to make it thread safe.
  class SmartsMatcher
  {
    protected:
      //recursive smarts cache
      std::vector<std::pair<const SmartsPattern*, std::vector<bool> > > RSCACHE;
      // list of fragment patterns (e.g., (*).(*)
      //std::vector<const OpenBabel::Pattern*> Fragments;
      //void SetupAtomMatchTable(std::vector<std::vector<bool> > &ttab,
      //    const OpenBabel::Pattern *pat, OpenBabel::OBMol &mol);
      template<typename SmartsPatternType>
      void FastSingleMatch(OpenBabel::OBMol &mol, SmartsPatternType*,
          std::vector<std::vector<int> > &mlist);

      //template<typename> friend class SSMatch<SmartsPattern>;
      //friend class SSMatch<SmartsPattern>;
    public:
      SmartsMatcher() {}
      virtual ~SmartsMatcher() {}

#ifndef SWIG
      template<typename SmartsPatternType>
      bool Match(OpenBabel::OBMol &mol, SmartsPatternType *pat, std::vector<std::vector<int> > &mlist, bool single = false);
#endif
#if defined(HAVE_PYTHON) || defined(SWIG)
      bool Match(PyObject *mol, PythonSmartsPattern *pat, PyObject *mlist, bool single = false);
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
  bool Match(const std::string &pythonFileOrModuleName, OpenBabel::OBMol &mol, const std::string &smarts, std::vector<std::vector<int> > &mlist, bool single = false);
#endif

}

#endif
