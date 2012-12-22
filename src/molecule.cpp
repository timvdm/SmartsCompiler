#include "molecule.h"

namespace SC {

  void writeMolecule(std::ostream &os, OpenBabel::OBMol *mol)
  {
    int numAtoms = 0;
    FOR_ATOMS_OF_MOL (a, mol)
      if (!a->IsHydrogen())
        ++numAtoms;
    int numBonds = 0;
    FOR_BONDS_OF_MOL (b, mol)
      if (!b->GetBeginAtom()->IsHydrogen() && !b->GetEndAtom()->IsHydrogen())
        ++numBonds;


    os << numAtoms << " " << numBonds << std::endl;
    FOR_ATOMS_OF_MOL (a, mol) {
      if (a->IsHydrogen())
        continue;
      OpenBabelAtom atom(&*a);
      // TODO isInRingSize
      os << atom.isAromatic() << " ";
      os << atom.isCyclic() << " ";
      os << atom.element() << " ";
      os << atom.mass() << " ";
      os << atom.degree() << " ";
      os << atom.valence() << " ";
      os << atom.connectivity() << " ";
      os << atom.totalHydrogens() << " ";
      os << atom.implicitHydrogens() << " ";
      os << atom.ringMembership() << " ";
      os << atom.ringConnectivity() << " ";
      os << atom.charge() << " ";
      os << atom.atomClass() << std::endl;
    }

    FOR_BONDS_OF_MOL (b, mol) {
      if (b->GetBeginAtom()->IsHydrogen() || b->GetEndAtom()->IsHydrogen())
        continue;
      OpenBabelBond bond(&*b);
      os << b->GetBeginAtom()->GetIndex() << " ";
      os << b->GetEndAtom()->GetIndex() << " ";
      os << bond.isAromatic() << " ";
      os << bond.isCyclic() << " ";
      os << bond.order() << std::endl;
    }
  }

  bool readMolecule(std::istream &is, Molecule &mol)
  {
    mol.m_atoms.clear();
    mol.m_atomPtrs.clear();
    mol.m_bonds.clear();

    int numAtoms, numBonds;
    if (!(is >> numAtoms >> numBonds))
      return false;

    mol.m_atoms.reserve(numAtoms);
    mol.m_atomPtrs.reserve(numAtoms);
    mol.m_bonds.reserve(numBonds);

    bool aromatic, cyclic;
    int element, mass, degree, valence, connectivity, totalH, implicitH;
    int ringMembership, ringConnectivity, charge, atomClass;
    for (int i = 0; i < numAtoms; ++i) {
      is >> aromatic >> cyclic;
      is >> element >> mass >> degree >> valence;
      is >> connectivity >> totalH >> implicitH;
      is >> ringMembership >> ringConnectivity;
      is >> charge >> atomClass;
      std::vector<int> ringSizes;

      mol.m_atoms.push_back(Atom(i, ringSizes, aromatic, cyclic,
                                 element, mass, degree, valence, connectivity,
                                 totalH, implicitH, ringMembership,
                                 ringConnectivity, charge, atomClass));
      mol.m_atomPtrs.push_back(&mol.m_atoms.back());
    }

    int source, target, order;
    for (int i = 0; i < numBonds; ++i) {
      is >> source >> target >> aromatic;
      is >> cyclic >> order;

      Atom *s = mol.m_atomPtrs[source];
      Atom *t = mol.m_atomPtrs[target];

      mol.m_bonds.push_back(Bond(s, t, aromatic, cyclic, order));

      s->addBond(&mol.m_bonds.back());
      t->addBond(&mol.m_bonds.back());
    }

    return is;
  }

}
