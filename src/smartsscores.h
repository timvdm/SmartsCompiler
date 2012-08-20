#ifndef SC_SMARTSSCORES_H
#define SC_SMARTSSCORES_H

#include <openbabel/parsmart.h>

namespace SC {

  class SmartsScores
  {
    public:
      virtual ~SmartsScores()
      {
      }

      virtual double GetExprScore(const OpenBabel::AtomExpr *expr)
      {
        return 1.0;
      }

      virtual double GetExprScore(const OpenBabel::BondExpr *expr)
      {
        return 1.0;
      }

      virtual double GetExprEnvironmentScore(const OpenBabel::Pattern *pattern, const OpenBabel::AtomExpr *expr, int radius)
      {
        return 1.0;
      }

      virtual double GetExprEnvironmentScore(const OpenBabel::Pattern *pattern, const OpenBabel::BondExpr *expr, int radius)
      {
        return 1.0;
      }

      virtual void Sort(std::vector<OpenBabel::AtomSpec*> &list, bool increasing = true)
      {
      }

      virtual void Sort(std::vector<OpenBabel::BondSpec*> &list, bool increasing = true)
      {
      }

      virtual void Sort(std::vector<OpenBabel::AtomExpr*> &list, bool increasing = true)
      {
      }

      virtual void Sort(std::vector<OpenBabel::BondExpr*> &list, bool increasing = true)
      {
      }
  };

  template<typename SpecOrExpr, template<typename> class Compare>
    struct ScoreSortFunctor
    {
      ScoreSortFunctor(SmartsScores *scores_) : scores(scores_)
      {
      }

      const OpenBabel::AtomExpr* GetExpr(const OpenBabel::AtomSpec *spec) const { return spec->expr; }
      const OpenBabel::BondExpr* GetExpr(const OpenBabel::BondSpec *spec) const { return spec->expr; }
      const OpenBabel::AtomExpr* GetExpr(const OpenBabel::AtomExpr *expr) const { return expr; }
      const OpenBabel::BondExpr* GetExpr(const OpenBabel::BondExpr *expr) const { return expr; }

      bool operator()(const SpecOrExpr *left, const SpecOrExpr *right) const
      {
        return compare(scores->GetExprScore(GetExpr(left)), scores->GetExprScore(GetExpr(right)));
      }


      SmartsScores *scores;
      Compare<double> compare;
    };

  class PrettySmartsScores : public SmartsScores
  {
    public:
      virtual double GetExprScore(const OpenBabel::AtomExpr *expr);
      virtual double GetExprScore(const OpenBabel::BondExpr *expr);
      virtual void Sort(std::vector<OpenBabel::AtomSpec*> &list, bool increasing = true);
      virtual void Sort(std::vector<OpenBabel::BondSpec*> &list, bool increasing = true);
      virtual void Sort(std::vector<OpenBabel::AtomExpr*> &list, bool increasing = true);
      virtual void Sort(std::vector<OpenBabel::BondExpr*> &list, bool increasing = true);
  };


  class ListSmartsScores : public SmartsScores
  {
    public:
      ListSmartsScores(const std::string &filename);
      double GetExprScore(const OpenBabel::AtomExpr *expr);
      double GetExprScore(const OpenBabel::BondExpr *expr);
      double GetExprEnvironmentScore(const OpenBabel::Pattern *pattern, const OpenBabel::AtomExpr *expr, int radius);
      double GetExprEnvironmentScore(const OpenBabel::Pattern *pattern, const OpenBabel::BondExpr *expr, int radius);
      virtual void Sort(std::vector<OpenBabel::AtomSpec*> &list, bool increasing = true);
      virtual void Sort(std::vector<OpenBabel::BondSpec*> &list, bool increasing = true);
      virtual void Sort(std::vector<OpenBabel::AtomExpr*> &list, bool increasing = true);
      virtual void Sort(std::vector<OpenBabel::BondExpr*> &list, bool increasing = true);

    private:
      double EnvironmentScoreDFS(const OpenBabel::Pattern *pattern, const OpenBabel::BondSpec *bond, int radius, int depth);

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
