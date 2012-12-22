#include "smartsscores.h"
#include "defines.h"
#include "util.h"

#include <cassert>
#include <fstream>

namespace SC {

  double PrettySmartsScores::GetExprScore(const SmartsAtomExpr *expr)
  {
    switch (expr->type) {
      case Smiley::OP_AndHi:
      case Smiley::OP_AndLo:
        return std::min(GetExprScore(expr->binary.lft), GetExprScore(expr->binary.rgt));
      case Smiley::OP_Or:
        return std::min(GetExprScore(expr->binary.lft), GetExprScore(expr->binary.rgt));
      //case Smiley::AE_RECUR:
      //  return 1.0;
      case Smiley::OP_Not:
        return GetExprScore(expr->unary.arg);
      case Smiley::AE_True:
        return 0.0;
      case Smiley::AE_False:
        return 0.0;
      case Smiley::AE_Isotope:
        return 0.1 + expr->leaf.value * 0.0001;
      case Smiley::AE_Aromatic:
        return 0.2;
      case Smiley::AE_Aliphatic:
        return 0.21;
      case Smiley::AE_AtomicNumber:
        return 0.3 + expr->leaf.value * 0.0001;
      case Smiley::AE_AromaticElement:
        return 0.3 + expr->leaf.value * 0.0001;
      case Smiley::AE_AliphaticElement:
        return 0.3 + expr->leaf.value * 0.0001;
      case Smiley::AE_Chirality:
        return 0.4;
      case Smiley::AE_Connectivity:
        return 0.5 + expr->leaf.value * 0.0001;
      case Smiley::AE_Degree:
        return 0.51 + expr->leaf.value * 0.0001;
      case Smiley::AE_Valence:
        return 0.52 + expr->leaf.value * 0.0001;
      //case Smiley::AE_HYB:
      //  return 0.53 + expr->leaf.value * 0.0001;
      case Smiley::AE_Cyclic:
        return 0.6;
      case Smiley::AE_Acyclic:
        return 0.61;
      case Smiley::AE_RingMembership:
        return 0.62 + expr->leaf.value * 0.0001;
      case Smiley::AE_RingSize:
        return 0.63 + expr->leaf.value * 0.0001;
      case Smiley::AE_RingConnectivity:
        return 0.64 + expr->leaf.value * 0.0001;
      case Smiley::AE_ImplicitH:
        return 0.7 + expr->leaf.value * 0.0001;
      case Smiley::AE_TotalH:
        return 0.71 + expr->leaf.value * 0.0001;
      case Smiley::AE_Charge:
        if (expr->leaf.value < 0)
          return 0.8 - expr->leaf.value * 0.0001;
        return 0.81 + expr->leaf.value * 0.0001;
      default:
        return 1.0;
    }
  }

  double PrettySmartsScores::GetExprScore(const SmartsBondExpr *expr)
  {
    switch (expr->type) {
      case Smiley::OP_AndHi:
      case Smiley::OP_AndLo:
        return std::min(GetExprScore(expr->binary.lft), GetExprScore(expr->binary.rgt));
      case Smiley::OP_Or:
        return std::min(GetExprScore(expr->binary.lft), GetExprScore(expr->binary.rgt));
      case Smiley::OP_Not:
        return GetExprScore(expr->unary.arg);
      case Smiley::BE_True:
        return 0.0;
      case Smiley::BE_Single:
        return 0.1;
      case Smiley::BE_Double:
        return 0.2;
      case Smiley::BE_Triple:
        return 0.3;
      case Smiley::BE_Aromatic:
        return 0.4;
      case Smiley::BE_Ring:
        return 0.5;
      default:
        return 1.0;
    }
  }

  void PrettySmartsScores::Sort(std::vector<SmartsAtom*> &list, bool increasing)
  {
    std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsAtom, std::less>(this));
  }

  void PrettySmartsScores::Sort(std::vector<SmartsBond*> &list, bool increasing)
  {
    std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsBond, std::less>(this));
  }

  void PrettySmartsScores::Sort(std::vector<SmartsAtomExpr*> &list, bool increasing)
  {
    std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsAtomExpr, std::less>(this));
  }

  void PrettySmartsScores::Sort(std::vector<SmartsBondExpr*> &list, bool increasing)
  {
    std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsBondExpr, std::less>(this));
  }



  double ListSmartsScores::EnvironmentScoreDFS(const Smarts *pattern, const SmartsBond *bond, int radius, int depth)
  {
    ++depth;
    if (depth == radius)
      return 0.0;

    SmartsAtomExpr *nbr = pattern->atoms[bond->target].expr;

    double score = GetExprScore(bond->expr) * GetExprScore(nbr) / depth;

    for (int i = 0; i < pattern->bonds.size(); ++i)
      if (pattern->bonds[i].source == bond->target) {
        score += EnvironmentScoreDFS(pattern, &pattern->bonds[i], radius, depth);
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
      if (line.find("Smiley::AE_AROMATIC") != std::string::npos) {
        m_numAromaticAtoms = string2number<unsigned long>(line.substr(16));
        continue;
      }
      if (line.find("Smiley::AE_ALIPHATIC") != std::string::npos) {
        m_numAliphaticAtoms = string2number<unsigned long>(line.substr(17));
        continue;
      }
      if (line.find("Smiley::AE_CYCLIC") != std::string::npos) {
        m_numCyclicAtoms = string2number<unsigned long>(line.substr(14));
        continue;
      }
      if (line.find("Smiley::AE_ACYCLIC") != std::string::npos) {
        m_numAcyclicAtoms = string2number<unsigned long>(line.substr(16));
        continue;
      }
      if (line.find("Smiley::AE_MASS") != std::string::npos) {
        m_mass[string2number<int>(line.substr(9, line.find(":") - 9))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_ELEM") != std::string::npos) {
        m_elem[string2number<int>(line.substr(9, line.find(":") - 9))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_AROMELEM") != std::string::npos) {
        m_aromelem[string2number<int>(line.substr(13, line.find(":") - 13))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_ALIPHELEM") != std::string::npos) {
        m_aliphelem[string2number<int>(line.substr(14, line.find(":") - 14))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_HCOUNT") != std::string::npos) {
        m_hcount[string2number<int>(line.substr(12, line.find(":") - 12))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_CHARGE") != std::string::npos) {
        m_charge[string2number<int>(line.substr(11, line.find(":") - 11))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_CONNECT") != std::string::npos) {
        m_connect[string2number<int>(line.substr(13, line.find(":") - 13))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_DEGREE") != std::string::npos) {
        m_degree[string2number<int>(line.substr(12, line.find(":") - 12))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_IMPLICIT") != std::string::npos) {
        m_implicit[string2number<int>(line.substr(14, line.find(":") - 14))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_RINGS") != std::string::npos) {
        m_rings[string2number<int>(line.substr(11, line.find(":") - 11))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_SIZE") != std::string::npos) {
        m_size[string2number<int>(line.substr(10, line.find(":") - 10))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_VALENCE") != std::string::npos) {
        m_valence[string2number<int>(line.substr(13, line.find(":") - 13))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_HYB") != std::string::npos) {
        m_hyb[string2number<int>(line.substr(9, line.find(":") - 9))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::AE_RINGCONNECT") != std::string::npos) {
        m_ringconnect[string2number<int>(line.substr(17, line.find(":") - 17))] = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::BE_SINGLE") != std::string::npos) {
        m_numSingleBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::BE_DOUBLE") != std::string::npos) {
        m_numDoubleBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::BE_TRIPLE") != std::string::npos) {
        m_numTripleBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::BE_AROM") != std::string::npos) {
        m_numAromaticBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
      if (line.find("Smiley::BE_RING") != std::string::npos) {
        m_numRingBonds = string2number<unsigned long>(line.substr(line.find(":") + 2));
        continue;
      }
    }
  }

  double ListSmartsScores::GetExprScore(const SmartsAtomExpr *expr)
  {
    switch (expr->type) {
      case Smiley::OP_AndHi:
      case Smiley::OP_AndLo:
        return std::min(GetExprScore(expr->binary.lft), GetExprScore(expr->binary.rgt));
      case Smiley::OP_Or:
        return std::max(GetExprScore(expr->binary.lft), GetExprScore(expr->binary.rgt));
      //case Smiley::OP_RECUR:
      //  return 0.0;
      case Smiley::OP_Not:
        return 1.0 - GetExprScore(expr->unary.arg);
      case Smiley::AE_True:
        return 1.0;
      case Smiley::AE_False:
        return 0.0;
      case Smiley::AE_Aromatic:
        return m_numAromaticAtoms / static_cast<double>(m_numAtoms);
      case Smiley::AE_Aliphatic:
        return m_numAliphaticAtoms / static_cast<double>(m_numAtoms);
      case Smiley::AE_Cyclic:
        return m_numCyclicAtoms / static_cast<double>(m_numAtoms);
      case Smiley::AE_Acyclic:
        return m_numAcyclicAtoms / static_cast<double>(m_numAtoms);
      case Smiley::AE_Isotope:
        return m_mass[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_AtomicNumber:
        return m_elem[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_AromaticElement:
        return m_aromelem[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_AliphaticElement:
        return m_aliphelem[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_TotalH:
        return m_hcount[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_Charge:
        return m_charge[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_Connectivity:
        return m_connect[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_Degree:
        return m_degree[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_ImplicitH:
        return m_implicit[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_RingMembership:
        return m_rings[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_RingSize:
        return m_size[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_Valence:
        return m_valence[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_Chirality:
        return 1.0;
      //case Smiley::AE_HYB:
      //  return m_hyb[expr->leaf.value] / static_cast<double>(m_numAtoms);
      case Smiley::AE_RingConnectivity:
        return m_ringconnect[expr->leaf.value] / static_cast<double>(m_numAtoms);
      default:
        return 1.0;
    }
  }

  double ListSmartsScores::GetExprScore(const SmartsBondExpr *expr)
  {
    switch (expr->type) {
      case Smiley::OP_AndHi:
      case Smiley::OP_AndLo:
        return std::min(GetExprScore(expr->binary.lft), GetExprScore(expr->binary.rgt));
      case Smiley::OP_Or:
        return std::max(GetExprScore(expr->binary.lft), GetExprScore(expr->binary.rgt));
      case Smiley::OP_Not:
        return 1.0 - GetExprScore(expr->unary.arg);
      case Smiley::BE_True:
        return 1.0;
      case Smiley::BE_Single:
        return m_numSingleBonds / static_cast<double>(m_numBonds);
      case Smiley::BE_Double:
        return m_numDoubleBonds / static_cast<double>(m_numBonds);
      case Smiley::BE_Triple:
        return m_numTripleBonds / static_cast<double>(m_numBonds);
      case Smiley::BE_Aromatic:
        return m_numAromaticBonds / static_cast<double>(m_numBonds);
      case Smiley::BE_Ring:
        return m_numRingBonds / static_cast<double>(m_numBonds);
      default:
        return 1.0;
    }
  }

  double ListSmartsScores::GetExprEnvironmentScore(const Smarts *pattern, const SmartsAtomExpr *expr, int radius)
  {
    double score = GetExprScore(expr);
    int atomIndex;
    for (int i = 0; i < pattern->atoms.size(); ++i)
      if (pattern->atoms[i].expr == expr) {
        atomIndex = i;
        break;
      }

    for (int i = 0; i < pattern->bonds.size(); ++i)
      if (pattern->bonds[i].source == atomIndex) {
        score += EnvironmentScoreDFS(pattern, &pattern->bonds[i], radius, 0);
      }

    return score;
  }

  double ListSmartsScores::GetExprEnvironmentScore(const Smarts *pattern, const SmartsBondExpr *expr, int radius)
  {
    for (int i = 0; i < pattern->bonds.size(); ++i)
      if (pattern->bonds[i].expr == expr)
        return EnvironmentScoreDFS(pattern, &pattern->bonds[i], radius, 0);
    return 0.0;
  }

  void ListSmartsScores::Sort(std::vector<SmartsAtom*> &list, bool increasing)
  {
    if (increasing)
      std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsAtom, std::less>(this));
    else
      std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsAtom, std::greater>(this));
  }

  void ListSmartsScores::Sort(std::vector<SmartsBond*> &list, bool increasing)
  {
    if (increasing)
      std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsBond, std::less>(this));
    else
      std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsBond, std::greater>(this));
  }

  void ListSmartsScores::Sort(std::vector<SmartsAtomExpr*> &list, bool increasing)
  {
    if (increasing)
      std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsAtomExpr, std::less>(this));
    else
      std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsAtomExpr, std::greater>(this));
  }

  void ListSmartsScores::Sort(std::vector<SmartsBondExpr*> &list, bool increasing)
  {
    if (increasing)
      std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsBondExpr, std::less>(this));
    else
      std::sort(list.begin(), list.end(), ScoreSortFunctor<SmartsBondExpr, std::greater>(this));
  }

}
