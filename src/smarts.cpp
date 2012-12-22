#include "smarts.h"
#include "smartsprint.h"

#include <cassert>
#include <sstream>
#include <algorithm>

namespace SC {

  void print_expr_vector(const std::string &label, const std::vector<SmartsAtomExpr*> &v)
  {
    std::cout << label << ": ";
    for (std::size_t i = 0; i < v.size(); ++i) {
      switch (v[i]->type) {
        case Smiley::OP_Not:
          std::cout << "! ";
          break;
        case Smiley::OP_AndHi:
          std::cout << "& ";
          break;
        case Smiley::OP_AndLo:
          std::cout << "; ";
          break;
        case Smiley::OP_Or:
          std::cout << ", ";
          break;
        default:
          std::cout << GetExprString(v[i]) << " ";
          break;
      }
    }
    std::cout << std::endl;
  }

  /**
   * Callback for Smiley SMARTS parser.
   */
  struct SmartsCallback : public Smiley::CallbackBase
  {
    SmartsCallback(Smarts *s) : smarts(s)
    {
    }

    /**
     * The clear() method is invoked when Parser::parse() is called and should
     * be used to initialize the callback function object to receive events for
     * a new SMILES/SMARTS.
     */
    void clear()
    {
      smarts->clear();
    }

    /**
     * The addAtom() method is invoked when an atom is completly parsed.
     */
    void addAtom(int element, bool aromatic, int isotope, int hCount, int charge, int atomClass)
    {
      std::cout << "addAtom:" << std::endl
        << "    element: " << element << std::endl
        << "    aromatic: " << aromatic << std::endl
        << "    isotope: " << isotope << std::endl
        << "    hCount: " << hCount << std::endl
        << "    charge: " << charge << std::endl
        << "    atomClass: " << atomClass << std::endl;
    }

    /**
     * The addBond() method is invoked once both of the bond's atoms have been
     * added (by calling addAtom()). Therefore are the bond indices always
     * valid.
     */
    void addBond(int source, int target, int order, bool isUp, bool isDown)
    {
      int type;
      switch (order) {
        case 1:
          type = Smiley::BE_Single;
          break;
        case 2:
          type = Smiley::BE_Double;
          break;
        case 3:
          type = Smiley::BE_Triple;
          break;
        case 4:
          type = Smiley::BE_Quadriple;
          break;
        case 5:
          type = Smiley::BE_Aromatic;
          break;
      }

      std::cout << "addBond(" << source << ", " << target << ", " << order << ")" << std::endl;

      smarts->bonds.resize(smarts->bonds.size() + 1);
      smarts->bonds.back().index = smarts->bonds.size() - 1;
      smarts->bonds.back().source = source;
      smarts->bonds.back().target = target;
      if (bondExpr.empty())
        smarts->bonds.back().expr = new SmartsBondExpr(type);
      else {
        // convert infix to postfix
        std::vector<SmartsBondExpr*> operations, postfix;
        infixToPostfix(bondExpr, operations, postfix);
        // postfix to tree
        smarts->bonds.back().expr = postfixToTree(postfix);
        // clear bond expressions
        bondExpr.clear();
      }
    }
    
    /**
     * The setChiral() method is invoked at the end of parsing for each atom
     * that has a chirality specified.
     */
    void setChiral(int index, Smiley::Chirality chirality, const std::vector<int> &nbrs)
    {
      std::cout << "setChiral:" << std::endl
                << "    index: " << index << std::endl
                << "    chirality: " << chirality << std::endl
                << "    nbrs: ";
      for (std::size_t i = 0; i < nbrs.size(); ++i)
        std::cout << nbrs[i] << " ";
      std::cout << std::endl;
    }

    void operation(int type)
    {
      std::cout << "operation(";
      switch (type) {
        case Smiley::OP_AndHi:
          std::cout << "&";
          break;
        case Smiley::OP_AndLo:
          std::cout << ";";
          break;
        case Smiley::OP_Or:
          std::cout << ",";
          break;
        case Smiley::OP_Not:
          std::cout << "!";
          break;
      }
      std::cout << ")" << std::endl;

      atomExpr.push_back(new SmartsAtomExpr(type));
    }

    void addOrganicSubsetAtom(int element, bool aromatic)
    {
      std::cout << "addOrganicSubsetAtom(" << element << ", " << aromatic << ")" << std::endl;
      SmartsAtomExpr *expr;
      if (element == 0)
        expr = new SmartsAtomExpr(Smiley::AE_True);
      else if (element == -1)
        expr = new SmartsAtomExpr(aromatic ? Smiley::AE_Aromatic : Smiley::AE_Aliphatic);
      else if (aromatic)
        expr = new SmartsAtomExpr(Smiley::AE_AromaticElement);
      else
        expr = new SmartsAtomExpr(Smiley::AE_AliphaticElement);
      expr->leaf.value = element;
      smarts->atoms.resize(smarts->atoms.size() + 1);
      smarts->atoms.back().expr = expr;
      smarts->atoms.back().index = smarts->atoms.size() - 1;
    }

    void atomPrimitive(int type, int value)
    {
      std::cout << "atomPrimitive(" << type << ", " << value << ")" << std::endl;
      atomExpr.push_back(new SmartsAtomExpr(type));
      atomExpr.back()->leaf.value = value;
    }

    void bondPrimitive(int type)
    {
      if (atomExpr.size())
        createAtomExprTree();
      std::cout << "bondPrimitive(" << type << ")" << std::endl;
      bondExpr.push_back(new SmartsBondExpr(type));
    }

    void setPrevious(int index)
    {
    }

    void startRingBond(int number)
    {
    }

    void startRingBond(int number, int index)
    {
    }

    int precedence(int type) const
    {
      switch (type) {
        case Smiley::OP_Not:
          return 4;
        case Smiley::OP_AndHi:
          return 3;
        case Smiley::OP_Or:
          return 2;
        case Smiley::OP_AndLo:
          return 1;
      }
      return 5;
    }

    template<typename Expr>
    void infixToPostfix(const std::vector<Expr*> &infix, std::vector<Expr*> &operations, std::vector<Expr*> &postfix)
    {
      for (std::size_t i = 0; i < infix.size(); ++i) {
        int prec = precedence(infix[i]->type);

        switch (infix[i]->type) {
          case Smiley::OP_Not:
          case Smiley::OP_AndHi:
          case Smiley::OP_AndLo:
          case Smiley::OP_Or:
            while (operations.size() && precedence(operations.back()->type) > prec) {
              postfix.push_back(operations.back());
              operations.pop_back();
            }
            operations.push_back(infix[i]);
            break;
          default:
            postfix.push_back(infix[i]);
            break;
        }
      }

      while (!operations.empty()) {
        postfix.push_back(operations.back());
        operations.pop_back();
      }
    }

    template<typename Expr>
    Expr* postfixToTree(const std::vector<Expr*> &postfix)
    {
      // stack for converting postfix to tree
      std::vector<Expr*> operands;

      for (std::size_t i = 0; i < postfix.size(); ++i) {
        switch (postfix[i]->type) {
          case Smiley::OP_Not:
            assert(!operands.empty());
            postfix[i]->unary.arg = operands.back();
            // pop argument
            operands.pop_back();
            // push result
            operands.push_back(postfix[i]);
            break;
          case Smiley::OP_AndHi:
          case Smiley::OP_AndLo:
          case Smiley::OP_Or:
            assert(operands.size() > 1);
            postfix[i]->binary.lft = operands[operands.size() - 2];
            postfix[i]->binary.rgt = operands[operands.size() - 1];
            // pop arguments
            operands.pop_back();
            operands.pop_back();
            // push result
            operands.push_back(postfix[i]);
            break;
          default:
            // push operand
            operands.push_back(postfix[i]);
            break;
        }
      }

      assert(operands.size() == 1);

      return operands.front();
    }

    void createAtomExprTree()
    {
      // convert infix to postfix
      std::vector<SmartsAtomExpr*> operations, postfix;
      infixToPostfix(atomExpr, operations, postfix);

      print_expr_vector("postfix", postfix);
      print_expr_vector("operations", operations);

      // add the SmartsAtom
      smarts->atoms.resize(smarts->atoms.size() + 1);
      smarts->atoms.back().expr = postfixToTree(postfix);
      smarts->atoms.back().index = smarts->atoms.size() - 1;

      // clear the list of atom expressions
      atomExpr.clear();
    }

    void end()
    {
      if (atomExpr.size())
        createAtomExprTree();

      for (std::size_t i = 0; i < smarts->bonds.size(); ++i) {
        smarts->atoms[smarts->bonds[i].source].bonds.push_back(&smarts->bonds[i]);
        smarts->atoms[smarts->bonds[i].target].bonds.push_back(&smarts->bonds[i]);
      }
    }

    std::vector<SmartsAtomExpr*> atomExpr;
    std::vector<SmartsBondExpr*> bondExpr;
    Smarts *smarts;
  };

  Smarts* parse(const std::string &smarts)
  {
    SmartsCallback callback(new Smarts);
    Smiley::Parser<SmartsCallback> parser(callback, Smiley::Parser<SmartsCallback>::SmartsMode);

    try {
      parser.parse(smarts);
    } catch (Smiley::Exception &e) {
      if (e.type() == Smiley::Exception::SyntaxError)
        std::cerr << "Syntax";
      else
        std::cerr << "Semantics";
      std::cerr << "Error: " << e.what() << "." << std::endl;
      std::cerr << smarts << std::endl;
      for (std::size_t i = 0; i < e.pos(); ++i)
        std::cerr << " ";
      for (std::size_t i = 0; i < e.length(); ++i)
        std::cerr << "^";
      std::cerr << std::endl;
    }

    return callback.smarts;
  }

  struct SmartsWriter
  {
    SmartsWriter(const Smarts *s) : smarts(s), visited(s->atoms.size()), written(0)
    {
    }

    void findRingBonds(int prevIndex, int atomIndex, int rnum = 0)
    {
      const SmartsAtom &atom = smarts->atoms[atomIndex];
      visited[atomIndex] = true;

      // visit neighbors
      for (std::size_t i = 0; i < atom.bonds.size(); ++i) {
        int nbrIndex = atom.bonds[i]->source == atomIndex ? atom.bonds[i]->target : atom.bonds[i]->source;
        if (nbrIndex == prevIndex)
          continue;
        if (visited[nbrIndex]) {
          ++rnum;
          if (std::find(ringBonds[atomIndex].begin(), ringBonds[atomIndex].end(), rnum) == ringBonds[atomIndex].end())
            ringBonds[atomIndex].push_back(rnum);
          if (std::find(ringBonds[nbrIndex].begin(), ringBonds[nbrIndex].end(), rnum) == ringBonds[nbrIndex].end())
            ringBonds[nbrIndex].push_back(rnum);
          continue;
        }
        // continue DFS search
        findRingBonds(atomIndex, nbrIndex, rnum);
      }
    }

    bool isDefaultBond(const SmartsBond *bond)
    {
      if (smarts->atoms[bond->source].expr->type == Smiley::AE_AromaticElement &&
          smarts->atoms[bond->target].expr->type == Smiley::AE_AromaticElement &&
          bond->expr->type == Smiley::BE_Aromatic)
        return true;
      return bond->expr->type == Smiley::BE_Single;
    }

    void writeDFS(int prevIndex, int atomIndex)
    {
      const SmartsAtom &atom = smarts->atoms[atomIndex];
      visited[atomIndex] = true;

      // write the atom
      switch (atom.expr->type) {
        case Smiley::OP_Not:
        case Smiley::OP_AndHi:
        case Smiley::OP_AndLo:
        case Smiley::OP_Or:
          ss << "[" << GetExprString(atom.expr) << "]";
          break;
        default:
          ss << GetExprString(atom.expr);
          break;
      }
      ++written;

      // count neighbors that still have to be written
      int numNbrs = 0;
      for (std::size_t i = 0; i < atom.bonds.size(); ++i) {
        int nbrIndex = atom.bonds[i]->source == atomIndex ? atom.bonds[i]->target : atom.bonds[i]->source;
        if (nbrIndex == prevIndex || visited[nbrIndex])
          continue;
        ++numNbrs;
      }

      if (ringBonds.find(atomIndex) != ringBonds.end()) {
        numNbrs -= ringBonds[atomIndex].size();
        for (std::size_t i = 0; i < ringBonds[atomIndex].size(); ++i)
          ss << ringBonds[atomIndex][i];
      }

      
      if (!numNbrs && written < smarts->atoms.size()) {
        ss << ")";
        return;
      }

      for (std::size_t i = 0; i < atom.bonds.size(); ++i) {
        int nbrIndex = atom.bonds[i]->source == atomIndex ? atom.bonds[i]->target : atom.bonds[i]->source;
        if (nbrIndex == prevIndex || visited[nbrIndex])
          continue;
        // write bond
        if (!isDefaultBond(atom.bonds[i]))
          ss << GetExprString(atom.bonds[i]->expr);
        // write neighbor
        if (numNbrs > 1)
          ss << "(";
        writeDFS(atomIndex, nbrIndex);
        --numNbrs;
      }

    }

    std::string write()
    {
      findRingBonds(-1, 0);
      visited.clear();
      visited.resize(smarts->atoms.size());
      writeDFS(-1, 0);
      return ss.str();
    }

    const Smarts *smarts;
    std::vector<bool> visited;
    std::vector<bool> aromatic;
    std::map<int, std::vector<int> > ringBonds;
    std::stringstream ss;
    int written;
  };

  std::string write(const Smarts *smarts)
  {
    SmartsWriter sw(smarts);
    return sw.write();
  }

}
