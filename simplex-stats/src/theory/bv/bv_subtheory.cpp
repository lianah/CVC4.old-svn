/*********************                                                        */
/*! \file bv_subtheory.cpp
 ** \verbatim
 ** Original author: lianah
 ** Major contributors: 
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

#include "theory/bv/bv_subtheory.h"
#include "theory/bv/theory_bv.h"
#include "theory/bv/theory_bv_utils.h"
#include "theory/bv/bitblaster.h"


using namespace CVC4;
using namespace CVC4::theory;
using namespace CVC4::theory::bv;
using namespace CVC4::context;

using namespace std;
using namespace CVC4::theory::bv::utils;


BitblastSolver::BitblastSolver(context::Context* c, TheoryBV* bv)
  : SubtheorySolver(c, bv), 
    d_bitblaster(new Bitblaster(c, bv)),
    d_bitblastQueue(c)
{}

BitblastSolver::~BitblastSolver() {
  delete d_bitblaster; 
}

void BitblastSolver::preRegister(TNode node) {
  if ((node.getKind() == kind::EQUAL ||
       node.getKind() == kind::BITVECTOR_ULT ||
       node.getKind() == kind::BITVECTOR_ULE ||
       node.getKind() == kind::BITVECTOR_SLT ||
       node.getKind() == kind::BITVECTOR_SLE) &&
      !d_bitblaster->hasBBAtom(node)) {
    d_bitblastQueue.push_back(node); 
  }
}

void BitblastSolver::explain(TNode literal, std::vector<TNode>& assumptions) {
  d_bitblaster->explain(literal, assumptions); 
}

bool BitblastSolver::addAssertions(const std::vector<TNode>& assertions, Theory::Effort e) {
  BVDebug("bitvector::bitblaster") << "BitblastSolver::addAssertions (" << e << ")" << std::endl;

  //// Eager bit-blasting
  if (Options::current()->bitvectorEagerBitblast) {
    for (unsigned i = 0; i < assertions.size(); ++i) {
      TNode atom = assertions[i].getKind() == kind::NOT ? assertions[i][0] : assertions[i];
      if (atom.getKind() != kind::BITVECTOR_BITOF) {
        d_bitblaster->bbAtom(atom);
      }
    }
    return true; 
  }

  //// Lazy bit-blasting
    
  // bit-blast enqueued nodes
  while (!d_bitblastQueue.empty()) {
    TNode atom = d_bitblastQueue.front();
    d_bitblaster->bbAtom(atom);
    d_bitblastQueue.pop(); 
  }

  // propagation
  for (unsigned i = 0; i < assertions.size(); ++i) {
    TNode fact = assertions[i];
    if (!d_bv->inConflict() && !d_bv->propagatedBy(fact, TheoryBV::SUB_BITBLAST)) {
      // Some atoms have not been bit-blasted yet
      d_bitblaster->bbAtom(fact);
      // Assert to sat
      bool ok = d_bitblaster->assertToSat(fact, d_useSatPropagation);
      if (!ok) {
        std::vector<TNode> conflictAtoms;
        d_bitblaster->getConflict(conflictAtoms);
        d_bv->setConflict(mkConjunction(conflictAtoms)); 
        return false; 
      }
    }
  }

  // solving
  if (e == Theory::EFFORT_FULL || Options::current()->bitvectorEagerFullcheck) {
    Assert(!d_bv->inConflict());
    BVDebug("bitvector::bitblaster") << "BitblastSolver::addAssertions solving. \n"; 
    bool ok = d_bitblaster->solve();
    if (!ok) {
      std::vector<TNode> conflictAtoms;
      d_bitblaster->getConflict(conflictAtoms);
      Node conflict = mkConjunction(conflictAtoms);
      d_bv->setConflict(conflict); 
      return false; 
    }
  }

  return true; 
}

EqualityStatus BitblastSolver::getEqualityStatus(TNode a, TNode b) {
  return d_bitblaster->getEqualityStatus(a, b);
}

EqualitySolver::EqualitySolver(context::Context* c, TheoryBV* bv)
  : SubtheorySolver(c, bv),
    d_notify(bv),
    d_equalityEngine(d_notify, c, "theory::bv::TheoryBV")
{
  if (d_useEqualityEngine) {

    // The kinds we are treating as function application in congruence
    d_equalityEngine.addFunctionKind(kind::BITVECTOR_CONCAT);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_AND);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_OR);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_XOR);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_NOT);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_NAND);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_NOR);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_XNOR);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_COMP);
    d_equalityEngine.addFunctionKind(kind::BITVECTOR_MULT);
    d_equalityEngine.addFunctionKind(kind::BITVECTOR_PLUS);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_SUB);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_NEG);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_UDIV);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_UREM);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_SDIV);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_SREM);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_SMOD);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_SHL);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_LSHR);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_ASHR);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_ULT);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_ULE);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_UGT);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_UGE);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_SLT);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_SLE);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_SGT);
    //    d_equalityEngine.addFunctionKind(kind::BITVECTOR_SGE);
  }
}

void EqualitySolver::preRegister(TNode node) {
  if (!d_useEqualityEngine)
    return;

  if (node.getKind() == kind::EQUAL) {
      d_equalityEngine.addTriggerEquality(node);
  } else {
    d_equalityEngine.addTerm(node);
  }
}

void EqualitySolver::explain(TNode literal, std::vector<TNode>& assumptions) {
  bool polarity = literal.getKind() != kind::NOT;
  TNode atom = polarity ? literal : literal[0];
  if (atom.getKind() == kind::EQUAL) {
    d_equalityEngine.explainEquality(atom[0], atom[1], polarity, assumptions);
  } else {
    d_equalityEngine.explainPredicate(atom, polarity, assumptions);
  }
}

bool EqualitySolver::addAssertions(const std::vector<TNode>& assertions, Theory::Effort e) {
  BVDebug("bitvector::equality") << "EqualitySolver::addAssertions \n"; 
  Assert (!d_bv->inConflict());
  
  for (unsigned i = 0; i < assertions.size(); ++i) {
    TNode fact = assertions[i];
    
    // Notify the equality engine
    if (d_useEqualityEngine && !d_bv->inConflict() && !d_bv->propagatedBy(fact, TheoryBV::TheoryBV::SUB_EQUALITY) ) {
      bool negated = fact.getKind() == kind::NOT;
      TNode predicate = negated ? fact[0] : fact;
      if (predicate.getKind() == kind::EQUAL) {
        if (negated) {
          // dis-equality
          d_equalityEngine.assertEquality(predicate, false, fact);
        } else {
          // equality
          d_equalityEngine.assertEquality(predicate, true, fact);
        }
      } else {
        // Adding predicate if the congruence over it is turned on
        if (d_equalityEngine.isFunctionKind(predicate.getKind())) {
          d_equalityEngine.assertPredicate(predicate, !negated, fact);
        }
      }
    }

    // checking for a conflict 
    if (d_bv->inConflict()) {
      return false;
    }
  }
  
  return true; 
}

bool EqualitySolver::NotifyClass::eqNotifyTriggerEquality(TNode equality, bool value) {
  BVDebug("bitvector::equality") << "NotifyClass::eqNotifyTriggerEquality(" << equality << ", " << (value ? "true" : "false" )<< ")" << std::endl;
  if (value) {
    return d_bv->storePropagation(equality, TheoryBV::SUB_EQUALITY);
  } else {
    return d_bv->storePropagation(equality.notNode(), TheoryBV::SUB_EQUALITY);
  }
}

bool EqualitySolver::NotifyClass::eqNotifyTriggerPredicate(TNode predicate, bool value) {
  BVDebug("bitvector::equality") << "NotifyClass::eqNotifyTriggerPredicate(" << predicate << ", " << (value ? "true" : "false" )<< ")" << std::endl;
  if (value) {
    return d_bv->storePropagation(predicate, TheoryBV::SUB_EQUALITY);
  } else {
    return d_bv->storePropagation(predicate.notNode(), TheoryBV::SUB_EQUALITY);
  }
}

bool EqualitySolver::NotifyClass::eqNotifyTriggerTermEquality(TheoryId tag, TNode t1, TNode t2, bool value) {
  Debug("bitvector::equality") << "NotifyClass::eqNotifyTriggerTermMerge(" << t1 << ", " << t2 << std::endl;
  if (value) {
    return d_bv->storePropagation(t1.eqNode(t2), TheoryBV::SUB_EQUALITY);
  } else {
    return d_bv->storePropagation(t1.eqNode(t2).notNode(), TheoryBV::SUB_EQUALITY);
  }
}

bool EqualitySolver::NotifyClass::eqNotifyConstantTermMerge(TNode t1, TNode t2) {
  Debug("bitvector::equality") << "NotifyClass::eqNotifyConstantTermMerge(" << t1 << ", " << t2 << std::endl;
  if (Theory::theoryOf(t1) == THEORY_BOOL) {
    return d_bv->storePropagation(t1.iffNode(t2), TheoryBV::SUB_EQUALITY);
  }
  return d_bv->storePropagation(t1.eqNode(t2), TheoryBV::SUB_EQUALITY);
}