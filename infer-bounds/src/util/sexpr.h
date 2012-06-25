/*********************                                                        */
/*! \file sexpr.h
 ** \verbatim
 ** Original author: cconway
 ** Major contributors: mdeters
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Simple representation of SMT S-expressions.
 **
 ** Simple representation of SMT S-expressions.
 **/

#include "cvc4_public.h"

#ifndef __CVC4__SEXPR_H
#define __CVC4__SEXPR_H

#include <vector>
#include <string>

#include "util/integer.h"
#include "util/rational.h"
#include "util/Assert.h"

namespace CVC4 {

/**
 * A simple S-expression. An S-expression is either an atom with a
 * string value, or a list of other S-expressions.
 */
class CVC4_PUBLIC SExpr {

  enum SexprTypes {
    SEXPR_STRING,
    SEXPR_INTEGER,
    SEXPR_RATIONAL,
    SEXPR_NOT_ATOM
  } d_sexprType;
  friend std::ostream &operator<<(std::ostream&, SexprTypes);

  /** The value of an atomic integer-valued S-expression. */
  CVC4::Integer d_integerValue;

  /** The value of an atomic rational-valued S-expression. */
  CVC4::Rational d_rationalValue;

  /** The value of an atomic S-expression. */
  std::string d_stringValue;

  /** The children of a list S-expression. */
  std::vector<SExpr> d_children;

public:
  SExpr() :
    d_sexprType(SEXPR_STRING),
    d_integerValue(0),
    d_rationalValue(0),
    d_stringValue(""),
    d_children() {
  }

  SExpr(const CVC4::Integer& value) :
    d_sexprType(SEXPR_INTEGER),
    d_integerValue(value),
    d_rationalValue(0),
    d_stringValue(""),
    d_children() {
  }

  SExpr(const CVC4::Rational& value) :
    d_sexprType(SEXPR_RATIONAL),
    d_integerValue(0),
    d_rationalValue(value),
    d_stringValue(""),
    d_children() {
  }

  SExpr(const std::string& value) :
    d_sexprType(SEXPR_STRING),
    d_integerValue(0),
    d_rationalValue(0),
    d_stringValue(value),
    d_children() {
  }

  SExpr(const std::vector<SExpr> children) :
    d_sexprType(SEXPR_NOT_ATOM),
    d_integerValue(0),
    d_rationalValue(0),
    d_stringValue(""),
    d_children(children) {
  }

  /** Is this S-expression an atom? */
  bool isAtom() const;

  /** Is this S-expression an integer? */
  bool isInteger() const;

  /** Is this S-expression a rational? */
  bool isRational() const;

  /** Is this S-expression a string? */
  bool isString() const;

  /**
   * Get the string value of this S-expression. This will cause an
   * error if this S-expression is not an atom.
   */
  const std::string getValue() const;

  /**
   * Get the integer value of this S-expression. This will cause an
   * error if this S-expression is not an integer.
   */
  const CVC4::Integer& getIntegerValue() const;

  /**
   * Get the rational value of this S-expression. This will cause an
   * error if this S-expression is not a rational.
   */
  const CVC4::Rational& getRationalValue() const;

  /**
   * Get the children of this S-expression. This will cause an error
   * if this S-expression is not a list.
   */
  const std::vector<SExpr> getChildren() const;

};/* class SExpr */

inline std::ostream &operator<<(std::ostream& out, SExpr::SexprTypes type){
 switch(type){
 case SExpr::SEXPR_STRING:
   out << "SEXPR_STRING";
   break;
 case SExpr::SEXPR_INTEGER:
   out << "SEXPR_INTEGER";
   break;
 case SExpr::SEXPR_RATIONAL:
   out << "SEXPR_RATIONAL";
   break;
 case SExpr::SEXPR_NOT_ATOM:
   out << "SEXPR_NOT_ATOM";
   break;
 default:
   Unimplemented();
   break;
 }
 return out;
}

inline bool SExpr::isAtom() const {
  return d_sexprType != SEXPR_NOT_ATOM;
}

inline bool SExpr::isInteger() const {
  return d_sexprType == SEXPR_INTEGER;
}

inline bool SExpr::isRational() const {
  return d_sexprType == SEXPR_RATIONAL;
}

inline bool SExpr::isString() const {
  return d_sexprType == SEXPR_STRING;
}

inline const std::string SExpr::getValue() const {
  AlwaysAssert( isAtom() );
  switch(d_sexprType) {
  case SEXPR_INTEGER:
    return d_integerValue.toString();
  case SEXPR_RATIONAL:
    return d_rationalValue.toString();
  case SEXPR_STRING:
    return d_stringValue;
  default:
    Unhandled(d_sexprType);
  }
  return d_stringValue;
}

inline const CVC4::Integer& SExpr::getIntegerValue() const {
  AlwaysAssert( isInteger() );
  return d_integerValue;
}

inline const CVC4::Rational& SExpr::getRationalValue() const {
  AlwaysAssert( isRational() );
  return d_rationalValue;
}

inline const std::vector<SExpr> SExpr::getChildren() const {
  AlwaysAssert( !isAtom() );
  return d_children;
}

inline std::ostream& operator<<(std::ostream& out, const SExpr& sexpr) {
  if( sexpr.isAtom() ) {
    out << sexpr.getValue();
  } else {
    std::vector<SExpr> children = sexpr.getChildren();
    out << "(";
    bool first = true;
    for( std::vector<SExpr>::iterator it = children.begin();
         it != children.end();
         ++it ) {
      if( first ) {
        first = false;
      } else {
        out << " ";
      }
        out << *it;
    }
    out << ")";
  }
  return out;
}

}/* CVC4 namespace */

#endif /* __CVC4__SEXPR_H */