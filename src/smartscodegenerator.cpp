#include "smartscodegenerator.h"
#include "smartspattern.h"
#include "smartsprint.h"
#include "util.h"
#include "toolkit.h"
#include "pattern.h"
#include "defines.h"

#include "openbabel.h"

#include <set>
#include <cassert>

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
 
  struct SmartsCodeGeneratorPrivate
  {
    Toolkit *m_toolkit;
    enum SmartsCodeGenerator::Language m_language;
    int m_not;
    int m_and;
    int m_or;
    int m_switch;

    std::stringstream m_os;
    std::map<int, std::string> m_atomEvalExpr;
    std::map<int, std::string> m_bondEvalExpr;
    std::vector<std::string> m_smarts;
    std::vector<SmartsPattern<OBAtom, OBBond> > m_patterns;
    std::set<int> m_singleatoms;
    std::vector<std::string> m_functions;

    bool m_noinline;
    bool m_noswitch;
    bool m_nomatch;
    bool m_optfunc;

    SmartsCodeGeneratorPrivate(Toolkit *toolkit, enum SmartsCodeGenerator::Language language)
        : m_toolkit(toolkit), m_language(language), m_and(0), m_or(0), m_not(0),
        m_switch(0), m_noinline(false), m_noswitch(false), m_optfunc(false)
    {
    }

    std::string CommentString()
    {
      switch (m_language) {
        case SmartsCodeGenerator::Cpp:
          return "// ";
        case SmartsCodeGenerator::Python:
          return "# ";
      }
    }

    std::string UnaryNotString()
    {
      switch (m_language) {
        case SmartsCodeGenerator::Cpp:
          return "!";
        case SmartsCodeGenerator::Python:
          return "not ";
        default:
          return "";
      }
    }

    std::string BinaryAndString()
    {
      switch (m_language) {
        case SmartsCodeGenerator::Cpp:
          return " && ";
        case SmartsCodeGenerator::Python:
          return " and ";
        default:
          return "";
      }
    }

    std::string BinaryOrString()
    {
      switch (m_language) {
        case SmartsCodeGenerator::Cpp:
          return " || ";
        case SmartsCodeGenerator::Python:
          return " or ";
        default:
          return "";
      }
    }

    std::string Bracket(const std::string &code)
    {
      if (code.find(stripped(BinaryAndString())) != std::string::npos)
        return "(" + code + ")";
      if (code.find(stripped(BinaryOrString())) != std::string::npos)
        return "(" + code + ")";
      return code;
    }

    std::string ExprFunctionName(const SmartsAtomExpr *expr)
    {
      switch (expr->type) {
        case Smiley::OP_Not:
          return make_string("Not_", ExprFunctionName(expr->unary.arg));
        case Smiley::AE_True:
          return "EvalTrueExpr";
        case Smiley::AE_False:
          return "EvalFalseExpr";
        case Smiley::AE_Aromatic:
          return "EvalAromaticExpr";
        case Smiley::AE_Aliphatic:
          return "EvalAliphaticExpr";
        case Smiley::AE_Cyclic:
          return "EvalCyclicExpr";
        case Smiley::AE_Acyclic:
          return "EvalAcyclicExpr";
        case Smiley::AE_Isotope:
          return make_string("EvalMassExpr_", expr->leaf.value);
        case Smiley::AE_AtomicNumber:
          return make_string("EvalElementExpr_", expr->leaf.value);
        case Smiley::AE_AromaticElement:
          return make_string("EvalAromaticElementExpr_", expr->leaf.value);
        case Smiley::AE_AliphaticElement:
          return make_string("EvalAliphaticElementExpr_", expr->leaf.value);
        case Smiley::AE_TotalH:
          return make_string("EvalHydrogenCountExpr_", expr->leaf.value);
        case Smiley::AE_Charge:
          return make_string("EvalChargeExpr_", expr->leaf.value);
        case Smiley::AE_Connectivity:
          return make_string("EvalConnectExpr_", expr->leaf.value);
        case Smiley::AE_Degree:
          return make_string("EvalDegreeExpr_", expr->leaf.value);
        case Smiley::AE_ImplicitH:
          return make_string("EvalImplicitExpr_", expr->leaf.value);
        case Smiley::AE_RingMembership:
          return make_string("EvalRingsExpr_", expr->leaf.value);
        case Smiley::AE_RingSize:
          return make_string("EvalSizeExpr_", expr->leaf.value);
        case Smiley::AE_Valence:
          return make_string("EvalValenceExpr_", expr->leaf.value);
        case Smiley::AE_Chirality:
          return "EvalChiralExpr";
        //case AE_HYB:
        //  return make_string("EvalHybridizationExpr_", expr->leaf.value);
        case Smiley::AE_RingConnectivity:
          return make_string("EvalRingConnectExpr_", expr->leaf.value);
        default:
          return "EvalTrueExpr";
      }
    }

    std::string ExprFunctionName(const SmartsBondExpr *expr)
    {
      switch (expr->type) {
        case Smiley::OP_Not:
          return make_string("Not_", ExprFunctionName(expr->unary.arg));
        case Smiley::BE_True:
          return "EvalAnyExpr";
        case BE_DEFAULT:
          return "EvalDefaultExpr";
        case Smiley::BE_Single:
          return "EvalSingleExpr";
        case Smiley::BE_Double:
          return "EvalDoubleExpr";
        case Smiley::BE_Triple:
          return "EvalTripleExpr";
        case Smiley::BE_Aromatic:
          return "EvalAromaticExpr";
        case Smiley::BE_Ring:
          return "EvalRingExpr";
        default:
          return "EvalAnyExpr";
      }
    }

    std::string ExprString(const SmartsAtomExpr *expr)
    {
      if (!IsLeaf(expr) && !IsNegatedLeaf(expr))
        return "";
      std::string code;
      switch (expr->type) {
        case Smiley::OP_Not:
          code = ExprString(expr->unary.arg);
          break;
        case Smiley::AE_True:
          code = "true";
          break;
        case Smiley::AE_False:
          code = "false";
          break;
        case Smiley::AE_Aromatic:
          code = m_toolkit->AromaticAtomTemplate(m_language);
          break;
        case Smiley::AE_Aliphatic:
          code = m_toolkit->AliphaticAtomTemplate(m_language);
          break;
        case Smiley::AE_Cyclic:
          code = m_toolkit->CyclicAtomTemplate(m_language);
          break;
        case Smiley::AE_Acyclic:
          code = m_toolkit->AcyclicAtomTemplate(m_language);
          break;
        case Smiley::AE_Isotope:
          code = m_toolkit->MassAtomTemplate(m_language);
          break;
        case Smiley::AE_AtomicNumber:
          code = m_toolkit->ElementAtomTemplate(m_language);
          break;
        case Smiley::AE_AromaticElement:
          code = m_toolkit->AromaticElementAtomTemplate(m_language);
          break;
        case Smiley::AE_AliphaticElement:
          code = m_toolkit->AliphaticElementAtomTemplate(m_language);
          break;
        case Smiley::AE_TotalH:
          code = m_toolkit->HydrogenCountAtomTemplate(m_language);
          break;
        case Smiley::AE_Charge:
          code = m_toolkit->ChargeAtomTemplate(m_language);
          break;
        case Smiley::AE_Connectivity:
          code = m_toolkit->ConnectAtomTemplate(m_language);
          break;
        case Smiley::AE_Degree:
          code = m_toolkit->DegreeAtomTemplate(m_language);
          break;
        case Smiley::AE_ImplicitH:
          code = m_toolkit->ImplicitAtomTemplate(m_language);
          break;
        case Smiley::AE_RingMembership:
          code = m_toolkit->NumRingsAtomTemplate(m_language);
          break;
        case Smiley::AE_RingSize:
          code = m_toolkit->RingSizeAtomTemplate(m_language);
          break;
        case Smiley::AE_Valence:
          code = m_toolkit->ValenceAtomTemplate(m_language);
          break;
        case Smiley::AE_Chirality:
          code = "true";
          break;
        //case AE_HYB:
        //  code = m_toolkit->HybAtomTemplate(m_language);
        //  break;
        case Smiley::AE_RingConnectivity:
          code = m_toolkit->RingConnectAtomTemplate(m_language);
          break;
        default:
          break;
      }
      if (code.empty())
        return code;
      if (IsNegatedLeaf(expr))
        code = make_string(UnaryNotString(), "(", code, ")");
      if (IsValued(expr))
        replace_all(code, "$value", make_string(expr->leaf.value));
      if (IsNegatedLeaf(expr) && IsValued(expr->unary.arg))
        replace_all(code, "$value", make_string(expr->unary.arg->leaf.value));
      return code; 
    }
    
    std::string ExprString(const SmartsBondExpr *expr)
    {
      if (!IsLeaf(expr) && !IsNegatedLeaf(expr))
        return "";
      std::string code;
      switch (expr->type) {
        case Smiley::OP_Not:
          code = ExprString(expr->unary.arg);
        case Smiley::BE_True:
          code = "true";
        case BE_DEFAULT:
          code = m_toolkit->DefaultBondTemplate(m_language);
        case Smiley::BE_Single:
          code = m_toolkit->SingleBondTemplate(m_language);
        case Smiley::BE_Double:
          code = m_toolkit->DoubleBondTemplate(m_language);
        case Smiley::BE_Triple:
          code = m_toolkit->TripleBondTemplate(m_language);
        case Smiley::BE_Aromatic:
          code = m_toolkit->AromaticBondTemplate(m_language);
        case Smiley::BE_Ring:
          code = m_toolkit->RingBondTemplate(m_language);
        case Smiley::BE_Up:
        case Smiley::BE_Down:
        //case Smiley::BE_UpUNSPEC:
        //case BE_DOWNUNSPEC:
        default:
          break;
      }
      if (IsNegatedLeaf(expr))
        code = make_string(UnaryNotString(), "(", code, ")");
      return code;
    }

    std::string FunctionTemplate(const std::string &function, enum SmartsCodeGenerator::ArgType arg, const std::string &expr, bool isOp)
    {
      std::string temp;
      switch (m_language) {
        case SmartsCodeGenerator::Cpp:
          temp = cpp_template;
          break;
        case SmartsCodeGenerator::Python:
          if (isOp)
            temp = python_op_template;
          else
            temp = python_template;
          break;
        default:
          break;
      }

      replace_all(temp, "$function", function);

      std::string arg_name = arg == SmartsCodeGenerator::AtomArg ? "atom" : "bond";
      std::string arg_type = arg == SmartsCodeGenerator::AtomArg ? m_toolkit->AtomArgType(m_language) : m_toolkit->BondArgType(m_language);
      if (arg_type.size())
        arg_type = make_string(arg_type, " ", arg_name);
      else
        arg_type = arg_name;
      replace_all(temp, "$arg", arg_type);

      if (m_language == SmartsCodeGenerator::Python) {
        std::string obj = arg == SmartsCodeGenerator::AtomArg ? "Atom" : "Bond";
        replace_all(temp, "$obj", obj);
      }

      replace_all(temp, "$expr", expr);
      return temp;
    }


    int GetValue(SmartsAtomExpr *expr)
    {
      return expr->leaf.value;
    }

    int GetValue(SmartsBondExpr *expr)
    {
      return 0;
    }

    // function
    template<typename Expr>
    std::string ExprFunction(std::ostream &os, const std::string &function, const std::string &exprTemplate, Expr *expr)
    {
      std::string functionName = function;
      std::string expr2 = exprTemplate;
      if (IsValued(expr)) {
        int value = GetValue(expr);
        functionName = value < 0 ? make_string(function, "_neg", -value) : make_string(function, "_", value);
        replace_all(expr2, "$value", make_string(value));
      }

      if (std::find(m_functions.begin(), m_functions.end(), functionName) != m_functions.end())
        return functionName;
      
      enum SmartsCodeGenerator::ArgType argType = SameType<SmartsAtomExpr, Expr>::result ? SmartsCodeGenerator::AtomArg : SmartsCodeGenerator::BondArg;
      os << CommentString() << GetExprString(expr) << std::endl;
      os << FunctionTemplate(functionName, argType, expr2, false);
      os << std::endl;

      m_functions.push_back(functionName);
      return functionName;
    }

    enum UnaryOp
    {
      UnaryNot
    };

    // unary function
    template<typename Expr>
    std::string UnaryExprFunction(std::ostream &os, enum UnaryOp op, const Expr *expr)
    {
      enum SmartsCodeGenerator::ArgType argType = SameType<Expr, SmartsAtomExpr>::result ? SmartsCodeGenerator::AtomArg : SmartsCodeGenerator::BondArg;
      std::string arg = argType == SmartsCodeGenerator::AtomArg ? "(atom)" : "(bond)";

      std::string code = m_noinline ? "" : ExprString(expr);
 
      std::string functionName, expr_str;
      if (code.empty() || m_noinline) {
        switch (op) {
          case UnaryNot:
            functionName = make_string("EvalNotExpr_", ++m_not);
            std::string argFunctionName = GenerateExprFunction(os, expr->unary.arg);
            expr_str = UnaryNotString() + argFunctionName + arg;
            break;
        }
      } else {
        switch (op) {
          case UnaryNot:
            functionName = ExprFunctionName(expr);
            if (std::find(m_functions.begin(), m_functions.end(), functionName) != m_functions.end())
              return functionName;
            expr_str = code;
            break;
        }
      }

      os << CommentString() << GetExprString(expr) << std::endl;
      os << FunctionTemplate(functionName, argType, expr_str, true);
      os << std::endl;

      m_functions.push_back(functionName);
      return functionName;
    }


    enum BinaryOp
    {
      BinaryAnd,
      BinaryOr,
    };

    // binary function
    template<typename Expr>
    std::string BinaryExprFunction(std::ostream &os, enum BinaryOp op, const Expr *expr)
    {
      enum SmartsCodeGenerator::ArgType argType = SameType<Expr, SmartsAtomExpr>::result ? SmartsCodeGenerator::AtomArg : SmartsCodeGenerator::BondArg;
      std::string arg = argType == SmartsCodeGenerator::AtomArg ? "(atom)" : "(bond)";
      
      std::string lft_expr = m_noinline ? "" : Bracket(ExprString(expr->binary.lft));
      std::string rgt_expr = m_noinline ? "" :  Bracket(ExprString(expr->binary.rgt));

      std::string op_str, op_expr;
      switch (op) {
        case BinaryAnd:
          op_expr = BinaryAndString();
          op_str = "And";
          break;
        case BinaryOr:
          op_expr = BinaryOrString();
          op_str = "Or";
          break;
      }
 
      bool lft_leaf = true, rgt_leaf = true;
      if (lft_expr.empty()) {
        lft_expr = GenerateExprFunction(os, expr->binary.lft) + arg;
        lft_leaf = false;
      }
      if (rgt_expr.empty()) {
        rgt_expr = GenerateExprFunction(os, expr->binary.rgt) + arg;
        rgt_leaf = false;
      }
        
      std::string functionName;
      if (lft_leaf && rgt_leaf && !m_noinline) {
        functionName = ExprFunctionName(expr->binary.lft) + "_" + op_str + "_" + ExprFunctionName(expr->binary.rgt);
        if (std::find(m_functions.begin(), m_functions.end(), functionName) != m_functions.end())
          return functionName;
      } else
        switch (op) {
          case BinaryAnd:
            functionName = make_string("EvalAndExpr_", ++m_and);
            break;
          case BinaryOr:
            functionName = make_string("EvalOrExpr_", ++m_or);
            break;
        }
      
      os << CommentString() << GetExprString(expr) << std::endl;
      std::string expr_str = lft_expr + op_expr + rgt_expr ;
      os << FunctionTemplate(functionName, argType, expr_str, true);
      os << std::endl;

      m_functions.push_back(functionName);
      return functionName;
    }


    std::string GetValuedExprCode(int type)
    {
      switch (type) {
        case Smiley::AE_Isotope:
          return m_toolkit->MassAtomTemplate(m_language);
        case Smiley::AE_AtomicNumber:
          return m_toolkit->ElementAtomTemplate(m_language);
        case Smiley::AE_AromaticElement:
          return m_toolkit->AromaticElementAtomTemplate(m_language);
        case Smiley::AE_AliphaticElement:
          return m_toolkit->AliphaticElementAtomTemplate(m_language);
        case Smiley::AE_TotalH:
          return m_toolkit->HydrogenCountAtomTemplate(m_language);
        case Smiley::AE_Charge:
          return m_toolkit->ChargeAtomTemplate(m_language);
        case Smiley::AE_Connectivity:
          return m_toolkit->ConnectAtomTemplate(m_language);
        case Smiley::AE_Degree:
          return m_toolkit->DegreeAtomTemplate(m_language);
        case Smiley::AE_ImplicitH:
          return m_toolkit->ImplicitAtomTemplate(m_language);
        case Smiley::AE_RingMembership:
          return m_toolkit->NumRingsAtomTemplate(m_language);
        case Smiley::AE_RingSize:
          return m_toolkit->RingSizeAtomTemplate(m_language);
        case Smiley::AE_Valence:
          return m_toolkit->ValenceAtomTemplate(m_language);
        //case AE_HYB:
        //  return m_toolkit->HybAtomTemplate(m_language);
        case Smiley::AE_RingConnectivity:
          return m_toolkit->RingConnectAtomTemplate(m_language);
      }

      return "";
    }

    template<typename Expr>
    bool IsTypeOrNegatedType(Expr *expr, int type)
    {
      return expr->type == type || (IsNot(expr) && expr->unary.arg->type == type);
    }

    void FindSwitchableLeafs(const std::vector<SmartsAtomExpr*> &leafs, std::vector<SmartsAtomExpr*> &switchable, std::vector<SmartsAtomExpr*> &not_switchable)
    {
      for (std::size_t i = 0; i < leafs.size(); ++i)
        if ((IsNot(leafs[i]) && m_toolkit->IsSwitchable(leafs[i]->unary.arg->type)) || m_toolkit->IsSwitchable(leafs[i]->type))
          switchable.push_back(leafs[i]);
        else
          not_switchable.push_back(leafs[i]);
    }
    
    void SortSwitchableByType(const std::vector<SmartsAtomExpr*> &switchable, std::map<int, std::vector<SmartsAtomExpr*> > &type_to_switchable)
    {
      for (std::size_t i = 0; i < switchable.size(); ++i)
        if (IsNot(switchable[i]))
          type_to_switchable[switchable[i]->unary.arg->type].push_back(switchable[i]);
        else
          type_to_switchable[switchable[i]->type].push_back(switchable[i]);
    }

    void SortSplitSwitchableByType(const std::vector<std::vector<SmartsAtomExpr*> > &switchable, std::vector<std::vector<SmartsAtomExpr*> > &not_switched,
        std::map<int, std::vector<std::pair<int, SmartsAtomExpr*> > > &type_to_switchable)
    {
      for (std::size_t i = 0; i < switchable.size(); ++i) {
        std::set<int> added_types;
        for (std::size_t j = 0; j < switchable[i].size(); ++j) {
          if (IsNot(switchable[i][j])) {
            int type = switchable[i][j]->unary.arg->type;
            // add only a single leaf for each type for each and
            if (added_types.find(type) == added_types.end())
              type_to_switchable[type].push_back(std::make_pair(i, switchable[i][j]));
            else
              not_switched[i].push_back(switchable[i][j]);
            added_types.insert(type);
          } else {
            int type = switchable[i][j]->type;
            // add only a single leaf for each type for each and
            if (added_types.find(type) == added_types.end())
              type_to_switchable[type].push_back(std::make_pair(i, switchable[i][j]));
            else
              not_switched[i].push_back(switchable[i][j]);
            added_types.insert(type);
          }
        }
      }

      // don't try to switch single leafs
      for (std::map<int, std::vector<std::pair<int, SmartsAtomExpr*> > >::iterator i = type_to_switchable.begin(); i != type_to_switchable.end(); ++i)
        if (i->second.size() < 2)
          not_switched[i->second[0].first].push_back(i->second[0].second);
    }
   
    enum {
      InsideOr = true,
      NotInsideOr = false,
      InsideAnd = true,
      NotInsideAnd = false,
      Split = true,
      NotSplit = false
    };

    /**
     * // comment
     * if (predicate)
     * switch (expr) {
     *   // switchables
     *   case i:
     * $case_i
     *   case j:
     * $case_j
     *   default:
     * $default
     * }
     *
     *
     *
     *
     *
     */
    void GenerateSwitches(std::ostream &os, const std::vector<SmartsAtomExpr*> &same_type_switchable, int binaryOp, bool split)
    {
      assert(same_type_switchable.size() > 1);
      std::stringstream code;
      // print comment
      std::string op = binaryOp == BinaryOr ? "," : "&";
      code << "  " << CommentString() << GetBinaryExprString(same_type_switchable, op) << std::endl;

      int type = IsNot(same_type_switchable[0]) ? same_type_switchable[0]->unary.arg->type : same_type_switchable[0]->type;

      // sort by value
      std::map<int, std::vector<SmartsAtomExpr*> > value_to_switchable;
      std::map<int, std::vector<SmartsAtomExpr*> > value_to_negated_switchable;
      for (std::size_t i = 0; i < same_type_switchable.size(); ++i)
        if (IsNot(same_type_switchable[i]))
          value_to_negated_switchable[same_type_switchable[i]->unary.arg->leaf.value].push_back(same_type_switchable[i]->unary.arg);
        else
          value_to_switchable[same_type_switchable[i]->leaf.value].push_back(same_type_switchable[i]);

      if (!split) {
        for (std::map<int, std::vector<SmartsAtomExpr*> >::iterator i = value_to_switchable.begin(); i != value_to_switchable.end(); ++i)
          if (value_to_negated_switchable.find(i->first) != value_to_negated_switchable.end()) {
            if (binaryOp == BinaryAnd) {
              // C!C -> false
              os << "  return false;" << std::endl;
              return;
            } else {
              // C,!C -> true
              os << "  return true;" << std::endl;
              return;            
            }
          }
        if (binaryOp == BinaryAnd && value_to_switchable.size() > 1) {
          // CN -> false
          os << "  return false;" << std::endl;
          return;        
        }
        if (binaryOp == BinaryOr && value_to_negated_switchable.size() > 1) {
          // !C,!N -> true
          os << "  return true;" << std::endl;
          return;        
        }
      }

      // get expression and predicate code
      std::string expr_code = m_toolkit->GetSwitchExpr(m_language, type);
      std::string pred_code = m_toolkit->GetSwitchPredicate(m_language, type);

      // predicates
      std::string indent;
      if (pred_code.size())
        code << "  if (" << pred_code << ")" << std::endl;
     
      std::set<int> handled_cases;
      // generate switch statement
      code << indent << "  switch (" << expr_code << ") {" << std::endl;
      // switchable cases
      if (value_to_switchable.size() && (split || binaryOp != BinaryOr || value_to_negated_switchable.empty())) {
        code << indent << "    " << CommentString() << "switchables" << std::endl;
        for (std::map<int, std::vector<SmartsAtomExpr*> >::iterator i = value_to_switchable.begin(); i != value_to_switchable.end(); ++i) {
          code << indent << "    case " << i->first << ":" << std::endl;
          if (split)
            code << "      $case_" << i->first << std::endl;
          handled_cases.insert(i->first);
        }
        if (!split) {
          if (binaryOp == BinaryAnd)
            code << indent << "      break;" << std::endl;
          else
            code << indent << "      return true;" << std::endl;
        }
        // default
        if (!split) {
          code << indent << "    default:" << std::endl;
          if (binaryOp == BinaryAnd)
            code << indent << "      return false;" << std::endl;
          else
            code << indent << "      break;" << std::endl;
        }
      }
      // !switchable cases
      if (value_to_negated_switchable.size() && (split || binaryOp != BinaryAnd || value_to_switchable.empty())) {
        code << indent << "    " << CommentString() << "negated switchables" << std::endl;
        for (std::map<int, std::vector<SmartsAtomExpr*> >::iterator i = value_to_negated_switchable.begin(); i != value_to_negated_switchable.end(); ++i) {
          if (handled_cases.find(i->first) != handled_cases.end())
            continue;
          code << indent << "    case " << i->first << ":" << std::endl;
          if (split)
            code << "      $case_" << i->first << std::endl;
        }
        if (!split) {
          if (binaryOp == BinaryAnd)
            code << indent << "      return false;" << std::endl;
          else
            code << indent << "      break;" << std::endl;
        }
        // default
        if (!split) {
          code << indent << "    default:" << std::endl;
          if (binaryOp == BinaryAnd)
            code << indent << "      break;" << std::endl;
          else
            code << indent << "      return true;" << std::endl;
        }
      }
      if (split) {
        code << indent << "    default:" << std::endl;
        code << "      $default" << std::endl;
      }
      code << indent << "  }" << std::endl;
      // write comment
      if (binaryOp == BinaryAnd)
        os << "  " << CommentString() << "AND switchables" << std::endl;
      else
        os << "  " << CommentString() << "OR switchables" << std::endl;
      os << code.str();
    }
    
    void GenerateNotSwitched(std::ostream &os, const std::vector<SmartsAtomExpr*> &not_switched, int binaryOp, bool split)
    {
      if (not_switched.empty())
        return;
      if (binaryOp == BinaryAnd) {
        os << "  " << CommentString() << "AND not switched" << std::endl;
        os << "  " << CommentString() << GetBinaryExprString(not_switched, "&") << std::endl;
      } else {
        os << "  " << CommentString() << "OR not switched" << std::endl;
        os << "  " << CommentString() << GetBinaryExprString(not_switched, ",") << std::endl;
      }
      os << "  if (";
      for (std::size_t i = 0; i < not_switched.size(); ++i) {
        if (i) {
          if (binaryOp == BinaryAnd)
            os << BinaryAndString();
          else
            os << BinaryOrString();
        }
        /*
        if (binaryOp == BinaryAnd) {
          if (IsNot(not_switched[i]))
            os << ExprString(not_switched[i]->unary.arg);
          else
            os << "!(" << ExprString(not_switched[i]) << ")";
        } else {
        */
          os << ExprString(not_switched[i]);
        //}
      }
      os << ")" << std::endl;
      /*
      if (binaryOp == BinaryAnd)
        os << "    return false;" << std::endl;
      else
      */
        os << "    return true;" << std::endl;
    }

    void Debug(const std::string &label, std::ostream &os, const std::vector<SmartsAtomExpr*> &expr, bool endl = true)
    {
      if (endl)
        os << CommentString();
      if (expr.empty())
        os << label << ": []";
      else
        os << label << ": [ ";
      for (std::size_t i = 0; i < expr.size(); ++i) {
        if (i)
          os << " ";
        os << GetExprString(expr[i]);
      }
      if (!expr.empty())
        os << " ]";
      if (endl)
        os << std::endl;
    }
    void Debug(const std::string &label, std::ostream &os, const std::vector<std::vector<SmartsAtomExpr*> > &expr, bool endl = true)
    {
      os << CommentString();
      if (expr.empty())
        os << label << ": []";
      else
        os << label << ": [ ";
      for (std::size_t i = 0; i < expr.size(); ++i) {
        if (i)
          os << " ], [ ";
        Debug(make_string(i), os, expr[i], false);
      }
      if (!expr.empty())
        os << " ]";
      if (endl)
        os << std::endl;
    }
    void Debug(const std::string &label, std::ostream &os, const std::vector<std::pair<int, SmartsAtomExpr*> > &expr, bool endl = true)
    {
      os << CommentString();
      if (expr.empty())
        os << label << ": []";
      else
        os << label << ": [ ";
      for (std::size_t i = 0; i < expr.size(); ++i) {
        if (i)
          os << " ], [ ";
        os << GetExprString(expr[i].second);
      }
      if (!expr.empty())
        os << " ]";
      if (endl)
        os << std::endl;
    }

    /**
     * Not switchable: [RA]
     *
     *   if (!R)
     *     return false
     *   if (!A)
     *     return false
     *   return true
     *
     * Switchable: [!C!O]
     *
     *   switch (expr)
     *     case C:
     *       return false
     *     case O:
     *       return false
     *     default:
     *       break
     *   return true
     *
     *   note: [CC] = [C], [C!C] = [!*], [CO] = [!*], [!CO] = [O]
     *
     * Switchable & not switchable: [R!C!O]
     *
     *   switch (expr)
     *     case C:
     *       return false
     *     case O:
     *       return false
     *     default:
     *       break
     *   if (!R)
     *     return false
     *   return true
     */
    bool GenerateHighAndSwitchCode(std::ostream &os, std::vector<SmartsAtomExpr*> &other, bool insideOr = false, bool split = false)
    {
      // find all ANDs switchable leafs
      std::vector<SmartsAtomExpr*> switchable, not_switched; // [expr]
      FindSwitchableLeafs(other, switchable, not_switched);

      // sort switchable by type
      std::map<int, std::vector<SmartsAtomExpr*> > switchable_types; // type -> [expr]
      SortSwitchableByType(switchable, switchable_types);

      // generate all switch statements
      std::stringstream code;
      std::string op = insideOr ? "," : "&";
      for (std::map<int, std::vector<SmartsAtomExpr*> >::iterator i = switchable_types.begin(); i != switchable_types.end(); ++i) {
        if (i->second.size() < 2) {
          for (std::size_t j = 0; j < i->second.size(); ++j)
            not_switched.push_back(i->second[j]);
          continue;
        }

        // generate switch satements
        std::stringstream switch_code;
        if (split)
          GenerateSwitches(switch_code, i->second, BinaryOr, Split);
        else
          GenerateSwitches(switch_code, i->second, BinaryAnd, NotSplit);
        // check if the whole expression is false
        if (switch_code.str() == "  return false;\n") {
          os << "  " << CommentString() << GetBinaryExprString(i->second, "&") << std::endl;
          os << switch_code.str();
          return false;
        }
        code << switch_code.str();
      }
    
      // handle and leafs that were not switched
      GenerateNotSwitched(code, not_switched, BinaryAnd, insideOr);

      os << code.str();
      if (!insideOr)
        //os << "  return true;" << std::endl;
        os << "  return false;" << std::endl;
      return true;
    }
    
    std::string GenerateHighAndSwitchFunction(std::ostream &os, SmartsAtomExpr *expr)
    {
      if (m_noswitch)
        return "";
      // find all ANDs
      std::vector<SmartsAtomExpr*> ands, other; // [expr]
      FindSameBinaryExpr(expr, ands, other);
      std::stringstream code;
      GenerateHighAndSwitchCode(code, other, NotInsideOr, NotSplit);
      
      // open function
      std::string arg_type = m_toolkit->AtomArgType(m_language);
      if (arg_type.size())
        arg_type = make_string(arg_type, " ");
      os << CommentString() + GetExprString(expr) << std::endl;
      os << "inline bool EvalAndExpr_" << ++m_and << "(" << arg_type << "atom)" << std::endl;
      os << "{" << std::endl;
      // code
      os << code.str();
      os << "}" << std::endl;
      os << std::endl;
      std::string functionName = make_string("EvalAndExpr_", m_and);
      m_functions.push_back(functionName);
      return functionName;
    }

    std::string ExtractCase(const std::string &str)
    {
      std::string case_str = str.substr(str.find("_", 5) + 1); // $and_i_X_Y -> X_Y
      case_str.resize(case_str.rfind("_")); // X_Y -> X
      return case_str;
    }

    void ReplaceAndIndent(std::string &str, const std::string &what, const std::string &with)
    {
      // find what's pos in str
      std::size_t pos = str.find(what);
      std::size_t newline_pos = str.rfind("\n", pos);
      // find with's indentation
      std::string withCopy(with);
      while (withCopy.size() && withCopy[0] == ' ')
        withCopy = withCopy.substr(1);

      int what_indent = pos - newline_pos - 1;
      int with_indent = with.size() - withCopy.size();

      if (what_indent > with_indent) {
        std::string indent(what_indent - with_indent, ' ');
        replace_all(withCopy, "\n", "\n" + indent);
        replace_all(withCopy, "\n" + indent + "\n", "\n");
        withCopy.resize(withCopy.rfind("\n"));
      } else if (with_indent > what_indent) {
        std::string indent(with_indent - what_indent, ' ');
        replace_all(withCopy, "\n" + indent, "\n");
        replace_all(withCopy, "\n" + indent + "\n", "\n");
      }
      

      replace_first(str, what, withCopy);
    }
      
    void RemoveEmptyLines(std::string &str)
    {
      replace_all(str, "\n\n", "\n");
      std::size_t last_pos = str.find("\n");
      if (last_pos++ == std::string::npos)
        return;
      std::size_t pos = str.find("\n", last_pos);
      if (pos == std::string::npos)
        return;
      while ((pos = str.find("\n", last_pos)) != std::string::npos) {
        bool all_space = true;
        for (std::size_t i = last_pos; i < str.size(); ++i) {
          if (str[i] == '\n')
            break;
          if (str[i] != ' ') {
            all_space = false;
            break;
          }
        }
        if (all_space)
          str.replace(last_pos, pos - last_pos + 1, "");
        last_pos = pos + 1;
      }
      //while (str.size() && str[str.size() - 1] == ' ')
      //  str.resize(str.size() - 1);
      replace_all(str, "\n  \n", "\n");
    }

    /**
     * Not switchable: [R,A]
     *
     *   if (R)
     *     return true
     *   if (A)
     *     return true
     *   return false
     *
     * Switchable: [C,O]
     *
     *   switch (expr)
     *     case C:
     *       return true
     *     case O:
     *       return true
     *     default:
     *       break
     *   return false
     *
     * Switchable & not switchable: [R,C,O]
     *
     *   switch (expr)
     *     case C:
     *       return true
     *     case O:
     *       return true
     *     default:
     *       break
     *   if (R)
     *     return true
     *   return false
     *
     * Not switchable ANDs: [r5a,r6A]
     *
     *   and_1 = true
     *   and_2 = true
     *   if (!r5 || !a)               if (r5 && a)
     *     and_1 = false                return true
     *   if (and_1)
     *     return true
     *   if (!r6 || !A)               if (r6 && A)
     *     and_2 = false                return true
     *   if (and_2)
     *     return true
     *   return false                 return false
     *
     * Switchable ANDs in same leaf: [RA,!C!O]
     *
     *   and_1 = true
     *   and_2 = true
     *   ...and_1 code...
     *   switch (expr)
     *     case C:
     *       and_2 = false
     *     case O:
     *       and_2 = false
     *     default:
     *       break
     *   if (and_2)
     *     return true
     *   return false
     *
     * Switchable ANDs in different leafs: [CA,NR]
     *
     *   switch (expr)
     *     case C:
     *       if (A)
     *         return true
     *       break
     *     case N:
     *       if (R)
     *         return true
     *       break
     *     default:
     *       break
     *
     */
    bool GenerateOrSwitchCode(std::ostream &os, std::vector<SmartsAtomExpr*> &or_other, bool insideAnd = false)
    {
      // find all ORs switchable leafs
      std::vector<SmartsAtomExpr*> or_switchable, or_not_switched; // [expr]
      FindSwitchableLeafs(or_other, or_switchable, or_not_switched);

      // sort or_switchable by type
      std::map<int, std::vector<SmartsAtomExpr*> > or_switchable_types; // type -> [expr]
      SortSwitchableByType(or_switchable, or_switchable_types);

      // generate all switch statements
      std::stringstream or_code;
      for (std::map<int, std::vector<SmartsAtomExpr*> >::iterator i = or_switchable_types.begin(); i != or_switchable_types.end(); ++i) {
        if (i->second.size() < 2) {
          for (std::size_t j = 0; j < i->second.size(); ++j)
            or_not_switched.push_back(i->second[j]);
          continue;
        }

        // generate switch satements
        std::stringstream switch_code;
        if (insideAnd)
          GenerateSwitches(switch_code, i->second, BinaryAnd, Split);
        else
          GenerateSwitches(switch_code, i->second, BinaryOr, NotSplit);
        // check if the whole expression is false
        if (switch_code.str() == "  return true;\n") {
          os << switch_code.str();
          return false;
        }
        or_code << switch_code.str();
      }

      // find all OR's ANDs
      std::vector<std::vector<SmartsAtomExpr*> > and_same(or_not_switched.size()), and_other(or_not_switched.size()); // [and_index][expr]
      std::vector<SmartsAtomExpr*> or_not_switched_not_and;
      for (std::size_t i = 0; i < or_not_switched.size(); ++i) {
        if (IsAnd(or_not_switched[i]))
          FindSameBinaryExpr(or_not_switched[i], and_same[i], and_other[i]);
        else
          or_not_switched_not_and.push_back(or_not_switched[i]);
      }

      // find all AND's switchable leafs
      std::vector<std::vector<SmartsAtomExpr*> > and_switchable(and_other.size()), and_not_switched(and_other.size()); // [and_index][expr]
      for (std::size_t i = 0; i < and_other.size(); ++i)
        FindSwitchableLeafs(and_other[i], and_switchable[i], and_not_switched[i]);
        
      // sort and_switchable by type
      std::map<int, std::vector<std::pair<int, SmartsAtomExpr*> > > and_switchable_types; // type -> [(and_index, expr)]
      SortSplitSwitchableByType(and_switchable, and_not_switched, and_switchable_types);

      // sort switchable types by size
      std::vector<std::pair<int, int> > and_switchable_sizes; // [(type, and_switchable_types size)]
      for (std::map<int, std::vector<std::pair<int, SmartsAtomExpr*> > >::iterator i = and_switchable_types.begin(); i != and_switchable_types.end(); ++i)
        and_switchable_sizes.push_back(std::make_pair(i->first, i->second.size()));
      std::sort(and_switchable_sizes.begin(), and_switchable_sizes.end(), SortPairs<int, int, PairSecond, std::greater>());
    
      // create mapping from expr pointer to and index
      std::map<SmartsAtomExpr*, int> expr_to_and_index; // expr -> and index
      for (std::map<int, std::vector<std::pair<int, SmartsAtomExpr*> > >::iterator i = and_switchable_types.begin(); i != and_switchable_types.end(); ++i)
        for (std::size_t j = 0; j < i->second.size(); ++j)
          expr_to_and_index[i->second[j].second] = i->second[j].first;

      std::string switch_code;
      std::map<int, std::vector<std::string> > index_to_case; // and index -> $case_i or $default

      //
      // [A,CRH2,!NRH3,!OaH4]
      //  ~ ~~~~ ~~~~~ ~~~~~
      //  0   1    2     3      <- and index
      //
      // switchable[0]: []
      // switchable[1]: [ C H2 ]
      // switchable[2]: [ !N H3 ]
      // switchable[3]: [ !O H4 ]
      //
      // not_switched[1]: [ R ]
      // not_switched[2]: [ R ]
      // not_switched[3]: [ a ]
      //
      // switchable_types[Smiley::AE_AliphaticElement]: [ C !N !O ]
      // switchable_types[Smiley::AE_TotalH]: [ H2 H3 H4 ]
      //
      // Generate switch 1:
      //
      //   index_to_case[1]: [ $and_1_code_ ]
      //   index_to_case[2]: [ $and_2_code_ ]
      //   index_to_case[3]: [ $and_3_code_ ]
      //
      //   // switch C,!N,!O
      //   switch (expr)
      //     case C:
      //       $and_1_case_6_C
      //       $and_2_case_6_!N
      //       $and_3_case_6_!O
      //     case N:
      //       $and_3_case_7_!O
      //     case O:
      //       $and_2_case_8_!N
      //     default:
      //       $and_2_default_!N
      //       $and_3_default_!O
      // 
      //   index_to_case[1]: [ $and_1_case_6_C ]
      //   index_to_case[2]: [ $and_2_case_6_!N , $and_2_case_8_!N , $and_2_default_!N ]
      //   index_to_case[3]: [ $and_3_case_6_!O , $and_3_case_7_!O , $and_3_default_!O ]
      //
      // Generate switches 2 & 3:
      //
      //   switches[0]: [ H2 H3 H4 ] // for $and_i_case_6_X
      //                                           ~~~~~~
      //                                             |                  
      //                                             +--> select on case_i
      //
      //   switches[1]: [ H3 H4 ] // for $and_i_default_X
      //                                        ~~~~~~~
      //
      //  // switch C,!N,!O
      //  switch (expr)
      //    case C:
      //      // switch H2,H3,H4
      //      switch (expr)
      //        case H2:
      //          $and_1_case_6_case_2_CH2
      //        case H3:
      //          $and_2_case_6_case_3_!NH3
      //        case H4:
      //          $and_3_case_6_case_4_!OH4
      //        default:
      //          break
      //    case N:
      //      $and_3_case_7_!O
      //    case O:
      //      $and_2_case_8_!N
      //    default:
      //      switch (expr)
      //        case H3:
      //          $and_2_default_case_2_!NH2
      //        case H4:
      //          $and_3_default_case_3_!OH3

      //
      // initialize switch code and index_to_case
      //
      // switch_code:
      //
      //   $and_1_code_
      //   $and_2_code_
      //   $and_3_code_
      //
      // index_to_case[1] = [ $and_1_code_ ]
      // index_to_case[2] = [ $and_2_code_ ]
      // index_to_case[3] = [ $and_3_code_ ]
      //
      for (std::size_t i = 0; i < and_other.size(); ++i) {
        if (and_other[i].empty())
          continue;
        index_to_case[i].push_back(make_string("$and_", i, "_code_"));
        switch_code += make_string("  $and_", i, "_code_\n");
      }

      for (std::size_t i = 0; i < and_switchable_sizes.size(); ++i) {
        int type = and_switchable_sizes[i].first;
        if (and_switchable_types[type].size() < 2)
          break;
        
        //
        // initialize case_to_expr from index_to_case
        //
        // pass i = 0:
        //
        //   $and_i_code_
        //          ~~~~
        //
        //   case_to_expr[code]: [ C !N !O ]
        //
        // pass i = 1:
        //
        //   $and_1_case_6_C
        //   $and_2_case_6_!N
        //   $and_3_case_6_!O
        //   $and_2_case_8_!N
        //   $and_3_case_7_!O
        //          ~~~~~~
        //   $and_2_default_!N
        //   $and_3_default_!O
        //          ~~~~~~~
        //
        //   case_to_expr[case_6]: [ H2 H3 H4 ]
        //   case_to_expr[case_7]: []
        //   case_to_expr[case_8]: []
        //   case_to_expr[default]: [ H3 H4 ]
        //
        std::map<std::string, std::vector<SmartsAtomExpr*> > case_to_expr; // X -> [expr]
        for (std::size_t j = 0; j < and_switchable_types[type].size(); ++j) {
          int and_index = and_switchable_types[type][j].first;
          SmartsAtomExpr *expr = and_switchable_types[type][j].second;
          for (std::size_t k = 0; k < index_to_case[and_index].size(); ++k) {
            std::string case_str = ExtractCase(index_to_case[and_index][k]); // $and_i_X_Y -> X
            case_to_expr[case_str].push_back(expr);
          }
        }
        // remove case_to_expr lists of size 1
        for (std::size_t j = 0; j < and_switchable_types[type].size(); ++j) {
          int and_index = and_switchable_types[type][j].first;
          SmartsAtomExpr *expr = and_switchable_types[type][j].second;
          for (std::size_t k = 0; k < index_to_case[and_index].size(); ++k) {
            std::string case_str = ExtractCase(index_to_case[and_index][k]); // $and_i_X_Y -> X
            if (case_to_expr[case_str].size() < 2) {
              and_not_switched[and_index].push_back(expr);
              case_to_expr.erase(case_to_expr.find(case_str));
            }
          }
        }

        for (std::map<std::string, std::vector<SmartsAtomExpr*> >::iterator j = case_to_expr.begin(); j != case_to_expr.end(); ++j) {

          std::string code;
          std::stringstream ss;
          if (GenerateHighAndSwitchCode(ss, j->second, InsideOr, Split))
            code = ss.str();

          //
          // remove redundant $and_i_X_Y in switch_code
          //
          // pass i = 1 , j = 0:
          //
          //   before:
          //
          //     switch_code:  switch (expr)
          //                     case C:
          //                       $and_1_case_6_C
          //                       $and_2_case_6_!N    <- remove
          //                       $and_3_case_6_!O    <- remove
          //                     case N:
          //                       $and_3_case_7_!O
          //                     case O:
          //                       $and_2_case_8_!N
          //                     default:
          //                       $and_2_default_!N
          //                       $and_3_default_!O
          //
          //   after:
          //
          //     switch_code:  switch (expr)
          //                     case C:
          //                       $and_1_case_6_C
          //                     case N:
          //                       $and_3_case_7_!O
          //                     case O:
          //                       $and_2_case_8_!N
          //                     default:
          //                       $and_2_default_!N
          //                       $and_3_default_!O
          //
          // pass i = 1 , j = 1:
          //
          //   before:
          //
          //     switch_code:  switch (expr)
          //                     case C:
          //                       ...
          //                     case N:
          //                       $and_3_case_7_!O
          //                     case O:
          //                       $and_2_case_8_!N
          //                     default:
          //                       $and_2_default_!N
          //                       $and_3_default_!O   <- remove
          //
          //   after:
          //
          //     switch_code:  switch (expr)
          //                     case C:
          //                       ...
          //                     case N:
          //                       $and_3_case_7_!O
          //                     case O:
          //                       $and_2_case_8_!N
          //                     default:
          //                       $and_2_default_!N
          //
          std::string switch_case;
          for (std::size_t k = 0; k < j->second.size(); ++k) {
            int and_index = expr_to_and_index[j->second[k]];
            for (std::size_t l = 0; l < index_to_case[and_index].size(); ++l) {
              std::string case_str = ExtractCase(index_to_case[and_index][l]); // $and_i_X_Y -> X
              if (case_str == j->first) {
                if (switch_case.empty())
                  switch_case = index_to_case[and_index][l];
                else
                  replace_first(switch_code, index_to_case[and_index][l], ""); // $and_i_X_Y -> "" in switch_code
              }
            }
          }

          // 
          // create case_code[value] and default_code
          //
          // pass i = 0:
          //
          //   case_code[6]: $and_1_case_6_C
          //                 $and_2_case_6_!N
          //                 $and_3_case_6_!O
          //
          //   case_code[7]:
          //
          //   case_code[8]:
          //
          //   default_code: $and_2_default_!N
          //                 $and_3_default_!O
          //
          // pass i = 1 , j = 0:
          //
          //   case_code[2]: $and_1_case_6_case_2_CH2
          //   case_code[3]: $and_2_case_6_case_3_!NH3
          //   case_code[4]: $and_3_case_6_case_4_!OH4
          //   default_code: 
          //
          // pass i = 1 , j = 1:
          //
          //   case_code[3]: $and_2_case_6_case_3_!NH3
          //   case_code[4]: $and_3_case_6_case_4_!OH4
          //   default_code: 
          //
          std::string default_code;
          std::map<int, std::string> case_code;
          // add $and_i_case_j_Y to case_code[j] for not negated leafs
          // add $and_i_default_!Y to default_code for negated leafs
          for (std::size_t k = 0; k < j->second.size(); ++k) {
            SmartsAtomExpr *expr = j->second[k];
            int and_index = expr_to_and_index[expr];
            int value = IsNot(expr) ? expr->unary.arg->leaf.value : expr->leaf.value;
            if (IsNot(expr)) {
              case_code[value] += ""; // make sure case_code[value] exists
              for (std::size_t l = 0; l < index_to_case[and_index].size(); ++l) {
                std::string prev_case_str = ExtractCase(index_to_case[and_index][l]);
                if (prev_case_str == j->first) {
                  if (prev_case_str == "code") {
                    std::string case_str = make_string("$and_", and_index, "_default_", GetExprString(expr));
                    default_code += case_str + "\n";
                    index_to_case[and_index].push_back(case_str);
                  } else {
                    std::string prev_expr_str = index_to_case[and_index][l].substr(index_to_case[and_index][l].rfind("_") + 1);
                    std::string expr_str = GetExprString(expr);
                    if (prev_expr_str == expr_str)
                      continue;
                    std::string case_str = make_string("$and_", and_index, "_", prev_case_str, "_default_", prev_expr_str, expr_str);
                    default_code += case_str + "\n";
                    index_to_case[and_index].push_back(case_str);
                  }
                }
              }
            } else {
              for (std::size_t l = 0; l < index_to_case[and_index].size(); ++l) {
                std::string prev_case_str = ExtractCase(index_to_case[and_index][l]);
                if (prev_case_str == j->first) {
                  if (prev_case_str == "code") {
                    std::string case_str = make_string("$and_", and_index, "_case_", value, "_", GetExprString(expr));
                    case_code[value] += case_str + "\n";
                    index_to_case[and_index].push_back(case_str);
                  } else {
                    std::string prev_expr_str = index_to_case[and_index][l].substr(index_to_case[and_index][l].rfind("_") + 1);
                    std::string expr_str = GetExprString(expr);
                    if (prev_expr_str == expr_str)
                      continue;
                    std::string case_str = make_string("$and_", and_index, "_", prev_case_str, "_case_", value, "_", prev_expr_str, expr_str);
                    case_code[value] += case_str + "\n";
                    index_to_case[and_index].push_back(case_str);
                  }
                }
              }
            }
          }
          // and $and_i_case_j_!Y to all case_code[!j] for negated leafs
          for (std::size_t k = 0; k < j->second.size(); ++k) {
            SmartsAtomExpr *expr = j->second[k];
            int and_index = expr_to_and_index[expr];
            if (!IsNot(expr))
              continue;
            int value = expr->unary.arg->leaf.value;
            for (std::map<int, std::string>::iterator l = case_code.begin(); l != case_code.end(); ++l) {
              if (value == l->first)
                continue;



              /*
              std::string case_str = make_string("$and_", and_index, "_case_", l->first, "_", GetExprString(expr));
              l->second += case_str + "\n";
              index_to_case[and_index].push_back(case_str);
              */

              for (std::size_t m = 0; m < index_to_case[and_index].size(); ++m) {
                std::string prev_case_str = ExtractCase(index_to_case[and_index][m]);
                if (prev_case_str == j->first) {
                  if (prev_case_str == "code") {
                    std::string case_str = make_string("$and_", and_index, "_case_", l->first, "_", GetExprString(expr));
                    l->second += case_str + "\n";
                    index_to_case[and_index].push_back(case_str);
                  } else {
                    std::string prev_expr_str = index_to_case[and_index][m].substr(index_to_case[and_index][m].rfind("_") + 1);
                    std::string expr_str = GetExprString(expr);
                    if (prev_expr_str == expr_str)
                      continue;
                    std::string case_str = make_string("$and_", and_index, "_", prev_case_str, "_case_", l->first, "_", prev_expr_str, expr_str);
                    l->second += case_str + "\n";
                    index_to_case[and_index].push_back(case_str);
                  }
                }
              }
 









            }
          }
 
          // 
          // replace $case_i and $default in code with $and_i_X_Y
          //
          // pass i = 0:
          //
          //   before:
          //
          //     code:  switch (expr)
          //              case C:
          //                $case_6
          //              case N:
          //                $case_7
          //              case O:
          //                $case_8
          //              default:
          //                $default
          //
          //   after:
          //
          //     code:  switch (expr)
          //              case C:
          //                $and_1_case_6_C
          //                $and_2_case_6_!N
          //                $and_3_case_6_!O
          //              case N:
          //                $and_3_case_7_!O
          //              case O:
          //                $and_2_case_8_!N
          //              default:
          //                $and_2_default_!N
          //                $and_3_default_!O
          //
          // pass i = 1 , j = 0:
          //
          //   before:
          //
          //     code:  switch (expr)
          //              case C:
          //                $and_1_case_6_C
          //              case N:
          //                $and_3_case_7_!O
          //              case O:
          //                $and_2_case_8_!N
          //              default:
          //                $and_2_default_!N
          //                $and_3_default_!O
          //
          //   after:
          //
          //     code:  switch (expr)
          //              case C:
          //                switch (expr)
          //                  case H2:
          //                    $and_1_case_6_case_2_CH2
          //                  case H3:
          //                    $and_2_case_6_case_3_!NH3
          //                  case H4:
          //                    $and_3_case_6_case_4_!OH4
          //                  default:
          //                    break
          //              case N:
          //                $and_3_case_7_!O
          //              case O:
          //                $and_2_case_8_!N
          //              default:
          //                $and_2_default_!N
          //                $and_3_default_!O
          //                   
          // pass i = 1 , j = 1:
          //
          //   after:
          //
          //     code:  switch (expr)
          //              case C:
          //                switch (expr)
          //                  case H2:
          //                    $and_1_case_6_case_2_CH2
          //                  case H3:
          //                    $and_2_case_6_case_3_!NH3
          //                  case H4:
          //                    $and_3_case_6_case_4_!OH4
          //                  default:
          //                    break
          //              case N:
          //                $and_3_case_7_!O
          //              case O:
          //                $and_2_case_8_!N
          //              default:
          //                switch (expr)
          //                  case H3:
          //                    $and_2_default_case_3_!NH3
          //                  case H4:
          //                    $and_3_default_case_3_!OH4
          //                  default:
          //                    break
          //                   
          for (std::size_t k = 0; k < j->second.size(); ++k) {
            SmartsAtomExpr *expr = j->second[k];
            int value = IsNot(expr) ? expr->unary.arg->leaf.value : expr->leaf.value;
            if (IsNot(expr)) {
              if (case_code[value].empty())
                replace_first(code, make_string("$case_", value), "break;");
              else
                //replace_first(code, make_string("$case_", value), case_code[value] + "\n      break;");
                ReplaceAndIndent(code, make_string("$case_", value), case_code[value] + "\n      break;");
            } else {
              //replace_first(code, make_string("$case_", value), case_code[value] + "\n      break;");
              ReplaceAndIndent(code, make_string("$case_", value), case_code[value] + "\n      break;");
            }
          }
          // replace $default with default code
          if (default_code.size())
            //replace_first(code, "$default\n", default_code);
            ReplaceAndIndent(code, "$default", default_code);
          else
            replace_first(code, "$default", "break;");

          ReplaceAndIndent(switch_code, switch_case, code);
          //replace_all(switch_code, "\n\n", "\n");
        }
      }

      RemoveEmptyLines(switch_code);

      std::stringstream and_code;
      and_code << "////////////////////////////////" << std::endl;
      and_code << switch_code;
      and_code << "////////////////////////////////" << std::endl;

      // handle not swicthed and leafs
      for (std::size_t i = 0; i < and_not_switched.size(); ++i)
        GenerateNotSwitched(and_code, and_not_switched[i], BinaryAnd, insideAnd);
      // handle not switched or leafs
      GenerateNotSwitched(or_code, or_not_switched_not_and, BinaryOr, insideAnd);
      // output code
      os << or_code.str();
      os << and_code.str();
      if (!insideAnd)
        os << "  return false;" << std::endl;
      return true;
    }
    
    std::string GenerateOrSwitchFunction(std::ostream &os, SmartsAtomExpr *expr)
    {
      return ""; // DISABLED for now until I finsish this...

      if (m_noswitch)
        return "";
      // find all ORs
      std::vector<SmartsAtomExpr*> or_same, or_other; // [expr]
      FindSameBinaryExpr(expr, or_same, or_other);
      std::stringstream code;
      GenerateOrSwitchCode(code, or_other);

      // open function
      std::string arg_type = m_toolkit->AtomArgType(m_language);
      if (arg_type.size())
        arg_type = make_string(arg_type, " ");
      os << CommentString() + GetExprString(expr) << std::endl;
      os << "inline bool EvalOrExpr_" << ++m_or << "(" << arg_type << "atom)" << std::endl;
      os << "{" << std::endl;
      // code
      os << code.str();
      os << "}" << std::endl;
      os << std::endl;
      std::string functionName = make_string("EvalOrExpr_", m_or);
      m_functions.push_back(functionName);
      return functionName;
    }
    
    bool GenerateLowAndSwitchCode(std::ostream &os, std::vector<SmartsAtomExpr*> &and_same, std::vector<SmartsAtomExpr*> &and_other)
    {
      // find all ANDs switchable leafs
      std::vector<SmartsAtomExpr*> and_switchable, and_not_switched; // [expr]
      FindSwitchableLeafs(and_other, and_switchable, and_not_switched);

      // sort and_switchable by type
      std::map<int, std::vector<SmartsAtomExpr*> > and_switchable_types; // type -> [expr]
      SortSwitchableByType(and_switchable, and_switchable_types);

      // generate all switch statements
      std::stringstream and_code;
      for (std::map<int, std::vector<SmartsAtomExpr*> >::iterator i = and_switchable_types.begin(); i != and_switchable_types.end(); ++i) {
        if (i->second.size() < 2) {
          for (std::size_t j = 0; j < i->second.size(); ++j)
            and_not_switched.push_back(i->second[j]);
          continue;
        }

        // generate switch satements
        std::stringstream switch_code;
        GenerateSwitches(switch_code, i->second, BinaryAnd, NotSplit);
        // check if the whole expression is false
        if (switch_code.str() == "  return false;\n") {
          os << switch_code.str();
          return false;
        }
        and_code << switch_code.str();
      }

      Debug("and_switchable", and_code, and_switchable);
      Debug("and_not_switched", and_code, and_not_switched);

      // find all AND's ORs
      std::vector<std::vector<SmartsAtomExpr*> > or_same(and_not_switched.size()), or_other(and_not_switched.size()); // [or_index][expr]
      std::vector<SmartsAtomExpr*> and_not_switched_not_or;
      for (std::size_t i = 0; i < and_not_switched.size(); ++i) {
        if (IsOr(and_not_switched[i])) {
          FindSameBinaryExpr(and_not_switched[i], or_same[i], or_other[i]);
        } else
          and_not_switched_not_or.push_back(and_not_switched[i]);
      }

      int num_ors = 0;
      std::map<SmartsAtomExpr*, int> expr_to_or_i;
      for (std::size_t i = 0; i < or_other.size(); ++i) {
        if (!or_other[i].size())
          continue;
        ++num_ors;
        for (std::size_t j = 0; j < or_other[i].size(); ++j)
          expr_to_or_i[or_other[i][j]] = num_ors;
      }

      std::stringstream or_code;
      for (int i = 0; i < num_ors; ++i)
        or_code << "  bool or_" << i + 1 << " = false;" << std::endl;


      // find all OR's switchable leafs
      std::vector<std::vector<SmartsAtomExpr*> > or_switchable(or_other.size()), or_not_switched(or_other.size()); // [or_index][expr]
      for (std::size_t i = 0; i < or_other.size(); ++i)
        FindSwitchableLeafs(or_other[i], or_switchable[i], or_not_switched[i]);

      // sort or_switchable by type
      std::map<int, std::vector<std::pair<int, SmartsAtomExpr*> > > or_switchable_types; // type -> [(or_index, expr)]
      SortSplitSwitchableByType(or_switchable, or_not_switched, or_switchable_types);

      std::vector<std::pair<int, int> > or_switchable_sizes; // [(type, or_switchable_types_size)]
      for (std::map<int, std::vector<std::pair<int, SmartsAtomExpr*> > >::iterator i = or_switchable_types.begin(); i != or_switchable_types.end(); ++i)
        or_switchable_sizes.push_back(std::make_pair(i->first, i->second.size()));
      std::sort(or_switchable_sizes.begin(), or_switchable_sizes.end(), SortPairs<int, int, PairSecond, std::greater>());
      
      Debug("or_switchable", or_code, or_switchable);
      Debug("or_not_switched", or_code, or_not_switched);
 
      //std::vector<int> or_is;
      for (std::size_t i = 0; i < or_switchable_sizes.size(); ++i) {
        int type = or_switchable_sizes[i].first;
        if (or_switchable_types[type].size() < 2)
          break;
      
        Debug("or_switchable_types", or_code, or_switchable_types[type]);
        
        std::vector<SmartsAtomExpr*> or_switched;
        for (std::size_t j = 0; j < or_switchable_types[type].size(); ++j) {
          /*
          if (std::find(or_is.begin(), or_is.end(), or_switchable_types[type][j].first) != or_is.end()) {
            or_not_switched[or_switchable_types[type][j].first].push_back(or_switchable_types[type][j].second);
            continue;
          }
          */
          or_switched.push_back(or_switchable_types[type][j].second);
          //or_is.push_back(or_switchable_types[type][j].first);
        }

        Debug("or_switched", or_code, or_switched);

        std::string code;
        std::stringstream ss;
        if (GenerateOrSwitchCode(ss, or_switched, true))
          code = ss.str();
          
        //or_code << "// Or code:\n" << ss.str() << "// End or code\n";

        std::string default_code;
        std::map<int, std::string> case_code;
        // generate the case & default code
        for (std::size_t j = 0; j < or_switchable_types[type].size(); ++j) {
          ss.str("");

          //GenerateHighAndSwitchCode(ss, or_not_switched[or_switchable_types[type][j].first], true);
          GenerateOrSwitchCode(ss, or_not_switched[or_switchable_types[type][j].first], true);



          //or_code << "// Or inner code:\n" << ss.str() << "// End or inner code\n";

          std::string inner_code = ss.str();
          int value = IsNot(or_switchable_types[type][j].second) ? or_switchable_types[type][j].second->unary.arg->leaf.value : or_switchable_types[type][j].second->leaf.value;

          if (inner_code.empty()) {
            inner_code = make_string("  or_", expr_to_or_i[or_switchable_types[type][j].second], " = true;");
          } else {
          //if (or_not_switched[or_switchable_types[type][j].first].size() > 1) {
            inner_code.resize(inner_code.size()  - 1);
            replace_all(inner_code, "return", make_string("or_", expr_to_or_i[or_switchable_types[type][j].second], " ="));
            //inner_code += make_string("  if (!or_", j + 1, ")\n");
            //inner_code += "    return false;\n";
            /*
            if (j + 1 == or_switchable_types[type].size())
              if (IsNot(or_switchable_types[type][j].second))
                inner_code += "\n  break;";
              else
                inner_code += "  break;";
                */
          //} else {
          //  inner_code += "  return true;";
          //}
          }
          replace_all(inner_code, "\n", "\n    ");

          if (IsNot(or_switchable_types[type][j].second)) {
            if (default_code.size())
              default_code += "\n";
            default_code += "    " + inner_code;
          } else {
            if (case_code[value].size())
              case_code[value] += "\n";
            case_code[value] += "    " + inner_code;
          }
          or_not_switched[or_switchable_types[type][j].first].clear();
        }
        // replace $case_i with case code
        for (std::size_t j = 0; j < or_switchable_types[type].size(); ++j) {
          int value = IsNot(or_switchable_types[type][j].second) ? or_switchable_types[type][j].second->unary.arg->leaf.value : or_switchable_types[type][j].second->leaf.value;
          if (IsNot(or_switchable_types[type][j].second)) {
            if (case_code[or_switchable_types[type][j].second->leaf.value].empty())
              replace_first(code, make_string("$case_", value), "      break;");
            else
              replace_first(code, make_string("$case_", value), case_code[value] + "\n      break;");
          } else {
            if (case_code[or_switchable_types[type][j].second->leaf.value].empty())
              replace_first(code, make_string("$case_", value), "      break;");
            else
              replace_first(code, make_string("$case_", value), case_code[value] + "\n      break;");
          }
        }
        // replace $default with default code
        if (default_code.size()) {
          //replace_all(default_code, "\n  ", "\n");
          replace_first(code, "$default", default_code);
        } else
          replace_first(code, "$default", "      break;");

        or_code << code;
      }
      // handle not swicthed or leafs
      for (std::size_t i = 0; i < or_not_switched.size(); ++i) {
        if (or_not_switched[i].empty())
          continue;
        std::stringstream ss;
        if (GenerateOrSwitchCode(ss, or_not_switched[i], true)) {
          std::string code = ss.str();
          replace_all(code, "return", make_string("or_", expr_to_or_i[or_not_switched[i][0]], " ="));
          or_code << code;
          or_code << "  if (!or_" << expr_to_or_i[or_not_switched[i][0]] << ")" << std::endl;
          or_code << "    return false;" << std::endl;
        }
      }
      // handle not switched or leafs
      if (and_not_switched_not_or.size())
        and_code << "  // AND not switched" << std::endl;
      for (std::size_t j = 0; j < and_not_switched_not_or.size(); ++j) {
        and_code << "  // " << GetExprString(and_not_switched_not_or[j]) << std::endl;
        if (IsNot(and_not_switched_not_or[j])) {
          std::string expr_code = ExprString(and_not_switched_not_or[j]->unary.arg); //assert(expr_code.size());
          and_code << "  if (" << expr_code << ")" << std::endl;
        } else {
          std::string expr_code = ExprString(and_not_switched_not_or[j]); //assert(expr_code.size());
          and_code << "  if (!(" << expr_code << "))" << std::endl;
        }
        and_code << "    return false;" << std::endl;
      }
      /*
      // handle not switched and leafs
      if (and_not_switched.size())
        and_code << "  // AND not switched" << std::endl;
      for (std::size_t j = 0; j < and_not_switched.size(); ++j) {
        and_code << "  // " << GetExprString(and_not_switched[j]) << std::endl;
        std::string expr_code = ExprString(and_not_switched[j]); //assert(expr_code.size());
        and_code << "  if (!(" << expr_code << "))" << std::endl;
        and_code << "    return false;" << std::endl;
      }
      */
      // output code
      os << and_code.str();
      os << or_code.str();
      if (num_ors) {
        os << "  return ";
        for (int i = 0; i < num_ors; ++i) {
          if (i)
            os << " && ";
          os << "or_" << i + 1;
        }
        os << ";" << std::endl;
      } else
        os << "  return true;" << std::endl;
      return true;  
    }
    
    std::string GenerateLowAndSwitchFunction(std::ostream &os, SmartsAtomExpr *expr)
    {
      return ""; // DISABLED for now until I finsish this...

      if (m_noswitch)
        return "";
      // find all ANDs
      std::vector<SmartsAtomExpr*> and_same, and_other; // [expr]
      FindSameBinaryExpr(expr, and_same, and_other);
      std::stringstream code;
      GenerateLowAndSwitchCode(code, and_same, and_other);

      // open function
      std::string arg_type = m_toolkit->AtomArgType(m_language);
      if (arg_type.size())
        arg_type = make_string(arg_type, " ");
      os << CommentString() + GetExprString(expr) << std::endl;
      os << "inline bool EvalAndExpr_" << ++m_and << "(" << arg_type << "atom)" << std::endl;
      os << "{" << std::endl;
      // code
      os << code.str();
      os << "}" << std::endl;
      os << std::endl;
      std::string functionName = make_string("EvalAndExpr_", m_and);
      m_functions.push_back(functionName);
      return functionName;
    }



    std::string GenerateExprFunction(std::ostream &os, SmartsAtomExpr *expr)
    {
      switch (expr->type) {
        case Smiley::OP_AndHi:
          {
            std::string functionName = GenerateHighAndSwitchFunction(os, expr);
            if (functionName.size())
              return functionName;
            return BinaryExprFunction(os, BinaryAnd, expr);
          }
        case Smiley::OP_AndLo:
          {
            std::string functionName = GenerateLowAndSwitchFunction(os, expr);
            if (functionName.size())
              return functionName;
            return BinaryExprFunction(os, BinaryAnd, expr);
          }
        case Smiley::OP_Or:
          {
            std::string functionName = GenerateOrSwitchFunction(os, expr);
            if (functionName.size())
              return functionName;
            return BinaryExprFunction(os, BinaryOr, expr);
          }
        //case AE_RECUR:
        //  break;
        case Smiley::OP_Not:
          return UnaryExprFunction(os, UnaryNot, expr);
        case Smiley::AE_True:
          return ExprFunction(os, "EvalTrueExpr", "true", expr);
        case Smiley::AE_False:
          return ExprFunction(os, "EvalFalseExpr", "false", expr);
        case Smiley::AE_Aromatic:
          return ExprFunction(os, "EvalAromaticExpr", m_toolkit->AromaticAtomTemplate(m_language), expr);
        case Smiley::AE_Aliphatic:
          return ExprFunction(os, "EvalAliphaticExpr", m_toolkit->AliphaticAtomTemplate(m_language), expr);
        case Smiley::AE_Cyclic:
          return ExprFunction(os, "EvalCyclicExpr", m_toolkit->CyclicAtomTemplate(m_language), expr);
        case Smiley::AE_Acyclic:
          return ExprFunction(os, "EvalAcyclicExpr", m_toolkit->AcyclicAtomTemplate(m_language), expr);
        case Smiley::AE_Isotope:
          return ExprFunction(os, "EvalMassExpr", m_toolkit->MassAtomTemplate(m_language), expr);
        case Smiley::AE_AtomicNumber:
          return ExprFunction(os, "EvalElementExpr", m_toolkit->ElementAtomTemplate(m_language), expr);
        case Smiley::AE_AromaticElement:
          return ExprFunction(os, "EvalAromaticElementExpr", m_toolkit->AromaticElementAtomTemplate(m_language), expr);
        case Smiley::AE_AliphaticElement:
          return ExprFunction(os, "EvalAliphaticElementExpr", m_toolkit->AliphaticElementAtomTemplate(m_language), expr);
        case Smiley::AE_TotalH:
          return ExprFunction(os, "EvalHydrogenCountExpr", m_toolkit->HydrogenCountAtomTemplate(m_language), expr);
        case Smiley::AE_Charge:
          return ExprFunction(os, "EvalChargeExpr", m_toolkit->ChargeAtomTemplate(m_language), expr);
        case Smiley::AE_Connectivity:
          return ExprFunction(os, "EvalConnectExpr", m_toolkit->ConnectAtomTemplate(m_language), expr);
        case Smiley::AE_Degree:
          return ExprFunction(os, "EvalDegreeExpr", m_toolkit->DegreeAtomTemplate(m_language), expr);
        case Smiley::AE_ImplicitH:
          return ExprFunction(os, "EvalImplicitExpr", m_toolkit->ImplicitAtomTemplate(m_language), expr);
        case Smiley::AE_RingMembership:
          return ExprFunction(os, "EvalRingsExpr", m_toolkit->NumRingsAtomTemplate(m_language), expr);
        case Smiley::AE_RingSize:
          return ExprFunction(os, "EvalSizeExpr", m_toolkit->RingSizeAtomTemplate(m_language), expr);
        case Smiley::AE_Valence:
          return ExprFunction(os, "EvalValenceExpr", m_toolkit->ValenceAtomTemplate(m_language), expr);
        case Smiley::AE_Chirality:
          return ExprFunction(os, "EvalChiralExpr", "true", expr);
        //case AE_HYB:
        //  return ExprFunction(os, "EvalHybridizationExpr", m_toolkit->HybAtomTemplate(m_language), expr);
        case Smiley::AE_RingConnectivity:
          return ExprFunction(os, "EvalRingConnectExpr", m_toolkit->RingConnectAtomTemplate(m_language), expr);
        default:
          return ExprFunction(os, "EvalTrueExpr", "true", expr);
      }

      return "";
    }

    std::string GenerateExprFunction(std::ostream &os, SmartsBondExpr *expr)
    {
      switch (expr->type) {
        case Smiley::OP_AndHi:
        case Smiley::OP_AndLo:
          return BinaryExprFunction(os, BinaryAnd, expr);
        case Smiley::OP_Or:
          return BinaryExprFunction(os, BinaryOr, expr);
        case Smiley::OP_Not:
          return UnaryExprFunction(os, UnaryNot, expr);
        case Smiley::BE_True:
          return ExprFunction(os, "EvalAnyExpr", "true", expr);
        case BE_DEFAULT:
          return ExprFunction(os, "EvalDefaultExpr", m_toolkit->DefaultBondTemplate(m_language), expr);
        case Smiley::BE_Single:
          return ExprFunction(os, "EvalSingleExpr", m_toolkit->SingleBondTemplate(m_language), expr);
        case Smiley::BE_Double:
          return ExprFunction(os, "EvalDoubleExpr", m_toolkit->DoubleBondTemplate(m_language), expr);
        case Smiley::BE_Triple:
          return ExprFunction(os, "EvalTripleExpr", m_toolkit->TripleBondTemplate(m_language), expr);
        case Smiley::BE_Aromatic:
          return ExprFunction(os, "EvalAromaticExpr", m_toolkit->AromaticBondTemplate(m_language), expr);
        case Smiley::BE_Ring:
          return ExprFunction(os, "EvalRingExpr", m_toolkit->RingBondTemplate(m_language), expr);
        case Smiley::BE_Up:
        case Smiley::BE_Down:
        //case Smiley::BE_UpUNSPEC:
        //case BE_DOWNUNSPEC:
        default:
          return ExprFunction(os, "EvalAnyExpr", "true", expr);
      }
    }
  
    void GenerateEvalExprFunction(std::ostream &os, Smarts *pattern)
    {
      if (pattern->atoms.size() < 2)
        return;
      switch (m_language) {
        case SmartsCodeGenerator::Cpp:
          os << "bool EvalAtomExpr_" << m_smarts.size() << "(int index, " + m_toolkit->AtomArgType(m_language) + " atom)" << std::endl;
          os << "{" << std::endl;
          os << "  switch (index) {" << std::endl;
          for (int i = 0; i < pattern->atoms.size(); ++i) {
            os << "    case " << i << ":" << std::endl;
            os << "      return " << m_atomEvalExpr[i] << "(atom);" << std::endl;
          }
          os << "  }" << std::endl;
          os << "}" << std::endl;
          os << std::endl;

          if (pattern->bonds.size()) {
            os << "bool EvalBondExpr_" << m_smarts.size() << "(int index, " + m_toolkit->BondArgType(m_language) + " bond)" << std::endl;
            os << "{" << std::endl;
            os << "  switch (index) {" << std::endl;
            for (int i = 0; i < pattern->bonds.size(); ++i) {
              os << "    case " << i << ":" << std::endl;
              os << "      return " << m_bondEvalExpr[i] << "(bond);" << std::endl;
            }
            os << "  }" << std::endl;
            os << "}" << std::endl;
            os << std::endl;
          }
          break;
        case SmartsCodeGenerator::Python:
          os << "def EvalAtomExpr_" << m_smarts.size() << "(index, atom):" << std::endl;
          os << "  return { " << 0 << ": " << m_atomEvalExpr[0];
          if (pattern->atoms.size() > 1)
            os << "," << std::endl;
          for (int i = 1; i < pattern->atoms.size(); ++i) {
            os << "           " << i << ": " << m_atomEvalExpr[i];
            if (i + 1 < pattern->atoms.size())
              os << "," << std::endl;
          }
          os << " }[index](atom)" << std::endl;
          os << std::endl;

          if (pattern->bonds.size()) {
            os << "def EvalBondExpr_" << m_smarts.size() << "(index, bond):" << std::endl;
            os << "  return { " << 0 << ": " << m_bondEvalExpr[0];
            if (pattern->bonds.size() > 1)
              os << "," << std::endl;
            for (int i = 1; i < pattern->bonds.size(); ++i) {
              os << "           " << i << ": " << m_bondEvalExpr[i];
              if (i + 1 < pattern->bonds.size())
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
        case SmartsCodeGenerator::Cpp:
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
        case SmartsCodeGenerator::Python:
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
        case SmartsCodeGenerator::Cpp:
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
        case SmartsCodeGenerator::Python:
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
        case SmartsCodeGenerator::Cpp:
          os << "SmartsPattern<" << m_toolkit->AtomType(m_language) << ", "
             << m_toolkit->BondType(m_language) << ">* GetSmartsPattern(int index)" << std::endl;
          os << "{" << std::endl;
          if (m_smarts.size() != m_singleatoms.size()) {
            os << "  switch (index) {" << std::endl;
            for (int i = 0; i < m_patterns.size(); ++i) {
              if (m_singleatoms.find(i) != m_singleatoms.end())
                continue;
              os << "    case " << i << ":" << std::endl;
              os << "      {" << std::endl;
              os << "        SmartsPattern<" << m_toolkit->AtomType(m_language) << ", "
                 << m_toolkit->BondType(m_language) << "> *pattern = new SmartsPattern;" << std::endl;
              os << "        pattern->numAtoms = " << m_patterns[i].numAtoms << ";" << std::endl;
              os << "        pattern->ischiral = " << m_patterns[i].ischiral << ";" << std::endl;
              for (int j = 0; j < m_patterns[i].bonds.size(); ++j)
                os << "        pattern->bonds.push_back(SmartsBond(" 
                  << m_patterns[i].bonds[j].source << ", " << m_patterns[i].bonds[j].target 
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
        case SmartsCodeGenerator::Python:
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
              os << "SmartsBond("  << m_patterns[i].bonds[j].source << ", "
                 << m_patterns[i].bonds[j].target << ", " 
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
        case SmartsCodeGenerator::Cpp:
          os << "template<typename MappingType>" << std::endl;
          os << "bool Match(OpenBabel::OBMol &mol, const std::string &smarts, MappingType &mapping)" << std::endl;
          os << "{" << std::endl;
          os << "  int index = SmartsIndex(smarts);" << std::endl;
          os << "  if (index < 0)" << std::endl;
          os << "    return false;" << std::endl;
          if (m_singleatoms.size()) {
            os << "  if (IsSingleAtom(index)) {" << std::endl;
            os << "    switch (index) {" << std::endl;
            for (std::set<int>::iterator i = m_singleatoms.begin(); i != m_singleatoms.end(); ++i) {
              os << "      case " << *i << ":" << std::endl;
              os << "        return SingleAtomMatch_" << *i << "(mol, mapping);" << std::endl;
            }
            os << "    }" << std::endl;
            os << "  }" << std::endl;
          }
          os << "  SmartsMatcher<> matcher;" << std::endl;
          os << "  SmartsPattern<" << m_toolkit->AtomType(m_language) << ", " 
             << m_toolkit->BondType(m_language) << "> *pat = GetSmartsPattern(index);" << std::endl;
          os << "  bool result = matcher.Match(mol, pat, mapping);" << std::endl;
          os << "  delete pat;" << std::endl;
          os << "  return result;" << std::endl;
          os << "}" << std::endl;
          os << std::endl;
          break;
        case SmartsCodeGenerator::Python:
          os << "def Match(molecule, smarts, mapping):" << std::endl;
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
            os << " }[index](molecule, mapping)" << std::endl;
          }
          os << "  matcher = SmartsMatcher()" << std::endl;
          os << "  pat = GetSmartsPattern(index)" << std::endl;
          os << "  result = matcher.Match(molecule, pat, mapping)" << std::endl;
          os << "  return result" << std::endl;
          os << std::endl;
          os << "def MatchGlobalMol(smarts, mapping):" << std::endl;
          os << "  return Match(mol, smarts, mapping)" << std::endl;
          os << std::endl;
          break;
      }
    }
     
    void GenerateSingleAtomMatch(std::ostream &os, SmartsAtomExpr *expr)
    {
      os << CommentString() << "[" << GetExprString(expr) << "]" << std::endl;
      switch (m_language) {
        case SmartsCodeGenerator::Cpp:
          os << "template<typename MappingType>" << std::endl;
          os << "bool SingleAtomMatch_" << m_smarts.size() << "(OpenBabel::OBMol &mol, MappingType &mapping)" << std::endl;
          os << "{" << std::endl;
          os << "  FOR_ATOMS_OF_MOL (atom, mol) {" << std::endl;
          os << "    if (" << m_atomEvalExpr[0] << "(&*atom)) {" << std::endl;
          os << "      AddMapping(mapping, std::vector<int>(1, atom->GetIdx()));" << std::endl;
          os << "      if (DoSingleMapping<MappingType>::result)" << std::endl;
          os << "        return true;" << std::endl;
          os << "    }" << std::endl;
          os << "  }" << std::endl;
          os << "  return mapping.size();" << std::endl;
          os << "}" << std::endl;
          os << std::endl;
          break;
        case SmartsCodeGenerator::Python:
          os << "def SingleAtomMatch_" << m_smarts.size() << "(molecule, mapping):" << std::endl;
          os << "  for atom in OBMolAtomIter(molecule):" << std::endl;
          os << "    if " << m_atomEvalExpr[0] << "(atom.GetIdx()):" << std::endl;
          os << "      mapping.append([atom.GetIdx()])" << std::endl;
          //os << "      if single:" << std::endl;
          //os << "        return True" << std::endl;
          os << "  return True if len(mapping) else False" << std::endl;
          os << std::endl;
          break;
      }
    }

    void GenerateCustomFunction(std::ostream &os, const std::string &function, bool nomap, bool count, bool atom)
    {
      // comment: SMARTS [options]
      os << CommentString() << *m_smarts.rbegin();
      if (nomap)
        os << " nomap";
      if (count)
        os << " count";
      os << std::endl;
      if (atom) {
        os << "inline bool " << function << "(" << m_toolkit->AtomArgType(m_language);
        if (m_toolkit->AtomArgType(m_language).size())
          os << " ";
        os << "atom)" << std::endl;
        os << "{" << std::endl;
        os << "  return " << m_atomEvalExpr[0] << "(atom);" << std::endl;
        os << "}" << std::endl;
        os << std::endl;
        return;
      }
      switch (m_language) {
        case SmartsCodeGenerator::Cpp:
          // template<typename>                                      [for !nomap/!count]
          if (!nomap && !count)
            os << "template<typename MappingType>" << std::endl;
          // inline [bool|int] function(mol[, mapping])              [int for count, no mapping for nomap/count]
          os << "inline ";
          if (count)
            os << "int ";
          else
            os << "bool ";
          os << function << "(OpenBabel::OBMol *mol";
          if (!nomap && !count)
            os << ", MappingType &mapping";
          os << ")" << std::endl;
          os << "{" << std::endl;
          //   [No|Count]Mapping tmp;                                [optional line for nomap/count]
          if (nomap)
            os << "  NoMapping tmp;" << std::endl;
          else if (count)
            os << "  CountMapping tmp;" << std::endl;
          //   bool result = SingleAtomMatch_X(mol[, mapping|tmp]);  [for single atom pattern, tmp for nomap/count]
          //   bool result = Match(mol[, mapping|tmp]);              [general match, tmp for nomap/count]
          if (count)
            os << "  ";
          else
            os << "  return ";
          if (m_singleatoms.size() && *m_singleatoms.rbegin() == m_smarts.size() - 1) {
            os << "SingleAtomMatch_" << m_smarts.size() - 1 << "(mol";
          } else
            os << "Match(mol";
          if (count || nomap)
            os << ", tmp";
          else
            os << ", mapping";
          os << ");" << std::endl;
          //   return [result|tmp.count];                            [tmp.count for count]
          if (count)
            os << "  return tmp.count;" << std::endl;
          os << "}" << std::endl;
          os << std::endl;
          break;
        case SmartsCodeGenerator::Python:      
          break;
      }
    }
 
  };


  SmartsCodeGenerator::SmartsCodeGenerator(Toolkit *toolkit, enum Language language) : d(new SmartsCodeGeneratorPrivate(toolkit, language))
  {
  }

  SmartsCodeGenerator::~SmartsCodeGenerator()
  {
    delete d;
  }

  void SmartsCodeGenerator::StartSmartsModule(const std::string &name, bool noinline, bool noswitch, bool nomatch, bool optfunc)
  {
    d->m_noinline = noinline;
    d->m_noswitch = noswitch;
    d->m_nomatch = nomatch;
    d->m_optfunc = optfunc;

    if (d->m_language == Cpp) {
      d->m_os << "#include <openbabel/atom.h>" << std::endl;
      d->m_os << "#include <openbabel/bond.h>" << std::endl;
      d->m_os << "#include \"smartspattern.h\"" << std::endl;
      d->m_os << "#include \"smartsmatcher.h\"" << std::endl;
      d->m_os << std::endl;
      d->m_os << "using namespace OpenBabel;" << std::endl;
      d->m_os << "using namespace SC;" << std::endl;
      d->m_os << std::endl;
      d->m_os << "namespace " << name << " {" << std::endl;
      d->m_os << std::endl;
    } else {
      d->m_os << "from smartscompiler import *" << std::endl;
      d->m_os << "from openbabel import *" << std::endl;
      d->m_os << std::endl;
      d->m_os << "mol = OBMol()" << std::endl;
      d->m_os << std::endl;
    }
  }

  void SmartsCodeGenerator::GeneratePatternCode(const std::string &smarts, Smarts *pattern, const std::string &function,
      bool nomap, bool count, bool atom)
  {
    d->m_atomEvalExpr.clear();
    d->m_bondEvalExpr.clear();

    for (int i = 0; i < pattern->atoms.size(); ++i)
      d->m_atomEvalExpr[i] = d->GenerateExprFunction(d->m_os, pattern->atoms[i].expr);
    for (int i = 0; i < pattern->bonds.size(); ++i)
      d->m_bondEvalExpr[i] = d->GenerateExprFunction(d->m_os, pattern->bonds[i].expr);

    d->GenerateEvalExprFunction(d->m_os, pattern);

    if (pattern->atoms.size() == 1) {
      // special case for single atom pattern
      d->GenerateSingleAtomMatch(d->m_os, pattern->atoms[0].expr);
      d->m_singleatoms.insert(d->m_patterns.size());
      // dummy pattern
      d->m_patterns.push_back(SmartsPattern<OBAtom, OBBond>());
    } else {
      SmartsPattern<OBAtom, OBBond> cpattern;
      cpattern.numAtoms = pattern->atoms.size();
      cpattern.ischiral = pattern->chiral;
      for (int i = 0; i < pattern->bonds.size(); ++i)
        cpattern.bonds.push_back(SmartsBond(pattern->bonds[i].source, pattern->bonds[i].target, pattern->bonds[i].grow));
      d->m_patterns.push_back(cpattern);
    }
    
    d->m_smarts.push_back(smarts);

    if (function.size())
        d->GenerateCustomFunction(d->m_os, function, nomap, count, atom);
  }

  void SmartsCodeGenerator::StopSmartsModule(std::ostream &os)
  {
    if (!d->m_nomatch) {
      d->GenerateSmartsIndexFunction(d->m_os);
      d->GenerateIsSingleAtomFunction(d->m_os);
      d->GenerateSmartsPatternFunction(d->m_os);
      d->GenerateMatchFunction(d->m_os);
    }

    if (d->m_optfunc) {
      std::string code = d->m_os.str();
      std::sort(d->m_functions.begin(), d->m_functions.end(), SortContainers<std::string, ContainerSize, std::greater>());
      for (std::size_t i = 0; i < d->m_functions.size(); ++i) {
        replace_all(code, d->m_functions[i], make_string("f", i + 1));
      }
      os << code;
    } else
      os << d->m_os.str();

    if (d->m_language == Cpp)
      os << "} // end namespace" << std::endl;
  }

}
