/*********************                                                        */
/*! \file theory_bv.cpp
 ** \verbatim
 ** Original author: dejan
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

#include "theory/bv/theory_bv.h"
#include "theory/bv/theory_bv_utils.h"
#include "theory/valuation.h"
#include "theory/bv/bv_sat.h"
#include "theory/uf/equality_engine_impl.h"

using namespace CVC4;
using namespace CVC4::theory;
using namespace CVC4::theory::bv;
using namespace CVC4::context;

using namespace std;
using namespace CVC4::theory::bv::utils;

TheoryBV::TheoryBV(context::Context* c, context::UserContext* u, OutputChannel& out, Valuation valuation)
  : Theory(THEORY_BV, c, u, out, valuation), 
    d_context(c),
    d_assertions(c),
    d_bitblaster(new Bitblaster(c) ),
    d_statistics(),
    d_notify(*this),
    d_equalityEngine(d_notify, c, "theory::bv::TheoryBV"),
    d_conflict(c, false),
    d_literalsToPropagate(c),
    d_literalsToPropagateIndex(c, 0),
    d_toBitBlast(c)
  {
    d_true = utils::mkTrue();
    d_false = utils::mkFalse();

    d_equalityEngine.addTerm(d_true);
    d_equalityEngine.addTerm(d_false);
    d_equalityEngine.addTriggerEquality(d_true, d_false, d_false);

    // The kinds we are treating as function application in congruence
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_CONCAT);
  }

TheoryBV::~TheoryBV() {
  delete d_bitblaster; 
}
TheoryBV::Statistics::Statistics():
  d_avgConflictSize("theory::bv::AvgBVConflictSize"),
  d_solveSubstitutions("theory::bv::NumberOfSolveSubstitutions", 0),
  d_solveTimer("theory::bv::solveTimer")
{
  StatisticsRegistry::registerStat(&d_avgConflictSize);
  StatisticsRegistry::registerStat(&d_solveSubstitutions);
  StatisticsRegistry::registerStat(&d_solveTimer); 
}

TheoryBV::Statistics::~Statistics() {
  StatisticsRegistry::unregisterStat(&d_avgConflictSize);
  StatisticsRegistry::unregisterStat(&d_solveSubstitutions);
  StatisticsRegistry::unregisterStat(&d_solveTimer); 
}

void TheoryBV::preRegisterTerm(TNode node) {
  BVDebug("bitvector-preregister") << "TheoryBV::preRegister(" << node << ")" << std::endl;
  //marker literal: bitblast all terms before we start
  //d_bitblaster->bitblast(node); 

  switch (node.getKind()) {
  case kind::EQUAL:
    // Add the terms
    d_equalityEngine.addTerm(node);
    // Add the trigger for equality
    d_equalityEngine.addTriggerEquality(node[0], node[1], node);
    d_equalityEngine.addTriggerDisequality(node[0], node[1], node.notNode());
    break;
  default:
    d_equalityEngine.addTerm(node);
    break;
  }

}

void TheoryBV::check(Effort e) {

  while (!done() && !d_conflict) 
  {
    // Get all the assertions
    Assertion assertion = get();
    TNode fact = assertion.assertion;
    d_toBitBlast.push(fact);

    Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::check(): processing " << fact << std::endl;

    // If the assertion doesn't have a literal, it's a shared equality, so we set it up
    if (!assertion.isPreregistered) {
      Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::check(): shared equality, setting up " << fact << std::endl;
      if (fact.getKind() == kind::NOT) {
        if (!d_equalityEngine.hasTerm(fact[0])) {
          preRegisterTerm(fact[0]);
        }
      } else if (!d_equalityEngine.hasTerm(fact)) {
        preRegisterTerm(fact);
      }
    }

    // Do the work
    switch (fact.getKind()) {
      case kind::EQUAL:
        d_equalityEngine.addEquality(fact[0], fact[1], fact);
        break;
      case kind::NOT:
        if (fact[0].getKind() == kind::EQUAL) {
          // Assert the dis-equality
          d_equalityEngine.addDisequality(fact[0][0], fact[0][1], fact);
        } else {
          //TODO: this causes an assertion failure due to building an equality between Booleans
          // disable for now
          //d_equalityEngine.addEquality(fact[0], d_false, fact);
          break;
        }
        break;
      default:
        break;
    }
  }

  // If in conflict, output the conflict
  if (d_conflict) {
    Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::check(): conflict " << d_conflictNode << std::endl;
    d_out->conflict(d_conflictNode);
    return;
  }

  if (fullEffort(e) && !d_toBitBlast.empty()) {
    Trace("bitvector")<< "TheoryBV::check(" << e << ")" << std::endl;
    std::vector<TNode> assertions;
      
    do {
      TNode assertion = d_toBitBlast.front();
      d_toBitBlast.pop();
      Trace("bitvector-assertions") << "TheoryBV::check assertion " << assertion << "\n"; 
      d_bitblaster->bitblast(assertion); 
      d_bitblaster->assertToSat(assertion); 
    } while (!d_toBitBlast.empty());

    TimerStat::CodeTimer codeTimer(d_statistics.d_solveTimer); 
    bool res = d_bitblaster->solve();
    if (res == false) {
      std::vector<TNode> conflictAtoms;
      d_bitblaster->getConflict(conflictAtoms);
      d_statistics.d_avgConflictSize.addEntry(conflictAtoms.size());
      
      Node conflict = mkConjunction(conflictAtoms);
      d_out->conflict(conflict);
      Trace("bitvector") << "TheoryBV::check returns conflict. \n ";
      return; 
    }
  }
}


Node TheoryBV::getValue(TNode n) {
  //NodeManager* nodeManager = NodeManager::currentNM();

  switch(n.getKind()) {

  case kind::VARIABLE:
    Unhandled(kind::VARIABLE);

  case kind::EQUAL: // 2 args
    Unhandled(kind::VARIABLE);

  default:
    Unhandled(n.getKind());
  }
}

// Node TheoryBV::preprocessTerm(TNode term) {
//   return term; //d_staticEqManager.find(term);
// }

Theory::PPAssertStatus TheoryBV::ppAssert(TNode in, SubstitutionMap& outSubstitutions) {
  switch(in.getKind()) {
  case kind::EQUAL:
    
    if (in[0].getMetaKind() == kind::metakind::VARIABLE && !in[1].hasSubterm(in[0])) {
      ++(d_statistics.d_solveSubstitutions); 
      outSubstitutions.addSubstitution(in[0], in[1]);
      return PP_ASSERT_STATUS_SOLVED;
    }
    if (in[1].getMetaKind() == kind::metakind::VARIABLE && !in[0].hasSubterm(in[1])) {
      ++(d_statistics.d_solveSubstitutions); 
      outSubstitutions.addSubstitution(in[1], in[0]);
      return PP_ASSERT_STATUS_SOLVED;
    }
    // to do constant propagations

    break;
  case kind::NOT:
    break;
  default:
    // TODO other predicates
    break;
  }
  return PP_ASSERT_STATUS_UNSOLVED;
}


bool TheoryBV::propagate(TNode literal)
{
  Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::propagate(" << literal  << ")" << std::endl;

  // If already in conflict, no more propagation
  if (d_conflict) {
    Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::propagate(" << literal << "): already in conflict" << std::endl;
    return false;
  }

  // See if the literal has been asserted already
  Node normalized = Rewriter::rewrite(literal);
  bool satValue = false;
  bool isAsserted = normalized == d_false || d_valuation.hasSatValue(normalized, satValue);

  // If asserted, we might be in conflict
  if (isAsserted) {
    if (!satValue) {
      Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::propagate(" << literal << ", normalized = " << normalized << ") => conflict" << std::endl;
      std::vector<TNode> assumptions;
      Node negatedLiteral;
      if (normalized != d_false) {
        negatedLiteral = normalized.getKind() == kind::NOT ? (Node) normalized[0] : normalized.notNode();
        assumptions.push_back(negatedLiteral);
      }
      explain(literal, assumptions);
      d_conflictNode = mkAnd(assumptions);
      d_conflict = true;
      return false;
    }
    // Propagate even if already known in SAT - could be a new equation between shared terms
    // (terms that weren't shared when the literal was asserted!)
  }

  // Nothing, just enqueue it for propagation and mark it as asserted already
  Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::propagate(" << literal << ") => enqueuing for propagation" << std::endl;
  d_literalsToPropagate.push_back(literal);

  return true;
}/* TheoryBV::propagate(TNode) */


void TheoryBV::propagate(Effort e)
{
  Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::propagate()" << std::endl;
  if (!d_conflict) {
    for (; d_literalsToPropagateIndex < d_literalsToPropagate.size(); d_literalsToPropagateIndex = d_literalsToPropagateIndex + 1) {
      TNode literal = d_literalsToPropagate[d_literalsToPropagateIndex];
      Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::propagate(): propagating " << literal << std::endl;
      bool satValue;
      Node normalized = Rewriter::rewrite(literal);
      if (!d_valuation.hasSatValue(normalized, satValue) || satValue) {
        d_out->propagate(literal);
      } else {
        Debug("bitvector") << spaces(getContext()->getLevel()) << "TheoryBV::propagate(): in conflict, normalized = " << normalized << std::endl;
        Node negatedLiteral;
        std::vector<TNode> assumptions;
        if (normalized != d_false) {
          negatedLiteral = normalized.getKind() == kind::NOT ? (Node) normalized[0] : normalized.notNode();
          assumptions.push_back(negatedLiteral);
        }
        explain(literal, assumptions);
        d_conflictNode = mkAnd(assumptions);
        d_conflict = true;
        break;
      }
    }
  }
}


void TheoryBV::explain(TNode literal, std::vector<TNode>& assumptions) {
  TNode lhs, rhs;
  switch (literal.getKind()) {
    case kind::EQUAL:
      lhs = literal[0];
      rhs = literal[1];
      break;
    case kind::NOT:
      if (literal[0].getKind() == kind::EQUAL) {
        // Disequalities
        d_equalityEngine.explainDisequality(literal[0][0], literal[0][1], assumptions);
        return;
      } else {
        // Predicates
        lhs = literal[0];
        rhs = d_false;
        break;
      }
    case kind::CONST_BOOLEAN:
      // we get to explain true = false, since we set false to be the trigger of this
      lhs = d_true;
      rhs = d_false;
      break;
    default:
      Unreachable();
  }
  d_equalityEngine.explainEquality(lhs, rhs, assumptions);
}


Node TheoryBV::explain(TNode node) {
  BVDebug("bitvector") << "TheoryBV::explain(" << node << ")" << std::endl;
  std::vector<TNode> assumptions;
  explain(node, assumptions);
  return mkAnd(assumptions);
}


void TheoryBV::addSharedTerm(TNode t) {
  Debug("bitvector::sharing") << spaces(getContext()->getLevel()) << "TheoryBV::addSharedTerm(" << t << ")" << std::endl;
  d_equalityEngine.addTriggerTerm(t);
}


EqualityStatus TheoryBV::getEqualityStatus(TNode a, TNode b)
{
  if (d_equalityEngine.areEqual(a, b)) {
    // The terms are implied to be equal
    return EQUALITY_TRUE;
  }
  if (d_equalityEngine.areDisequal(a, b)) {
    // The terms are implied to be dis-equal
    return EQUALITY_FALSE;
  }
  return EQUALITY_UNKNOWN;
}

