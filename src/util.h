#ifndef SC_UTIL_H
#define SC_UTIL_H

#include <string>
#include <sstream>

namespace SC {

  /**
   * Type traits
   */

  struct NullType {};
  typedef NullType EmptyType;

  template<int Expr>
  struct StaticAssert;
  template<>
  struct StaticAssert<true> {};

  template<typename T1, typename T2>
  struct SameType { enum { result = false }; };
  template<typename T> 
  struct SameType<T, T> { enum { result = true }; };

  template<typename T>
  struct IsPointer { enum { result = false }; };
  template<typename T>
  struct IsPointer<T*> { enum { result = true }; };

  template<typename T>
  struct RemovePointer { typedef T result; };
  template<typename T>
  struct RemovePointer<T*> { typedef T result; };

  template<typename T>
  struct IsInteger { enum { result = false }; };
  /// @cond IMPL
  template<> struct IsInteger<char> { enum { result = true }; };
  template<> struct IsInteger<signed char> { enum { result = true }; };
  template<> struct IsInteger<unsigned char> { enum { result = true }; };
  template<> struct IsInteger<signed short> { enum { result = true }; };
  template<> struct IsInteger<unsigned short> { enum { result = true }; };
  template<> struct IsInteger<signed int> { enum { result = true }; };
  template<> struct IsInteger<unsigned int> { enum { result = true }; };
  template<> struct IsInteger<signed long> { enum { result = true }; };
  template<> struct IsInteger<unsigned long> { enum { result = true }; };
  /// @endcond

  /**
   * Integer to type template. This is used to turn an integer to type and is
   * mainly used to dispatch to helper functions.
   *
   * Example:
   * @code
   * template<typename T>
   * void fooHelper(T v, Int2Type<true>)
   * {
   *   // v is a pointer
   *   v->bar();
   * }
   *
   * template<typename T>
   * void fooHelper(T v, Int2Type<false>)
   * {
   *   // v is a reference
   *   v.bar();
   * }
   *
   * template<typename T>
   * void foo(T v)
   * {
   *   fooHelper(v, Int2Type<IsPointer<T>::result>());
   * }
   * @endcode
   */
  template<int N>
  struct Int2Type { enum { result = N }; };
  template<typename T>
  struct Type2Type { typedef T result; };

  template<typename IfTrueType, typename IfFalseType, int Expr>
  struct SelectHelper { typedef IfTrueType result; };
  template<typename IfTrueType, typename IfFalseType>
  struct SelectHelper<IfTrueType, IfFalseType, false> { typedef IfFalseType result; };
  /**
   * Select a type based on an value of an expression. This works like the
   * expr ? true_value : false_value statement in C++ but operates on types
   * instead.
   *
   * Example:
   * @code
   * // ? : expression with values
   * bool foo = true;
   * int bar = foo ? 0 : 42; // bar = 0;
   *
   * Select<false, int, int*>::result v; // v is of type int*
   * @endcode
   */
  template<int Expr, typename IfTrueType, typename IfFalseType>
  struct Select { typedef typename SelectHelper<IfTrueType, IfFalseType, Expr>::result result; };

  /**
   * Strings
   */
  template<typename T1>
  inline std::string make_string(const T1 &t1)
  {
    std::stringstream ss;
    ss << t1;
    return ss.str();
  }

  template<typename T1, typename T2>
  inline std::string make_string(const T1 &t1, const T2 &t2)
  {
    std::stringstream ss;
    ss << t1 << t2;
    return ss.str();
  }

  template<typename T1, typename T2, typename T3>
  inline std::string make_string(const T1 &t1, const T2 &t2, const T3 &t3)
  {
    std::stringstream ss;
    ss << t1 << t2 << t3;
    return ss.str();
  }

  template<typename T1, typename T2, typename T3, typename T4>
  inline std::string make_string(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
  {
    std::stringstream ss;
    ss << t1 << t2 << t3 << t4;
    return ss.str();
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5>
  inline std::string make_string(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
  {
    std::stringstream ss;
    ss << t1 << t2 << t3 << t4 << t5;
    return ss.str();
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  inline std::string make_string(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
  {
    std::stringstream ss;
    ss << t1 << t2 << t3 << t4 << t5 << t6;
    return ss.str();
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  inline std::string make_string(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7)
  {
    std::stringstream ss;
    ss << t1 << t2 << t3 << t4 << t5 << t6 << t7;
    return ss.str();
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
  inline std::string make_string(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8)
  {
    std::stringstream ss;
    ss << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
    return ss.str();
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
  inline std::string make_string(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9)
  {
    std::stringstream ss;
    ss << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
    return ss.str();
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
  inline std::string make_string(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9, const T10 &t10)
  {
    std::stringstream ss;
    ss << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10;
    return ss.str();
  }
  
  inline std::vector<std::string> tokenize(const std::string &str, const std::string &delimiter)
  {
    std::vector<std::string> tokens;
    std::size_t currpos = 0, nextpos = 0;

    while ((nextpos = str.find(delimiter, currpos)) != std::string::npos) {
      tokens.push_back(str.substr(currpos, nextpos - currpos));
      currpos = nextpos + 1;
    }
    tokens.push_back(str.substr(currpos, str.length() - currpos));

    return tokens;
  }

  inline void replace_all(std::string &str, const std::string &what, const std::string &with = std::string())
  {
    std::size_t last_pos = 0, pos;
    while ((pos = str.find(what, last_pos)) != std::string::npos) {
      str.replace(pos, what.size(), with);
      last_pos = pos + 1;
    }
  }

  inline std::string replace_all(const std::string &str, const std::string &what, const std::string &with = std::string())
  {
    std::string tmp(str);
    replace_all(tmp, what, with);
    return tmp;
  }

  template<typename T>
  inline T string2number(const std::string &str)
  {
    std::stringstream ss(str);
    T number;
    ss >> number;
    return number;
  }

  /**
   * Containers
   */
  static const int PairFirst = 0;
  static const int PairSecond = 1;

  template<typename T1, typename T2, int index = PairFirst, template<typename> class Compare = std::less>
  struct SortPairs
  {
    enum { First = 0, Second = 1 };
    bool operator()(const std::pair<T1, T2> &left, const std::pair<T1, T2> &right) const
    {
      return index == PairFirst ? compare(left.first, right.first) : compare(left.second, right.second);
    }

    typename Select<index == PairFirst, Compare<T1>, Compare<T2> >::result compare;
  };

}

#endif
