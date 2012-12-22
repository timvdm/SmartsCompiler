#include "openbabel.h"

namespace SC {

  class Bond;

  class Atom
  {
    public:
      Atom(int index, const std::vector<int> &ringSizes, bool aromatic, bool cyclic, int element, 
           int mass, int degree, int valence, int connectivity, int totalH, int implicitH,
           int ringMembership, int ringConnectivity, int charge, int atomClass) : m_ringSizes(ringSizes),
           m_index(index), m_aromatic(aromatic), m_cyclic(cyclic), m_element(element),
           m_mass(mass), m_degree(degree), m_valence(valence), m_connectivity(connectivity),
           m_totalH(totalH), m_implicitH(implicitH), m_ringMembership(ringMembership),
           m_charge(charge), m_atomClass(atomClass)
      {
      }

      void addBond(Bond *bond)
      {
        m_bonds.push_back(bond);
      }

      std::vector<Bond*>::iterator beginBonds()
      {
        return m_bonds.begin();
      }

      std::vector<Bond*>::iterator endBonds()
      {
        return m_bonds.end();
      }

      int index() const
      {
        return m_index;
      }

      bool isAromatic() const
      {
        return m_aromatic;
      }

      bool isAliphatic() const
      {
        return !m_aromatic;
      }

      bool isCyclic() const
      {
        return m_cyclic;
      }

      bool isAcyclic() const
      {
        return !m_cyclic;
      }

      int element() const
      {
        return m_element;
      }

      int mass() const
      {
        return m_mass;
      }

      int degree() const
      {
        return m_degree;
      }

      int valence() const
      {
        return m_valence;
      }

      int connectivity() const
      {
        return m_connectivity;
      }

      int totalHydrogens() const
      {
        return m_totalH;
      }

      int implicitHydrogens() const
      {
        return m_implicitH;
      }

      int ringMembership() const
      {
        return m_ringMembership;
      }

      bool isInRingSize(int size) const
      {
        return std::find(m_ringSizes.begin(), m_ringSizes.end(), size) != m_ringSizes.end();
      }

      int ringConnectivity() const
      {
        return m_ringConnectivity;
      }

      int charge() const
      {
        return m_charge;
      }

      int atomClass() const
      {
        return m_atomClass;
      }

    private:
      std::vector<Bond*> m_bonds;
      std::vector<int> m_ringSizes;
      int m_index;
      bool m_aromatic;
      bool m_cyclic;
      int m_element;
      int m_mass;
      int m_degree;
      int m_valence;
      int m_connectivity;
      int m_totalH;
      int m_implicitH;
      int m_ringMembership;
      int m_ringConnectivity;
      int m_charge;
      int m_atomClass;
  };

  class AtomWrapper
  {
    public:
      AtomWrapper(Atom *atom) : m_atom(atom)
      {
      }

      bool isAromatic() const
      {
        return m_atom->isAromatic();
      }

      bool isAliphatic() const
      {
        return m_atom->isAliphatic();
      }

      bool isCyclic() const
      {
        return m_atom->isCyclic();
      }

      bool isAcyclic() const
      {
        return m_atom->isAcyclic();
      }

      int element() const
      {
        return m_atom->element();
      }

      int mass() const
      {
        return m_atom->mass();
      }

      int degree() const
      {
        return m_atom->degree();
      }

      int valence() const
      {
        return m_atom->valence();
      }

      int connectivity() const
      {
        return m_atom->connectivity();
      }

      int totalHydrogens() const
      {
        return m_atom->totalHydrogens();
      }

      int implicitHydrogens() const
      {
        return m_atom->implicitHydrogens();
      }

      int ringMembership() const
      {
        return m_atom->ringMembership();
      }

      bool isInRingSize(int size) const
      {
        return m_atom->isInRingSize(size);
      }

      int ringConnectivity() const
      {
        return m_atom->ringConnectivity();
      }

      int charge() const
      {
        return m_atom->charge();
      }

      int atomClass() const
      {
        return m_atom->atomClass();
      }

    private:
      Atom *m_atom;
  };


  class Bond
  {
    public:
      Bond(Atom *source, Atom *target, bool aromatic, bool cyclic, int order) :
           m_source(source), m_target(target), m_aromatic(aromatic), m_cyclic(cyclic),
           m_order(order)
      {
      }

      Atom* other(const Atom *atom) const
      {
        return m_source == atom ? m_target : m_source;
      }

      bool isAromatic() const
      {
        return m_aromatic;
      }

      bool isCyclic() const
      {
        return m_cyclic;
      }

      int order() const
      {
        return m_order;
      }

    private:
      Atom *m_source;
      Atom *m_target;
      bool m_aromatic;
      bool m_cyclic;
      int m_order;
  };

  class BondWrapper
  {
    public:
      BondWrapper(Bond *bond) : m_bond(bond)
      {
      }

      bool isAromatic() const
      {
        return m_bond->isAromatic();
      }

      bool isCyclic() const
      {
        return m_bond->isCyclic();
      }

      int order() const
      {
        return m_bond->order();
      }

    private:
      Bond *m_bond;
  };


  class Molecule
  {
    public:
      std::vector<Atom*>::iterator beginAtoms()
      {
        return m_atomPtrs.begin();
      }

      std::vector<Atom*>::iterator endAtoms()
      {
        return m_atomPtrs.end();
      }

    private:
      friend bool readMolecule(std::istream &is, Molecule &mol);

      std::vector<Atom> m_atoms;
      std::vector<Bond> m_bonds;
      std::vector<Atom*> m_atomPtrs;
  };

  template<>
  inline std::vector<Atom*>::iterator GetBeginAtoms<Molecule*, std::vector<Atom*>::iterator>(Molecule *mol)
  {
    return mol->beginAtoms();
  }
  template<>
  inline std::vector<Atom*>::iterator GetEndAtoms<Molecule*, std::vector<Atom*>::iterator>(Molecule *mol)
  {
    return mol->endAtoms();
  }
  template<>
  inline std::vector<Bond*>::iterator GetBeginBonds<Molecule*, Atom*, std::vector<Bond*>::iterator>(Molecule *mol, Atom *atom)
  {
    return atom->beginBonds();
  }
  template<>
  inline std::vector<Bond*>::iterator GetEndBonds<Molecule*, Atom*, std::vector<Bond*>::iterator>(Molecule *mol, Atom *atom)
  {
    return atom->endBonds();
  }

  template<>
  inline std::size_t GetAtomIndex<Molecule*, Atom*>(Molecule*, Atom *atom)
  {
    return atom->index();
  }

  template<>
  inline Atom* GetOtherAtom<Molecule*, Bond*, Atom*>(Molecule *mol, Bond *bond, Atom *atom)
  {
    return bond->other(atom);
  }
 
  template<>
  struct molecule_traits<Molecule>
  {
    typedef Atom* atom_arg_type;
    typedef Bond* bond_arg_type;

    typedef std::vector<Atom*>::iterator mol_atom_iterator_type;
    typedef std::vector<Bond*>::iterator atom_bond_iterator_type;

    typedef AtomWrapper atom_wrapper_type;
    typedef BondWrapper bond_wrapper_type;
  };

  void writeMolecule(std::ostream &os, OpenBabel::OBMol *mol);

  bool readMolecule(std::istream &is, Molecule &mol);

}
