/*********************                                                        */
/*! \file delta_rational.h
 ** \verbatim
 ** Original author: taking
 ** Major contributors: mdeters
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Add one-line brief description here ]]
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

#include "cvc4_private.h"

#include "util/integer.h"
#include "util/rational.h"

#include <ostream>


#ifndef __CVC4__THEORY__ARITH__DELTA_RATIONAL_H
#define __CVC4__THEORY__ARITH__DELTA_RATIONAL_H

namespace CVC4 {

/**
 * A DeltaRational is a pair of rationals (c,k) that represent the number
 *   c + kd
 * where d is an implicit system wide symbolic infinitesimal.
 */
class DeltaRational {
private:
  CVC4::Rational c;
  CVC4::Rational k;

public:
  DeltaRational() : c(0,1), k(0,1) {}
  DeltaRational(const CVC4::Rational& base) : c(base), k(0,1) {}
  DeltaRational(const CVC4::Rational& base, const CVC4::Rational& coeff) :
    c(base), k(coeff) {}

  const CVC4::Rational& getInfinitesimalPart() const {
    return k;
  }

  const CVC4::Rational& getNoninfinitesimalPart() const {
    return c;
  }

  int sgn() const {
    int s = getNoninfinitesimalPart().sgn();
    if(s == 0){
      return infinitesimalSgn();
    }else{
      return s;
    }
  }

  int infinitesimalSgn() const {
    return getInfinitesimalPart().sgn();
  }

  int cmp(const DeltaRational& other) const{
    int cmp = c.cmp(other.c);
    if(cmp == 0){
      return k.cmp(other.k);
    }else{
      return cmp;
    }
  }

  DeltaRational operator+(const DeltaRational& other) const{
    CVC4::Rational tmpC = c+other.c;
    CVC4::Rational tmpK = k+other.k;
    return DeltaRational(tmpC, tmpK);
  }

  DeltaRational operator*(const Rational& a) const{
    CVC4::Rational tmpC = a*c;
    CVC4::Rational tmpK = a*k;
    return DeltaRational(tmpC, tmpK);
  }

  DeltaRational operator-(const DeltaRational& a) const{
    CVC4::Rational negOne(CVC4::Integer(-1));
    return *(this) + (a * negOne);
  }

  DeltaRational operator-() const{
    return DeltaRational(-c, -k);
  }

  DeltaRational operator/(const Rational& a) const{
    CVC4::Rational tmpC = c/a;
    CVC4::Rational tmpK = k/a;
    return DeltaRational(tmpC, tmpK);
  }

  DeltaRational operator/(const Integer& a) const{
    CVC4::Rational tmpC = c/a;
    CVC4::Rational tmpK = k/a;
    return DeltaRational(tmpC, tmpK);
  }

  bool operator==(const DeltaRational& other) const{
    return (k == other.k) && (c == other.c);
  }

  bool operator!=(const DeltaRational& other) const{
    return (*this == other);
  }

  bool operator<=(const DeltaRational& other) const{
    int cmp = c.cmp(other.c);
    return (cmp < 0) || ((cmp==0)&&(k <= other.k));
  }
  bool operator<(const DeltaRational& other) const{
    return (other  > *this);
  }
  bool operator>=(const DeltaRational& other) const{
    return (other <= *this);
  }
  bool operator>(const DeltaRational& other) const{
    return !(*this <= other);
  }

  DeltaRational& operator=(const DeltaRational& other){
    c = other.c;
    k = other.k;
    return *(this);
  }

  DeltaRational& operator*=(const CVC4::Rational& a){
    c *=  a;
    k *=  a;

    return *(this);
  }

  DeltaRational& operator+=(DeltaRational& other){
    c += other.c;
    k += other.k;

    return *(this);
  }

  bool isIntegral() const {
    if(getInfinitesimalPart().sgn() == 0){
      return getNoninfinitesimalPart().isIntegral();
    }else{
      return false;
    }
  }

  Integer floor() const {
    if(getNoninfinitesimalPart().isIntegral()){
      if(getInfinitesimalPart().sgn() >= 0){
        return getNoninfinitesimalPart().getNumerator();
      }else{
        return getNoninfinitesimalPart().getNumerator() - Integer(1);
      }
    }else{
      return getNoninfinitesimalPart().floor();
    }
  }

  Integer ceiling() const {
    if(getNoninfinitesimalPart().isIntegral()){
      if(getInfinitesimalPart().sgn() <= 0){
        return getNoninfinitesimalPart().getNumerator();
      }else{
        return getNoninfinitesimalPart().getNumerator() + Integer(1);
      }
    }else{
      return getNoninfinitesimalPart().ceiling();
    }
  }

  std::string toString() const;

  Rational substituteDelta(const Rational& d) const{
    return getNoninfinitesimalPart() + (d * getInfinitesimalPart());
  }

};

std::ostream& operator<<(std::ostream& os, const DeltaRational& n);

}/* CVC4 namespace */

#endif /* __CVC4__THEORY__ARITH__DELTA_RATIONAL_H */