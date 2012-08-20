#include "smartsmatcher.h"
#include "smartspattern.h"
#include "defines.h"
#include "util.h"

#include <openbabel/stereo/stereo.h>
#include <openbabel/stereo/tetrahedral.h>

#ifdef HAVE_PYTHON
#include "swig.h"
#endif

namespace SC {

  bool SmartsPattern::CallEvalExpr(int index, OpenBabel::OBAtom *atom) const
  {
    return EvalAtomExpr(index, atom);
  }

  bool SmartsPattern::CallEvalExpr(int index, OpenBabel::OBBond *bond) const
  {
    return EvalBondExpr(index, bond);
  }

#ifdef HAVE_PYTHON
  bool PythonSmartsPattern::CallEvalExpr(int index, OpenBabel::OBAtom *atom) const
  {
    //std::cout << "PyCallable_Check: " << PyCallable_Check(EvalAtomExpr) << std::endl;
    //std::cout << "PyFunction_Check: " << PyFunction_Check(EvalAtomExpr) << std::endl;
    PyObject *result = PyEval_CallFunction(EvalAtomExpr, "ii", index, atom->GetIdx());
    //std::cout << "PyBool_Check: " << PyBool_Check(result) << std::endl;
    PyErr_Print();
    return result == Py_True;
  }

  bool PythonSmartsPattern::CallEvalExpr(int index, OpenBabel::OBBond *bond) const
  {
    PyObject *result = PyEval_CallFunction(EvalBondExpr, "ii", index, bond->GetIdx());
    PyErr_Print();
    return result == Py_True;
  }
#endif

using namespace OpenBabel;
  
const int SmartsImplicitRef = -9999; // Used as a placeholder when recording atom nbrs for chiral atoms

//! \class SSMatch parsmart.h <openbabel/parsmart.h>
//! \brief Internal class: performs fast, exhaustive matching used to find
//! just a single match in match() using recursion and explicit stack handling.
template<typename PatternType>
class SSMatch
{
  public:
    SSMatch(OBMol &mol, PatternType *pattern)
    {
      m_mol = &mol;
      m_pat = pattern;
      m_map.resize(pattern->numAtoms);

      if (!mol.Empty()) {
        m_uatoms = new bool [mol.NumAtoms()+1];
        memset((char*)m_uatoms, '\0', sizeof(bool) * (mol.NumAtoms() + 1));
      } else
        m_uatoms = (bool*)NULL;

    }
    
    ~SSMatch()
    {
      if (m_uatoms)
        delete [] m_uatoms;
    }

    void Match(std::vector<std::vector<int> > &v, int bidx=-1);
  private:
    bool *m_uatoms;
    OBMol *m_mol;
    PatternType *m_pat;
    std::vector<int>  m_map;
};

template<typename PatternType>
void SSMatch<PatternType>::Match(std::vector<std::vector<int> > &mlist, int bidx)
{
  SmartsMatcher matcher;
  if (bidx == -1) {
    OBAtom *atom;
    std::vector<OBAtom*>::iterator i;
    for (atom = m_mol->BeginAtom(i); atom; atom = m_mol->NextAtom(i))
      if (m_pat->CallEvalExpr(0, atom)) {
        m_map[0] = atom->GetIdx();
        m_uatoms[atom->GetIdx()] = true;
        Match(mlist,0);
        m_map[0] = 0;
        m_uatoms[atom->GetIdx()] = false;
      }
    return;
  }

  //if (bidx == m_pat->bcount) { //save full match here
  if (bidx == m_pat->bonds.size()) { //save full match here
    mlist.push_back(m_map);
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
      if (!m_uatoms[nbr->GetIdx()] && m_pat->CallEvalExpr(dst, nbr) &&
          m_pat->CallEvalExpr(bidx, ((OBBond*) *i))) {
        m_map[dst] = nbr->GetIdx();
        m_uatoms[nbr->GetIdx()] = true;
        Match(mlist,bidx+1);
        m_uatoms[nbr->GetIdx()] = false;
        m_map[dst] = 0;
      }
  } else { //just check bond here
    OBBond *bond = m_mol->GetBond(m_map[m_pat->bonds[bidx].src], m_map[m_pat->bonds[bidx].dst]);
    if (bond && m_pat->CallEvalExpr(bidx, bond))
      Match(mlist, bidx + 1);
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

template<typename PatternType>
void SmartsMatcher::FastSingleMatch(OBMol &mol, PatternType *pat,
    std::vector<std::vector<int> > &mlist)
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
  for (atom = mol.BeginAtom(i); atom; atom=mol.NextAtom(i))
    if (pat->CallEvalExpr(0, atom)) {
      map[0] = atom->GetIdx();
      if (pat->bonds.size())
        vif[0] = false;
      bv.Clear();
      bv.SetBitOn(atom->GetIdx());

      for (bcount = 0; bcount >= 0;) {
        //***entire pattern matched***
        if (bcount == pat->bonds.size()) { //save full match here
          mlist.push_back(map);
          bcount--;
          return; //found a single match
        }

        //***match the next bond***
        if (!pat->bonds[bcount].grow) { //just check bond here
          if (!vif[bcount]) {
            OBBond *bond = mol.GetBond(map[pat->bonds[bcount].src], map[pat->bonds[bcount].dst]);
            if (bond && pat->CallEvalExpr(bcount, bond)) {
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
              if (pat->CallEvalExpr(pat->bonds[bcount].dst, nbr)
                  && pat->CallEvalExpr(bcount, (OBBond *)*(vi[bcount]))) {
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
      }
    }
}


template<typename PatternType>
bool SmartsMatcher::Match(OBMol &mol, PatternType *pat,
    std::vector<std::vector<int> > &mlist, bool single)
{
  mlist.clear();
  if (!pat || pat->numAtoms == 0)
    return(false);//shouldn't ever happen

  if (single && !pat->ischiral) {
    // perform a fast single match (only works for non-chiral SMARTS)
    FastSingleMatch(mol, pat, mlist);
  } else {
    // perform normal match (chirality ignored and checked below)
    SSMatch<PatternType> ssm(mol, pat);
    ssm.Match(mlist);
  }

  /*
  if (pat->ischiral) {
    std::vector<std::vector<int> >::iterator m;
    std::vector<std::vector<int> > tmpmlist;

    tmpmlist.clear();
    // iterate over the atom mappings
    for (m = mlist.begin();m != mlist.end();++m) {

      bool allStereoCentersMatch = true;

      // for each pattern atom
      for (int j = 0; j < pat->numAtoms; ++j) {
        // skip non-chiral pattern atoms
        if (!pat->atom[j].chiral_flag)
          continue;
        // ignore @? in smarts, parse like any other smarts
        if (pat->atom[j].chiral_flag == AL_UNSPECIFIED)
          continue;

        // use the mapping the get the chiral atom in the molecule being queried
        OBAtom *center = mol.GetAtom((*m)[j]);

        // get the OBTetrahedralStereo::Config from the molecule
        OBStereoFacade stereo(&mol);
        OBTetrahedralStereo *ts = stereo.GetTetrahedralStereo(center->GetId());
        if (!ts || !ts->GetConfig().specified) {
          // no stereochemistry specified in molecule for the atom
          // corresponding to the chiral pattern atom using the current
          // mapping --> no match
          allStereoCentersMatch = false;
          break;
        }

        std::vector<int> nbrs = pat->atom[j].nbrs;

        if (nbrs.size() != 4) { // 3 nbrs currently not supported. Other values are errors.
          //stringstream ss;
          //ss << "Ignoring stereochemistry. There are " << nbrs.size() << " connections to this atom instead of 4. Title: " << mol.GetTitle();
          //obErrorLog.ThrowError(__FUNCTION__, ss.str(), obWarning);
          continue;
        }

        // construct a OBTetrahedralStereo::Config using the smarts pattern
        OBTetrahedralStereo::Config smartsConfig;
        smartsConfig.center = center->GetId();
        if (nbrs.at(0) == SmartsImplicitRef)
          smartsConfig.from = OBStereo::ImplicitRef;
        else
          smartsConfig.from = mol.GetAtom( (*m)[nbrs.at(0)] )->GetId();
        OBStereo::Ref firstref;
        if (nbrs.at(1) == SmartsImplicitRef)
          firstref = OBStereo::ImplicitRef;
        else
          firstref = mol.GetAtom( (*m)[nbrs.at(1)] )->GetId();
        OBAtom *ra2 = mol.GetAtom( (*m)[nbrs.at(2)] );
        OBAtom *ra3 = mol.GetAtom( (*m)[nbrs.at(3)] );
        smartsConfig.refs = OBStereo::MakeRefs(firstref, ra2->GetId(), ra3->GetId());

        smartsConfig.view = OBStereo::ViewFrom;
        switch (pat->atom[j].chiral_flag) {
          case AL_CLOCKWISE:
            smartsConfig.winding = OBStereo::Clockwise;
            break;
          case AL_ANTICLOCKWISE:
            smartsConfig.winding = OBStereo::AntiClockwise;
            break;
          default:
            smartsConfig.specified = false;
        }

        // cout << "smarts config = " << smartsConfig << endl;
        // cout << "molecule config = " << ts->GetConfig() << endl;
        // cout << "match = " << (ts->GetConfig() == smartsConfig) << endl;

        // and save the match if the two configurations are the same
        if (ts->GetConfig() != smartsConfig)
          allStereoCentersMatch = false;

        // don't waste time checking more stereocenters using this mapping if one didn't match
        if (!allStereoCentersMatch)
          break;
      }

      // if all the atoms in the molecule match the stereochemistry specified
      // in the smarts pattern, save this mapping as a match
      if (allStereoCentersMatch)
        tmpmlist.push_back(*m);
    }

    mlist = tmpmlist;
  }
  */

  return(!mlist.empty());
}

#ifdef HAVE_PYTHON

bool SmartsMatcher::Match(PyObject *obj, PythonSmartsPattern *pat,
    PyObject *mlist, bool single)
{
  if (!PyObject_HasAttrString(obj, "this")) {
    std::cout << "SmilesMatch::Match: mol argument is not a SWIG proxy." << std::endl;
    return false;
  }
  PySwigObject *swigObject = reinterpret_cast<PySwigObject*>(PyObject_GetAttrString(obj, "this"));
  if (std::string(swigObject->ty->name) != "_p_OpenBabel__OBMol") {
    std::cout << "SmilesMatch::Match: mol argument has wrong type." << std::endl;
    return false;
  }

  if (!PyList_Check(mlist))
    std::cout << "SmilesMatch::Match: mlist argument is not a python list." << std::endl;

  OBMol *mol = static_cast<OBMol*>(swigObject->ptr);

  std::vector<std::vector<int> > mappings;
  bool result = Match(*mol, pat, mappings, single); 

  for (std::size_t i = 0; i < mappings.size(); ++i) {
    PyObject *list = PyList_New(0);
    PyList_Append(mlist, list);
    for (std::size_t j = 0; j < mappings[i].size(); ++j)
      PyList_Append(list, PyInt_FromLong(mappings[i][j]));
  }

  return result;
}

bool Match(const std::string &pythonFileOrModuleName, OpenBabel::OBMol &mol, const std::string &smarts, std::vector<std::vector<int> > &mlist, bool single)
{
  if (!Py_IsInitialized())
    Py_Initialize();

  std::string path, moduleName = pythonFileOrModuleName;
  if (moduleName.find(".py") != std::string::npos)
    moduleName.resize(moduleName.size() - 3);
  std::size_t forwardslash = moduleName.rfind("/");
  if (forwardslash != std::string::npos) {
    path = moduleName.substr(0, forwardslash);
    moduleName = moduleName.substr(forwardslash + 1);
  } else {
    std::size_t backwardslash = moduleName.rfind("\\");
    path = moduleName.substr(0, backwardslash);
    moduleName = moduleName.substr(backwardslash + 1);
  }

  PyRun_SimpleString("import sys\nsys.path.append('.')");
  if (path.size())
    PyRun_SimpleString(make_string("import sys\nsys.path.append('" + path + "')").c_str());
  PyErr_Print();

  PyObject *module = PyImport_ImportModule(moduleName.c_str());
  if (!module) {
    PyErr_Print();
    return false;
  }
  
  if (!PyObject_HasAttrString(module, "mol")) {
    std::cout << moduleName << " is not a valid SmartsCompiler module (no mol)." << std::endl;
    return false;
  }
  
  PyObject *proxy = PyObject_GetAttrString(module, "mol");
  if (!PyObject_HasAttrString(proxy, "this")) {
    std::cout << moduleName << " is not a valid SmartsCompiler module (no mol.this)." << std::endl;
    return false;
  }

  PySwigObject *swigObject = reinterpret_cast<PySwigObject*>(PyObject_GetAttrString(proxy, "this"));
  if (std::string(swigObject->ty->name) != "_p_OpenBabel__OBMol") {
    std::cout << moduleName << " is not a valid SmartsCompiler module (mol type incorrect)." << std::endl;
    return false;
  }

  *static_cast<OBMol*>(swigObject->ptr) = mol;

  if (!PyObject_HasAttrString(module, "MatchGlobalMol")) {
    std::cout << moduleName << " is not a valid SmartsCompiler module (no MatchGlobalMol)." << std::endl;
    return false;
  }
  
  PyObject *match = PyObject_GetAttrString(module, "MatchGlobalMol");
  if (!PyFunction_Check(match)) {
    std::cout << moduleName << " is not a valid SmartsCompiler module (MatchGlobalMol is not a function)." << std::endl;
    return false;
  }

  PyObject *pymlist = PyList_New(0);
  //PyObject *pysmarts = PyString_FromString(smarts.c_str());
  //PyObject *pysingle = single ? Py_True : Py_False;

  PyObject *result = PyEval_CallFunction(match, "sOb", smarts.c_str(), pymlist, single);
  PyErr_Print();

  return result == Py_True;
}

template bool SmartsMatcher::Match<PythonSmartsPattern>(OpenBabel::OBMol &mol, PythonSmartsPattern *pat, std::vector<std::vector<int> > &mlist, bool single = false);

#endif

template bool SmartsMatcher::Match<SmartsPattern>(OpenBabel::OBMol &mol, SmartsPattern *pat, std::vector<std::vector<int> > &mlist, bool single = false);

}
