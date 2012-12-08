#include "../src/smartsassembler.h"
#include "../src/smartsvirtualmachine.h"

#include <fstream>

using namespace SC;

int main(int argc, char **argv)
{
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    return 1;  
  }

  std::ifstream ifs(argv[1]);
  std::ofstream ofs("a.out", std::ios_base::out | std::ios_base::binary);

  SmartsAssembler assembler;
  assembler.assemble(ifs, ofs);

  ifs.close();
  ofs.close();
  ifs.open("a.out", std::ios_base::in | std::ios_base::binary);

  SmartsVirtualMachine smartsvm;
  smartsvm.load(ifs);
  
  smartsvm.disassemble();


}
