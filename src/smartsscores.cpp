#include "smartsscores.h"
#include "defines.h"
#include "util.h"

#include <cassert>

namespace SC {

  using namespace OpenBabel;

  double PrettySmartsScores::GetExprScore(const AtomExpr *expr)
  {
    switch (expr->type) {
      case AE_ANDHI:
      case AE_ANDLO:
        return std::min(GetExprScore(expr->bin.lft), GetExprScore(expr->bin.rgt));
      case AE_OR:
        return std::min(GetExprScore(expr->bin.lft), GetExprScore(expr->bin.rgt));
      case AE_RECUR:
        return 1.0;
      case AE_NOT:
        return GetExprScore(expr->mon.arg);
      case AE_TRUE:
        return 0.0;
      case AE_FALSE:
        return 0.0;
      case AE_MASS:
        return 0.1 + expr->leaf.value * 0.0001;
      case AE_AROMATIC:
        return 0.2;
      case AE_ALIPHATIC:
        return 0.21;
      case AE_ELEM:
        return 0.3 + expr->leaf.value * 0.0001;
      case AE_AROMELEM:
        return 0.3 + expr->leaf.value * 0.0001;
      case AE_ALIPHELEM:
        return 0.3 + expr->leaf.value * 0.0001;
      case AE_CHIRAL:
        return 0.4;
      case AE_CONNECT:
        return 0.5 + expr->leaf.value * 0.0001;
      case AE_DEGREE:
        return 0.51 + expr->leaf.value * 0.0001;
      case AE_VALENCE:
        return 0.52 + expr->leaf.value * 0.0001;
      case AE_HYB:
        return 0.53 + expr->leaf.value * 0.0001;
      case AE_CYCLIC:
        return 0.6;
      case AE_ACYCLIC:
        return 0.61;
      case AE_RINGS:
        return 0.62 + expr->leaf.value * 0.0001;
      case AE_SIZE:
        return 0.63 + expr->leaf.value * 0.0001;
      case AE_RINGCONNECT:
        return 0.64 + expr->leaf.value * 0.0001;
      case AE_IMPLICIT:
        return 0.7 + expr->leaf.value * 0.0001;
      case AE_HCOUNT:
        return 0.71 + expr->leaf.value * 0.0001;
      case AE_CHARGE:
        if (expr->leaf.value < 0)
          return 0.8 - expr->leaf.value * 0.0001;
        return 0.81 + expr->leaf.value * 0.0001;
      default:
        return 1.0;
    }
  }

  double PrettySmartsScores::GetExprScore(const BondExpr *expr)
  {
    switch (expr->type) {
      case BE_ANDHI:
      case BE_ANDLO:
        return std::min(GetExprScore(expr->bin.lft), GetExprScore(expr->bin.rgt));
      case BE_OR:
        return std::min(GetExprScore(expr->bin.lft), GetExprScore(expr->bin.rgt));
      case BE_NOT:
        return GetExprScore(expr->mon.arg);
      case BE_ANY:
        return 0.0;
      case BE_DEFAULT:
        return 1.0;
      case BE_SINGLE:
        return 0.1;
      case BE_DOUBLE:
        return 0.2;
      case BE_TRIPLE:
        return 0.3;
      case BE_AROM:
        return 0.4;
      case BE_RING:
        return 0.5;
      default:
        return 1.0;
    }
  }

  void PrettySmartsScores::Sort(std::vector<AtomSpec*> &list, bool increasing)
  {
    std::sort(list.begin(), list.end(), ScoreSortFunctor<AtomSpec, std::less>(this));
  }

  void PrettySmartsScores::Sort(std::vector<BondSpec*> &list, bool increasing)
  {
    std::sort(list.begin(), list.end(), ScoreSortFunctor<BondSpec, std::less>(this));
  }

  void PrettySmartsScores::Sort(std::vector<AtomExpr*> &list, bool increasing)
  {
    std::sort(list.begin(), list.end(), ScoreSortFunctor<AtomExpr, std::less>(this));
  }

  void PrettySmartsScores::Sort(std::vector<BondExpr*> &list, bool increasing)
  {
    std::sort(list.begin(), list.end(), ScoreSortFunctor<BondExpr, std::less>(this));
  }



  double ListSmartsScores::EnvironmentScoreDFS(const Pattern *pattern, const BondSpec *bond, int radius, int depth)
  {
    ++depth;
    if (depth == radius)
      return 0.0;

    AtomExpr *nbr = pattern->atom[bond->dst].expr;

    double score = GetExprScore(bond->expr) * GetExprScore(nbr) / depth;

    int bondIndex = bond - pattern->bond;
    //for (int i = bondIndex + 1; i < pattern->bcount; ++i)
    for (int i = 0; i < pattern->bcount; ++i)
      if (pattern->bond[i].src == bond->dst) {
        score += EnvironmentScoreDFS(pattern, &pattern->bond[i], radius, depth);
      }

    return score;
  }

  ListSmartsScores::ListSmartsScores(const std::string &filename) : SmartsScores()
  {
    std::ifstream ifs(filename.c_str());
    std::string line;
    assert(std::getline(ifs, line));
    m_numAtoms = string2number<unsigned long>(line.substr(9));
    assert(std::getline(ifs, line));
    m_numBonds = string2number<unsigned long>(line.substr(9));

    while (std::getline(ifs, line)) {
      if (line.find("AE_AROMATIC") != std::string::npos) {
        m_numAromaticAtoms = string2number<unsigned long>(line.substr(16));
        continue;
      }
      if (line.find("AE_ALIPHATIC") != std::string::npos) {
        m_numAliphaticAtoms = string2number<unsigned long>(line.substr(17));
        continue;
      }
      if (line.find("AE_CYCLIC") != std::string::npos) {
        m_numCyclicAtoms = string2number<unsigned long>(line.substr(14));
        continue;
      }
      if (line.find("AE_ACYCLIC") != std::string::npos) {
        m_numAcyclicAtoms = string2number<unsigned long>(line.substr(16));
        continue;
      }
      if (line.find("AE_MASS") != std::string::npos) {
        m_mass[string2number<int>(line.substr(9, line.find(":") - 9))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_ELEM") != std::string::npos) {
        m_elem[string2number<int>(line.substr(9, line.find(":") - 9))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_AROMELEM") != std::string::npos) {
        m_aromelem[string2number<int>(line.substr(13, line.find(":") - 13))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_ALIPHELEM") != std::string::npos) {
        m_aliphelem[string2number<int>(line.substr(14, line.find(":") - 14))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_HCOUNT") != std::string::npos) {
        m_hcount[string2number<int>(line.substr(12, line.find(":") - 12))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_CHARGE") != std::string::npos) {
        m_charge[string2number<int>(line.substr(11, line.find(":") - 11))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_CONNECT") != std::string::npos) {
        m_connect[string2number<int>(line.substr(13, line.find(":") - 13))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_DEGREE") != std::string::npos) {
        m_degree[string2number<int>(line.substr(12, line.find(":") - 12))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_IMPLICIT") != std::string::npos) {
        m_implicit[string2number<int>(line.substr(14, line.find(":") - 14))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_RINGS") != std::string::npos) {
        m_rings[string2number<int>(line.substr(11, line.find(":") - 11))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_SIZE") != std::string::npos) {
        m_size[string2number<int>(line.substr(10, line.find(":") - 10))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_VALENCE") != std::string::npos) {
        m_valence[string2number<int>(line.substr(13, line.find(":") - 13))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_HYB") != std::string::npos) {
        m_hyb[string2number<int>(line.substr(9, line.find(":") - 9))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("AE_RINGCONNECT") != std::string::npos) {
        m_ringconnect[string2number<int>(line.substr(17, line.find(":") - 17))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("BE_SINGLE") != std::string::npos) {
        m_numSingleBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("BE_DOUBLE") != std::string::npos) {
        m_numDoubleBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("BE_TRIPLE") != std::string::npos) {
        m_numTripleBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("BE_AROM") != std::string::npos) {
        m_numAromaticBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("BE_RING") != std::string::npos) {
        m_numRingBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
    }
  }

  double ListSmartsScores::GetExprScore(const AtomExpr *expr)
  {
    switch (expr->type) {
      case AE_ANDHI:
      case AE_ANDLO:
        return std::min(GetExprScore(expr->bin.lft), GetExprScore(expr->bin.rgt));
      case AE_OR:
        return std::max(GetExprScore(expr->bin.lft), GetExprScore(expr->bin.rgt));
      case AE_RECUR:
        return 0.0;
      case AE_NOT:
        return 1.0 - GetExprScore(expr->mon.arg);
      case AE_TRUE:
        return 1.0;
      case AE_FALSE:
        return 0.0;
      case AE_AROMATIC:
        return m_numAromaticAtoms / static_cast<double>(m_numAtoms);
      case AE_ALIPHATIC:
        return m_numAliphaticAtoms / static_cast<double>(m_numAtoms);
      case AE_CYCLIC:
        return m_numCyclicAtoms / static_cast<double>(m_numAtoms);
      case AE_ACYCLIC:
        return m_numAcyclicAtoms / static_cast<double>(m_numAtoms);
      case AE_MASS:
        return m_mass[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_ELEM:
        return m_elem[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_AROMELEM:
        return m_aromelem[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_ALIPHELEM:
        return m_aliphelem[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_HCOUNT:
        return m_hcount[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_CHARGE:
        return m_charge[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_CONNECT:
        return m_connect[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_DEGREE:
        return m_degree[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_IMPLICIT:
        return m_implicit[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_RINGS:
        return m_rings[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_SIZE:
        return m_size[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_VALENCE:
        return m_valence[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_CHIRAL:
        return 1.0;
      case AE_HYB:
        return m_hyb[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case AE_RINGCONNECT:
        return m_ringconnect[expr->leaf.value] / static_cast<double>(m_numAtoms);
      default:
        return 1.0;
    }
  }

  double ListSmartsScores::GetExprScore(const BondExpr *expr)
  {
    switch (expr->type) {
      case BE_ANDHI:
      case BE_ANDLO:
        return std::min(GetExprScore(expr->bin.lft), GetExprScore(expr->bin.rgt));
      case BE_OR:
        return std::max(GetExprScore(expr->bin.lft), GetExprScore(expr->bin.rgt));
      case BE_NOT:
        return 1.0 - GetExprScore(expr->mon.arg);
      case BE_ANY:
        return 1.0;
      case BE_DEFAULT:
        return 1.0;
      case BE_SINGLE:
        return m_numSingleBonds / static_cast<double>(m_numBonds);
      case BE_DOUBLE:
        return m_numDoubleBonds / static_cast<double>(m_numBonds);
      case BE_TRIPLE:
        return m_numTripleBonds / static_cast<double>(m_numBonds);
      case BE_AROM:
        return m_numAromaticBonds / static_cast<double>(m_numBonds);
      case BE_RING:
        return m_numRingBonds / static_cast<double>(m_numBonds);
      default:
        return 1.0;
    }
  }

  double ListSmartsScores::GetExprEnvironmentScore(const Pattern *pattern, const AtomExpr *expr, int radius)
  {
    double score = GetExprScore(expr);
    int atomIndex;
    for (int i = 0; i < pattern->acount; ++i)
      if (pattern->atom[i].expr == expr) {
        atomIndex = i;
        break;
      }

    for (int i = 0; i < pattern->bcount; ++i)
      if (pattern->bond[i].src == atomIndex) {
        score += EnvironmentScoreDFS(pattern, &pattern->bond[i], radius, 0);
      }

    return score;
  }

  double ListSmartsScores::GetExprEnvironmentScore(const Pattern *pattern, const BondExpr *expr, int radius)
  {
    for (int i = 0; i < pattern->bcount; ++i)
      if (pattern->bond[i].expr == expr)
        return EnvironmentScoreDFS(pattern, &pattern->bond[i], radius, 0);
  }

  void ListSmartsScores::Sort(std::vector<AtomSpec*> &list, bool increasing)
  {
    if (increasing)
      std::sort(list.begin(), list.end(), ScoreSortFunctor<AtomSpec, std::less>(this));
    else
      std::sort(list.begin(), list.end(), ScoreSortFunctor<AtomSpec, std::greater>(this));
  }

  void ListSmartsScores::Sort(std::vector<BondSpec*> &list, bool increasing)
  {
    if (increasing)
      std::sort(list.begin(), list.end(), ScoreSortFunctor<BondSpec, std::less>(this));
    else
      std::sort(list.begin(), list.end(), ScoreSortFunctor<BondSpec, std::greater>(this));
  }

  void ListSmartsScores::Sort(std::vector<AtomExpr*> &list, bool increasing)
  {
    if (increasing)
      std::sort(list.begin(), list.end(), ScoreSortFunctor<AtomExpr, std::less>(this));
    else
      std::sort(list.begin(), list.end(), ScoreSortFunctor<AtomExpr, std::greater>(this));
  }

  void ListSmartsScores::Sort(std::vector<BondExpr*> &list, bool increasing)
  {
    if (increasing)
      std::sort(list.begin(), list.end(), ScoreSortFunctor<BondExpr, std::less>(this));
    else
      std::sort(list.begin(), list.end(), ScoreSortFunctor<BondExpr, std::greater>(this));
  }

}
