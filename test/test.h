#include <iostream>
#include <cstdlib>

#ifdef _MVER
#define FUNCTION_SIGNATURE __FUNCSIG__
#else
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

/*inline*/ void report_error(const char* msg, const char* file, int line, const char* func_name, bool require = false)
{
    std::cout << file << ":" << line << ": " << msg << " (FAIL)" << std::endl;
    if (require)
      exit(-1);
}

template <typename T1, typename T2>
void ___test_compare(T1 a, T2 b, const char *expr, const char *file, int line, const char *func_name)
{
  if (!(a == b))
    std::cout << file << ":" << line << ": " << expr << " [" << a << " == " << b << "] (FAIL)" << std::endl;
}

#define ASSERT(exp) \
  ( (exp) ? static_cast<void>(0) : report_error(#exp, __FILE__, __LINE__, FUNCTION_SIGNATURE, false) )

#define REQUIRE(exp) \
  ( (exp) ? static_cast<void>(0) : report_error(#exp, __FILE__, __LINE__, FUNCTION_SIGNATURE, true) )

const char* ___test_expr(const char *expr) { return expr; }
#define EXPR(expr) ___test_expr(#expr)

#define COMPARE(a,b) \
  ___test_compare(a, b, EXPR( a == b ), __FILE__, __LINE__, FUNCTION_SIGNATURE)
