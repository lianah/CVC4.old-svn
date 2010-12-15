/*********************                                                        */
/*! \file theory_arrays.cpp
 ** \verbatim
 ** Original author: barrett
 ** Major contributors: none
 ** Minor contributors (to current version): mdeters
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Implementation of the theory of arrays.
 **
 ** Implementation of the theory of arrays.
 **/


#include "theory/arrays/theory_arrays.h"
#include "theory/theory_engine.h"
#include "expr/kind.h"

#include <map>

using namespace std;
using namespace CVC4;
using namespace CVC4::kind;
using namespace CVC4::context;
using namespace CVC4::theory;
using namespace CVC4::theory::arrays;


TheoryArrays::TheoryArrays(int id, Context* c, OutputChannel& out) :
  Theory(id, c, out),
  d_ccChannel(this),
  d_cc(c, &d_ccChannel),
  d_unionFind(c),
  d_disequalities(c),
  d_equalities(c),
  d_conflict()
{
}


TheoryArrays::~TheoryArrays() {
}


void TheoryArrays::addSharedTerm(TNode t) {
  Debug("arrays") << "TheoryArrays::addSharedTerm(): "
                  << t << endl;
}


void TheoryArrays::notifyEq(TNode lhs, TNode rhs) {
  Debug("arrays") << "TheoryArrays::notifyEq(): "
                  << lhs << " = " << rhs << endl;
  // FIXME: is this enough and is there a better way to do this?
  d_unionFind.setCanon(lhs, rhs);
  NodeManager* nm = NodeManager::currentNM();
  Node eq = nm->mkNode(kind::EQUAL, lhs, rhs);
  d_cc.addEquality(eq);
}

void TheoryArrays::notifyCongruent(TNode a, TNode b) {
  Debug("arrays") << "TheoryArrays::notifyCongruent(): "
       << a << " = " << b << endl;
  if(!d_conflict.isNull()) {
    return;
  }
  merge(a,b);
}

void TheoryArrays::check(Effort e) {
  while(!done()) {
    Node assertion = get();
    Debug("arrays") << "TheoryArrays::check(): " << assertion << endl;


    switch(assertion.getKind()) {
    case kind::EQUAL:
    case kind::IFF:
      d_cc.addEquality(assertion);
      if(!d_conflict.isNull()) {
        // addEquality can cause a notify congruent which calls merge
        // which can lead to a conflict
        Node conflict = constructConflict(d_conflict);
        d_conflict = Node::null();
        d_out->conflict(conflict, false);
        return;
      }
      merge(assertion[0], assertion[1]);
      break;
    case kind::NOT:
    {
      Assert(assertion[0].getKind() == kind::EQUAL ||
         assertion[0].getKind() == kind::IFF );
      Node a = assertion[0][0];
      Node b = assertion[0][1];

      addDiseq(assertion[0]);
      d_cc.addTerm(a);
      d_cc.addTerm(b);

      if(!d_conflict.isNull()) {
        // we got notified through notifyCongruent which called merge
        // after addTerm since we weren't watching a or b before
        Node conflict = constructConflict(d_conflict);
        Debug("arrays")<<" conflict2 "<<conflict<<endl;
        d_conflict = Node::null();
        d_out->conflict(conflict, false);
        return;
      }
      else if(find(a) == find(b)) {
        Node conflict = constructConflict(assertion[0]);
        d_out->conflict(conflict, false);
        return;
        }
      Assert(!d_cc.areCongruent(a,b));
      break;
    }
    default:
      Unhandled(assertion.getKind());
    }
  }

  Debug("arrays") << "TheoryArrays::check(): done" << endl;
}

Node TheoryArrays::getValue(TNode n, TheoryEngine* engine) {
  NodeManager* nodeManager = NodeManager::currentNM();

  switch(n.getKind()) {

  case kind::VARIABLE:
    Unhandled(kind::VARIABLE);

  case kind::EQUAL: // 2 args
    return nodeManager->
      mkConst( engine->getValue(n[0]) == engine->getValue(n[1]) );

  default:
    Unhandled(n.getKind());
  }
}


void TheoryArrays::merge(TNode a, TNode b) {
  Assert(d_conflict.isNull());

  Debug("arrays-merge")<<"TheoryArrays::merge() " << a <<" and " <<b <<endl;

  // make "a" the one with shorter diseqList
  EqLists::iterator deq_ia = d_disequalities.find(a);
  EqLists::iterator deq_ib = d_disequalities.find(b);

  if(deq_ia != d_disequalities.end()) {
    if(deq_ib == d_disequalities.end() ||
       (*deq_ia).second->size() > (*deq_ib).second->size()) {
      TNode tmp = a;
      a = b;
      b = tmp;
    }
  }
  a = find(a);
  b = find(b);

  if( a == b) {
    return;
  }

  // b becomes the canon of a
  d_unionFind.setCanon(a, b);

  deq_ia = d_disequalities.find(a);
  map<TNode, TNode> alreadyDiseqs;
  if(deq_ia != d_disequalities.end()) {
    /*
     * Collecting the disequalities of b, no need to check for conflicts
     * since the representative of b does not change and we check all the things
     * in a's class when we look at the diseq list of find(a)
     */

    EqLists::iterator deq_ib = d_disequalities.find(b);
    if(deq_ib != d_disequalities.end()) {
      EqList* deq = (*deq_ib).second;
      for(EqList::const_iterator j = deq->begin(); j!=deq->end(); j++) {
        TNode deqn = *j;
        TNode s = deqn[0];
        TNode t = deqn[1];
        TNode sp = find(s);
        TNode tp = find(t);
        Assert(sp == b || tp == b);
        if(sp == b) {
          alreadyDiseqs[tp] = deqn;
        } else {
          alreadyDiseqs[sp] = deqn;
        }
      }
    }

    /*
     * Looking for conflicts in the a disequality list. Note
     * that at this point a and b are already merged. Also has
     * the side effect that it adds them to the list of b (which
     * became the canonical representative)
     */

    EqList* deqa = (*deq_ia).second;
    for(EqList::const_iterator i = deqa->begin(); i!= deqa->end(); i++) {
      TNode deqn = (*i);
      Assert(deqn.getKind() == kind::EQUAL || deqn.getKind() == kind::IFF);
      TNode s = deqn[0];
      TNode t = deqn[1];
      TNode sp = find(s);
      TNode tp = find(t);

      if(find(s) == find(t)) {
        d_conflict = deqn;
        return;
      }
      Assert( sp == b || tp == b);

      // make sure not to add duplicates

      if(sp == b) {
        if(alreadyDiseqs.find(tp) == alreadyDiseqs.end()) {
          appendToDiseqList(b, deqn);
          alreadyDiseqs[tp] = deqn;
        }
      } else {
        if(alreadyDiseqs.find(sp) == alreadyDiseqs.end()) {
          appendToDiseqList(b, deqn);
          alreadyDiseqs[sp] = deqn;
        }
      }

    }
  }

}

Node TheoryArrays::constructConflict(TNode diseq) {
  Debug("arrays") << "arrays: begin constructConflict()" << endl;
  Debug("arrays") << "arrays:   using diseq == " << diseq << endl;

  // returns the reason the two terms are equal
  Node explanation = d_cc.explain(diseq[0], diseq[1]);

  NodeBuilder<> nb(kind::AND);

  if(explanation.getKind() == kind::EQUAL ||
     explanation.getKind() == kind::IFF) {
    // if the explanation is only one literal
    nb<<explanation;
  }
  else {
    Assert(explanation.getKind() == kind::AND);
    for(TNode::iterator i  = TNode(explanation).begin();
        i != TNode(explanation).end(); i++) {
      nb<<*i;
    }
  }

  nb<<diseq.notNode();
  Node conflict = nb;
  Debug("arrays") << "conflict constructed : " << conflict << endl;
  return conflict;
}


void TheoryArrays::addDiseq(TNode diseq) {
  Assert(diseq.getKind() == kind::EQUAL ||
         diseq.getKind() == kind::IFF);
  TNode a = diseq[0];
  TNode b = diseq[1];

  appendToDiseqList(find(a), diseq);
  appendToDiseqList(find(b), diseq);

}

void TheoryArrays::appendToDiseqList(TNode of, TNode eq) {
  Debug("arrays") << "appending " << eq << endl
              << "  to diseq list of " << of << endl;
  Assert(eq.getKind() == kind::EQUAL ||
         eq.getKind() == kind::IFF);

  EqLists::iterator deq_i = d_disequalities.find(of);
  EqList* deq;
  if(deq_i == d_disequalities.end()) {
    deq = new(getContext()->getCMM()) EqList(true, getContext(), false,
                                             ContextMemoryAllocator<TNode>(getContext()->getCMM()));
    d_disequalities.insertDataFromContextMemory(of, deq);
  } else {
    deq = (*deq_i).second;
  }

  deq->push_back(eq);
}

void TheoryArrays::appendToEqList(TNode of, TNode eq) {
  Debug("arrays") << "appending "<< eq << endl
      << " to equality list of " << of << endl;

  Assert(eq.getKind() == kind::EQUAL ||
         eq.getKind() == kind::IFF);

  Assert(of == debugFind(of));

  EqLists::iterator eq_i = d_equalities.find(of);
  EqList* eql;
  if(eq_i == d_equalities.end()) {
    eql = new(getContext()->getCMM()) EqList(true, getContext(), false,
                                             ContextMemoryAllocator<TNode>(getContext()->getCMM()));
    d_equalities.insertDataFromContextMemory(of, eql);
  } else {
    eql = (*eq_i).second;
  }
  eql->push_back(eq);

}



void TheoryArrays::addRoW0Lemma(TNode n) {
  Assert(n.getKind() == kind::SELECT);
  TNode c = n[0];
  TNode j = n[1];

  for (std::set<TNode>::iterator it = store_terms.begin(); it != store_terms.end(); it++) {
    if(c.getType() == (*it).getType()) {
      TNode b = *it;
      Assert(b.getKind() == kind::STORE);
      TNode a = b[0];
      TNode i = b[1];

      NodeManager* nm = NodeManager::currentNM();
      Node aj = nm->mkNode(kind::SELECT, a, j);
      Node bj = nm->mkNode(kind::SELECT, b, j);
      Node eq = nm->mkNode(kind::EQUAL, aj, bj);
      Node neq = nm->mkNode(kind::NOT, nm->mkNode(kind::EQUAL, i, j));
      Node impl = nm->mkNode(kind::IMPLIES, neq, eq);

      d_out->lemma(impl);
      Debug("arrays-lemma") << "array-lemma RoW0 "<< impl << std::endl;

    }

  }
}

void TheoryArrays::addExt0Lemma(TNode a, TNode b) {
  // add the Ext0 lemma
  //    for all two arrays a, b of the same type add a != b => a[i]!= b[i]
  //    for a new variable i.

  NodeManager* nm = NodeManager::currentNM();
  Node neq1 = nm->mkNode(kind::NOT, nm->mkNode(kind::EQUAL, a, b));
  Node new_var = nm->mkVar(a.getType()[0]);
  Node select0 = nm->mkNode(kind::SELECT, a, new_var);
  Node select1 = nm->mkNode(kind::SELECT, b, new_var);
  Node neq2 = nm->mkNode(kind::NOT, nm->mkNode(kind::EQUAL, select0, select1));
  Node impl = nm->mkNode(kind::IMPLIES, neq1, neq2);

  d_out->lemma(impl);
  Debug("arrays-lemma") << "array-lemma Ext0 "<< impl << std::endl;
  // add the new terms a[i], b[i] to the list of proxied variables
  addProxy(select0);
  addProxy(select1);s

}
