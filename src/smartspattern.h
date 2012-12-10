#ifndef SC_SMARTSPATTERN_H
#define SC_SMARTSPATTERN_H

#include "util.h"
#include "smarts.h"

#include <vector>

namespace SC {

  template<typename AtomType, typename BondType>
  struct SmartsPattern
  {
    bool CallEvalAtomExpr(int index, AtomType *atom) const;
    bool CallEvalBondExpr(int index, BondType *bond) const;


    int numAtoms;
    std::vector<SmartsBond> bonds;
    bool ischiral;

    bool (*EvalAtomExpr)(int, AtomType*);
    bool (*EvalBondExpr)(int, BondType*);
  };

}

#endif
