#ifndef SC_SMARTSSCORES_H
#define SC_SMARTSSCORES_H

#include "smarts.h"

namespace SC {

  class SmartsScores
  {
    public:
      virtual ~SmartsScores()
      {
      }

      virtual double GetExprScore(const SmartsAtomExpr *expr)
      {
        return 1.0;
      }

      virtual double GetExprScore(const SmartsBondExpr *expr)
      {
        return 1.0;
      }

      virtual double GetExprEnvironmentScore(const Smarts *pattern, const SmartsAtomExpr *expr, int radius)
      {
        return 1.0;
      }

      virtual double GetExprEnvironmentScore(const Smarts *pattern, const SmartsBondExpr *expr, int radius)
      {
        return 1.0;
      }

      virtual void Sort(std::vector<SmartsAtom*> &list, bool increasing = true)
      {
      }

      virtual void Sort(std::vector<SmartsBond*> &list, bool increasing = true)
      {
      }

      virtual void Sort(std::vector<SmartsAtomExpr*> &list, bool increasing = true)
      {
      }

      virtual void Sort(std::vector<SmartsBondExpr*> &list, bool increasing = true)
      {
      }
  };

  template<typename AtomBondExpr, template<typename> class Compare>
  struct ScoreSortFunctor
  {
    ScoreSortFunctor(SmartsScores *scores_) : scores(scores_)
    {
    }

    const SmartsAtomExpr* GetExpr(const SmartsAtom *atom) const { return atom->expr; }
    const SmartsBondExpr* GetExpr(const SmartsBond *bond) const { return bond->expr; }
    const SmartsAtomExpr* GetExpr(const SmartsAtomExpr *expr) const { return expr; }
    const SmartsBondExpr* GetExpr(const SmartsBondExpr *expr) const { return expr; }

    bool operator()(const AtomBondExpr *left, const AtomBondExpr *right) const
    {
      return compare(scores->GetExprScore(GetExpr(left)), scores->GetExprScore(GetExpr(right)));
    }


    SmartsScores *scores;
    Compare<double> compare;
  };

  class PrettySmartsScores : public SmartsScores
  {
    public:
      virtual double GetExprScore(const SmartsAtomExpr *expr);
      virtual double GetExprScore(const SmartsBondExpr *expr);
      virtual void Sort(std::vector<SmartsAtom*> &list, bool increasing = true);
      virtual void Sort(std::vector<SmartsBond*> &list, bool increasing = true);
      virtual void Sort(std::vector<SmartsAtomExpr*> &list, bool increasing = true);
      virtual void Sort(std::vector<SmartsBondExpr*> &list, bool increasing = true);
  };


  class ListSmartsScores : public SmartsScores
  {
    public:
      ListSmartsScores(const std::string &filename);
      double GetExprScore(const SmartsAtomExpr *expr);
      double GetExprScore(const SmartsBondExpr *expr);
      double GetExprEnvironmentScore(const Smarts *pattern, const SmartsAtomExpr *expr, int radius);
      double GetExprEnvironmentScore(const Smarts *pattern, const SmartsBondExpr *expr, int radius);
      virtual void Sort(std::vector<SmartsAtom*> &list, bool increasing = true);
      virtual void Sort(std::vector<SmartsBond*> &list, bool increasing = true);
      virtual void Sort(std::vector<SmartsAtomExpr*> &list, bool increasing = true);
      virtual void Sort(std::vector<SmartsBondExpr*> &list, bool increasing = true);

    private:
      double EnvironmentScoreDFS(const Smarts *pattern, const SmartsBond *bond, int radius, int depth);

      unsigned long m_numAtoms;
      unsigned long m_numAromaticAtoms;
      unsigned long m_numCyclicAtoms;
      unsigned long m_numAliphaticAtoms;
      unsigned long m_numAcyclicAtoms;
      std::map<int, unsigned long> m_mass;
      std::map<int, unsigned long> m_elem;
      std::map<int, unsigned long> m_aromelem;
      std::map<int, unsigned long> m_aliphelem;
      std::map<int, unsigned long> m_hcount;
      std::map<int, unsigned long> m_charge;
      std::map<int, unsigned long> m_connect;
      std::map<int, unsigned long> m_degree;
      std::map<int, unsigned long> m_implicit;
      std::map<int, unsigned long> m_rings;
      std::map<int, unsigned long> m_size;
      std::map<int, unsigned long> m_valence;
      std::map<int, unsigned long> m_chiral;
      std::map<int, unsigned long> m_hyb;
      std::map<int, unsigned long> m_ringconnect;

      unsigned long m_numBonds;
      unsigned long m_numSingleBonds;
      unsigned long m_numDoubleBonds;
      unsigned long m_numTripleBonds;
      unsigned long m_numAromaticBonds;
      unsigned long m_numRingBonds;
  };

}
    
#endif
