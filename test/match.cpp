#include "../src/smarts.h"
#include "../src/smartsmatcher.h"

#include "test.h"

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

using namespace SC;
using namespace OpenBabel;

void readSmiles(const std::string &smiles, OBMol &mol)
{
  OBConversion conv;
  conv.SetInFormat("smi");
  conv.ReadString(&mol, smiles);
}

void TestMatch(const std::string &smarts, const std::string &smiles, bool expected)
{
  std::cout << "Testing: " << smarts << " in " << smiles << std::endl;
  // parse smarts
  Smarts *s = parse(smarts);
  // parse smiles
  OBMol mol;
  readSmiles(smiles, mol);

  NoMapping mapping;
  COMPARE(match(&mol, s, mapping), expected);
  
  delete s;
}

int main()
{
  ////////////////////////////////////////////////
  //
  // Primitives
  //
  ////////////////////////////////////////////////

  // Organic subset atoms
  TestMatch("C", "C", true);
  TestMatch("C", "N", false);

  // true 
  TestMatch("*", "C", true);
  TestMatch("[*]", "C", true);
  // false
  TestMatch("[!*]", "*", false);
  // aromatic
  TestMatch("a", "c1ccccc1", true);
  TestMatch("[a]", "c1ccccc1", true);
  TestMatch("a", "C1CCCCC1", false);
  TestMatch("[a]", "C1CCCCC1", false);
  // aliphatic
  TestMatch("A", "c1ccccc1", false);
  TestMatch("[A]", "c1ccccc1", false);
  TestMatch("A", "C1CCCCC1", true);
  TestMatch("[A]", "C1CCCCC1", true);
  // cyclic
  TestMatch("[R]", "C1CC1", true);
  TestMatch("[R]", "CCC", false);
  // acyclic
  TestMatch("[R0]", "C1CC1", false);
  TestMatch("[R0]", "CCC", true);
  TestMatch("[R0]", "C1CC1C", true);
  // isotope
  TestMatch("[13]", "[13C]", true);
  TestMatch("[13]", "C", false);
  // atomic number
  TestMatch("[#6]", "C", true);
  TestMatch("[#7]", "c1c[nH]cc1", true);
  TestMatch("[#6]", "N", false);
  TestMatch("[#7]", "c1ccccc1", false);
  // aromatic element
  TestMatch("[c]", "c1ccccc1", true);
  TestMatch("[n]", "c1ccccc1", false);
  // aliphatic element
  TestMatch("[C]", "C", true);
  TestMatch("[C]", "N", false);
  // degree
  TestMatch("[D0]", "C", true);
  TestMatch("[D1]", "C", false);
  TestMatch("[D1]", "CC", true);
  TestMatch("[D2]", "CCC", true);
  TestMatch("[D2]", "CC(C)C", false);
  TestMatch("[D3]", "CC(C)C", true);
  // valence
  TestMatch("[v4]", "C", true);
  TestMatch("[v3]", "C", false);
  TestMatch("[v4]", "C=C", true);
  // connectivity
  TestMatch("[X4]", "C", true);
  TestMatch("[X3]", "C", false);
  TestMatch("[X4]", "C=C", false);
  // total hydrogens
  TestMatch("[H4]", "C", true);
  TestMatch("[H3]", "C", false);
  // implicit hydrogens
  TestMatch("[h4]", "C", true);
  TestMatch("[h3]", "C", false);
  // ring membership
  TestMatch("[R1]", "CCC", false);
  TestMatch("[R1]", "C1CC1", true);
  TestMatch("[R1]", "C12CC1CC2", true);
  TestMatch("[R2]", "C12CC1CC2", true);
  // ring size
  TestMatch("[r3]", "CCC", false);
  TestMatch("[r3]", "C1CC1", true);
  TestMatch("[r4]", "C1CC1", false);
  // ring connectivity
  TestMatch("[x2]", "CCC", false);
  TestMatch("[x2]", "C1CC1", true);
  TestMatch("[x2]", "C12CC1CC2", true);
  TestMatch("[x3]", "C12CC1CC2", true);
  // charge
  TestMatch("[-]", "CC[O-]", true);
  TestMatch("[+]", "[NH4+]", true);
  TestMatch("[-]", "C", false);
  TestMatch("[+]", "C", false);
  // chirality TODO
  // atom class TODO
 
  ////////////////////////////////////////////////
  //
  // Simple chains
  //
  ////////////////////////////////////////////////

  // single bond
  TestMatch("CC", "C", false);
  TestMatch("CC", "CC", true);
  TestMatch("C-C", "CC", true);
  // aromatic bond
  TestMatch("cc", "c1ccccc1", true);
  TestMatch("c:c", "c1ccccc1", true);
  TestMatch("c-c", "c1ccccc1", false); // should match?
  TestMatch("c=c", "c1ccccc1", false); // should match?
  // bond orders 
  TestMatch("C=C", "C=C", true);
  TestMatch("C=C", "CC", false);
  TestMatch("C#C", "C#C", true);
  TestMatch("C#C", "CC", false);
  // ring bond
  TestMatch("C@C", "CCC", false);
  TestMatch("C@C", "C1CC1", true);
  TestMatch("C1CCC1", "C1CCC1", true);
  TestMatch("C1CCC1", "CCCC", false);
  TestMatch("C1CCC12CC2", "C1CCC12CC2", true);
  TestMatch("C1CCC12CC2", "C1CCC1CC", false);
  TestMatch("C1CCC12CC2", "C1CC1CCC", false);




}
