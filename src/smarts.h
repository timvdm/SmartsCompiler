#ifndef SC_SMARTS_H
#define SC_SMARTS_H

#include "smiley.h"

namespace SC {

  struct SmartsAtomExpr
  {
    SmartsAtomExpr(int typ) : type(typ) {}
    int type;
    union {
      struct {
        int value;
      } leaf;
      struct {
        void *recursive;
      } recursive;
      struct {
        SmartsAtomExpr *arg;
      } unary;
      struct {
        SmartsAtomExpr *lft;
        SmartsAtomExpr *rgt;
      } binary;
    };
  };

  struct SmartsBondExpr
  {
    SmartsBondExpr(int typ) : type(typ) {}
    int type;
    struct {
      SmartsBondExpr *arg;
    } unary;
    struct {
      SmartsBondExpr *lft;
      SmartsBondExpr *rgt;
    } binary;
  };

  struct SmartsBond
  {
    SmartsBond() : source(0), target(0), grow(false) {}
    SmartsBond(int src, int trg, bool grw = false)
        : source(src), target(trg), grow(grw) {}

    SmartsBondExpr *expr;
    int source;
    int target;
    bool grow;
  };

  struct SmartsAtom
  {
    SmartsAtom() : expr(0), atomClass(0), chiral(false) {}
    SmartsAtom(SmartsAtomExpr *expr_, int ac, bool chrl)
        : expr(expr_), atomClass(ac), chiral(chrl) {}

    SmartsAtomExpr *expr;
    std::vector<const SmartsBond*> bonds;
    int atomClass;
    bool chiral;
  };

  struct Smarts
  {
    Smarts() : chiral(false) {}

    void clear()
    {
      atoms.clear();
      bonds.clear();
      chiral = false;
    }

    std::vector<SmartsAtom> atoms;
    std::vector<SmartsBond> bonds;
    bool chiral;
  };

  /**
   * Parse a SMARTS string and create an expression trees.
   *
   * @param smarts The SMARTS string to parse.
   *
   * @return Smarts object containing expression trees.
   */
  Smarts* parse(const std::string &smarts);

  /**
   * Write a SMARTS string.
   *
   * @param smarts Smarts object with expression trees.
   *
   * @return The SMARTS string.
   */
  std::string write(const Smarts *smarts);

}

#endif
