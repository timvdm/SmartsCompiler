#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

using namespace OpenBabel;

int main(int argc, char **argv)
{
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <molecule_file> <output_score_file>" << std::endl;
    return 1;
  }

  unsigned long numAtoms = 0;
  unsigned long numAromaticAtoms = 0;
  unsigned long numCyclicAtoms = 0;
  std::map<int, unsigned long> mass;
  std::map<int, unsigned long> elem;
  std::map<int, unsigned long> aromelem;
  std::map<int, unsigned long> aliphelem;
  std::map<int, unsigned long> hcount;
  std::map<int, unsigned long> charge;
  std::map<int, unsigned long> connect;
  std::map<int, unsigned long> degree;
  std::map<int, unsigned long> implicit;
  std::map<int, unsigned long> rings;
  std::map<int, unsigned long> size;
  std::map<int, unsigned long> valence;
  std::map<int, unsigned long> chiral;
  std::map<int, unsigned long> hyb;
  std::map<int, unsigned long> ringconnect;

  unsigned long numBonds = 0;
  unsigned long numSingleBonds = 0;
  unsigned long numDoubleBonds = 0;
  unsigned long numTripleBonds = 0;
  unsigned long numAromaticBonds = 0;
  unsigned long numRingBonds = 0;

  OBConversion conv;
  conv.SetInFormat(conv.FormatFromExt(argv[1]));
  std::ifstream ifs(argv[1]);
  conv.SetInStream(&ifs);

  OBMol mol;
  unsigned long molecule = 0;
  while (conv.Read(&mol)) {
    ++molecule;
    //if ((molecule % 1000) == 0)
    //  std::cout << molecule << std::endl;

    FOR_ATOMS_OF_MOL (atom, mol) {
      numAtoms++;
      if (atom->IsAromatic()) {
        numAromaticAtoms++;
        aromelem[atom->GetAtomicNum()]++;
      } else
        aliphelem[atom->GetAtomicNum()]++;
      if (atom->IsInRing())
        numCyclicAtoms++;
      mass[atom->GetIsotope()]++;
      elem[atom->GetAtomicNum()]++;
      hcount[atom->ExplicitHydrogenCount() + atom->ImplicitHydrogenCount()]++;
      charge[atom->GetFormalCharge()]++;
      connect[atom->GetImplicitValence()]++;
      degree[atom->GetValence()]++;
      implicit[atom->ImplicitHydrogenCount()]++;
      rings[atom->MemberOfRingCount()]++;
      for (int i = 3; i < 25; ++i)
        if (atom->IsInRingSize(i))
          size[i]++;
      valence[atom->KBOSum() - (atom->GetSpinMultiplicity() ? atom->GetSpinMultiplicity() - 1 : 0)]++;
      hyb[atom->GetHyb()]++;
      ringconnect[atom->CountRingBonds()]++;    
    }

    FOR_BONDS_OF_MOL (bond, mol) {
      numBonds++;
      if (bond->IsSingle())
        numSingleBonds++;
      else if (bond->IsDouble())
        numDoubleBonds++;
      else if (bond->IsTriple())
        numTripleBonds++;
      if (bond->IsAromatic())
        numAromaticBonds++;
      if (bond->IsInRing())
        numRingBonds++;;
    }
  }


  unsigned long numAliphaticAtoms = numAtoms - numAromaticAtoms;
  unsigned long numAcyclicAtoms = numAtoms - numCyclicAtoms;

  std::ofstream ofs(argv[2]);

  ofs << "# atoms: " << numAtoms << std::endl;
  ofs << "# bonds: " << numBonds << std::endl;
  ofs << "AE_AROMATIC  a: " << numAromaticAtoms << std::endl;
  ofs << "AE_ALIPHATIC  A: " << numAliphaticAtoms << std::endl;
  ofs << "AE_CYCLIC  R: " << numCyclicAtoms << std::endl;
  ofs << "AE_ACYCLIC  R0: " << numAcyclicAtoms << std::endl;
  for (std::map<int, unsigned long>::iterator i = mass.begin(); i != mass.end(); ++i)
    ofs << "AE_MASS  " << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = elem.begin(); i != elem.end(); ++i)
    ofs << "AE_ELEM  " << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = aromelem.begin(); i != aromelem.end(); ++i)
    ofs << "AE_AROMELEM  " << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = aliphelem.begin(); i != aliphelem.end(); ++i)
    ofs << "AE_ALIPHELEM  " << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = hcount.begin(); i != hcount.end(); ++i)
    ofs << "AE_HCOUNT  H" << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = charge.begin(); i != charge.end(); ++i)
    ofs << "AE_CHARGE  " << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = connect.begin(); i != connect.end(); ++i)
    ofs << "AE_CONNECT  X" << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = degree.begin(); i != degree.end(); ++i)
    ofs << "AE_DEGREE  D" << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = implicit.begin(); i != implicit.end(); ++i)
    ofs << "AE_IMPLICIT  h" << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = rings.begin(); i != rings.end(); ++i)
    ofs << "AE_RINGS  R" << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = size.begin(); i != size.end(); ++i)
    ofs << "AE_SIZE  r" << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = valence.begin(); i != valence.end(); ++i)
    ofs << "AE_VALENCE  v" << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = hyb.begin(); i != hyb.end(); ++i)
    ofs << "AE_HYB  ^" << i->first << ": " << i->second << std::endl;
  for (std::map<int, unsigned long>::iterator i = ringconnect.begin(); i != ringconnect.end(); ++i)
    ofs << "AE_RINGCONNECT  x" << i->first << ": " << i->second << std::endl;

  ofs << "BE_SINGLE : " << numSingleBonds << std::endl;
  ofs << "BE_DOUBLE : " << numDoubleBonds << std::endl;
  ofs << "BE_TRIPLE : " << numTripleBonds << std::endl;
  ofs << "BE_AROM : " << numAromaticBonds << std::endl;
  ofs << "BE_RING : " << numRingBonds << std::endl;
}
