/*********************                                                        */
/*! \file normal_form.h
 ** \verbatim
 ** Original author: taking
 ** Major contributors: mdeters
 ** Minor contributors (to current version): dejan
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

#ifndef __CVC4__THEORY__ARITH__NORMAL_FORM_H
#define __CVC4__THEORY__ARITH__NORMAL_FORM_H

#include "expr/node.h"
#include "expr/node_self_iterator.h"
#include "util/rational.h"
#include "theory/theory.h"
#include "theory/arith/arith_utilities.h"

#include <list>
#include <algorithm>
#include <ext/algorithm>

namespace CVC4 {
namespace theory {
namespace arith {

/***********************************************/
/***************** Normal Form *****************/
/***********************************************/
/***********************************************/

/**
 * Section 1: Languages
 * The normal form for arithmetic nodes is defined by the language
 * accepted by the following BNFs with some guard conditions.
 * (The guard conditions are in Section 3 for completeness.)
 *
 * variable := n
 *   where
 *     n.getMetaKind() == metakind::VARIABLE
 *
 * interpreted_function := (div m n) | (mod m n)
 *   where
 *     m,n are polynomials
 *
 * field_foreign := f
 *   where
 *     f has type real or integer, and
 *       the theory of f is not arithmetic.
 *
 * leaf := variable | field_foreign | axiomatized_function
 *   See the notes on leaf below.
 *
 * constant := n
 *   where
 *     n.getKind() == kind::CONST_RATIONAL
 *
 * prod_list := base | (* [base])
 *   where
 *     len [base] >= 2
 *     isSorted baseOrder [base]
 *
 * monomial := constant | prod_list | (* constant' prod_list')
 *   where
 *     \f$ constant' \not\in {0,1} \f$
 *
 * polynomial := monomial' | (+ [monomial])
 *   where
 *     len [monomial] >= 2
 *     isStrictlySorted monoOrder [monomial]
 *     forall (\x -> x != 0) [monomial]
 *
 * restricted_cmp := (|><| polynomial constant)
 *   where
 *     |><| is GEQ, EQ, or EQ
 *     not (exists constantMonomial (monomialList polynomial))
 *     monomialCoefficient (head (monomialList polynomial)) == 1
 *
 * comparison := TRUE | FALSE | restricted_cmp | (not restricted_cmp)
 *
 * Normal Form for terms := polynomial
 * Normal Form for atoms := comparison
 */

/**
 * Leaf:
 * Shared terms must be handled by the theory and the normal form.
 * Terms that are handled purely by axiom instantiation, div and mod,
 * need to be in the normal form as well.
 * These are handled as field_foreign terms.
 * Strictly speaking these break the normality of the "normal form".
 */

/**
 * Section 2: Helper Classes
 * The langauges accepted by each of these defintions
 * roughly corresponds to one of the following helper classes:
 *  Variable
 *  Constant
 *  InterpretedFunction
 *  FieldForeign
 *  Leaf
 *  ProdList
 *  Monomial
 *  Polynomial
 *  Comparison
 *
 * Each of the classes obeys the following contracts/design decisions:
 * -Calling isMember(Node node) on a node returns true iff that node is a
 *  a member of the language. Note: isMember is O(n).
 * -Calling isNormalForm() on a helper class object returns true iff that
 *  helper class currently represents a normal form object.
 * -If isNormalForm() is false, then this object must have been made
 *  using a mk*() factory function.
 * -If isNormalForm() is true, calling getNode() on all of these classes
 *  returns a node that would be accepted by the corresponding language.
 *  And if isNormalForm() is false, returns Node::null().
 * -Each of the classes is immutable.
 * -Public facing constuctors have a 1-to-1 correspondence with one of
 *  production rules in the above grammar.
 * -Public facing constuctors are required to fail in debug mode when the
 *  guards of the production rule are not strictly met.
 *  For example: Monomial(Constant(1),ProdList(Variable(x))) must fail.
 * -When a class has a Class parseClass(Node node) function,
 *  if isMember(node) is true, the function is required to return an instance
 *  of the helper class, instance, s.t. instance.getNode() == node.
 *  And if isMember(node) is false, this throws an assertion failure in debug
 *  mode and has undefined behaviour if not in debug mode.
 * -Only public facing constructors, parseClass(node), and mk*() functions are
 *  considered privileged functions for the helper class.
 * -Only privileged functions may use private constructors, and access
 *  private data members.
 * -All non-privileged functions are considered utility functions and
 *  must use a privileged function in order to create an instance of the class.
 */

/**
 * Section 3: Guard Conditions Misc.
 *
 *
 *  leaf_list_len vl =
 *    match vl with
 *       base -> 1
 *     | (* [leaf]) -> len [leaf]
 *
 *  order res =
 *    match res with
 *       Empty -> (0,Node::null())
 *     | NonEmpty(vl) -> (var_list_len vl, vl)
 *
 *  leaf_listOrder a b = tuple_cmp (order a) (order b)
 *
 *  monomialLeafList monomial =
 *    match monomial with
 *        constant -> Empty
 *      | leaf_list -> NonEmpty(leaf_list)
 *      | (* constant' leaf_list') -> NonEmpty(leaf_list')
 *
 *  monoOrder m0 m1 = var_listOrder (monomialLeafList m0) (monomialLeafList m1)
 *
 *  constantMonomial monomial =
 *    match monomial with
 *        constant -> true
 *      | leaf_list -> false
 *      | (* constant' leaf_list') -> false
 *
 *  monomialCoefficient monomial =
 *    match monomial with
 *        constant -> constant
 *      | leaf_list -> Constant(1)
 *      | (* constant' leaf_list') -> constant'
 *
 *  monomialList polynomial =
 *    match polynomial with
 *        monomial -> monomial::[]
 *      | (+ [monomial]) -> [monomial]
 */


/**
 * A NodeWrapper is a class that is a thinly veiled container of a Node object.
 */
class NodeWrapper {
private:
  Node node;
public:
  NodeWrapper(Node n) : node(n) {}
  const Node& getNode() const { return node; }
};/* class NodeWrapper */


class Variable : public NodeWrapper {
public:
  Variable(Node n) : NodeWrapper(n) {
    Assert(isMember(getNode()));
  }
  static bool isMember(Node n) {
    return
      n.getMetaKind() == kind::metakind::VARIABLE &&
      Theory::isLeafOf(n, theory::THEORY_ARITH);
  }

  bool isNormalForm() const { return isMember(getNode()); }
};/* class Variable */


class Constant : public NodeWrapper {
public:
  Constant(Node n) : NodeWrapper(n) {
    Assert(isMember(getNode()));
  }

  static bool isMember(Node n) {
    return n.getKind() == kind::CONST_RATIONAL;
  }

  bool isNormalForm() { return isMember(getNode()); }

  static Constant mkConstant(Node n) {
    return Constant(coerceToRationalNode(n));
  }

  static Constant mkConstant(const Rational& rat) {
    return Constant(mkRationalNode(rat));
  }

  const Rational& getValue() const {
    return getNode().getConst<Rational>();
  }

  bool isZero() const { return getValue() == 0; }
  bool isOne() const { return getValue() == 1; }

  Constant operator*(const Constant& other) const {
    return mkConstant(getValue() * other.getValue());
  }
  Constant operator+(const Constant& other) const {
    return mkConstant(getValue() + other.getValue());
  }
  Constant operator-() const {
    return mkConstant(-getValue());
  }

};/* class Constant */

class FieldForeign : public NodeWrapper {
public:
  FieldForeign(Node n) : NodeWrapper(n) {
    Assert(isMember(n));
  }

  static bool isMember(Node n){
    if (n.getKind() == kind::CONST_INTEGER) return false;
    if (n.getKind() == kind::CONST_RATIONAL) return false;
    if (isRelationOperator(n.getKind())) return false;
    if (n.getMetaKind() == kind::metakind::VARIABLE) return false;
    return Theory::isLeafOf(n, theory::THEORY_ARITH);
  }

  bool isNormalForm() const { return isMember(getNode()); }

}; /* class Foreign */

class InterpretedFunction : public NodeWrapper {
public:
  InterpretedFunction(Node n): NodeWrapper(n){
    Assert(isMember(getNode()));
  }

  static bool isMember(Node n);
  bool isNormalForm() const { return isMember(getNode()); }
};

class Leaf : public NodeWrapper {
public:
  Leaf(Node n) : NodeWrapper(n){
    AssertArgument(isMember(n), n);
  }

  static bool isMember(Node n){
    return
      Variable::isMember(n) ||
      FieldForeign::isMember(n) ||
      InterpretedFunction::isMember(n);
  }

  bool isNormalForm() const { return isMember(getNode()); }

  bool operator<(const Leaf& v) const { return getNode() < v.getNode();}
  bool operator==(const Leaf& v) const { return getNode() == v.getNode();}
};

template <class GetNodeIterator>
inline Node makeNode(Kind k, GetNodeIterator start, GetNodeIterator end) {
  NodeBuilder<> nb(k);

  while(start != end) {
    nb << (*start).getNode();
    ++start;
  }

  return Node(nb);
}/* makeNode<GetNodeIterator>(Kind, iterator, iterator) */


template <class GetNodeIterator, class T>
static void copy_range(GetNodeIterator begin, GetNodeIterator end, std::vector<T>& result){
  while(begin != end){
    result.push_back(*begin);
    ++begin;
  }
}

template <class GetNodeIterator, class T>
static void merge_ranges(GetNodeIterator first1,
                  GetNodeIterator last1,
                  GetNodeIterator first2,
                  GetNodeIterator last2,
                  std::vector<T>& result) {

  while(first1 != last1 && first2 != last2){
    if( (*first1) < (*first2) ){
      result.push_back(*first1);
      ++ first1;
    }else{
      result.push_back(*first2);
      ++ first2;
    }
  }
  copy_range(first1, last1, result);
  copy_range(first2, last2, result);
}

/**
 * A LeafList is a sorted list of variables representing a product.
 * If the LeafList is empty, it represents an empty product or 1.
 * If the LeafList has size 1, it represents a single variable.
 *
 * A non-sorted LeafList can never be successfully made in debug mode.
 */
class LeafList : public NodeWrapper {
private:

  static Node multList(const std::vector<Leaf>& list) {
    Assert(list.size() >= 2);

    return makeNode(kind::MULT, list.begin(), list.end());
  }

  LeafList() : NodeWrapper(Node::null()) {}

  LeafList(Node n) : NodeWrapper(n) {
    Assert(isSorted(begin(), end()));
  }

  typedef expr::NodeSelfIterator internal_iterator;

  internal_iterator internalBegin() const {
    if(singleton()){
      return expr::NodeSelfIterator::self(getNode());
    }else{
      return getNode().begin();
    }
  }

  internal_iterator internalEnd() const {
    if(singleton()){
      return expr::NodeSelfIterator::selfEnd(getNode());
    }else{
      return getNode().end();
    }
  }

public:

  class iterator {
  private:
    internal_iterator d_iter;

  public:
    explicit iterator(internal_iterator i) : d_iter(i) {}

    inline Leaf operator*() {
      return Leaf(*d_iter);
    }

    bool operator==(const iterator& i) {
      return d_iter == i.d_iter;
    }

    bool operator!=(const iterator& i) {
      return d_iter != i.d_iter;
    }

    iterator operator++() {
      ++d_iter;
      return *this;
    }

    iterator operator++(int) {
      return iterator(d_iter++);
    }
  };

  iterator begin() const {
    return iterator(internalBegin());
  }

  iterator end() const {
    return iterator(internalEnd());
  }

  LeafList(Leaf v) : NodeWrapper(v.getNode()) {
    Assert(isSorted(begin(), end()));
  }

  LeafList(const std::vector<Leaf>& l) : NodeWrapper(multList(l)) {
    Assert(l.size() >= 2);
    Assert(isSorted(begin(), end()));
  }

  static bool isMember(Node n);

  bool isNormalForm() const {
    return !empty();
  }

  static LeafList mkEmptyLeafList() {
    return LeafList();
  }


  /** There are no restrictions on the size of l */
  static LeafList mkLeafList(const std::vector<Leaf>& l) {
    if(l.size() == 0) {
      return mkEmptyLeafList();
    } else if(l.size() == 1) {
      return LeafList((*l.begin()).getNode());
    } else {
      return LeafList(l);
    }
  }

  bool empty() const { return getNode().isNull(); }
  bool singleton() const {
    return !empty() && getNode().getKind() != kind::MULT;
  }

  int size() const {
    if(singleton())
      return 1;
    else
      return getNode().getNumChildren();
  }

  static LeafList parseLeafList(Node n);

  LeafList operator*(const LeafList& vl) const;

  int cmp(const LeafList& vl) const;

  bool operator<(const LeafList& vl) const { return cmp(vl) < 0; }

  bool operator==(const LeafList& vl) const { return cmp(vl) == 0; }

private:
  bool isSorted(iterator start, iterator end);

};/* class LeafList */


class Monomial : public NodeWrapper {
private:
  Constant constant;
  LeafList leafList;
  Monomial(Node n, const Constant& c, const LeafList& ll):
    NodeWrapper(n), constant(c), leafList(ll)
  {
    Assert(!c.isZero() ||  ll.empty() );
    Assert( c.isZero() || !ll.empty() );

    Assert(!c.isOne() || !multStructured(n));
  }

  static Node makeMultNode(const Constant& c, const LeafList& ll) {
    Assert(!c.isZero());
    Assert(!c.isOne());
    Assert(!ll.empty());
    return NodeManager::currentNM()->mkNode(kind::MULT, c.getNode(), ll.getNode());
  }

  static bool multStructured(Node n) {
    return n.getKind() ==  kind::MULT &&
      n[0].getKind() == kind::CONST_RATIONAL &&
      n.getNumChildren() == 2;
  }

public:

  Monomial(const Constant& c):
    NodeWrapper(c.getNode()), constant(c), leafList(LeafList::mkEmptyLeafList())
  { }

  Monomial(const LeafList& ll):
    NodeWrapper(ll.getNode()), constant(Constant::mkConstant(1)), leafList(ll)
  {
    Assert( !leafList.empty() );
  }

  Monomial(const Constant& c, const LeafList& ll):
    NodeWrapper(makeMultNode(c,ll)), constant(c), leafList(ll)
  {
    Assert( !c.isZero() );
    Assert( !c.isOne() );
    Assert( !leafList.empty() );

    Assert(multStructured(getNode()));
  }

  static bool isMember(TNode n);

  /** Makes a monomial with no restrictions on c and vl. */
  static Monomial mkMonomial(const Constant& c, const LeafList& ll);


  static Monomial parseMonomial(Node n);

  static Monomial mkZero() {
    return Monomial(Constant::mkConstant(0));
  }
  static Monomial mkOne() {
    return Monomial(Constant::mkConstant(1));
  }
  const Constant& getConstant() const { return constant; }
  const LeafList& getLeafList() const { return leafList; }

  bool isConstant() const {
    return leafList.empty();
  }

  bool isZero() const {
    return constant.isZero();
  }

  bool coefficientIsOne() const {
    return constant.isOne();
  }

  Monomial operator*(const Monomial& mono) const;


  int cmp(const Monomial& mono) const {
    return getLeafList().cmp(mono.getLeafList());
  }

  bool operator<(const Monomial& vl) const {
    return cmp(vl) < 0;
  }

  bool operator==(const Monomial& vl) const {
    return cmp(vl) == 0;
  }

  static bool isSorted(const std::vector<Monomial>& m) {
    return __gnu_cxx::is_sorted(m.begin(), m.end());
  }

  static bool isStrictlySorted(const std::vector<Monomial>& m) {
    return isSorted(m) && std::adjacent_find(m.begin(),m.end()) == m.end();
  }

  /**
   * Given a sorted list of monomials, this function transforms this
   * into a strictly sorted list of monomials that does not contain zero.
   */
  static std::vector<Monomial> sumLikeTerms(const std::vector<Monomial>& monos);

  void print() const;
  static void printList(const std::vector<Monomial>& list);

};/* class Monomial */


class Polynomial : public NodeWrapper {
private:
  bool d_singleton;

  Polynomial(TNode n) : NodeWrapper(n), d_singleton(Monomial::isMember(n)) {
    Assert(isMember(getNode()));
  }

  static Node makePlusNode(const std::vector<Monomial>& m) {
    Assert(m.size() >= 2);

    return makeNode(kind::PLUS, m.begin(), m.end());
  }

  typedef expr::NodeSelfIterator internal_iterator;

  internal_iterator internalBegin() const {
    if(singleton()){
      return expr::NodeSelfIterator::self(getNode());
    }else{
      return getNode().begin();
    }
  }

  internal_iterator internalEnd() const {
    if(singleton()){
      return expr::NodeSelfIterator::selfEnd(getNode());
    }else{
      return getNode().end();
    }
  }

  bool singleton() const { return d_singleton; }

public:
  static bool isMember(TNode n) {
    if(Monomial::isMember(n)){
      return true;
    }else if(n.getKind() == kind::PLUS){
      Assert(n.getNumChildren() >= 2);
      for(Node::iterator curr = n.begin(), end = n.end(); curr != end;++curr){
        if(!Monomial::isMember(*curr)){
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
  }

  class iterator {
  private:
    internal_iterator d_iter;

  public:
    explicit iterator(internal_iterator i) : d_iter(i) {}

    inline Monomial operator*() {
      return Monomial::parseMonomial(*d_iter);
    }

    bool operator==(const iterator& i) {
      return d_iter == i.d_iter;
    }

    bool operator!=(const iterator& i) {
      return d_iter != i.d_iter;
    }

    iterator operator++() {
      ++d_iter;
      return *this;
    }

    iterator operator++(int) {
      return iterator(d_iter++);
    }
  };

  iterator begin() const { return iterator(internalBegin()); }
  iterator end() const {  return iterator(internalEnd()); }

  Polynomial(const Monomial& m):
    NodeWrapper(m.getNode()), d_singleton(true)
  {}

  Polynomial(const std::vector<Monomial>& m):
    NodeWrapper(makePlusNode(m)), d_singleton(false)
  {
    Assert( m.size() >= 2);
    Assert( Monomial::isStrictlySorted(m) );
  }


  static Polynomial mkPolynomial(const std::vector<Monomial>& m) {
    if(m.size() == 0) {
      return Polynomial(Monomial::mkZero());
    } else if(m.size() == 1) {
      return Polynomial((*m.begin()));
    } else {
      return Polynomial(m);
    }
  }

  static Polynomial parsePolynomial(Node n) {
    return Polynomial(n);
  }

  static Polynomial mkZero() {
    return Polynomial(Monomial::mkZero());
  }
  static Polynomial mkOne() {
    return Polynomial(Monomial::mkOne());
  }
  bool isZero() const {
    return singleton() && (getHead().isZero());
  }

  bool isConstant() const {
    return singleton() && (getHead().isConstant());
  }

  bool containsConstant() const {
    return getHead().isConstant();
  }

  Monomial getHead() const {
    return *(begin());
  }

  Polynomial getTail() const {
    Assert(! singleton());

    iterator tailStart = begin();
    ++tailStart;
    std::vector<Monomial> subrange;
    copy_range(tailStart, end(), subrange);
    return mkPolynomial(subrange);
  }

  void printList() const {
    if(Debug.isOn("normal-form")){
      Debug("normal-form") << "start list" << std::endl;
      for(iterator i = begin(), oend = end(); i != oend; ++i) {
        const Monomial& m =*i;
        m.print();
      }
      Debug("normal-form") << "end list" << std::endl;
    }
  }

  Polynomial operator+(const Polynomial& vl) const;

  Polynomial operator*(const Monomial& mono) const;

  Polynomial operator*(const Polynomial& poly) const;

};/* class Polynomial */


class Comparison : public NodeWrapper {
private:
  Kind oper;
  Polynomial left;
  Constant right;

  static Node toNode(Kind k, const Polynomial& l, const Constant& r);

  Comparison(TNode n, Kind k, const Polynomial& l, const Constant& r):
    NodeWrapper(n), oper(k), left(l), right(r)
  { }

  /**
   * Possibly simplify a comparison with a pseudoboolean-typed LHS.  k
   * is one of LT, LEQ, EQUAL, GEQ, GT, and left must be PB-typed.  If
   * possible, "left k right" is fully evaluated, "true" is returned,
   * and the result of the evaluation is returned in "result".  If no
   * evaluation is possible, false is returned and "result" is
   * untouched.
   *
   * For example, pbComparison(kind::EQUAL, "x", 0.5, result) returns
   * true, and updates "result" to false, since pseudoboolean variable
   * "x" can never equal 0.5.  pbComparison(kind::GEQ, "x", 1, result)
   * returns false, since "x" can be >= 1, but could also be less.
   */
  static bool pbComparison(Kind k, TNode left, const Rational& right, bool& result);

public:
  Comparison(bool val) :
    NodeWrapper(NodeManager::currentNM()->mkConst(val)),
    oper(kind::CONST_BOOLEAN),
    left(Polynomial::mkZero()),
    right(Constant::mkConstant(0))
  { }

  Comparison(Kind k, const Polynomial& l, const Constant& r):
    NodeWrapper(toNode(k, l, r)), oper(k), left(l), right(r)
  {
    Assert(isRelationOperator(oper));
    Assert(!left.containsConstant());
    Assert(left.getHead().getConstant().isOne());
  }

  static Comparison mkComparison(Kind k, const Polynomial& left, const Constant& right);

  bool isBoolean() const {
    return (oper == kind::CONST_BOOLEAN);
  }

  bool isNormalForm() const {
    if(isBoolean()) {
      return true;
    } else if(left.containsConstant()) {
      return false;
    } else if(left.getHead().getConstant().isOne()) {
      return true;
    } else {
      return false;
    }
  }

  const Polynomial& getLeft() const { return left; }
  const Constant& getRight() const { return right; }

  Comparison addConstant(const Constant& constant) const;
  Comparison multiplyConstant(const Constant& constant) const;

  static Comparison parseNormalForm(TNode n);

  inline static bool isNormalAtom(TNode n){
    Comparison parse = Comparison::parseNormalForm(n);
    return parse.isNormalForm();
  }

};/* class Comparison */

}/* CVC4::theory::arith namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__ARITH__NORMAL_FORM_H */
