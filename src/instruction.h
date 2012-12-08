#ifndef SC_INSTRUCTION_H
#define SC_INSTRUCTION_H

namespace SC {

  enum Opcode
  {
    // Execution Flow
    Instr_JMP = 0x01,
    Instr_JNE = 0x02,
    Instr_JE = 0x03,
    Instr_RET = 0x04,
    // Atom/Bond
    Instr_Aromatic = 0x51,
    Instr_Aliphatic = 0x52,
    Instr_Cyclic = 0x53,
    Instr_Acyclic = 0x54,
    Instr_Chirality = 0x55,
    // Atom
    Instr_Element = 0xA01,
    Instr_Mass = 0xA02,
    Instr_Degree = 0xA03,
    Instr_Valence = 0xA04,
    Instr_Connectivity = 0xA05,
    Instr_TotalH = 0xA06,
    Instr_ImplicitH = 0xA07,
    Instr_RingMembership = 0xA08,
    Instr_RingSize = 0xA09,
    Instr_RingConnectivity = 0xA0A,
    Instr_Charge = 0xA0B,
    Instr_AtomClass = 0xA0C,
    // Bond
    Instr_Order = 0xB01,
    // Implementation specific
    Instr_Invalid = 0xFFFF
  };

  typedef unsigned short Address;
  typedef unsigned short Constant;

  struct Instruction
  {
    Instruction() : opcode(Instr_Invalid), address(0)
    {
    }

    Instruction(Opcode instr, unsigned int operand = 0) : opcode(instr), address(operand)
    {
    }

    unsigned short opcode;
    union {
      Address address;
      Constant constant;
    };
  };

}

#endif
