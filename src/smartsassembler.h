#include "util.h"
#include "instruction.h"
#include "smartsbytecodefile.h"

#include <iostream>
#include <cassert>

namespace SC {

  class SmartsAssembler
  {
    private:

      struct InstructionInfo
      {
        InstructionInfo(std::size_t lineNumber, const std::string &lbl = std::string())
            : line(lineNumber), label(lbl)
        {
        }

        std::size_t line;
        std::string label;
      };

      void syntaxError(std::size_t line, const std::string &msg)
      {
        std::cerr << "SyntaxError on line " << line << ": " << msg << std::endl;
      }

      bool validateLabel(const std::string &label) const
      {
        // must start with A-Z, a-z or _
        if (!std::isalpha(label[0]) && !label[0] != '_')
          return false;
        // may only contain A-Z, a-z, 0-9 and _
        for (std::size_t i = 0; i < label.size(); ++i) {
          if (std::isalpha(label[i]))
            continue;
          if (std::isdigit(label[i]))
            continue;
          if (label[i] == '_')
            continue;

          return false;
        }

        return true;
      }

      Constant parseConstant(const std::string &constant, bool &success)
      {
        std::stringstream ss(constant);
        Constant result;
        success = ss >> result;
        return result;
      }

      Opcode opcodeFromString(const std::string &instruction) const
      {
        if (instruction == "jmp")
          return Instr_JMP;
        if (instruction == "jne")
          return Instr_JNE;
        if (instruction == "je")
          return Instr_JE;
        if (instruction == "ret")
          return Instr_RET;
        if (instruction == "arom")
          return Instr_Aromatic;
        if (instruction == "aliph")
          return Instr_Aliphatic;
        if (instruction == "cyclic")
          return Instr_Cyclic;
        if (instruction == "acyclic")
          return Instr_Acyclic;
        if (instruction == "chiral")
          return Instr_Chirality;
        if (instruction == "elem")
          return Instr_Element;
        if (instruction == "mass")
          return Instr_Mass;
        if (instruction == "deg")
          return Instr_Degree;
        if (instruction == "val")
          return Instr_Valence;
        if (instruction == "conn")
          return Instr_Connectivity;
        if (instruction == "totalh")
          return Instr_TotalH;
        if (instruction == "implh")
          return Instr_ImplicitH;
        if (instruction == "rmem")
          return Instr_RingMembership;
        if (instruction == "rsize")
          return Instr_RingSize;
        if (instruction == "rconn")
          return Instr_RingConnectivity;
        if (instruction == "chg")
          return Instr_Charge;
        if (instruction == "class")
          return Instr_AtomClass;
        if (instruction == "order")
          return Instr_Order;
        return Instr_Invalid;
      }

      bool parse(std::istream &is)
      {
        Address offset = 0;

        std::size_t lineNumber = 0;
        std::string line;
        while (std::getline(is, line)) {
          ++lineNumber;
          // empty line?
          if (line.empty())
            continue;
          // replace all tabs with spaces
          replace_all(line, "\t", " ");
          // check if this is a comment line, starts with #
          std::size_t pos = 0;
          while (pos < line.size() && line[pos] == ' ')
            ++pos;
          // withespace onle line?
          if (pos == line.size())
            continue;
          if (line[pos] == '#')
            continue;

          // check for comment at end of instruction or label
          pos = line.find('#');
          if (pos != std::string::npos)
            line = line.substr(0, pos);

          // label line?
          pos = line.find(':');
          if (pos != std::string::npos) {
            line = line.substr(0, pos);
            // strip whitespace
            strip(line);

            if (!validateLabel(line)) {
              syntaxError(lineNumber, make_string("\"", line, "\" is not valid label, labels may only contain A-Z, a-z, 0-9 and _"));
              return false;
            }

            // store label as symbol
            m_symbols.push_back(SmartsByteCodeFile::Symbol(line, offset));
            std::cout << "found label: \"" << line << "\" at offset " << offset << std::endl;
            continue;
          }

          // line must be an instruction
          strip(line);
          std::vector<std::string> tokens = tokenize(line, " ", true);
          // convert string to instruction opcode
          Opcode opcode = opcodeFromString(tokens[0]);
          switch (opcode) {
            // instructions with label operand
            case Instr_JMP:
            case Instr_JNE:
            case Instr_JE:
              {
                // check if the operand is there
                if (tokens.size() == 1) {
                  syntaxError(lineNumber, make_string("instruction \"", tokens[0], "\" requires a label opereand"));
                  return false;
                }
                // check for trailing garbage
                if (tokens.size() > 2) {
                  syntaxError(lineNumber, make_string("instruction \"", tokens[0], "\" requires only 1 opereand"));
                  return false;
                }
                // validate label
                if (!validateLabel(tokens[1])) { 
                  syntaxError(lineNumber, make_string("Could not parse constant \"", tokens[1], "\""));
                  return false;
                }
                // phew! add the instruction
                m_instructionInfos.push_back(InstructionInfo(lineNumber, tokens[1]));
                m_instructions.push_back(Instruction(opcode));
                offset += 4; // 32 bit instructions
                std::cout << "Instruction with label operand: opcode = 0x" << std::hex << opcode << std::dec << ", label = " << tokens[1] << std::endl;
                continue;
               }
            // instructions without operand
            case Instr_Aromatic:
            case Instr_Aliphatic:
            case Instr_Cyclic:
            case Instr_Acyclic:
              if (tokens.size() > 1) {
                syntaxError(lineNumber, make_string("instruction \"", tokens[0], "\" does not require any operands"));
                return false;
              }
              m_instructionInfos.push_back(InstructionInfo(lineNumber));
              m_instructions.push_back(Instruction(opcode));
              offset += 4; // 32-bit instructions
              std::cout << "Instruction without operand: opcode = 0x" << std::hex << opcode << std::dec << std::endl;
              continue;
            // instructions with constant operand
            case Instr_RET:
            case Instr_Chirality:
            case Instr_Element:
            case Instr_Mass:
            case Instr_Degree:
            case Instr_Valence:
            case Instr_Connectivity:
            case Instr_TotalH:
            case Instr_ImplicitH:
            case Instr_RingMembership:
            case Instr_RingSize:
            case Instr_RingConnectivity:
            case Instr_Charge:
            case Instr_AtomClass:
            case Instr_Order:
              {
                // check if the operand is there
                if (tokens.size() == 1) {
                  syntaxError(lineNumber, make_string("instruction \"", tokens[0], "\" requires a constant opereand"));
                  return false;
                }
                // check for trailing garbage
                if (tokens.size() > 2) {
                  syntaxError(lineNumber, make_string("instruction \"", tokens[0], "\" requires only 1 opereand"));
                  return false;
                }
                // parse constant
                bool success;
                Constant constant = parseConstant(tokens[1], success);
                // parsing successful?
                if (!success) {
                  syntaxError(lineNumber, make_string("Could not parse constant \"", tokens[1], "\""));
                  return false;
                }
                // phew! add the instruction
                m_instructionInfos.push_back(InstructionInfo(lineNumber));
                m_instructions.push_back(Instruction(opcode, constant));
                offset += 4; // 32 bit instructions
                std::cout << "Instruction with constant operand: opcode = 0x" << std::hex << opcode << std::dec << ", constant = " << constant << std::endl;
                continue;
              }
            default:
              syntaxError(lineNumber, make_string("\"", tokens[0], "\" is not a valid instruction"));
              return false;
          }
        }

        return true;
      }

      bool resolveLabels()
      {
        for (std::size_t i = 0; i < m_instructions.size(); ++i) {
          InstructionInfo &instr = m_instructionInfos[i];
          switch (m_instructions[i].opcode) {
            case Instr_JMP:
            case Instr_JNE:
            case Instr_JE:
              {
                // find symbol
                std::size_t j;
                for (j = 0; j < m_symbols.size(); ++j)
                  if (m_symbols[j].label == instr.label) {
                    m_instructions[i].address = m_symbols[j].address;
                    std::cout << "Resolved label " << instr.label << ": address = 0x" << std::hex << m_instructions[i].address << std::dec << std::endl;
                    break;
                  }
                // found symbol?
                if (j == m_symbols.size()) {
                  std::cerr << "Error: Could not resolve label on line " << instr.line << std::endl;
                  return false;
                }
                // next instruction
                continue;
              }
            default:
              break;
          }
        }

        return true;
      }


    public:


      bool assemble(std::istream &is, std::ostream &os)
      {
        m_symbols.clear();
        m_instructions.clear();

        if (!parse(is))
          return false;
        if (!resolveLabels())
          return false;

        SmartsByteCodeFile file;
        file.write(os, m_instructions, m_symbols);

        return true;
      }

    private:
      std::vector<Instruction> m_instructions;
      std::vector<SmartsByteCodeFile::Symbol> m_symbols;
      std::vector<InstructionInfo> m_instructionInfos;
  };

}
