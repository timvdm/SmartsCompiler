#include "../src/smartsassembler.h"
#include "../src/smartsvirtualmachine.h"
#include "../src/openbabel.h"

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <fstream>

#include "test.h"

using namespace OpenBabel;

void readSmiles(const std::string &smiles, OBMol &mol)
{
  OBConversion conv;
  conv.SetInFormat("smi");
  conv.ReadString(&mol, smiles);
}

using namespace SC;

void test_matchAtom(const std::string &filename, const std::string &start, const std::string &smiles, int index, bool expected)
{
  std::cout << "Testing: " << start << " with SMILES " << smiles << " and atom index " << index << std::endl;

  std::ifstream ifs(filename.c_str());
  std::ofstream ofs("a.out");

  SmartsAssembler assembler;
  assembler.assemble(ifs, ofs);

  ifs.close();
  ofs.close();
  ifs.open("a.out", std::ios_base::in | std::ios_base::binary);

  SmartsVirtualMachine smartsvm;
  smartsvm.load(ifs);
  
//  smartsvm.disassemble();

  OBMol mol;
  readSmiles(smiles, mol);

  SC_COMPARE(expected, smartsvm.matchAtom(start, OpenBabelAtom(mol.GetAtom(index))));
}

int main(int argc, char **argv)
{
  // arom
  test_matchAtom("../data/simple.sasm", "is_aromatic", "C1CCCCC1", 1, false);
  test_matchAtom("../data/simple.sasm", "is_aromatic", "c1ccccc1", 1, true);
  // elem
  test_matchAtom("../data/simple.sasm", "is_carbon", "CNO", 1, true);
  test_matchAtom("../data/simple.sasm", "is_carbon", "CNO", 2, false);
  test_matchAtom("../data/simple.sasm", "is_nitrogen", "CNO", 2, true);
  test_matchAtom("../data/simple.sasm", "is_nitrogen", "CNO", 1, false);
  // mass
  test_matchAtom("../data/simple.sasm", "mass_13", "C", 1, false);
  test_matchAtom("../data/simple.sasm", "mass_13", "[13CH4]", 1, true);
  // degree
  test_matchAtom("../data/simple.sasm", "degree_2", "CCC", 1, false);
  test_matchAtom("../data/simple.sasm", "degree_2", "CCC", 2, true);
  test_matchAtom("../data/simple.sasm", "degree_3", "CCC", 2, false);
  test_matchAtom("../data/simple.sasm", "degree_3", "CC(C)C", 2, true);
  // valence
  test_matchAtom("../data/simple.sasm", "valence_4", "C", 1, true);
  test_matchAtom("../data/simple.sasm", "valence_4", "O", 1, false);
  test_matchAtom("../data/simple.sasm", "valence_2", "O", 1, true);
  // connectivity
  test_matchAtom("../data/simple.sasm", "connectivity_4", "O", 1, false);
  test_matchAtom("../data/simple.sasm", "connectivity_4", "CCC", 1, true);
  test_matchAtom("../data/simple.sasm", "connectivity_4", "CCC", 2, true);
  test_matchAtom("../data/simple.sasm", "connectivity_4", "CCC", 2, true);
  test_matchAtom("../data/simple.sasm", "connectivity_4", "CC(C)C", 2, true);
  // totalh
  test_matchAtom("../data/simple.sasm", "totalh_4", "C", 1, true);
  test_matchAtom("../data/simple.sasm", "totalh_4", "N", 1, false);
  test_matchAtom("../data/simple.sasm", "totalh_3", "N", 1, true);
  // implicith
  test_matchAtom("../data/simple.sasm", "implh_4", "C", 1, true);
  test_matchAtom("../data/simple.sasm", "implh_4", "[CH4]", 1, false);
  // ring membership
  test_matchAtom("../data/simple.sasm", "ring_membership_1", "C", 1, false);
  test_matchAtom("../data/simple.sasm", "ring_membership_1", "C1CCCC1", 1, true);
  test_matchAtom("../data/simple.sasm", "ring_membership_2", "C1CCCC1", 1, false);
  test_matchAtom("../data/simple.sasm", "ring_membership_2", "C12CCCC1CCCCC2", 1, true);
  // ring size
  test_matchAtom("../data/simple.sasm", "ring_size_5", "C1CCCC1", 1, true);
  test_matchAtom("../data/simple.sasm", "ring_size_6", "C1CCCC1", 1, false);
  test_matchAtom("../data/simple.sasm", "ring_size_6", "C1CCCCC1", 1, true);
  // ring connectivity
  test_matchAtom("../data/simple.sasm", "ring_connectivity_2", "C", 1, false);
  test_matchAtom("../data/simple.sasm", "ring_connectivity_2", "C1CCCC1", 1, true);
  test_matchAtom("../data/simple.sasm", "ring_connectivity_3", "C1CCCC1", 1, false);
  test_matchAtom("../data/simple.sasm", "ring_connectivity_3", "C12CCCC1CCCCC2", 1, true);
  // charge
  test_matchAtom("../data/simple.sasm", "charge_neg1", "C", 1, false);
  test_matchAtom("../data/simple.sasm", "charge_neg1", "[O-]C", 1, true);
  test_matchAtom("../data/simple.sasm", "charge_pos1", "[NH3+]C", 1, true);
 

}
