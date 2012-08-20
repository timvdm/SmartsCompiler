#include "smartscompiler.h"
#include "smartspattern.h"
#include "util.h"
#include "toolkit.h"
#include "pattern.h"
#include "defines.h"

#include <set>

namespace SC {

  using namespace OpenBabel;
  
  static const char *cpp_template = "inline bool $function($arg)\n"
                                    "{\n"
                                    "  return $expr;\n"
                                    "}\n";

  static const char *python_template = "def $function(index):\n"
                                       "  $arg = mol.Get$obj(index)\n"
                                       "  return $expr\n";
 
  static const char *python_op_template = "def $function($arg):\n"
                                          "  return $expr\n";
 
  struct SmartsCompilerPrivate
  {
    Toolkit *m_toolkit;
    enum SmartsCompiler::Language m_language;
    bool m_true;
    bool m_false;
    bool m_arom;
    bool m_aliph;
    bool m_cyclic;
    bool m_acyclic;
    bool m_chiral;
    std::set<int> m_mass;
    std::set<int> m_elem;
    std::set<int> m_aliphelem;
    std::set<int> m_aromelem;
    std::set<int> m_hcount;
    std::set<int> m_charge;
    std::set<int> m_connect;
    std::set<int> m_degree;
    std::set<int> m_implicit;
    std::set<int> m_rings;
    std::set<int> m_size;
    std::set<int> m_valence;
    std::set<int> m_hyb;
    std::set<int> m_ringconnect;
    int m_not;
    int m_and;
    int m_or;
    int m_switch;

    bool m_default;
    bool m_any;
    bool m_single;
    bool m_double;
    bool m_triple;
    bool m_barom;
    bool m_ring;

    std::map<int, std::string> m_atomEvalExpr;
    std::map<int, std::string> m_bondEvalExpr;
    std::vector<std::string> m_smarts;
    std::vector<SmartsPattern> m_patterns;
    std::set<int> m_singleatoms;

    SmartsCompilerPrivate(Toolkit *toolkit, enum SmartsCompiler::Language language) : m_toolkit(toolkit),
    m_language(language), m_true(false), m_false(false), m_arom(false), m_aliph(false),
    m_cyclic(false), m_acyclic(false), m_chiral(false), m_and(0), m_or(0), m_not(0),
    m_any(false), m_default(false), m_single(false), m_double(false), m_triple(false),
    m_barom(false), m_ring(false)
    {
    }
 
    std::string FunctionTemplate(const std::string &function, enum SmartsCompiler::ArgType arg, const std::string &expr, bool isOp)
    {
      std::string temp;
      cpp_template : python_template;
      switch (m_language) {
        case SmartsCompiler::Cpp:
          temp = cpp_template;
          break;
        case SmartsCompiler::Python:
          if (isOp)
            temp = python_op_template;
          else
            temp = python_template;
          break;
        default:
          break;
      }

      replace_all(temp, "$function", function);

      std::string arg_name = arg == SmartsCompiler::AtomArg ? "atom" : "bond";
      std::string arg_type = arg == SmartsCompiler::AtomArg ? m_toolkit->AtomArgType(m_language) : m_toolkit->BondArgType(m_language);
      if (arg_type.size())
        arg_type = make_string(arg_type, " ", arg_name);
      else
        arg_type = arg_name;
      replace_all(temp, "$arg", arg_type);

      if (m_language == SmartsCompiler::Python) {
        std::string obj = arg == SmartsCompiler::AtomArg ? "Atom" : "Bond";
        replace_all(temp, "$obj", obj);
      }

      replace_all(temp, "$expr", expr);
      return temp;
    }

    // function with value
    std::string ExprFunction(std::ostream &os, std::set<int> &exprSet, const std::string &function,
        const std::string &expr, int value, enum SmartsCompiler::ArgType argType)
    {
      std::string functionName = make_string(function, "_", value);

      if (exprSet.find(value) != exprSet.end())
        return functionName;
      exprSet.insert(value);

      std::string expr2 = expr;
      replace_all(expr2, "$value", make_string(value));

      os << FunctionTemplate(functionName, argType, expr2, false);
      os << std::endl;

      return functionName;
    }

    // function without value
    std::string ExprFunction(std::ostream &os, bool &exists, const std::string &functionName,
        const std::string &expr, enum SmartsCompiler::ArgType argType)
    {
      if (exists)
        return functionName;
      exists = true;

      os << FunctionTemplate(functionName, argType, expr, false);
      os << std::endl;

      return functionName;
    }

    enum BinaryOp
    {
      BinaryAnd,
      BinaryOr,
    };

    std::string BinaryAndString()
    {
      switch (m_language) {
        case SmartsCompiler::Cpp:
          return " && ";
        case SmartsCompiler::Python:
          return " and ";
        default:
          return "";
      }
    }

    std::string BinaryOrString()
    {
      switch (m_language) {
        case SmartsCompiler::Cpp:
          return " || ";
        case SmartsCompiler::Python:
          return " or ";
        default:
          return "";
      }
    }

    // binary function
    template<typename Expr>
    std::string BinaryExprFunction(std::ostream &os, enum BinaryOp op, const Expr *expr)
    {
      std::string functionName, op_str;

      switch (op) {
        case BinaryAnd:
          functionName = make_string("EvalAndExpr_", ++m_and);
          op_str = BinaryAndString();
          break;
        case BinaryOr:
          functionName = make_string("EvalOrExpr_", ++m_or);
          op_str = BinaryOrString();
          break;
      }
 
      std::string lftFunctionName = GenerateExprCode(os, expr->bin.lft);
      std::string rgtFunctionName = GenerateExprCode(os, expr->bin.rgt);

      enum SmartsCompiler::ArgType argType = SameType<Expr, AtomExpr>::result ? SmartsCompiler::AtomArg : SmartsCompiler::BondArg;
      std::string arg = argType == SmartsCompiler::AtomArg ? "(atom)" : "(bond)";
 
      std::string expr_str = lftFunctionName + arg + op_str + rgtFunctionName + arg;

      os << FunctionTemplate(functionName, argType, expr_str, true);
      os << std::endl;

      return functionName;
    }

    enum UnaryOp
    {
      UnaryNot
    };

    std::string UnaryNotString()
    {
      switch (m_language) {
        case SmartsCompiler::Cpp:
          return "!";
        case SmartsCompiler::Python:
          return "not ";
        default:
          return "";
      }
    }

    // unary function
    template<typename Expr>
    std::string UnaryExprFunction(std::ostream &os, enum UnaryOp op, const Expr *expr)
    {
      std::string functionName, op_str;

      switch (op) {
        case UnaryNot:
          functionName = make_string("EvalNotExpr_", ++m_not);
          op_str = UnaryNotString();
          break;
      }
 
      std::string argFunctionName = GenerateExprCode(os, expr->mon.arg);

      enum SmartsCompiler::ArgType argType = SameType<Expr, AtomExpr>::result ? SmartsCompiler::AtomArg : SmartsCompiler::BondArg;
      std::string arg = argType == SmartsCompiler::AtomArg ? "(atom)" : "(bond)";
 
      std::string expr_str = op_str + argFunctionName + arg;

      os << FunctionTemplate(functionName, argType, expr_str, true);
      os << std::endl;

      return functionName;
    }


    std::string GetValuedExprCode(int type)
    {
      switch (type) {
        case AE_MASS:
          return m_toolkit->MassAtomTemplate(m_language);
        case AE_ELEM:
          return m_toolkit->ElementAtomTemplate(m_language);
        case AE_AROMELEM:
          return m_toolkit->AromaticElementAtomTemplate(m_language);
        case AE_ALIPHELEM:
          return m_toolkit->AliphaticElementAtomTemplate(m_language);
        case AE_HCOUNT:
          return m_toolkit->HydrogenCountAtomTemplate(m_language);
        case AE_CHARGE:
          return m_toolkit->ChargeAtomTemplate(m_language);
        case AE_CONNECT:
          return m_toolkit->ConnectAtomTemplate(m_language);
        case AE_DEGREE:
          return m_toolkit->DegreeAtomTemplate(m_language);
        case AE_IMPLICIT:
          return m_toolkit->ImplicitAtomTemplate(m_language);
        case AE_RINGS:
          return m_toolkit->NumRingsAtomTemplate(m_language);
        case AE_SIZE:
          return m_toolkit->RingSizeAtomTemplate(m_language);
        case AE_VALENCE:
          return m_toolkit->ValenceAtomTemplate(m_language);
        case AE_HYB:
          return m_toolkit->HybAtomTemplate(m_language);
        case AE_RINGCONNECT:
          return m_toolkit->RingConnectAtomTemplate(m_language);
       }
    }

    template<typename Expr>
    bool IsTypeOrNegatedType(Expr *expr, int type)
    {
      return expr->type == type || (IsNot(expr) && expr->mon.arg->type == type);
    }

    void GenerateOrSwicthCode(std::stringstream &ss, const std::vector<AtomExpr*> &or_other, std::vector<AtomExpr*> &or_not_switched)
    {
      // find all ORs switchable leafs
      std::vector<AtomExpr*> or_switchable; // [expr]
      for (std::size_t i = 0; i < or_other.size(); ++i)
        if (m_toolkit->IsSwitchable(or_other[i]->type))
          or_switchable.push_back(or_other[i]);
        else
          or_not_switched.push_back(or_other[i]);

      // sort or_switchable by type
      std::map<int, std::vector<AtomExpr*> > or_switchable_types; // type -> [expr]
      for (std::size_t i = 0; i < or_switchable.size(); ++i)
        or_switchable_types[or_switchable[i]->type].push_back(or_switchable[i]);

      // generate all switch statements
      for (typename std::map<int, std::vector<AtomExpr*> >::iterator i = or_switchable_types.begin(); i != or_switchable_types.end(); ++i) {
        if (i->second.size() < 2) {
          for (std::size_t j = 0; j < i->second.size(); ++j)
            or_not_switched.push_back(i->second[j]);
          continue;
        }

        // get the values
        std::vector<int> values; // [value]
        for (std::size_t j = 0; j < i->second.size(); ++j)
          values.push_back(i->second[j]->leaf.value);
        std::sort(values.begin(), values.end());
        values.resize(std::unique(values.begin(), values.end()) - values.begin());

        if (ss.str().empty())
          ss << "  // OR switchables" << std::endl;

        // get expression and predicate code
        std::string expr_code = m_toolkit->GetSwitchExpr(m_language, i->first);
        std::string pred_code = m_toolkit->GetSwitchPredicate(m_language, i->first);

        std::string indent;
        if (pred_code.size()) {
          ss << "  if (" << pred_code << ")" << std::endl;
          indent = "  ";
        }
       
        // generate switch statement
        ss << indent << "  switch (" << expr_code << ") {" << std::endl;
        for (std::size_t j = 0; j < values.size(); ++j)
          ss << indent << "    case " << values[j] << ":" << std::endl;
        ss << indent << "      return true;" << std::endl;
        ss << indent << "    default:" << std::endl;
        ss << indent << "      break;" << std::endl;
        ss << indent << "  }" << std::endl;
      }
    }

    std::string GenerateSwitchCode(std::ostream &os, AtomExpr *expr)
    {
      std::stringstream or_code;
      std::vector<AtomExpr*> or_not_switched;

      if (IsOr(expr)) {
        // find all ORs
        std::vector<AtomExpr*> or_same, or_other; // [expr]
        FindSameBinaryExpr(expr, or_same, or_other);
        // generate OR switches
        GenerateOrSwicthCode(or_code, or_other, or_not_switched);
      }
      
      bool generated_or_switch = or_code.str().size();
      
      std::stringstream and_code;

      // find all OR's ANDs
      std::vector<std::vector<AtomExpr*> > and_same(or_not_switched.size()), and_other(or_not_switched.size()); // [and_index][expr]
      std::vector<AtomExpr*> or_not_switched_not_and;
      for (std::size_t i = 0; i < or_not_switched.size(); ++i)
        if (IsAnd(or_not_switched[i]))
          FindSameBinaryExpr(or_not_switched[i], and_same[i], and_other[i]);
        else
          or_not_switched_not_and.push_back(or_not_switched[i]);
      

      // find all AND's switchable leafs
      std::vector<std::vector<AtomExpr*> > and_switchable(and_other.size()), and_not_switched(and_other.size()); // [and_index][expr]
      for (std::size_t i = 0; i < and_other.size(); ++i)
        for (std::size_t j = 0; j < and_other[i].size(); ++j)
          if (m_toolkit->IsSwitchable(and_other[i][j]->type) || (IsNot(and_other[i][j]) && m_toolkit->IsSwitchable(and_other[i][j]->mon.arg->type)))
            and_switchable[i].push_back(and_other[i][j]);
          else
            and_not_switched[i].push_back(and_other[i][j]);

      // sort and_switchable by type
      std::map<int, std::vector<std::pair<int, AtomExpr*> > > and_switchable_types; // type -> [(and_index, expr)]
      for (std::size_t i = 0; i < and_switchable.size(); ++i)
        for (std::size_t j = 0; j < and_switchable[i].size(); ++j) {
          if (IsNot(and_switchable[i][j]))
            and_switchable_types[-and_switchable[i][j]->mon.arg->type].push_back(std::make_pair(i, and_switchable[i][j]));
          else
            and_switchable_types[and_switchable[i][j]->type].push_back(std::make_pair(i, and_switchable[i][j]));
        }
    
      std::vector<std::pair<int, int> > and_switchable_sizes; // [(type, and_switchable_types_size)]
      for (typename std::map<int, std::vector<std::pair<int, AtomExpr*> > >::iterator i = and_switchable_types.begin(); i != and_switchable_types.end(); ++i)
        and_switchable_sizes.push_back(std::make_pair(i->first, i->second.size()));
      std::sort(and_switchable_sizes.begin(), and_switchable_sizes.end(), SortPairs<int, int, PairSecond, std::greater>());
      
      bool generated_and_switch = false;
      for (std::size_t i = 0; i < and_switchable_sizes.size(); ++i) {
        int type = and_switchable_sizes[i].first;
        if (and_switchable_types[type].size() < 2)
          break;

        generated_and_switch = true;
      
        // get the values
        std::vector<int> values; // [value]
        for (std::size_t j = 0; j < and_switchable_types[type].size(); ++j)
          if (IsNot(and_switchable_types[type][j].second))
            values.push_back(and_switchable_types[type][j].second->mon.arg->leaf.value);
          else
            values.push_back(and_switchable_types[type][j].second->leaf.value);
        std::sort(values.begin(), values.end());
        values.resize(std::unique(values.begin(), values.end()) - values.begin());

        if (and_code.str().empty())
          and_code << "  // AND switchables" << std::endl;

        std::string expr_code = m_toolkit->GetSwitchExpr(m_language, type);
        std::string pred_code = m_toolkit->GetSwitchPredicate(m_language, type);

        std::string indent;
        if (pred_code.size()) {
          and_code << "  if (" << pred_code << ")" << std::endl;
          indent = "  ";
        }
 
        and_code << indent << "  switch (" << expr_code << ") {" << std::endl;
        for (std::size_t j = 0; j < values.size(); ++j) {
          and_code << indent << "    case " << values[j] << ":" << std::endl;
          for (std::size_t k = 0; k < and_switchable.size(); ++k) {
            bool found_value = false;
            for (std::size_t l = 0; l < and_switchable[k].size(); ++l) {
              if (IsTypeOrNegatedType(and_switchable[k][l], std::abs(type)))
                if (IsNot(and_switchable[k][l]) && and_switchable[k][l]->mon.arg->leaf.value == values[j] || and_switchable[k][l]->leaf.value == values[j]) {
                  found_value = true;
                  break;
                }
            }
            if (!found_value)
              continue;
            // handle and switchable leafs that were not switched
            for (std::size_t l = 0; l < and_switchable[k].size(); ++l) {
              if (IsTypeOrNegatedType(and_switchable[k][l], std::abs(type)))
                if (IsNot(and_switchable[k][l]) && and_switchable[k][l]->mon.arg->leaf.value == values[j] || and_switchable[k][l]->leaf.value == values[j])
                  continue;
              std::string functionName = GenerateExprCode(os, and_switchable[k][l]);
              and_code << indent << "      if (" << functionName << "(atom))" << std::endl;
            }
            // handle not switched and leafs
            for (std::size_t l = 0; l < and_not_switched[k].size(); ++l) {
              if (IsTypeOrNegatedType(and_not_switched[k][l], std::abs(type)))
                if (IsNot(and_not_switched[k][l]) && and_not_switched[k][l]->mon.arg->leaf.value == values[j] || and_not_switched[k][l]->leaf.value == values[j])
                  continue;
              std::string functionName = GenerateExprCode(os, and_not_switched[k][l]);
              and_code << indent << "      if (" << functionName << "(atom))" << std::endl;
            }
            if (type < 0)
              and_code << indent << "        return false;" << std::endl;
            else
              and_code << indent << "        return true;" << std::endl;
            and_switchable[k].clear();
            and_not_switched[k].clear();
          }
          if (type < 0)
            and_code << indent << "      return true;" << std::endl;
          else
            and_code << indent << "      return false;" << std::endl;
        }
        and_code << indent << "    default:" << std::endl;
        and_code << indent << "      break;" << std::endl;
        and_code << indent << "  }" << std::endl;
      }

      if (generated_and_switch) {
        // handle not switched or leafs
        if (or_not_switched_not_and.size())
          or_code << "  // OR not switched" << std::endl;
        for (std::size_t j = 0; j < or_not_switched_not_and.size(); ++j) {
          std::string functionName = GenerateExprCode(os, or_not_switched_not_and[j]);
          or_code << "  if (" << functionName << "(atom))" << std::endl;
          or_code << "    return true;" << std::endl;
        }
        bool written_comment = false;
        // handle switchable and leafs that were not switched
        for (std::size_t k = 0; k < and_switchable.size(); ++k) {
          std::string indent;
          bool written_if = false;
          for (std::size_t l = 0; l < and_switchable[k].size(); ++l) {
            if (!written_comment) {
              and_code << "  // AND not switched" << std::endl;
              written_comment = true;            
            }
            std::string functionName = GenerateExprCode(os, and_switchable[k][l]);
            and_code << indent << "  if (" << functionName << "(atom))" << std::endl;
            written_if = true;
            indent += "  ";
          }
          // handle not switchable and leafs
          for (std::size_t l = 0; l < and_not_switched[k].size(); ++l) {
            if (!written_comment) {
              and_code << "  // AND not switched" << std::endl;
              written_comment = true;            
            }
            std::string functionName = GenerateExprCode(os, and_not_switched[k][l]);
            and_code << indent<< "  if (" << functionName << "(atom))" << std::endl;
            written_if = true;
            indent += "  ";
          }
          if (written_if)
            and_code << indent << "  return true;" << std::endl;
        }
      } else if (generated_or_switch) {
        // handle not switched or leafs
        if (or_not_switched.size())
          or_code << "  // OR not switched" << std::endl;
        for (std::size_t j = 0; j < or_not_switched.size(); ++j) {
          std::string functionName = GenerateExprCode(os, or_not_switched[j]);
          or_code << "  if (" << functionName << "(atom))" << std::endl;
          or_code << "    return true;" << std::endl;
        }
      }
      // write function
      if (generated_or_switch || generated_and_switch) {
        // open function
        std::string arg_type = m_toolkit->AtomArgType(m_language);
        if (arg_type.size())
          arg_type = make_string(arg_type, " ");
        os << "inline bool EvalSwitchExpr_" << ++m_switch << "(" << arg_type << "atom)" << std::endl;
        os << "{" << std::endl;
        // code
        os << or_code.str();
        os << and_code.str();
        // close function
        os << "  return false;" << std::endl;
        os << "}" << std::endl;
        os << std::endl;
        return make_string("EvalSwitchExpr_", m_switch);
      }

      return "";
    }

    std::string GenerateExprCode(std::ostream &os, AtomExpr *expr)
    {
      switch (expr->type) {
        case AE_ANDHI:
        case AE_ANDLO:
          return BinaryExprFunction(os, BinaryAnd, expr);
        case AE_OR:
          {
            std::string functionName = GenerateSwitchCode(os, expr);
            if (functionName.size())
              return functionName;
            return BinaryExprFunction(os, BinaryOr, expr);
          }
        case AE_RECUR:
          break;
        case AE_NOT:
          return UnaryExprFunction(os, UnaryNot, expr);
        case AE_TRUE:
          return ExprFunction(os, m_true, "EvalTrueExpr", "true", SmartsCompiler::AtomArg);
        case AE_FALSE:
          return ExprFunction(os, m_false, "EvalFalseExpr", "false", SmartsCompiler::AtomArg);
        case AE_AROMATIC:
          return ExprFunction(os, m_arom, "EvalAromaticExpr", m_toolkit->AromaticAtomTemplate(m_language), SmartsCompiler::AtomArg);
        case AE_ALIPHATIC:
          return ExprFunction(os, m_aliph, "EvalAliphaticExpr", m_toolkit->AliphaticAtomTemplate(m_language), SmartsCompiler::AtomArg);
        case AE_CYCLIC:
          return ExprFunction(os, m_cyclic, "EvalCyclicExpr", m_toolkit->CyclicAtomTemplate(m_language), SmartsCompiler::AtomArg);
        case AE_ACYCLIC:
          return ExprFunction(os, m_acyclic, "EvalAcyclicExpr", m_toolkit->AcyclicAtomTemplate(m_language), SmartsCompiler::AtomArg);
        case AE_MASS:
          return ExprFunction(os, m_mass, "EvalMassExpr", m_toolkit->MassAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_ELEM:
          return ExprFunction(os, m_elem, "EvalElementExpr", m_toolkit->ElementAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_AROMELEM:
          return ExprFunction(os, m_aromelem, "EvalAromaticElementExpr", m_toolkit->AromaticElementAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_ALIPHELEM:
          return ExprFunction(os, m_aliphelem, "EvalAliphaticElementExpr", m_toolkit->AliphaticElementAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_HCOUNT:
          return ExprFunction(os, m_hcount, "EvalHydrogenCountExpr", m_toolkit->HydrogenCountAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_CHARGE:
          return ExprFunction(os, m_charge, "EvalChargeExpr", m_toolkit->ChargeAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_CONNECT:
          return ExprFunction(os, m_connect, "EvalConnectExpr", m_toolkit->ConnectAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_DEGREE:
          return ExprFunction(os, m_degree, "EvalDegreeExpr", m_toolkit->DegreeAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_IMPLICIT:
          return ExprFunction(os, m_implicit, "EvalImplicitExpr", m_toolkit->ImplicitAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_RINGS:
          return ExprFunction(os, m_rings, "EvalRingsExpr", m_toolkit->NumRingsAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_SIZE:
          return ExprFunction(os, m_size, "EvalSizeExpr", m_toolkit->RingSizeAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_VALENCE:
          return ExprFunction(os, m_valence, "EvalValenceExpr", m_toolkit->ValenceAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_CHIRAL:
          return ExprFunction(os, m_chiral, "EvalChiralExpr", "true", SmartsCompiler::AtomArg);
        case AE_HYB:
          return ExprFunction(os, m_hyb, "EvalHybridizationExpr", m_toolkit->HybAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        case AE_RINGCONNECT:
          return ExprFunction(os, m_ringconnect, "EvalRingConnectExpr", m_toolkit->RingConnectAtomTemplate(m_language), expr->leaf.value, SmartsCompiler::AtomArg);
        default:
          return ExprFunction(os, m_true, "EvalTrueExpr", "true", SmartsCompiler::AtomArg);
      }
    }

    std::string GenerateExprCode(std::ostream &os, BondExpr *expr)
    {
      switch (expr->type) {
        case BE_ANDHI:
        case BE_ANDLO:
          return BinaryExprFunction(os, BinaryAnd, expr);
        case BE_OR:
          return BinaryExprFunction(os, BinaryOr, expr);
        case BE_NOT:
          return UnaryExprFunction(os, UnaryNot, expr);
        case BE_ANY:
          return ExprFunction(os, m_any, "EvalAnyExpr", "true", SmartsCompiler::BondArg);
        case BE_DEFAULT:
          return ExprFunction(os, m_default, "EvalDefaultExpr", m_toolkit->DefaultBondTemplate(m_language), SmartsCompiler::BondArg);
        case BE_SINGLE:
          return ExprFunction(os, m_single, "EvalSingleExpr", m_toolkit->SingleBondTemplate(m_language), SmartsCompiler::BondArg);
        case BE_DOUBLE:
          return ExprFunction(os, m_double, "EvalDoubleExpr", m_toolkit->DoubleBondTemplate(m_language), SmartsCompiler::BondArg);
        case BE_TRIPLE:
          return ExprFunction(os, m_triple, "EvalTripleExpr", m_toolkit->TripleBondTemplate(m_language), SmartsCompiler::BondArg);
        case BE_AROM:
          return ExprFunction(os, m_barom, "EvalAromaticExpr", m_toolkit->AromaticBondTemplate(m_language), SmartsCompiler::BondArg);
        case BE_RING:
          return ExprFunction(os, m_ring, "EvalRingExpr", m_toolkit->RingBondTemplate(m_language), SmartsCompiler::BondArg);
        case BE_UP:
        case BE_DOWN:
        case BE_UPUNSPEC:
        case BE_DOWNUNSPEC:
        default:
          return ExprFunction(os, m_any, "EvalAnyExpr", "true", SmartsCompiler::BondArg);
      }
    }
  
    void GenerateEvalExprFunction(std::ostream &os, Pattern *pattern)
    {
      switch (m_language) {
        case SmartsCompiler::Cpp:
          os << "bool EvalAtomExpr_" << m_smarts.size() << "(int index, " + m_toolkit->AtomArgType(m_language) + " atom)" << std::endl;
          os << "{" << std::endl;
          os << "  switch (index) {" << std::endl;
          for (int i = 0; i < pattern->acount; ++i) {
            os << "    case " << i << ":" << std::endl;
            os << "      return " << m_atomEvalExpr[i] << "(atom);" << std::endl;
          }
          os << "  }" << std::endl;
          os << "}" << std::endl;
          os << std::endl;

          if (pattern->bcount) {
            os << "bool EvalBondExpr_" << m_smarts.size() << "(int index, " + m_toolkit->BondArgType(m_language) + " bond)" << std::endl;
            os << "{" << std::endl;
            os << "  switch (index) {" << std::endl;
            for (int i = 0; i < pattern->bcount; ++i) {
              os << "    case " << i << ":" << std::endl;
              os << "      return " << m_bondEvalExpr[i] << "(bond);" << std::endl;
            }
            os << "  }" << std::endl;
            os << "}" << std::endl;
            os << std::endl;
          }
          break;
        case SmartsCompiler::Python:
          os << "def EvalAtomExpr_" << m_smarts.size() << "(index, atom):" << std::endl;
          os << "  return { " << 0 << ": " << m_atomEvalExpr[0];
          if (pattern->acount > 1)
            os << "," << std::endl;
          for (int i = 1; i < pattern->acount; ++i) {
            os << "           " << i << ": " << m_atomEvalExpr[i];
            if (i + 1 < pattern->acount)
              os << "," << std::endl;
          }
          os << " }[index](atom)" << std::endl;
          os << std::endl;

          if (pattern->bcount) {
            os << "def EvalBondExpr_" << m_smarts.size() << "(index, bond):" << std::endl;
            os << "  return { " << 0 << ": " << m_bondEvalExpr[0];
            if (pattern->bcount > 1)
              os << "," << std::endl;
            for (int i = 1; i < pattern->bcount; ++i) {
              os << "           " << i << ": " << m_bondEvalExpr[i];
              if (i + 1 < pattern->bcount)
                os << "," << std::endl;
            }
            os << " }[index](bond)" << std::endl;
            os << std::endl;
          }
          break;
      }
    }

    void GenerateSmartsIndexFunction(std::ostream &os)
    {
      switch (m_language) {
        case SmartsCompiler::Cpp:
          os << "int SmartsIndex(const std::string &smarts)" << std::endl;
          os << "{" << std::endl;
          for (int i = 0; i < m_smarts.size(); ++i) {
            os << "  if (smarts == \"" << m_smarts[i] << "\")" << std::endl;
            os << "    return " << i << ";" << std::endl;
          }
          os << "  std::cerr << \"SMARTS \" << smarts << \" not in module.\" << std::endl;" << std::endl;
          os << "  return -1;" << std::endl;
          os << "}" << std::endl;
          os << std::endl;
          break;
        case SmartsCompiler::Python:
          os << "def SmartsIndex(smarts):" << std::endl;
          os << "  try:" << std::endl;
          if (m_smarts.size()) {
            os << "    return { '" << m_smarts[0] << "': " << 0;
            if (m_smarts.size() > 1)
              os << "," << std::endl;
            for (int i = 1; i < m_smarts.size(); ++i) {
              os << "             '" << m_smarts[i] << "': " << i;
              if (i + 1 < m_smarts.size())
                os << "," << std::endl;
            }
          }
          os << " }[smarts]" << std::endl;
          os << "  except:" << std::endl;
          os << "    print 'SMARTS', smarts, 'not in module.'" << std::endl;
          os << "    return -1" << std::endl;
          os << std::endl;
          break;
       }
    }

    void GenerateIsSingleAtomFunction(std::ostream &os)
    {
      if (m_singleatoms.empty())
        return;

      switch (m_language) {
        case SmartsCompiler::Cpp:
          os << "bool IsSingleAtom(int index)" << std::endl;
          os << "{" << std::endl;
          os << "  switch (index) {" << std::endl;
          for (std::set<int>::iterator i = m_singleatoms.begin(); i != m_singleatoms.end(); ++i) {
            os << "    case " << *i << ":" << std::endl;
            os << "      return true;" << std::endl;
          }
          os << "    default:" << std::endl;
          os << "      return false;" << std::endl;
          os << "  }" << std::endl;
          os << "}" << std::endl;
          os << std::endl;
          break;
        case SmartsCompiler::Python:
          os << "def IsSingleAtom(index):" << std::endl;
          os << "  return index in [ "; 
          bool first = true;
          for (std::set<int>::iterator i = m_singleatoms.begin(); i != m_singleatoms.end(); ++i) {
            if (!first)
              os << ", ";
            os << *i;
            first = false;
          }
          os << " ]" << std::endl;
          os << std::endl;
          break;
      }
    }

    void GenerateSmartsPatternFunction(std::ostream &os)
    {
      switch (m_language) {
        case SmartsCompiler::Cpp:
          os << "SmartsPattern* GetSmartsPattern(int index)" << std::endl;
          os << "{" << std::endl;
          if (m_smarts.size() != m_singleatoms.size()) {
            os << "  switch (index) {" << std::endl;
            for (int i = 0; i < m_patterns.size(); ++i) {
              if (m_singleatoms.find(i) != m_singleatoms.end())
                continue;
              os << "    case " << i << ":" << std::endl;
              os << "      {" << std::endl;
              os << "        SmartsPattern *pattern = new SmartsPattern;" << std::endl;
              os << "        pattern->numAtoms = " << m_patterns[i].numAtoms << ";" << std::endl;
              os << "        pattern->ischiral = " << m_patterns[i].ischiral << ";" << std::endl;
              for (int j = 0; j < m_patterns[i].bonds.size(); ++j)
                os << "        pattern->bonds.push_back(SmartsBond(" 
                  << m_patterns[i].bonds[j].src << ", " << m_patterns[i].bonds[j].dst 
                  << ", " << m_patterns[i].bonds[j].grow << "));" << std::endl;
              os << "        pattern->EvalAtomExpr = &EvalAtomExpr_" << i << ";" << std::endl;
              os << "        pattern->EvalBondExpr = &EvalBondExpr_" << i << ";" << std::endl;
              os << "        return pattern;" << std::endl;
              os << "      }" << std::endl;
            }
            os << "    default:" << std::endl;
            os << "      return 0;" << std::endl;
            os << "  }" << std::endl;
          }
          os << "  return 0;" << std::endl;
          os << "}" << std::endl;
          os << std::endl;
          break;
        case SmartsCompiler::Python:
          os << "def GetSmartsPattern(index):" << std::endl;
          for (int i = 0; i < m_patterns.size(); ++i) {
            if (m_singleatoms.find(i) != m_singleatoms.end())
              continue;
            os << "  if index == " << i << ":" << std::endl;
            os << "    pattern = PythonSmartsPattern()" << std::endl;
            os << "    pattern.numAtoms = " << m_patterns[i].numAtoms << std::endl;
            os << "    pattern.ischiral = " << m_patterns[i].ischiral << std::endl;
            os << "    pattern.bonds = [";
            for (int j = 0; j < m_patterns[i].bonds.size(); ++j) {
              os << "SmartsBond("  << m_patterns[i].bonds[j].src << ", "
                 << m_patterns[i].bonds[j].dst << ", " 
                 << m_patterns[i].bonds[j].grow << ")";
              if (j + 1 < m_patterns[i].bonds.size())
                os << ", " << std::endl << "                     ";
            }
            os << "]" << std::endl;
            os << "    pattern.EvalAtomExpr = EvalAtomExpr_" << i << std::endl;
            os << "    pattern.EvalBondExpr = EvalBondExpr_" << i << std::endl;
            os << "    return pattern" << std::endl;
          }
          os << "  return None" << std::endl;
          os << std::endl;
          break;
      }
    }

    void GenerateMatchFunction(std::ostream &os)
    {
      switch (m_language) {
        case SmartsCompiler::Cpp:
          os << "bool Match(OpenBabel::OBMol &mol, const std::string &smarts, std::vector<std::vector<int> > &mlist, bool single = false)" << std::endl;
          os << "{" << std::endl;
          os << "  int index = SmartsIndex(smarts);" << std::endl;
          os << "  if (index < 0)" << std::endl;
          os << "    return false;" << std::endl;
          if (m_singleatoms.size()) {
            os << "  if (IsSingleAtom(index)) {" << std::endl;
            os << "    switch (index) {" << std::endl;
            for (std::set<int>::iterator i = m_singleatoms.begin(); i != m_singleatoms.end(); ++i) {
              os << "      case " << *i << ":" << std::endl;
              os << "        return SingleAtomMatch_" << *i << "(mol, mlist, single);" << std::endl;
            }
            os << "    }" << std::endl;
            os << "  }" << std::endl;
          }
          os << "  SmartsMatcher matcher;" << std::endl;
          os << "  SmartsPattern *pat = GetSmartsPattern(index);" << std::endl;
          os << "  bool result = matcher.Match(mol, pat, mlist, single);" << std::endl;
          os << "  delete pat;" << std::endl;
          os << "  return result;" << std::endl;
          os << "}" << std::endl;
          os << std::endl;
          break;
        case SmartsCompiler::Python:
          os << "def Match(molecule, smarts, mlist, single = False):" << std::endl;
          os << "  global mol" << std::endl;
          os << "  index = SmartsIndex(smarts)" << std::endl;
          os << "  if index < 0:" << std::endl;
          os << "    return False" << std::endl;
          os << "  mol = molecule" << std::endl;
          if (m_singleatoms.size()) {
            os << "  if IsSingleAtom(index):" << std::endl;
            os << "    return { ";
            bool first = true;
            for (std::set<int>::iterator i = m_singleatoms.begin(); i != m_singleatoms.end(); ++i) {
              if (!first)
                os << "," << std::endl << "               ";
              os << *i << ": SingleAtomMatch_" << *i;
              first = false;
            }
            os << " }[index](molecule, mlist, single)" << std::endl;
          }
          os << "  matcher = SmartsMatcher()" << std::endl;
          os << "  pat = GetSmartsPattern(index)" << std::endl;
          os << "  result = matcher.Match(molecule, pat, mlist, single)" << std::endl;
          os << "  return result" << std::endl;
          os << std::endl;
          os << "def MatchGlobalMol(smarts, mlist, single = False):" << std::endl;
          os << "  return Match(mol, smarts, mlist, single)" << std::endl;
          os << std::endl;
          break;
      }
    }
      
    void GenerateSingleAtomMatch(std::ostream &os)
    {
      switch (m_language) {
        case SmartsCompiler::Cpp:
          os << "bool SingleAtomMatch_" << m_smarts.size() << "(OpenBabel::OBMol &mol, std::vector<std::vector<int> > &mlist, bool single)" << std::endl;
          os << "{" << std::endl;
          os << "  FOR_ATOMS_OF_MOL (atom, mol) {" << std::endl;
          os << "    if (" << m_atomEvalExpr[0] << "(&*atom)) {" << std::endl;
          os << "      mlist.push_back(std::vector<int>(1, atom->GetIdx()));" << std::endl;
          os << "      if (single)" << std::endl;
          os << "        return true;" << std::endl;
          os << "    }" << std::endl;
          os << "  }" << std::endl;
          os << "  return mlist.size();" << std::endl;
          os << "}" << std::endl;
          os << std::endl;
          break;
        case SmartsCompiler::Python:
          os << "def SingleAtomMatch_" << m_smarts.size() << "(molecule, mlist, single):" << std::endl;
          os << "  for atom in OBMolAtomIter(molecule):" << std::endl;
          os << "    if " << m_atomEvalExpr[0] << "(atom.GetIdx()):" << std::endl;
          os << "      mlist.append([atom.GetIdx()])" << std::endl;
          os << "      if single:" << std::endl;
          os << "        return True" << std::endl;
          os << "  return True if len(mlist) else False" << std::endl;
          os << std::endl;
          break;
      }
    }

  };


  SmartsCompiler::SmartsCompiler(Toolkit *toolkit, enum Language language) : d(new SmartsCompilerPrivate(toolkit, language))
  {
  }

  SmartsCompiler::~SmartsCompiler()
  {
    delete d;
  }

  void SmartsCompiler::StartSmartsModule(const std::string &name, std::ostream &os)
  {
    if (d->m_language == Cpp) {
      os << "#include <openbabel/atom.h>" << std::endl;
      os << "#include <openbabel/bond.h>" << std::endl;
      os << "#include \"smartspattern.h\"" << std::endl;
      os << "#include \"smartsmatcher.h\"" << std::endl;
      os << std::endl;
      os << "using namespace OpenBabel;" << std::endl;
      os << "using namespace SC;" << std::endl;
      os << std::endl;
      os << "namespace " << name << " {" << std::endl;
      os << std::endl;
    } else {
      os << "from smartscompiler import *" << std::endl;
      os << "from openbabel import *" << std::endl;
      os << std::endl;
      os << "mol = OBMol()" << std::endl;
      os << std::endl;
    }
  }

  void SmartsCompiler::GeneratePatternCode(std::ostream &os, const std::string &smarts, Pattern *pattern)
  {
    d->m_atomEvalExpr.clear();
    d->m_bondEvalExpr.clear();

    for (int i = 0; i < pattern->acount; ++i)
      d->m_atomEvalExpr[i] = d->GenerateExprCode(os, pattern->atom[i].expr);
    for (int i = 0; i < pattern->bcount; ++i)
      d->m_bondEvalExpr[i] = d->GenerateExprCode(os, pattern->bond[i].expr);

    d->GenerateEvalExprFunction(os, pattern);

    if (pattern->acount == 1) {
      // special case for single atom pattern
      d->GenerateSingleAtomMatch(os);      
      d->m_singleatoms.insert(d->m_patterns.size());
      // dummy pattern
      d->m_patterns.push_back(SmartsPattern());
    } else {
      SmartsPattern cpattern;
      cpattern.numAtoms = pattern->acount;
      cpattern.ischiral = pattern->ischiral;
      for (int i = 0; i < pattern->bcount; ++i)
        cpattern.bonds.push_back(SmartsBond(pattern->bond[i].src, pattern->bond[i].dst, pattern->bond[i].grow));
      d->m_patterns.push_back(cpattern);
    }

    d->m_smarts.push_back(smarts);
  }

  void SmartsCompiler::StopSmartsModule(std::ostream &os)
  {
    d->GenerateSmartsIndexFunction(os);
    d->GenerateIsSingleAtomFunction(os);
    d->GenerateSmartsPatternFunction(os);
    d->GenerateMatchFunction(os);

    if (d->m_language == Cpp)
      os << "} // end namespace" << std::endl;
  }

}
