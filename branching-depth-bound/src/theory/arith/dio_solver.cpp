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

DioSolver::DioSolver(context::Context* ctxt, std::queue<Node>& substitutions, Lookup<uint32_t>& cuttingDepth, RequestNodeCallback& requestNewIntegerVariable) :
  d_inputConstraints(ctxt),
  d_nextInputConstraintToEnqueue(ctxt, 0),
  d_trail(ctxt),
  d_subs(ctxt),
  d_currentF(),
  d_savedQueue(ctxt),
  d_conflictHasBeenRaised(ctxt, false),
  d_maxInputCoefficientLength(ctxt, 0),
  d_usedDecomposeIndex(ctxt, false),
  d_substitutionStream(substitutions),
  d_cuttingDepth(cuttingDepth),
  d_requestNewVariable(requestNewIntegerVariable)
{}

DioSolver::Statistics::Statistics() :
  d_conflictCalls("theory::arith::dio::conflictCalls",0),
  d_cutCalls("theory::arith::dio::cutCalls",0),
  d_cuts("theory::arith::dio::cuts",0),
  d_conflicts("theory::arith::dio::conflicts",0),
  d_conflictTimer("theory::arith::dio::conflictTimer"),
  d_cutTimer("theory::arith::dio::cutTimer")
{
  StatisticsRegistry::registerStat(&d_conflictCalls);
  StatisticsRegistry::registerStat(&d_cutCalls);

  StatisticsRegistry::registerStat(&d_cuts);
  StatisticsRegistry::registerStat(&d_conflicts);

  StatisticsRegistry::registerStat(&d_conflictTimer);
  StatisticsRegistry::registerStat(&d_cutTimer);
}

DioSolver::Statistics::~Statistics(){
  StatisticsRegistry::unregisterStat(&d_conflictCalls);
  StatisticsRegistry::unregisterStat(&d_cutCalls);

  StatisticsRegistry::unregisterStat(&d_cuts);
  StatisticsRegistry::unregisterStat(&d_conflicts);

  StatisticsRegistry::unregisterStat(&d_conflictTimer);
  StatisticsRegistry::unregisterStat(&d_cutTimer);
}


Node DioSolver::nextSubstitution(){
  Unreachable();
  //This is currently unused!

  Assert(hasMoreSubstitutions());
  SubIndex curr = d_subs.frontIndex();
  d_subs.dequeue();

  Assert(d_subs[curr].d_fresh.isNull());
  Variable v = d_subs[curr].d_eliminated;

  SumPair sp = d_trail[d_subs[curr].d_constraint].d_eq;
  SumPair polyV = SumPair(Polynomial::mkPolynomial(v));
  SumPair cancelV = polyV + sp;

  Node eq = (v.getNode()).eqNode(cancelV.getNode());
  return eq;
}


bool DioSolver::debugEqualityInInputEquations(Node eq){
  typedef context::CDList<InputConstraint>::const_iterator const_iterator;
  const_iterator i=d_inputConstraints.begin(), end = d_inputConstraints.end();
  for(; i != end; ++i){
    Node reason_i = (*i).d_reason;
    if(eq == reason_i){
      return true;
    }
  }
  return false;
}

bool DioSolver::acceptableOriginalNodes(Node n){
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

void DioSolver::pushInputConstraint(const Comparison& eq, Node reason){
  Assert(!debugEqualityInInputEquations(reason));
  Assert(eq.isIntegral());
  Assert(eq.getNode().getKind() == kind::EQUAL);
  Assert(acceptableOriginalNodes(reason));

  SumPair sp = SumPair::comparisonToSumPair(eq);
  uint32_t length = sp.maxLength();
  if(length > d_maxInputCoefficientLength){
    d_maxInputCoefficientLength = length;
  }

  Variable proofVariable(d_requestNewVariable.request());

  TrailIndex posInTrail = d_trail.size();
  d_trail.push_back(Constraint(sp,Polynomial(Monomial(VarList(proofVariable)))));

  size_t posInConstraintList = d_inputConstraints.size();
  d_inputConstraints.push_back(InputConstraint(reason, posInTrail));

  d_varToInputConstraintMap[proofVariable.getNode()] = posInConstraintList;
}


DioSolver::TrailIndex DioSolver::scaleEqAtIndex(DioSolver::TrailIndex i, const Integer& g){
  Assert(g != 0);
  Constant invg = Constant::mkConstant(Rational(Integer(1),g));
  const SumPair& sp = d_trail[i].d_eq;
  const Polynomial& proof = d_trail[i].d_proof;

  SumPair newSP = sp * invg;
  Polynomial newProof = proof * invg;

  Assert(newSP.isIntegral());
  Assert(newSP.gcd() == 1);

  TrailIndex j = d_trail.size();

  d_trail.push_back(Constraint(newSP, newProof));

  Debug("arith::dio") << "scaleEqAtIndex(" << i <<","<<g<<")"<<endl;
  Debug("arith::dio") << "derived "<< newSP.getNode()
                      <<" with proof " << newProof.getNode() << endl;
  return j;
}

Node DioSolver::proveIndex(TrailIndex i){
  Assert(inRange(i));
  const Polynomial& proof = d_trail[i].d_proof;
  Assert(!proof.isConstant());

  NodeBuilder<> nb(kind::AND);
  for(Polynomial::iterator iter = proof.begin(), end = proof.end(); iter!= end; ++iter){
    Monomial m = (*iter);
    Assert(!m.isConstant());
    VarList vl = m.getVarList();
    Assert(vl.singleton());
    Variable v = vl.getHead();

    Node input = proofVariableToReason(v);
    Assert(acceptableOriginalNodes(input));
    if(input.getKind() == kind::AND){
      nb << input[0] << input[1];
    }else{
      nb << input;
    }
  }

  Node result = (nb.getNumChildren() == 1) ? nb[0] : (Node)nb;
  Debug("arith::dio") << "Proof at " << i << " is "
                      << d_trail[i].d_eq.getNode() << endl
                      << d_trail[i].d_proof.getNode() << endl
                      << " which becomes " << result << endl;
  return result;
}

/** This never returns true if the number of monomials is 1. */
bool DioSolver::anyCoefficientExceedsMaximum(TrailIndex j) const{
  uint32_t length = d_trail[j].d_eq.maxLength();
  uint32_t nmonos = d_trail[j].d_eq.getPolynomial().numMonomials();

  uint32_t currentMax = d_maxInputCoefficientLength
    + MAX_GROWTH_RATE
    + d_cuttingDepth.lookup();

  bool result = nmonos >= 2 && length > currentMax;
  if(Debug.isOn("arith::dio::max") && result){
    Debug("arith::dio::max") << "about to drop:";
    debugPrintTrail(j);
  }
  return result;
}

void DioSolver::enqueueInputConstraints(){
  Assert(d_currentF.empty());
  while(!d_savedQueue.empty()){
    d_currentF.push_back(d_savedQueue.front());
    d_savedQueue.pop();
  }

  while(d_nextInputConstraintToEnqueue < d_inputConstraints.size()  && !inConflict()){
    size_t curr = d_nextInputConstraintToEnqueue;
    d_nextInputConstraintToEnqueue = d_nextInputConstraintToEnqueue + 1;

    TrailIndex i = d_inputConstraints[curr].d_trailPos;
    TrailIndex j = applyAllSubstitutionsToIndex(i);

    if(!triviallySat(j)){
      if(triviallyUnsat(j)){
        raiseConflict(j);
      }else{
        TrailIndex k = reduceByGCD(j);

        if(!inConflict()){
          if(triviallyUnsat(k)){
            raiseConflict(k);
          }else if(!triviallySat(k)){
            if(anyCoefficientExceedsMaximum(k)){
              d_savedQueue.push(k);
            }else{
              pushToQueueBack(k);
            }
          }
        }
      }
    }
  }
}


/*TODO Currently linear in the size of the Queue
 *It is not clear if am O(log n) strategy would be better.
 *Right before this in the algorithm is a substition which could potentially
 *effect the key values of everything in the queue.
 */
void DioSolver::moveMinimumByAbsToQueueFront(){
  Assert(!queueEmpty());


  //Select the minimum element.
  size_t indexInQueue = 0;
  Monomial minMonomial = d_trail[d_currentF[indexInQueue]].d_minimalMonomial;

  size_t N = d_currentF.size();
  for(size_t i=1; i < N; ++i){
    Monomial curr = d_trail[d_currentF[i]].d_minimalMonomial;
    if(curr.absLessThan(minMonomial)){
      indexInQueue = i;
      minMonomial = curr;
    }
  }

  TrailIndex tmp = d_currentF[indexInQueue];
  d_currentF[indexInQueue] = d_currentF.front();
  d_currentF.front() = tmp;
}

bool DioSolver::queueEmpty() const{
  return d_currentF.empty();
}

Node DioSolver::columnGcdIsOne() const{
  std::hash_map<Node, Integer, NodeHashFunction> gcdMap;

  std::deque<TrailIndex>::const_iterator iter, end;
  for(iter = d_currentF.begin(), end = d_currentF.end(); iter != end; ++iter){
    TrailIndex curr = *iter;
    Polynomial p = d_trail[curr].d_eq.getPolynomial();
    Polynomial::iterator monoIter = p.begin(), monoEnd = p.end();
    for(; monoIter != monoEnd; ++monoIter){
      Monomial m = *monoIter;
      VarList vl = m.getVarList();
      Node vlNode = vl.getNode();

      Constant c = m.getConstant();
      Integer zc = c.getValue().getNumerator();
      if(gcdMap.find(vlNode) == gcdMap.end()){
        // have not seen vl yet.
        // gcd is c
        Assert(c.isIntegral());
        Assert(!m.absCoefficientIsOne());
        gcdMap.insert(make_pair(vlNode, zc.abs()));
      }else{
        const Integer& currentGcd = gcdMap[vlNode];
        Integer newGcd = currentGcd.gcd(zc);
        if(newGcd == 1){
          return vlNode;
        }else{
          gcdMap[vlNode] = newGcd;
        }
      }
    }
  }
  return Node::null();
}

void DioSolver::saveQueue(){
  std::deque<TrailIndex>::const_iterator iter, end;
  for(iter = d_currentF.begin(), end = d_currentF.end(); iter != end; ++iter){
    d_savedQueue.push_back(*iter);
  }
}

DioSolver::TrailIndex DioSolver::impliedGcdOfOne(){
  Node canReduce = columnGcdIsOne();
  if(canReduce.isNull()){
    return 0;
  }else{
    VarList vl = VarList::parseVarList(canReduce);

    TrailIndex current;
    Integer currentCoeff, currentGcd;

    //step 1 find the first equation containing vl
    //Set current and currentCoefficient
    std::deque<TrailIndex>::const_iterator iter, end;
    for(iter = d_currentF.begin(), end = d_currentF.end(); true; ++iter){
      Assert(iter != end);
      current = *iter;
      Constant coeff = d_trail[current].d_eq.getPolynomial().getCoefficient(vl);
      if(!coeff.isZero()){
        currentCoeff = coeff.getValue().getNumerator();
        currentGcd = currentCoeff.abs();

        ++iter;
        break;
      }
    }

    //For the rest of the equations keep reducing until the coefficient is one
    for(; iter != end; ++iter){
      TrailIndex inQueue = *iter;
      Constant iqc = d_trail[inQueue].d_eq.getPolynomial().getCoefficient(vl);
      if(!iqc.isZero()){
        Integer inQueueCoeff = iqc.getValue().getNumerator();

        //mpz_gcdext (mpz_t g, mpz_t s, mpz_t t, mpz_t a, mpz_t b);
        Integer g, s, t;
        // g = a*s + b*t
        Integer::extendedGcd(g, s, t, currentCoeff, inQueueCoeff);

        Debug("arith::dio") << "extendedReduction" << endl;
        Debug("arith::dio") << g << " = " << s <<"*"<< currentCoeff << " + " << t <<"*"<< inQueueCoeff << endl;
        
        Assert(g <= currentGcd);
        if(g < currentGcd){
          if(s.sgn() == 0){
            Debug("arith::dio") << "extendedReduction drop" << endl;
            Assert(inQueueCoeff.divides(currentGcd));
            current = *iter;
            currentCoeff = inQueueCoeff;
            currentGcd = inQueueCoeff;
          }else{
            Debug("arith::dio") << "extendedReduction combine" << endl;
          
            TrailIndex next = combineEqAtIndexes(current, s, inQueue, t);

            Assert(d_trail[next].d_eq.getPolynomial().getCoefficient(vl).getValue().getNumerator() == g);

            current = next;
            currentCoeff = g;
            currentGcd = g;
            if(currentGcd == 1){
              return current;
            }
          }
        }
      }
    }
    //This is not reachble as it is assured that the gcd of the column is 1
    Unreachable();
  }
}

bool DioSolver::processEquations(bool allowDecomposition){
  Assert(!inConflict());

  enqueueInputConstraints();
  while(! queueEmpty() && !inConflict()){
    moveMinimumByAbsToQueueFront();

    TrailIndex minimum = d_currentF.front();
    TrailIndex reduceIndex;

    Assert(inRange(minimum));
    Assert(!inConflict());

    Debug("arith::dio") << "processEquations " << minimum << " : " << d_trail[minimum].d_eq.getNode() << endl;

    Assert(queueConditions(minimum));

    bool canDirectlySolve = d_trail[minimum].d_minimalMonomial.absCoefficientIsOne();

    std::pair<SubIndex, TrailIndex> p;
    if(canDirectlySolve){
      d_currentF.pop_front();
      p = solveIndex(minimum);
      reduceIndex = minimum;
    }else{
      TrailIndex implied = impliedGcdOfOne();

      if(implied != 0){
        p = solveIndex(implied);
        reduceIndex = implied;
      }else if(allowDecomposition){
        d_currentF.pop_front();
        p = decomposeIndex(minimum);
        reduceIndex = minimum;
      }else {
        // Cannot make progress without decomposeIndex
        saveQueue();
        break;
      }
    }

    SubIndex subIndex = p.first;
    TrailIndex next = p.second;
    subAndReduceCurrentFByIndex(subIndex);

    if(next != reduceIndex){
      if(triviallyUnsat(next)){
        raiseConflict(next);
      }else if(! triviallySat(next) ){
        pushToQueueBack(next);
      }
    }
  }

  d_currentF.clear();
  return inConflict();
}

Node DioSolver::processEquationsForConflict(){
  TimerStat::CodeTimer codeTimer(d_statistics.d_conflictTimer);
  ++(d_statistics.d_conflictCalls);

  Assert(!inConflict());
  if(processEquations(true)){
    ++(d_statistics.d_conflicts);
    return proveIndex(getConflictIndex());
  }else{
    enqueueSubstitutions();
    return Node::null();
  }
}

SumPair DioSolver::processEquationsForCut(){
  TimerStat::CodeTimer codeTimer(d_statistics.d_cutTimer);
  ++(d_statistics.d_cutCalls);

  Assert(!inConflict());
  if(processEquations(true)){
    enqueueSubstitutions();
    ++(d_statistics.d_cuts);
    return d_trail[getConflictIndex()].d_eq;
    //return purifyIndex(getConflictIndex());
  }else{
    enqueueSubstitutions();
    return SumPair::mkZero();
  }
}


// SumPair DioSolver::purifyIndex(TrailIndex i){
// #warning "This uses the substition trail to reverse the substitions from the sum term. Using the proof term should be more efficient."

//   SumPair curr = d_trail[i].d_eq;

//   Constant negOne = Constant::mkConstant(-1);

//   for(uint32_t revIter = d_subs.size(); revIter > 0; --revIter){
//     uint32_t i = revIter - 1;
//     Node freshNode = d_subs[i].d_fresh;
//     if(freshNode.isNull()){
//       continue;
//     }else{
//       Variable var(freshNode);
//       Polynomial vsum = curr.getPolynomial();

//       Constant a = vsum.getCoefficient(VarList(var));
//       if(!a.isZero()){
//         const SumPair& sj = d_trail[d_subs[i].d_constraint].d_eq;
//         Assert(sj.getPolynomial().getCoefficient(VarList(var)).isOne());
//         SumPair newSi = (curr * negOne) + (sj * a);
//         Assert(newSi.getPolynomial().getCoefficient(VarList(var)).isZero());
//         curr = newSi;
//       }
//     }
//   }
//   return curr;
// }

DioSolver::TrailIndex DioSolver::combineEqAtIndexes(DioSolver::TrailIndex i, const Integer& q, DioSolver::TrailIndex j, const Integer& r){
  Constant cq = Constant::mkConstant(q);
  Constant cr = Constant::mkConstant(r);

  const SumPair& si = d_trail[i].d_eq;
  const SumPair& sj = d_trail[j].d_eq;

  Debug("arith::dio") << "combineEqAtIndexes(" << i <<","<<q<<","<<j<<","<<r<<")"<<endl;
  Debug("arith::dio") << "d_facts[i] = " << si.getNode() << endl
                      << "d_facts[j] = " << sj.getNode() << endl;


  SumPair newSi = (si * cq) + (sj * cr);


  const Polynomial& pi = d_trail[i].d_proof;
  const Polynomial& pj = d_trail[j].d_proof;
  Polynomial newPi = (pi * cq) + (pj * cr);

  TrailIndex k = d_trail.size();
  d_trail.push_back(Constraint(newSi, newPi));


  Debug("arith::dio") << "derived "<< newSi.getNode()
                      <<" with proof " << newPi.getNode() << endl;

  return k;

}

void DioSolver::printQueue(){
  Debug("arith::dio") << "DioSolver::printQueue()" << endl;
  for(TrailIndex i = 0, last = d_trail.size(); i < last; ++i){
    Debug("arith::dio") << "d_trail[i].d_eq = " << d_trail[i].d_eq.getNode() << endl;
    Debug("arith::dio") << "d_trail[i].d_proof = " << d_trail[i].d_proof.getNode() << endl;
  }

  Debug("arith::dio") << "DioSolver::printSubs()" << endl;
  for(SubIndex si=0, sN=d_subs.size(); si < sN; ++si){
    Debug("arith::dio") << "d_subs[i] = {"
                        << "d_fresh="<< d_subs[si].d_fresh <<","
                        << "d_eliminated="<< d_subs[si].d_eliminated.getNode() <<","
                        << "d_constraint="<< d_subs[si].d_constraint <<"}" << endl;
    Debug("arith::dio") << "d_trail[d_subs[i].d_constraint].d_eq="
                        << d_trail[d_subs[si].d_constraint].d_eq.getNode() << endl;
  }
}

DioSolver::TrailIndex DioSolver::applyAllSubstitutionsToIndex(DioSolver::TrailIndex trailIndex){
  TrailIndex currentIndex = trailIndex;
  for(SubIndex subIter = 0, siEnd = d_subs.size(); subIter < siEnd; ++subIter){
    currentIndex = applySubstitution(subIter, currentIndex);
  }
  return currentIndex;
}

bool DioSolver::debugSubstitutionApplies(DioSolver::SubIndex si, DioSolver::TrailIndex ti){
  Variable var = d_subs[si].d_eliminated;

  const SumPair& curr = d_trail[ti].d_eq;
  Polynomial vsum = curr.getPolynomial();

  Constant a = vsum.getCoefficient(VarList(var));
  return !a.isZero();
}

bool DioSolver::debugAnySubstitionApplies(DioSolver::TrailIndex i){
  for(SubIndex subIter = 0, siEnd = d_subs.size(); subIter < siEnd; ++subIter){
    if(debugSubstitutionApplies(subIter, i)){
      return true;
    }
  }
  return false;
}

std::pair<DioSolver::SubIndex, DioSolver::TrailIndex> DioSolver::solveIndex(DioSolver::TrailIndex i){
  const SumPair& si = d_trail[i].d_eq;

  Debug("arith::dio") << "before solveIndex("<<i<<":"<<si.getNode()<< ")" << endl;

  const Polynomial& p = si.getPolynomial();
  Assert(p.isIntegral());

  Assert(p.selectAbsMinimum() == d_trail[i].d_minimalMonomial);
  const Monomial& av = d_trail[i].d_minimalMonomial;

  VarList vl = av.getVarList();
  Assert(vl.singleton());
  Variable var = vl.getHead();
  Constant a = av.getConstant();
  Integer a_abs = a.getValue().getNumerator().abs();

  Assert(a_abs == 1);

  TrailIndex ci = !a.isNegative() ? scaleEqAtIndex(i, Integer(-1)) : i;

  SubIndex subBy = d_subs.size();
  d_subs.enqueue(Substitution(Node::null(), var, ci));

  Debug("arith::dio") << "after solveIndex " <<  d_trail[ci].d_eq.getNode() << " for " << av.getNode() << endl;
  Assert(d_trail[ci].d_eq.getPolynomial().getCoefficient(vl) == Constant::mkConstant(-1));

  return make_pair(subBy, i);
}

std::pair<DioSolver::SubIndex, DioSolver::TrailIndex> DioSolver::decomposeIndex(DioSolver::TrailIndex i){
  const SumPair& si = d_trail[i].d_eq;

  d_usedDecomposeIndex = true;

  Debug("arith::dio") << "before decomposeIndex("<<i<<":"<<si.getNode()<< ")" << endl;

  const Polynomial& p = si.getPolynomial();
  Assert(p.isIntegral());

  Assert(p.selectAbsMinimum() == d_trail[i].d_minimalMonomial);
  const Monomial& av = d_trail[i].d_minimalMonomial;

  VarList vl = av.getVarList();
  Assert(vl.singleton());
  Variable var = vl.getHead();
  Constant a = av.getConstant();
  Integer a_abs = a.getValue().getNumerator().abs();

  Assert(a_abs > 1);
  
  //It is not sufficient to reduce the case where abs(a) == 1 to abs(a) > 1.
  //We need to handle both cases seperately to ensure termination.
  Node qr = SumPair::computeQR(si, a.getValue().getNumerator());

  Assert(qr.getKind() == kind::PLUS);
  Assert(qr.getNumChildren() == 2);
  SumPair q = SumPair::parseSumPair(qr[0]);
  SumPair r = SumPair::parseSumPair(qr[1]);

  Assert(q.getPolynomial().getCoefficient(vl) == Constant::mkConstant(1));

  Assert(!r.isZero());
  Node freshNode = d_requestNewVariable.request();
  Variable fresh(freshNode);
  SumPair fresh_one=SumPair::mkSumPair(fresh);
  SumPair fresh_a = fresh_one * a;

  SumPair newSI = SumPair(fresh_one) - q;
  // this normalizes the coefficient of var to -1


  TrailIndex ci = d_trail.size();
  d_trail.push_back(Constraint(newSI, Polynomial::mkZero()));

  Debug("arith::dio") << "Decompose ci(" << ci <<":" <<  d_trail[ci].d_eq.getNode()
                      << ") for " << av.getNode() << endl;
  Assert(d_trail[ci].d_eq.getPolynomial().getCoefficient(vl) == Constant::mkConstant(-1));

  SumPair newFact = r + fresh_a;

  TrailIndex nextIndex = d_trail.size();
  d_trail.push_back(Constraint(newFact, d_trail[i].d_proof));

  SubIndex subBy = d_subs.size();
  d_subs.enqueue(Substitution(freshNode, var, ci));

  Debug("arith::dio") << "Decompose nextIndex " <<  d_trail[nextIndex].d_eq.getNode() << endl;
  return make_pair(subBy, nextIndex);
}


DioSolver::TrailIndex DioSolver::applySubstitution(DioSolver::SubIndex si, DioSolver::TrailIndex ti){
  Variable var = d_subs[si].d_eliminated;
  TrailIndex subIndex = d_subs[si].d_constraint;

  const SumPair& curr = d_trail[ti].d_eq;
  Polynomial vsum = curr.getPolynomial();

  Constant a = vsum.getCoefficient(VarList(var));
  Assert(a.isIntegral());
  if(!a.isZero()){
    Integer one(1);
    TrailIndex afterSub = combineEqAtIndexes(ti, one, subIndex, a.getValue().getNumerator());
    Assert(d_trail[afterSub].d_eq.getPolynomial().getCoefficient(VarList(var)).isZero());
    return afterSub;
  }else{
    return ti;
  }
}


DioSolver::TrailIndex DioSolver::reduceByGCD(DioSolver::TrailIndex ti){
  const SumPair& sp = d_trail[ti].d_eq;
  Polynomial vsum = sp.getPolynomial();
  Constant c = sp.getConstant();

  Debug("arith::dio") << "reduceByGCD " << vsum.getNode() << endl;
  Assert(!vsum.isConstant());
  Integer g = vsum.gcd();
  Assert(g >= 1);
  Debug("arith::dio") << "gcd("<< vsum.getNode() <<")=" << g << " " << c.getValue() << endl;
  if(g.divides(c.getValue().getNumerator())){
    if(g > 1){
      return scaleEqAtIndex(ti, g);
    }else{
      return ti;
    }
  }else{
    raiseConflict(ti);
    return ti;
  }
}

bool DioSolver::triviallySat(TrailIndex i){
  const SumPair& eq = d_trail[i].d_eq;
  if(eq.isConstant()){
    return eq.getConstant().isZero();
  }else{
    return false;
  }
}

bool DioSolver::triviallyUnsat(DioSolver::TrailIndex i){
  const SumPair& eq = d_trail[i].d_eq;
  if(eq.isConstant()){
    return !eq.getConstant().isZero();
  }else{
    return false;
  }
}


bool DioSolver::gcdIsOne(DioSolver::TrailIndex i){
  const SumPair& eq = d_trail[i].d_eq;
  return eq.gcd() == Integer(1);
}

void DioSolver::debugPrintTrail(DioSolver::TrailIndex i) const{
  const SumPair& eq = d_trail[i].d_eq;
  const Polynomial& proof = d_trail[i].d_proof;

  cout << "d_trail["<<i<<"].d_eq = " << eq.getNode() << endl;
  cout << "d_trail["<<i<<"].d_proof = " << proof.getNode() << endl;
}

void DioSolver::subAndReduceCurrentFByIndex(DioSolver::SubIndex subIndex){
  size_t N = d_currentF.size();

  size_t readIter = 0, writeIter = 0;
  for(; readIter < N && !inConflict(); ++readIter){
    TrailIndex curr = d_currentF[readIter];
    TrailIndex nextTI = applySubstitution(subIndex, curr);
    if(nextTI == curr){
      d_currentF[writeIter] = curr;
      ++writeIter;
    }else{
      Assert(nextTI != curr);

      if(triviallyUnsat(nextTI)){
        raiseConflict(nextTI);
      }else if(!triviallySat(nextTI)){
        TrailIndex nextNextTI = reduceByGCD(nextTI);

        if(!inConflict()){
          if(anyCoefficientExceedsMaximum(nextNextTI)){
            d_savedQueue.push(nextNextTI);
          }else{
            Assert(queueConditions(nextNextTI));
            d_currentF[writeIter] = nextNextTI;
            ++writeIter;
          }
        }
      }
    }
  }
  if(!inConflict() && writeIter < N){
    d_currentF.resize(writeIter);
  }
}

}/* CVC4::theory::arith namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */