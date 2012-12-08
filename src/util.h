#ifndef SC_UTIL_H
#define SC_UTIL_H

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <iostream>

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

  template<typename T>
  struct IsReference { enum { result = false }; };
  template<typename T>
  struct IsReference<T&> { enum { result = true }; };

  template<typename T>
  struct RemoveReference { typedef T result; };
  template<typename T>
  struct RemoveReference<T&> { typedef T result; };

  template<typename T>
  struct IsPointer { enum { result = false }; };
  template<typename T>
  struct IsPointer<T*> { enum { result = true }; };

  template<typename T>
  struct RemovePointer { typedef T result; };
  template<typename T>
  struct RemovePointer<T*> { typedef T result; };

  template<typename T>
  struct IsConst { enum { result = false }; };
  template<typename T>
  struct IsConst<const T> { enum { result = true }; };

  template<typename T>
  struct RemoveConst { typedef T result; };
  template<typename T>
  struct RemoveConst<const T> { typedef T result; };

  template<typename T1, typename T2>
  struct SameType { enum { result = false }; };
  template<typename T> 
  struct SameType<T, T> { enum { result = true }; };
  template<typename T> 
  struct SameType<const T, T> { enum { result = true }; };
  template<typename T> 
  struct SameType<T, const T> { enum { result = true }; };
  template<typename T> 
  struct SameType<T&, T> { enum { result = true }; };
  template<typename T> 
  struct SameType<T, T&> { enum { result = true }; };
  template<typename T> 
  struct SameType<T*, T> { enum { result = true }; };
  template<typename T> 
  struct SameType<T, T*> { enum { result = true }; };









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
  
  inline void replace_first(std::string &str, const std::string &what, const std::string &with = std::string())
  {
    std::size_t pos = str.find(what);
    if (pos != std::string::npos)
      str.replace(pos, what.size(), with);
  }

  inline std::string replace_first(const std::string &str, const std::string &what, const std::string &with = std::string())
  {
    std::string tmp(str);
    replace_first(tmp, what, with);
    return tmp;
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

  inline void strip(std::string &str)
  {
    while (str.size() && str[0] == ' ')
      str = str.substr(1);
    while (str.size() && str[str.size() - 1] == ' ')
      str.resize(str.size() - 1);
  }

  inline std::string stripped(const std::string &str)
  {
    std::string result(str);
    strip(result);
    return result;
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
   * @param repeat The delimiter may be repeated. Only works for single
   *        character delimiter.
   */
  inline std::vector<std::string> tokenize(const std::string &str, const std::string &delimiter, bool repeat = false)
  {
    std::vector<std::string> tokens;
    std::size_t currpos = 0, nextpos = 0;
    //std::cout << "tokenize: \"" << str << "\"" << std::endl;

    while ((nextpos = str.find(delimiter, currpos)) != std::string::npos) {
      if (repeat)
        while (nextpos < str.size() && str[nextpos] == delimiter[0])
          ++nextpos;
      if (nextpos == str.size())
        return tokens;
      tokens.push_back(str.substr(currpos, nextpos - currpos - 1));
      //std::cout << "token: \"" << tokens.back() << "\"" << std::endl;
      currpos = nextpos;
    }
    tokens.push_back(str.substr(currpos, str.length() - currpos));
    //std::cout << "token: \"" << tokens.back() << "\"" << std::endl;

    return tokens;
  }

  /**
   * Containers
   */
  static const int PairFirst = 0;
  static const int PairSecond = 1;

  template<typename T1, typename T2, int index = PairFirst, template<typename> class Compare = std::less>
  struct SortPairs
  {
    typedef typename Select<index == PairFirst, T1, T2>::result value_type;

    enum { First = 0, Second = 1 };
    bool operator()(const std::pair<T1, T2> &left, const std::pair<T1, T2> &right) const
    {
      return index == PairFirst ? compare(left.first, right.first) : compare(left.second, right.second);
    }

    Compare<value_type> compare;
  };

  template<typename T1, typename T2, int index = PairFirst, template<typename> class Compare = std::equal_to>
  struct FindPair
  {
    typedef typename Select<index == PairFirst, T1, T2>::result value_type;

    FindPair(value_type value_) : value(value_)
    {
    }

    bool operator()(const std::pair<T1, T2> &p) const 
    {
      return index == PairFirst ? compare(reinterpret_cast<value_type>(p.first), value) : compare(reinterpret_cast<value_type>(p.second), value);
    } 
    
    Compare<value_type> compare;
    value_type value;
  };

  template<typename T>
  struct ContainerSize
  {
    typedef typename T::size_type property_type;

    bool operator()(const T &c) const
    {
      return c.size();
    }
  };

  template<typename T>
  struct ContainerMinElement
  {
    typedef typename T::value_type property_type;

    bool operator()(const T &c) const
    {
      return *std::min_element(c.begin(), c.end());
    }
  };

  template<typename T>
  struct ContainerMaxElement
  {
    typedef typename T::value_type property_type;

    bool operator()(const T &c) const
    {
      return *std::max_element(c.begin(), c.end());
    }
  };

  template<typename T, template<typename> class Property = ContainerSize, template<typename> class Compare = std::less>
  struct SortContainers
  {
    bool operator()(const T &left, const T &right) const
    {
      return compare(property(left), property(right));
    }

    Property<T> property;
    Compare<typename Property<T>::property_type> compare;
  };

}

#endif
