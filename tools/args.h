#include "../src/smartsoptimizer.h"
#include "../src/util.h"

class ParseArgs
{
  struct OptionalArg
  {
    OptionalArg(const std::string &option_) : option(option_)
    {
    }

    OptionalArg(const std::string &option_, const std::vector<std::string> &args_)
        : option(option_), args(args_)
    {
    }

    std::string option;
    std::vector<std::string> args;
  };

  public:
    static std::vector<std::string> Args(
        const std::string &str1 = std::string(), const std::string &str2 = std::string(),
        const std::string &str3 = std::string(), const std::string &str4 = std::string(),
        const std::string &str5 = std::string(), const std::string &str6 = std::string(),
        const std::string &str7 = std::string(), const std::string &str8 = std::string(),
        const std::string &str9 = std::string(), const std::string &str10 = std::string(),
        const std::string &str11 = std::string(), const std::string &str12 = std::string(),
        const std::string &str13 = std::string(), const std::string &str14 = std::string())
    {
      std::vector<std::string> result;
      if (!str1.empty()) result.push_back(str1);
      if (!str2.empty()) result.push_back(str2);
      if (!str3.empty()) result.push_back(str3);
      if (!str4.empty()) result.push_back(str4);
      if (!str5.empty()) result.push_back(str5);
      if (!str6.empty()) result.push_back(str6);
      if (!str7.empty()) result.push_back(str7);
      if (!str8.empty()) result.push_back(str8);
      if (!str9.empty()) result.push_back(str9);
      if (!str10.empty()) result.push_back(str10);
      if (!str11.empty()) result.push_back(str11);
      if (!str12.empty()) result.push_back(str12);
      if (!str13.empty()) result.push_back(str13);
      if (!str14.empty()) result.push_back(str14);
      return result;
    }

    ParseArgs(int argc, char **argv, const std::vector<std::string> &optional, const std::vector<std::string> &required) : m_valid(false)
    {
      if (argc < required.size())
        return;

      if (argc < required.size() + 1) {
        std::cerr << argv[0] << " requires at least " << required.size() << " argument(s)." << std::endl;
        m_valid = false;
        return;
      }

      std::vector<OptionalArg> optionalArgs;
      for (std::size_t i = 0; i < optional.size(); ++i) {
        std::size_t open = optional[i].find("(");
        std::size_t close = optional[i].find(")");
        if (open != std::string::npos && close != std::string::npos)
          optionalArgs.push_back(OptionalArg(optional[i].substr(0, open), SC::tokenize(optional[i].substr(open + 1, close - open - 1), ",")));
        else
          optionalArgs.push_back(optional[i]);
      }

      optionalArgs.push_back(OptionalArg("-O$"));
      optionalArgs.push_back(OptionalArg("-dblneg"));
      optionalArgs.push_back(OptionalArg("-elim1"));
      optionalArgs.push_back(OptionalArg("-elim0"));
      optionalArgs.push_back(OptionalArg("-dup"));
      optionalArgs.push_back(OptionalArg("-neg"));
      optionalArgs.push_back(OptionalArg("-binary1"));
      optionalArgs.push_back(OptionalArg("-binary2"));
      optionalArgs.push_back(OptionalArg("-prop"));
      optionalArgs.push_back(OptionalArg("-factor"));
      optionalArgs.push_back(OptionalArg("-atomorder"));
      optionalArgs.push_back(OptionalArg("-bondorder"));

      for (int i = 1; i < argc - required.size(); ++i) {
        for (std::size_t j = 0; j < optionalArgs.size(); ++j) {
          std::string opt = optionalArgs[j].option;
          bool val = false;
          if (opt.rfind("$") != std::string::npos) {
            opt = opt.substr(0, opt.size() - 1);
            val = true;
          }

          if (std::string(argv[i]).substr(0, opt.size()) == opt) {
            if (val)
              m_args[opt] = std::string(argv[i]).substr(opt.size());
            else if (optionalArgs[j].args.size()) {
              if (i + optionalArgs[j].args.size() == argc) {
                std::cerr << "Option " << opt << " requires " << optionalArgs[j].args.size() << "argument(s)." << std::endl;
                m_valid = false;
                return;
              }
              for (std::size_t k = 0; k < optionalArgs[j].args.size(); ++k)
                m_argargs[opt].push_back(argv[i + k + 1]);
              i += optionalArgs[j].args.size();
            } else
              m_args[opt] = "true";
          }
        }
      }

      int req = argc - required.size();
      for (std::size_t j = 0; j < required.size(); ++j)
        m_args[required[j]] = argv[req + j];

      m_valid = true;
    }

    bool IsValid() const
    {
      return m_valid;
    }

    int GetArgCount() const
    {
      return m_args.size() + m_argargs.size();
    }

    bool IsArg(const std::string &key)
    {
      if (m_args.find(key) != m_args.end())
        return true;
      return m_argargs.find(key) != m_argargs.end();
    }

    std::string GetArgString(const std::string &key)
    {
      if (m_args.find(key) != m_args.end())
        return m_args[key];
      return "";
    }

    int GetArgInt(const std::string &key)
    {
      if (m_args.find(key) == m_args.end())
        return -1;
      std::stringstream ss(m_args[key]);
      int result;
      ss >> result;
      return result; 
    }

    std::string GetArgString(const std::string &key, int arg)
    {
      if (m_argargs.find(key) != m_argargs.end())
        return m_argargs[key][arg];
      return "";
    }

    int GetArgInt(const std::string &key, int arg)
    {
      if (m_argargs.find(key) == m_argargs.end())
        return -1;
      std::stringstream ss(m_argargs[key][arg]);
      int result;
      ss >> result;
      return result; 
    }

  private:
    std::map<std::string, std::string> m_args;
    std::map<std::string, std::vector<std::string> > m_argargs;
    bool m_valid;
};

void PrintOptimizationOptions()
{
  std::cerr << "  -On                  Optimization level" << std::endl;
  std::cerr << "                         -O0 no optimization (default)" << std::endl;
  std::cerr << "                         -O1 eliminate tree nodes" << std::endl;
  std::cerr << "                         -O2 optimize binary expressions + O1" << std::endl;
  std::cerr << "                         -O3 expression factorization + O2" << std::endl;
  std::cerr << "                         -O4 optimize atom order + O3" << std::endl;
  std::cerr << "                         -O5 optimize bond order + O4" << std::endl;
  std::cerr << "  -dblneg              Double negation (-O1) (e.g. [!!C] = [C])" << std::endl;
  std::cerr << "  -elim1               True elimination (-O1) (e.g. [*R] = [R])" << std::endl;
  std::cerr << "  -elim0               False elimination (-O1) (e.g. [C!*] = [!*])" << std::endl;
  std::cerr << "  -dup                 Duplicate elimination (-O1) (e.g. [CC])" << std::endl;
  std::cerr << "  -neg                 Negation (-O1) (e.g. [!a] = [A])" << std::endl;
  std::cerr << "  -binary1             Binary expression optimazation (-O2)" << std::endl;
  std::cerr << "  -binary2             Binary expreeion optimization (-O2)" << std::endl;
  std::cerr << "  -prop                False propagation (-O2) (e.g CC[!*] = [!*])" << std::endl;
  std::cerr << "  -factor              Expression factorization (-O3) (e.g. [OH,O-] = [O;H,-])" << std::endl;
  std::cerr << "  -atomorder           Atom order optimization (-O4)" << std::endl;
  std::cerr << "  -bondorder           Bond order optimization (-O5)" << std::endl;
}

int GetOptimizationFlags(ParseArgs &args)
{
  int opt = 0;

  if (args.IsArg("-dblneg"))
    opt |= SC::SmartsOptimizer::DoubleNegationElim;
  if (args.IsArg("-elim1"))
    opt |= SC::SmartsOptimizer::TrueElim;
  if (args.IsArg("-elim0"))
    opt |= SC::SmartsOptimizer::FalseElim;
  if (args.IsArg("-dup"))
    opt |= SC::SmartsOptimizer::DuplicateElim;
  if (args.IsArg("-neg"))
    opt |= SC::SmartsOptimizer::NegationElim;
  if (args.IsArg("-binary1"))
    opt |= SC::SmartsOptimizer::BinaryExpr1;
  if (args.IsArg("-binary2"))
    opt |= SC::SmartsOptimizer::BinaryExpr2;
  if (args.IsArg("-prop"))
    opt |= SC::SmartsOptimizer::AtomFalseProp | SC::SmartsOptimizer::BondFalseProp;
  if (args.IsArg("-factor"))
    opt |= SC::SmartsOptimizer::ExprFactor;
  if (args.IsArg("-atomorder"))
    opt |= SC::SmartsOptimizer::AtomOrder;
  if (args.IsArg("-bondorder"))
    opt |= SC::SmartsOptimizer::BondOrder;

  if (args.IsArg("-O")) {
    int optlvl = args.GetArgInt("-O");
    switch (optlvl) {
      case 1:
        opt |= SC::SmartsOptimizer::O1;
        break;
      case 2:
        opt |= SC::SmartsOptimizer::O2;
        break;
      case 3:
        opt |= SC::SmartsOptimizer::O3;
        break;
      case 4:
        opt |= SC::SmartsOptimizer::O4;
        break;
      case 5:
        opt |= SC::SmartsOptimizer::O5;
        break;
      default:
        break;
    }
  }
  return opt;
}
