/*********************                                                        */
/*! \file arith_utilities.h
 ** \verbatim
 ** Original author: taking
 ** Major contributors: none
 ** Minor contributors (to current version): dejan, mdeters
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

#ifndef __CVC4__THEORY__ARITH__ARITH_UTILITIES_H
#define __CVC4__THEORY__ARITH__ARITH_UTILITIES_H

#include "util/rational.h"
#include "expr/node.h"
#include "expr/attribute.h"
#include "theory/arith/delta_rational.h"
#include "context/cdset.h"
#include <vector>
#include <stdint.h>
#include <limits>
#include <ext/hash_map>

namespace CVC4 {
namespace theory {
namespace arith {


typedef uint32_t ArithVar;
//static const ArithVar ARITHVAR_SENTINEL = std::numeric_limits<ArithVar>::max();
#define ARITHVAR_SENTINEL std::numeric_limits<ArithVar>::max()

//Maps from Nodes -> ArithVars, and vice versa
typedef __gnu_cxx::hash_map<Node, ArithVar, NodeHashFunction> NodeToArithVarMap;
typedef __gnu_cxx::hash_map<ArithVar, Node> ArithVarToNodeMap;

//Sets of Nodes
typedef __gnu_cxx::hash_set<Node, NodeHashFunction> NodeSet;
typedef context::CDSet<Node, NodeHashFunction> CDNodeSet;


inline Node mkRationalNode(const Rational& q){
  return NodeManager::currentNM()->mkConst<Rational>(q);
}

inline Node mkBoolNode(bool b){
  return NodeManager::currentNM()->mkConst<bool>(b);
}

inline Node mkIntegerNode(const Integer& z){
  return NodeManager::currentNM()->mkConst<Integer>(z);
}


inline Node makeIntegerVariable(){
  NodeManager* curr = NodeManager::currentNM();
  return curr->mkVar(curr->integerType());
}

inline Rational coerceToRational(TNode constant){
  switch(constant.getKind()){
  case kind::CONST_INTEGER:
    {
      Rational q(constant.getConst<Integer>());
      return q;
    }
  case kind::CONST_RATIONAL:
    return constant.getConst<Rational>();
  default:
    Unreachable();
  }
  Rational unreachable(0,0);
  return unreachable;
}

inline Node coerceToRationalNode(TNode constant){
  switch(constant.getKind()){
  case kind::CONST_INTEGER:
    {
      Rational q(constant.getConst<Integer>());
      Node n = mkRationalNode(q);
      return n;
    }
  case kind::CONST_RATIONAL:
    return constant;
  default:
    Unreachable();
  }
  return Node::null();
}



/** \f$ k \in {LT, LEQ, EQ, GEQ, GT} \f$ */
inline bool isRelationOperator(Kind k){
  using namespace kind;

  switch(k){
  case LT:
  case LEQ:
  case EQUAL:
  case GEQ:
  case GT:
    return true;
  default:
    return false;
  }
}

/**
 * Given a relational kind, k, return the kind k' s.t.
 * swapping the lefthand and righthand side is equivalent.
 *
 * The following equivalence should hold, 
 *   (k l r) <=> (k' r l)
 */
inline Kind reverseRelationKind(Kind k){
  using namespace kind;

  switch(k){
  case LT:    return GT;
  case LEQ:   return GEQ;
  case EQUAL: return EQUAL;
  case GEQ:   return LEQ;
  case GT:    return LT;

  default:
    Unreachable();
  }
}

inline bool evaluateConstantPredicate(Kind k, const Rational& left, const Rational& right){
  using namespace kind;

  switch(k){
  case LT:    return left <  right;
  case LEQ:   return left <= right;
  case EQUAL: return left == right;
  case GEQ:   return left >= right;
  case GT:    return left >  right;
  default:
    Unreachable();
    return true;
  }
}

/**
 * Returns the appropriate coefficient for the infinitesimal given the kind
 * for an arithmetic atom inorder to represent strict inequalities as inequalities.
 *   x < c  becomes  x <= c + (-1) * \f$ \delta \f$
 *   x > c  becomes  x >= x + ( 1) * \f$ \delta \f$
 * Non-strict inequalities have a coefficient of zero.
 */
inline int deltaCoeff(Kind k){
  switch(k){
  case kind::LT:
    return -1;
  case kind::GT:
    return 1;
  default:
    return 0;
  }
}

/**
 * Given a literal to TheoryArith return a single kind to
 * to indicate its underlying structure.
 * The function returns the following in each case:
 * - (K left right) -> K where is a wildcard for EQUAL, LT, GT, LEQ, or GEQ:
 * - (NOT (EQUAL left right)) -> DISTINCT
 * - (NOT (LEQ left right))   -> GT
 * - (NOT (GEQ left right))   -> LT
 * - (NOT (LT left right))    -> GEQ
 * - (NOT (GT left right))    -> LEQ
 * If none of these match, it returns UNDEFINED_KIND.
 */
 inline Kind simplifiedKind(TNode assertion){
  switch(assertion.getKind()){
  case kind::LT:
  case kind::GT:
  case kind::LEQ:
  case kind::GEQ:
  case kind::EQUAL:
    return assertion.getKind();
  case  kind::NOT:
    {
      TNode atom = assertion[0];
      switch(atom.getKind()){
      case  kind::LEQ: //(not (LEQ x c)) <=> (GT x c)
        return  kind::GT;
      case  kind::GEQ: //(not (GEQ x c)) <=> (LT x c)
        return  kind::LT;
      case  kind::LT: //(not (LT x c)) <=> (GEQ x c)
        return  kind::GEQ;
      case  kind::GT: //(not (GT x c) <=> (LEQ x c)
        return  kind::LEQ;
      case  kind::EQUAL:
        return  kind::DISTINCT;
      default:
        Unreachable();
        return  kind::UNDEFINED_KIND;
      }
    }
  default:
    Unreachable();
    return kind::UNDEFINED_KIND;
  }
}

template <bool selectLeft>
inline TNode getSide(TNode assertion, Kind simpleKind){
  switch(simpleKind){
  case kind::LT:
  case kind::GT:
  case kind::DISTINCT:
    return selectLeft ? (assertion[0])[0] : (assertion[0])[1];
  case kind::LEQ:
  case kind::GEQ:
  case kind::EQUAL:
    return selectLeft ? assertion[0] : assertion[1];
  default:
    Unreachable();
    return TNode::null();
  }
}

inline DeltaRational determineRightConstant(TNode assertion, Kind simpleKind){
  TNode right = getSide<false>(assertion, simpleKind);

  Rational inf = Rational(Integer(deltaCoeff(simpleKind)));

  if(right.getKind() == kind::CONST_RATIONAL){
    const Rational& noninf = right.getConst<Rational>();

    return DeltaRational(noninf, inf);
  }else{
    Assert(right.getKind() == kind::CONST_INTEGER);
    Rational noninf = Rational( right.getConst<Integer>() );

    return DeltaRational(noninf, inf);
  }
}

inline DeltaRational asDeltaRational(TNode n){
  Kind simp = simplifiedKind(n);
  return determineRightConstant(n, simp);
}

 /**
  * Takes two nodes with exactly 2 children,
  * the second child of both are of kind CONST_RATIONAL or CONST_INTEGER,
  * and compares value of the two children.
  * This is for comparing inequality nodes.
  *   RightHandConstantLT((<= x 50), (< x 75)) == true
  */
struct RightHandConstantLT
{
  bool operator()(TNode s1, TNode s2) const
  {
    TNode rh1 = s1[1];
    TNode rh2 = s2[1];
    if(rh1.getKind() == kind::CONST_RATIONAL){
      const Rational& q1 = rh1.getConst<Rational>();
      if(rh2.getKind() == kind::CONST_RATIONAL){
        const Rational& q2 = rh2.getConst<Rational>();
        return q1 < q2;
      }else{
        const Integer& z2 = rh2.getConst<Integer>();
        return q1 < Rational(z2);
      }
    }else{
      const Integer& z1 = rh1.getConst<Integer>();
      if(rh2.getKind() == kind::CONST_RATIONAL){
        const Rational& q2 = rh2.getConst<Rational>();
        return Rational(z1) < q2;
      }else{
        const Integer& z2 = rh2.getConst<Integer>();
        return z1 < z2;
      }
    }
  }
};

 /**
  * Takes two nodes with exactly 2 children,
  * the second child of both are of kind CONST_RATIONAL or CONST_INTEGER,
  * and compares value of the two children.
  * This is for comparing inequality nodes.
  *   RightHandConstantLT((<= x 50), (< x 75)) == true
  */
struct RightHandConstantEQ
{
  bool operator()(TNode s1, TNode s2) const
  {
    TNode rh1 = s1[1];
    TNode rh2 = s2[1];
    if(rh1.getKind() == kind::CONST_RATIONAL){
      const Rational& q1 = rh1.getConst<Rational>();
      if(rh2.getKind() == kind::CONST_RATIONAL){
        const Rational& q2 = rh2.getConst<Rational>();
        return q1 == q2;
      }else{
        const Integer& z2 = rh2.getConst<Integer>();
        return q1 == Rational(z2);
      }
    }else{
      const Integer& z1 = rh1.getConst<Integer>();
      if(rh2.getKind() == kind::CONST_RATIONAL){
        const Rational& q2 = rh2.getConst<Rational>();
        return Rational(z1) == q2;
      }else{
        const Integer& z2 = rh2.getConst<Integer>();
        return z1 == z2;
      }
    }
  }
};

inline Node negateConjunctionAsClause(TNode conjunction){
  Assert(conjunction.getKind() == kind::AND);
  NodeBuilder<> orBuilder(kind::OR);

  for(TNode::iterator i = conjunction.begin(), end=conjunction.end(); i != end; ++i){
    TNode child = *i;
    Node negatedChild = NodeBuilder<1>(kind::NOT)<<(child);
    orBuilder << negatedChild;
  }
  return orBuilder;
}

inline Node maybeUnaryConvert(NodeBuilder<>& builder){
  Assert(builder.getKind() == kind::OR ||
         builder.getKind() == kind::AND ||
         builder.getKind() == kind::PLUS ||
         builder.getKind() == kind::MULT);
  Assert(builder.getNumChildren() >= 1);
  if(builder.getNumChildren() == 1){
    return builder[0];
  }else{
    return builder;
  }
}

}; /* namesapce arith */
}; /* namespace theory */
}; /* namespace CVC4 */



#endif /* __CVC4__THEORY__ARITH__ARITH_UTILITIES_H */
