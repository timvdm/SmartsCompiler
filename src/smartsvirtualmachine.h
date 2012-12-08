#include "instruction.h"

#include <iomanip>

namespace SC {

  class SmartsVirtualMachine
  {
     
      std::string stringFromOpcode(unsigned int opcode) const
      {
        switch (opcode) {
          case Instr_JMP:
            return "jmp";
          case Instr_JNE:
            return "jne";
          case Instr_JE:
            return "je";
          case Instr_RET:
            return "ret";
          case Instr_Aromatic:
            return "arom";
          case Instr_Aliphatic:
            return "aliph";
          case Instr_Cyclic:
            return "cyclic";
          case Instr_Acyclic:
            return "acyclic";
          case Instr_Chirality:
            return "chiral";
          case Instr_Element:
            return "elem";
          case Instr_Mass:
            return "mass";
          case Instr_Degree:
            return "deg";
          case Instr_Valence:
            return "val";
          case Instr_Connectivity:
            return "conn";
          case Instr_TotalH:
            return "totalh";
          case Instr_ImplicitH:
            return "implh";
          case Instr_RingMembership:
            return "rmem";
          case Instr_RingSize:
            return "rsize";
          case Instr_RingConnectivity:
            return "rconn";
          case Instr_Charge:
            return "chg";
          case Instr_AtomClass:
            return "class";
          case Instr_Order:
            return "order";
          default:
            std::cout << std::hex << opcode << std::endl;
            std::cout << std::dec << opcode << std::endl;
            //assert(0);
            break;
        }
        
        return "";
      }

    public:
      void disassemble() const
      {
        std::size_t symbol = 0;
        for (std::size_t i = 0; i < m_instructions.size(); ++i) {
          if (symbol < m_symbols.size() && m_symbols[symbol].address == 4 * i)
            std::cout << std::endl << m_symbols[symbol++].label << ":" << std::endl << std::endl;
          std::cout << "0x" << std::hex << std::setfill('0') << std::setw(8) << 4 * i << std::dec << std::setfill(' ') << "  ";
          switch (m_instructions[i].opcode) {
            case Instr_JMP:
            case Instr_JNE:
            case Instr_JE:
              std::cout << stringFromOpcode(m_instructions[i].opcode) << " 0x" << std::hex << m_instructions[i].address << std::dec << std::endl;
              break;
            case Instr_Aromatic:
            case Instr_Aliphatic:
            case Instr_Cyclic:
            case Instr_Acyclic:
            case Instr_Chirality:
              std::cout << stringFromOpcode(m_instructions[i].opcode) << std::endl;
              break;
            default:
              std::cout << stringFromOpcode(m_instructions[i].opcode) << " " << m_instructions[i].constant << std::endl;
              break;
          }
        }
      }

      void load(std::istream &is)
      {
        SmartsByteCodeFile file;
        file.load(is, m_instructions, m_symbols);
      }

      template<typename Atom>
      bool matchAtom(const std::string &symbol, const Atom &atom)
      {
        bool TF;
        Address IP = findSymbol(symbol);

        while (true) {
          const Instruction &instr = fetchInstruction(IP);
          std::cout << "executing: " << stringFromOpcode(instr.opcode) << " " << instr.constant << std::endl;

          switch (instr.opcode) {
            case Instr_RET:
              return instr.constant;
            case Instr_JMP:
              IP = instr.address;
              break;
            case Instr_JE:
              if (TF) {
                IP = instr.address;
                continue;
              }
              break;
            case Instr_JNE:
              if (!TF) {
                IP = instr.address;
                continue;
              }
              break;
            case Instr_Aromatic:
              TF = atom.isAromatic();
              break;
            case Instr_Aliphatic:
              TF = atom.isAliphatic();
              break;
            case Instr_Cyclic:
              TF = atom.isCyclic();
              break;
            case Instr_Acyclic:
              TF = atom.isAcyclic();
              break;
            case Instr_Element:
              TF = atom.element() == instr.constant;
              break;
            case Instr_Mass:
              TF = atom.mass() == instr.constant;
              break;
            case Instr_Degree:
              TF = atom.degree() == instr.constant;
              break;
            case Instr_Valence:
              TF = atom.valence() == instr.constant;
              break;
            case Instr_Connectivity:
              TF = atom.connectivity() == instr.constant;
              break;
            case Instr_TotalH:
              TF = atom.totalHydrogens() == instr.constant;
              break;
            case Instr_ImplicitH:
              TF = atom.implicitHydrogens() == instr.constant;
              break;
            case Instr_RingMembership:
              TF = atom.ringMembership() == instr.constant;
              break;
            case Instr_RingSize:
              TF = atom.isInRingSize(instr.constant);
              break;
            case Instr_RingConnectivity:
              TF = atom.ringConnectivity() == instr.constant;
              break;
            case Instr_Charge:
              TF = atom.charge() == static_cast<short>(instr.constant);
              break;
            case Instr_AtomClass:
              TF = atom.atomClass() == instr.constant;
              break;
            default:
              std::cerr << "Unknown instruction at address 0x" << std::hex << IP << std::dec << std::endl;
              break;
          }

          IP += 4;
        }
      }

    private:
      Address findSymbol(const std::string &symbol)
      {
        // TODO: use binary search, sort key symbol
        for (std::size_t i = 0; i < m_symbols.size(); ++i)
          if (m_symbols[i].label == symbol)
            return m_symbols[i].address;
        std::cerr << "Symbol " << symbol << "not found" << std::endl;
        return 0;
      }

      const Instruction& fetchInstruction(Address address) const
      {
        return m_instructions[address / 4];
      }

      std::vector<SmartsByteCodeFile::Symbol> m_symbols;
      std::vector<Instruction> m_instructions;
  };



}
