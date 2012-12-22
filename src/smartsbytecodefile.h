#ifndef SC_BYTECODEFILE_H
#define SC_BYTECODEFILE_H

namespace SC {

  class SmartsByteCodeFile
  {
    public:
      struct Symbol
      {
        Symbol() : address(0)
        {
        }

        Symbol(const std::string &lbl, Address addr)
            : label(lbl), address(addr)
        {
        }

        std::string label;
        Address address;
      };

      template<typename T>
      void write32(std::ostream &os, T value) const
      {
        unsigned int copy = value;
        os.write(reinterpret_cast<const char*>(&copy), sizeof(unsigned int));
      }

      template<typename T>
      void write16(std::ostream &os, T value) const
      {
        unsigned short copy = value;
        os.write(reinterpret_cast<const char*>(&copy), sizeof(unsigned short));
      }

      void read32(std::istream &is, unsigned int &value) const
      {
        is.read(reinterpret_cast<char *>(&value), sizeof(unsigned int));
      }

      void read16(std::istream &is, unsigned short &value) const
      {
        is.read(reinterpret_cast<char *>(&value), sizeof(unsigned short));
      }

      void write(std::ostream &os, const std::vector<Instruction> &instructions, const std::vector<Symbol> &symbols)
      {
        // write the number of instructions
        write32(os, instructions.size());
        // write code
        for (std::size_t i = 0; i < instructions.size(); ++i) {
          write16(os, instructions[i].opcode);
          write16(os, instructions[i].address);
        }

        // write number of symbols
        write32(os, symbols.size());
        // write symbol addresses
        for (std::size_t i = 0; i < symbols.size(); ++i)
          write16(os, symbols[i].address);
        // write symbols as C strings
        for (std::size_t i = 0; i < symbols.size(); ++i)
          os.write(symbols[i].label.c_str(), symbols[i].label.size() + 1); // include \0 character
      }


      void load(std::istream &is, std::vector<Instruction> &instructions, std::vector<Symbol> &symbols)
      {
        // read number of instructions
        unsigned int numInstructions;
        read32(is, numInstructions);

        // read code
        instructions.resize(numInstructions);
        for (std::size_t i = 0; i < instructions.size(); ++i) {
          read16(is, instructions[i].opcode);
          read16(is, instructions[i].address);
        }
        
        // read number of symbols
        unsigned int numSymbols;
        read32(is, numSymbols);
        symbols.resize(numSymbols);
        // read symbol addresses
        for (std::size_t i = 0; i < symbols.size(); ++i)
          read16(is, symbols[i].address);
 
        // read symbols
        for (unsigned int i = 0; i < numSymbols; ++i) {
          std::string symbol;
          char character;
          is.read(&character, 1);
          do {
            symbol += character;
            is.read(&character, 1);
          } while (character != '\0');

          std::cout << "symbol: " << symbol << std::endl;
          symbols[i].label = symbol;
        }
      }


  };

}

#endif
