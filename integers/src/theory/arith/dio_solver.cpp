/*********************                                                        */
/*! \file dio_solver.cpp
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Diophantine equation solver
 **
 ** A Diophantine equation solver for the theory of arithmetic.
 **/

#include "theory/arith/dio_solver.h"

#include <iostream>

using namespace std;

namespace CVC4 {
namespace theory {
namespace arith {

bool DioSolver::debugInInputEquations(Node eq){
  typedef context::CDList<Node>::const_iterator const_iterator;
  const_iterator i=d_inputConstraints.begin(), end = d_inputConstraints.end();
  for(; i != end; ++i){
    if(eq == *i){
      return true;
    }
  }
  return false;
}


Node DioSolver::addToPool(Node newEq){
  Assert(!debugInInputEquations(newEq));

  size_t pos = d_inputConstraints.size();
  d_inputConstraints.push_back(newEq);

  Node var = Node::null();
  Assert(pos <= d_variablePool.size());
  if(pos == d_variablePool.size()){
    Assert(d_inputConstraints.size() == 1+d_variablePool.size());
    var = makeIntegerVariable();
    d_variablePool.push_back(var);
  }else{
    var = d_variablePool[pos];
  }
  Assert(!var.isNull());

  //If it belongs to the map already or it is not in the map, update the address
  d_nodeToPoolMap[var] = pos;

  return var;
}


void DioSolver::internalAppendEquality(const SumPair& sp, const Polynomial& p){
  Index last = back();

  d_facts.reserve(last+1, SumPair::mkZero());
  d_proofs.reserve(last+1, Polynomial::mkZero());

  d_facts.set(last, sp);
  d_proofs.set(last, p);

  pushQueue();
}

bool acceptableOriginalNodes(Node n){
  Kind k = n.getKind();
  if(k == kind::EQUAL){
    return true;
  }else if(k == kind::AND){
    Node ub = n[0];
    Node lb = n[1];
    Kind kub = simplifiedKind(ub);
    Kind klb = simplifiedKind(lb);
    return (kub == kind::LEQ || kub==kind::LT) && (klb == kind::GEQ || klb == kind::GT);
  }else{
    return false;
  }
}

void DioSolver::addEquality(const Comparison& eq, Node orig){
  Assert(!debugInInputEquations(orig));
  Assert(eq.isInteger());
  Assert(acceptableOriginalNodes(orig));

  SumPair sp = comparisonToSumPair(eq);

  Node var = addToPool(orig);
  Assert(Variable::isMember(var));
  Polynomial pvar = Polynomial(Monomial(VarList(Variable(var))));
  internalAppendEquality(sp, pvar);
}


void DioSolver::scaleEqAtIndex(Index i, const Integer& g){
  Assert(g != 0);
  Constant invg = Constant::mkConstant(Rational(Integer(1),g));
  const SumPair& sp = d_facts[i];
  const Polynomial& proof = d_proofs[i];

  SumPair newSP = sp * invg;
  Polynomial newProof = proof * invg;

  Assert(newSP.isInteger());
  Assert(newSP.gcd() == 1);
  d_facts.set(i, newSP);
  d_proofs.set(i, newProof);

  Debug("arith::dio") << "scaleEqAtIndex(" << i <<","<<g<<")"<<endl;
  Debug("arith::dio") << "derived "<< newSP.getNode()
                      <<" with proof " << newProof.getNode() << endl;
}


Node DioSolver::proveIndex(Index i){
  Assert(hasProof(i));
  const Polynomial& proof = d_proofs[i];
  Assert(!proof.isConstant()); // Is it possible for a constant proof to be okay?

  NodeBuilder<> nb(kind::AND);
  for(Polynomial::iterator iter = proof.begin(), end = proof.end(); iter!= end; ++iter){
    Monomial m = (*iter);
    Assert(!m.isConstant());
    VarList vl = m.getVarList();
    Assert(vl.singleton());
    Variable v = vl.getHead();
    Assert(d_nodeToPoolMap.find(v.getNode()) != d_nodeToPoolMap.end());
    size_t pos = (*(d_nodeToPoolMap.find(v.getNode()))).second;
    Assert(pos < d_inputConstraints.size());

    Node input = d_inputConstraints[pos];
    Assert(acceptableOriginalNodes(input));
    if(input.getKind() == kind::AND){
      nb << input[0] << input[1];
    }else{
      nb << input;
    }
  }

  Node result = (nb.getNumChildren() == 1) ? nb[0] : (Node)nb;
  Debug("arith::dio") << "Proof at " << i << " is "
                      << d_facts[i].getNode() << endl
                      <<  d_proofs[i].getNode() << endl
                      << " which becomes " << result << endl;
  return result;
}

Node DioSolver::processEquationsForConflict(){
  while(!empty()){
    Index f = front();
    bool conflict = processFront(true);
    if(conflict){
      return proveIndex(f);
    }
  }
  return Node::null();
}

SumPair DioSolver::processEquationsForCut(){
  while(!empty()){
    Index f = front();
    bool cut = processFront(false);
    if(cut){
      return purifyIndex(f);
    }
  }
  return SumPair::mkZero();
}

SumPair DioSolver::purifyIndex(Index i){
  SumPair curr = d_facts[i];

  if(d_subRange.size() >= 1){
    Constant negOne = Constant::mkConstant(-1);

    for(uint32_t iter = 0, N = d_subRange.size(); iter < N; ++iter){
      uint32_t i = N-(iter+1);
      Node freshNode = d_subFresh[i];
      if(freshNode.isNull()){
        continue;
      }else{
        Variable var(freshNode);
        Polynomial vsum = curr.getPolynomial();

        Constant a = vsum.getCoefficient(VarList(var));
        if(!a.isZero()){
          const SumPair& sj = d_facts[d_subRange[i]];
          Assert(sj.getPolynomial().getCoefficient(VarList(var)).isOne());
          SumPair newSi = (curr * negOne) + (sj * a);
          Assert(newSi.getPolynomial().getCoefficient(VarList(var)).isZero());
          curr = newSi;
        }
      }
    }
  }
  return curr;
}

bool DioSolver::processFront(bool conflict){
  Assert(!empty());

  applySubstitutionsToFront();

  Index i = front();
  const SumPair& sp = d_facts[i];
  Polynomial vsum = sp.getPolynomial();
  Constant c = sp.getConstant();

  //This is unclear whether it is true or not.
  //We need to identify cases where it is false if it is not true.
  //For now this is worth it in order to assume we know
  //why conflicts occur.
  Assert(!vsum.isConstant());

  if(vsum.isConstant()){
    Assert(vsum.isZero());
    if(c.isZero()){
      // (+ 0 0) = 0, is trivially sat
      Debug("arith::dio") << "Proof of 0 = 0 at index " << i << endl;
      popQueue();
      return false;
    }else{
      Debug("arith::dio") << "Proof of c != 0 at index " << i << endl;
      return conflict;
    }
  }else{
    Assert(!vsum.isConstant());
    Integer g = vsum.gcd();
    if(g.divides(c.getIntegerValue())){
      if(g > 1){
        scaleEqAtIndex(i, g);
      }
      decomposeFront();
      return false;
    }else{
      Debug("arith::dio") << "The gcd of the coefficients of the variables "
                          << "does not divides the constant term in "
                          << sp.getNode() << endl;
      return true;
    }
  }
}


void DioSolver::combineEqAtIndexes(Index i, const Integer& q, Index j, const Integer& r){
  Constant cq = Constant::mkConstant(q);
  Constant cr = Constant::mkConstant(r);

  const SumPair& si = d_facts[i];
  const SumPair& sj = d_facts[j];

  SumPair newSi = (si * cq) + (sj * cr);
  d_facts.set(i, newSi);

  const Polynomial& pi = d_proofs[i];
  const Polynomial& pj = d_proofs[j];
  Polynomial newPi = (pi * cq) + (pj * cr);

  d_proofs.set(i, newPi);

  Debug("arith::dio") << "combineEqAtIndexes(" << i <<","<<q<<","<<j<<","<<r<<")"<<endl;
  Debug("arith::dio") << "derived "<< newSi.getNode()
                      <<" with proof " << newPi.getNode() << endl;
}

void DioSolver::applySubstitutionsToFront(){
  Index f = front();

  typedef context::CDList<Variable>::const_iterator EliminatedIter;
  typedef context::CDList<Index>::const_iterator RangeIter;

  Integer one(1);

  EliminatedIter ei = d_subEliminated.begin(), eiEnd = d_subEliminated.end();
  RangeIter ri = d_subRange.begin(), riEnd = d_subRange.end();
  for(; ei != eiEnd; ++ei, ++ri){
    const SumPair& curr = d_facts[f];
    Polynomial vsum = curr.getPolynomial();

    Variable var = *ei;
    Index subIndex = *ri;
    //const SumPair& sub = d_facts[subIndex];
    Constant a = vsum.getCoefficient(VarList(var));
    if(!a.isZero()){
      combineEqAtIndexes(f, one, subIndex, a.getIntegerValue());
      Assert(d_facts[f].getPolynomial().getCoefficient(VarList(var)).isZero());
    }
  }
}

void DioSolver::decomposeFront(){
  Index i = front();

  Debug("arith::dio") << "before Decompose front " <<  d_facts[i].getNode() << endl;

  const SumPair& si = d_facts[i];
  Polynomial p = si.getPolynomial();
  Monomial av = p.selectAbsMinimum();
  VarList vl = av.getVarList();
  Assert(vl.singleton());
  Variable var = vl.getHead();
  Constant a = av.getConstant();
  Integer a_abs = a.getIntegerValue().abs();

  Node freshNode = Node::null();

  Assert(a_abs >= 1);
  //It is not sufficient to reduce the case where abs(a) == 1 to abs(a) > 1.
  //We need to handle both cases seperately to ensure termination.
  if(a_abs > 1){

    Node qr = SumPair::computeQR(si, a_abs);
    Assert(qr.getKind() == kind::PLUS);
    Assert(qr.getNumChildren() == 2);
    SumPair q = SumPair::parseSumPair(qr[0]);
    SumPair r = SumPair::parseSumPair(qr[1]);

    Constant c_a_abs = Constant::mkConstant(a_abs);

    Assert(!r.isZero());
    freshNode = makeIntegerVariable();
    Variable fresh(freshNode);
    SumPair fresh_one=SumPair::mkSumPair(fresh);
    SumPair fresh_a = fresh_one * c_a_abs;

    SumPair newSI = a.isNegative() ? SumPair(fresh_one) + q: SumPair(fresh_one) - q;
    // this normalizes the coefficient of var to -1

    SumPair newFact = r + fresh_a;
    internalAppendEquality(newFact, d_proofs[i]);

    #warning "Not sure this is what we want, but following the paper"
    d_facts.set(i, newSI);
    d_proofs.set(i, Polynomial::mkZero());
  }else if(!a.isNegative()){
    Assert(a.isOne());
    scaleEqAtIndex(i, Integer(-1));
  }
  Debug("arith::dio") << "after Decompose front " <<  d_facts[i].getNode() << " for " << av.getNode() << endl;
  Assert(d_facts[i].getPolynomial().getCoefficient(vl) == Constant::mkConstant(-1));

  d_subEliminated.push_back(var);
  d_subFresh.push_back(freshNode);
  d_subRange.push_back(i);

  popQueue();
}
// /*
// static void printEquation(vector<Rational>& coeffs,
//                           vector<ArithVar>& variables,
//                           ostream& out) {
//   Assert(coeffs.size() == variables.size());
//   vector<Rational>::const_iterator i = coeffs.begin();
//   vector<ArithVar>::const_iterator j = variables.begin();
//   while(i != coeffs.end()) {
//     out << *i << "*" << *j;
//     ++i;
//     ++j;
//     if(i != coeffs.end()) {
//       out << " + ";
//     }
//   }
//   out << " = 0";
// }
// */

// bool DioSolver::solve() {
//   Trace("integers") << "DioSolver::solve()" << endl;
//   if(Debug.isOn("integers")) {
//     ScopedDebug dtab("tableau");
//     d_tableau.printTableau();
//   }
//   for(ArithVarSet::const_iterator i = d_tableau.beginBasic();
//       i != d_tableau.endBasic();
//       ++i) {
//     Debug("integers") << "going through row " << *i << endl;

//     Integer m = 1;
//     for(Tableau::RowIterator j = d_tableau.rowIterator(*i); !j.atEnd(); ++j) {
//       Debug("integers") << "  column " << (*j).getCoefficient() << " * " << (*j).getColVar() << endl;
//       m *= (*j).getCoefficient().getDenominator();
//     }
//     Assert(m >= 1);
//     Debug("integers") << "final multiplier is " << m << endl;

//     Integer gcd = 0;
//     Rational c = 0;
//     Debug("integers") << "going throw row " << *i << " to find gcd" << endl;
//     for(Tableau::RowIterator j = d_tableau.rowIterator(*i); !j.atEnd(); ++j) {
//       Rational r = (*j).getCoefficient();
//       ArithVar v = (*j).getColVar();
//       r *= m;
//       Debug("integers") << "  column " << r << " * " << v << endl;
//       Assert(r.getDenominator() == 1);
//       bool foundConstant = false;
//       // The tableau doesn't store constants.  Constants int he input
//       // are mapped to slack variables that are constrained with
//       // bounds in the partial model.  So we detect and accumulate all
//       // variables whose upper bound equals their lower bound, which
//       // catches these input constants as well as any (contextually)
//       // eliminated variables.
//       if(d_partialModel.hasUpperBound(v) && d_partialModel.hasLowerBound(v)) {
//         DeltaRational b = d_partialModel.getLowerBound(v);
//         if(b.getInfinitesimalPart() == 0 && b == d_partialModel.getUpperBound(v)) {
//           r *= b.getNoninfinitesimalPart();
//           Debug("integers") << "    var " << v << " == [" << b << "], so found a constant part " << r << endl;
//           c += r;
//           foundConstant = true;
//         }
//       }
//       if(!foundConstant) {
//         // calculate gcd of all (NON-constant) coefficients
//         gcd = (gcd == 0) ? r.getNumerator().abs() : gcd.gcd(r.getNumerator());
//         Debug("integers") << "    gcd now " << gcd << endl;
//       }
//     }
//     Debug("integers") << "addEquation: gcd is " << gcd << ", c is " << c << endl;
//     // The gcd must divide c for this equation to be satisfiable.
//     // If c is not an integer, there's no way it can.
//     if(c.getDenominator() == 1 && gcd == gcd.gcd(c.getNumerator())) {
//       Trace("integers") << "addEquation: this eqn is fine" << endl;
//     } else {
//       Trace("integers") << "addEquation: eqn not satisfiable, returning false" << endl;
//       return false;
//     }
//   }

//   return true;
// }

void DioSolver::getSolution() {
  Unimplemented();
}

}/* CVC4::theory::arith namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
