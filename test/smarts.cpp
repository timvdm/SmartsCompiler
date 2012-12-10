#include "../src/smarts.h"
#include "../src/smartsprint.h"

#include "test.h"

using namespace SC;

void TestParseWrite(const std::string &smarts)
{
  std::cout << "Testing: " << smarts << std::endl;
  Smarts *s = parse(smarts);

  COMPARE(smarts, write(s));
  
  delete s;
}

int main()
{
  //
  // Test simple SMILES
  //

  // single organic subset atoms
  TestParseWrite("C");
  // chains
  TestParseWrite("CCC");
  TestParseWrite("CC=CC");
  TestParseWrite("CCOCC");
  // branch
  TestParseWrite("CC(C)C");
  // ring bonds
  TestParseWrite("C1CC1");
  TestParseWrite("c1ccccc1");


  TestParseWrite("[C!*]");



}
