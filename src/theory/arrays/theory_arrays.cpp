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
#include "theory/valuation.h"
#include "expr/kind.h"
#include <map>

using namespace std;
using namespace CVC4;
using namespace CVC4::kind;
using namespace CVC4::context;
using namespace CVC4::theory;
using namespace CVC4::theory::arrays;


TheoryArrays::TheoryArrays(Context* c, OutputChannel& out) :
  Theory(THEORY_ARRAY, c, out),
  d_ccChannel(this),
  d_cc(c, &d_ccChannel),
  d_unionFind(c),
  d_disequalities(c),
  d_equalities(c),
  d_conflict(),
  d_true_const(),
  d_infoMap(c)
  //d_readIndicesMap(c),
  //d_storesMap(c)
{
  d_true_const = NodeManager::currentNM()->mkConst(true);
}


TheoryArrays::~TheoryArrays() {
}


void TheoryArrays::addSharedTerm(TNode t) {
  Debug("arrays") << "TheoryArrays::addSharedTerm(): "
                  << t << endl;
}


void TheoryArrays::notifyEq(TNode lhs, TNode rhs) {
  /*
  Debug("arrays") << "TheoryArrays::notifyEq(): "
                  << lhs << " = " << rhs << endl;

  NodeManager* nm = NodeManager::currentNM();
  TNode eq = nm->mkNode(kind::EQUAL, lhs, rhs);
  d_cc.addEquality(eq);
  */
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
  Debug("arrays") <<"start check ";
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
      if(a.getType().isArray()) {
        checkExtLemmas(a, b);
      }
      break;
    }
    case kind::CONST_BOOLEAN:
      Debug("arrays")<<"TheoryArrays::check() CONST_BOOLEAN \n";
      Assert(assertion == d_true_const);
      break;
    default:
      Unhandled(assertion.getKind());
    }

  }
  // generate lemmas

  Debug("arrays") << "TheoryArrays::check(): done" << endl;
}

Node TheoryArrays::getValue(TNode n, Valuation* valuation) {
  NodeManager* nodeManager = NodeManager::currentNM();

  switch(n.getKind()) {

  case kind::VARIABLE:
    Unhandled(kind::VARIABLE);

  case kind::EQUAL: // 2 args
    return nodeManager->
      mkConst( valuation->getValue(n[0]) == valuation->getValue(n[1]) );

  default:
    Unhandled(n.getKind());
  }
}

void TheoryArrays::merge(TNode a, TNode b) {
  Assert(d_conflict.isNull());

  Debug("arrays-merge")<<"TheoryArrays::merge() " << a <<" and " <<b <<endl;


  // make "a" the one with shorter diseqList
  CNodeTNodesMap::iterator deq_ia = d_disequalities.find(a);
  CNodeTNodesMap::iterator deq_ib = d_disequalities.find(b);

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
  setCanon(a, b);

  //FIXME: do i need to merge these if there is conflict?
  if(a.getType().isArray()) {
    checkRoWLemmas(a,b);
    checkRoWLemmas(b,a);
    Debug("arrays-merge")<<"after \n";
    // note the change in order, merge info adds the list of
    // the 2nd argument to the first
    d_infoMap.mergeInfo(b, a);
  }

  deq_ia = d_disequalities.find(a);
  map<TNode, TNode> alreadyDiseqs;
  if(deq_ia != d_disequalities.end()) {
    /*
     * Collecting the disequalities of b, no need to check for conflicts
     * since the representative of b does not change and we check all the things
     * in a's class when we look at the diseq list of find(a)
     */

    CNodeTNodesMap::iterator deq_ib = d_disequalities.find(b);
    if(deq_ib != d_disequalities.end()) {
      CTNodeListAlloc* deq = (*deq_ib).second;
      for(CTNodeListAlloc::const_iterator j = deq->begin(); j!=deq->end(); j++) {
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

    CTNodeListAlloc* deqa = (*deq_ia).second;
    for(CTNodeListAlloc::const_iterator i = deqa->begin(); i!= deqa->end(); i++) {
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

  CNodeTNodesMap::iterator deq_i = d_disequalities.find(of);
  CTNodeListAlloc* deq;
  if(deq_i == d_disequalities.end()) {
    deq = new(getContext()->getCMM()) CTNodeListAlloc(true, getContext(), false,
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

  CNodeTNodesMap::iterator eq_i = d_equalities.find(of);
  CTNodeListAlloc* eql;
  if(eq_i == d_equalities.end()) {
    eql = new(getContext()->getCMM()) CTNodeListAlloc(true, getContext(), false,
                                             ContextMemoryAllocator<TNode>(getContext()->getCMM()));
    d_equalities.insertDataFromContextMemory(of, eql);
  } else {
    eql = (*eq_i).second;
  }
  eql->push_back(eq);

}

void TheoryArrays::checkRoWLemmas(TNode a, TNode b) {

  //Assert(find(a)!= find(b));

  Debug("arrays-clr")<<"Arrays::checkLemmas "<<a<<"\n";
  if(Debug.isOn("arrays-clr"))
    d_infoMap.getInfo(a)->print();
  Debug("arrays-clr")<<"  ------------  and "<<b<<"\n";
  if(Debug.isOn("arrays-clr"))
    d_infoMap.getInfo(b)->print();
  CTNodeList* i_a = d_infoMap.getIndices(a);
  CTNodeList* inst_b = d_infoMap.getInStores(b);
  CTNodeList* eqst_b = d_infoMap.getEqStores(b);

  CTNodeList::const_iterator it = i_a->begin();
  CTNodeList::const_iterator its;

  for( ; it != i_a->end(); it++ ) {
    TNode i = *it;
    its = inst_b->begin();
    for ( ; its != inst_b->end(); its++) {

      TNode store = *its;
      Assert(store.getKind() == kind::STORE);
      TNode j = store[1];
      TNode c = store[0];

      NodeManager* nm = NodeManager::currentNM();
      Node eq1 = nm->mkNode(kind::EQUAL, i, j);
      Node ci = nm->mkNode(kind::SELECT, c, i);
      Node ai = nm->mkNode(kind::SELECT, a, i);
      Node eq2 = nm->mkNode(kind::EQUAL, ci, ai);

      // TODO add check if lemma exists and if any of the disjuncts are already
      // true
      Debug("arrays-clr")<<i<<" "<<j<<" | "<<ci<<" "<<ai<<"\n";
      if( i!= j && ci != ai) {
        addLemma(nm->mkNode(kind::OR, eq1, eq2));
      }
    }

    its = eqst_b->begin();
    for ( ; its != eqst_b->end(); its++) {
      TNode store = *its;
      Assert(store.getKind() == kind::STORE);
      TNode j = store[1];
      TNode c = store[0];


      NodeManager* nm = NodeManager::currentNM();
      Node eq1 = nm->mkNode(kind::EQUAL, i, j);
      Node ci = nm->mkNode(kind::SELECT, c, i);
      Node ai = nm->mkNode(kind::SELECT, a, i);
      Node eq2 = nm->mkNode(kind::EQUAL, ci, ai);

      // TODO add check if lemma exists and if any of the disjuncts are already
      // true
      if( i!= j && ci != ai ) {
        addLemma(nm->mkNode(kind::OR, eq1, eq2));
      }
    }

  }
}

void TheoryArrays::checkExtLemmas(TNode a, TNode b) {
  Assert(a.getType().isArray());
  Assert(b.getType().isArray());

  Debug("arrays-cle")<<"Arrays::checkExtLemmas "<<a<<" \n";
  Debug("arrays-cle")<<"                   and "<<b<<" \n";


  if(   d_extLemmaCache.count(make_pair(a, b)) == 0
     && d_extLemmaCache.count(make_pair(b, a)) == 0) {

    NodeManager* nm = NodeManager::currentNM();
    Node k = nm->mkVar(a.getType()[0]);
    Node eq = nm->mkNode(kind::EQUAL, a, b);
    Node ak = nm->mkNode(kind::SELECT, a, k);
    Node bk = nm->mkNode(kind::SELECT, b, k);
    Node neq = nm->mkNode(kind::NOT, nm->mkNode(kind::EQUAL, ak, bk));
    Node lem = nm->mkNode(kind::OR, eq, neq);

    addLemma(lem);
    d_extLemmaCache.insert(make_pair(a,b));
    return;
  }
  Debug("arrays-cle")<<"Arrays::checkExtLemmas lemma already generated. \n";

}

/*

inline void TheoryArrays::appendIndex(TNode a, TNode index) {
  Debug("arrays::index")<<"TheoryArrays::appendIndex a       = "<<a<<" i = "<<index<<"\n";
  //Assert(a.getKind() == kind::ARRAY_TYPE);

  a = find(a);

  Debug("arrays::index")<<"TheoryArrays::appendIndex find(a) = "<<a<<"\n";
  CTNodeListAlloc* ilist;
  CNodeTNodesMap::iterator it = d_readIndicesMap.find(a);
  if( it == d_readIndicesMap.end()) {
    ilist = new (getContext()->getCMM()) CTNodeListAlloc(true, getContext(), false,
                                                    ContextMemoryAllocator<TNode>(getContext()->getCMM()));
    d_readIndicesMap.insertDataFromContextMemory(a, ilist);
    Debug("arrays::index")<<"TheoryArrays::appendIndex adding (find(a), [index]) entry \n";
    ilist->push_back(index);
  } else {
    ilist = (*it).second;
    // check if index already in list
    //FIXME: maybe do this lazily when merging?
    CTNodeListAlloc::const_iterator i = ilist->begin();
    for(; i!= ilist->end(); i++) {
      if((*i) == index) {
        Debug("arrays::index")<<"TheoryArrays::appendIndex index already exits \n";
        return;
      }
    }
    Debug("arrays::index")<<"TheoryArrays::appendIndex appending index to find(a) \n";
    ilist->push_back(index);

  }

}

inline void TheoryArrays::appendStore(TNode a, TNode st) {
  Debug("arrays::store")<<"TheoryArrays::appendStore a       = "<<a<<" st = "<<st<<"\n";
  Assert(st.getKind() == kind::STORE);
  //TODO: good way to check it's an array?
  //Assert(a.getKind() == kind::ARRAY_TYPE);
  a = find(a);

  Debug("arrays::store")<<"TheoryArrays::appendStore find(a) = "<<a<<"\n";
  CTNodeListAlloc* ilist;
  CNodeTNodesMap::iterator it = d_storesMap.find(a);
  if( it == d_storesMap.end()) {
    ilist = new (getContext()->getCMM()) CTNodeListAlloc(true, getContext(), false,
                                                    ContextMemoryAllocator<TNode>(getContext()->getCMM()));
    d_storesMap.insertDataFromContextMemory(a, ilist);
    Debug("arrays::store")<<"TheoryArrays::appendStore adding (find(a), [st]) entry \n";
    ilist->push_back(st);
  } else {
    ilist = (*it).second;
    // check if index already in list
    //FIXME: maybe do this lazily when merging?
    CTNodeListAlloc::const_iterator i = ilist->begin();
    for(; i!= ilist->end(); i++) {
      if((*i) == st) {
        Debug("arrays::store")<<"TheoryArrays::appendStore store already exits \n";
        return;
      }
    }
    Debug("arrays::store")<<"TheoryArrays::appendStore appending store to find(a) \n";
    ilist->push_back(st);


  }
}
*/


// fixme: merge both things at the same time
// before merge iterate through indices and stores and see if any of them

/*
void TheoryArrays::mergeInfo(TNode a, TNode b, CNodeTNodesMap& info_map) {
  Debug("arrays::merge")<<"TheoryArrays::mergeInfo of nodes \n"<<a<<"\n";

  CNodeTNodesMap::iterator iblist = info_map.find(b);
  CNodeTNodesMap::iterator ialist = info_map.find(a);

  if(ialist != info_map.end()) {
    debugList((*ialist).second);
  }

  Debug("arrays::merge")<<b<<"\n";
  if(iblist != info_map.end()) {
     debugList((*iblist).second);
  }
  Assert(find(a) == find(b) && find(a) == b);

  set<TNode> binfo;

  if(ialist != info_map.end()) {
    if (iblist != info_map.end()) {
      // both a and b have info
      CTNodeListAlloc* blist = (*iblist).second;

      // collect b-info

      CTNodeListAlloc::const_iterator ib = blist->begin();
      for( ; ib!= blist ->end(); ib++ ) {
        binfo.insert(*ib);
      }

      // add a info to the b list of info if they are not already there

      CTNodeListAlloc* alist = (*ialist).second;
      CTNodeListAlloc::const_iterator ia = alist->begin();

      for( ; ia!= alist ->end(); ia++ ) {
        if(binfo.find(*ia) == binfo.end()) {
          blist->push_back(*ia);
        }
      }

    }
    else {
      // a has info but b has no info so we simply assigned
      // the info of a to b
      CTNodeListAlloc* alist = (*ialist).second;
      info_map.insert(b, alist);
    }
  }
  // if a has no info there's nothing to do because b is the representative

  Debug("arrays::merge")<<"New list of "<<b<<"\n";
  if(iblist != info_map.end()) {
     debugList((*iblist).second);
  }
  // FIXME: no erasing in CDMap? is that a problem?

}
*/
