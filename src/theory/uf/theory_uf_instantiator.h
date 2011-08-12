/*********************                                                        */
/*! \file theory_uf_instantiator.h
 ** \verbatim
 ** Original author: ajreynol
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Theory uf instantiator
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY_UF_INSTANTIATOR_H
#define __CVC4__THEORY_UF_INSTANTIATOR_H

#include "theory/instantiation_engine.h"

#include "context/context.h"
#include "context/context_mm.h"
#include "context/cdlist.h"
#include "context/cdlist_context_memory.h"

namespace CVC4 {
namespace theory {
namespace uf {

class GMatchNode
{
private:
  typedef context::CDList<GMatchNode* > GmnList;
  typedef context::CDList<Node > ObList;
  GmnList d_parents;
  ObList d_obligations;
  Node d_node;
  Node d_match;
public:
  GMatchNode( context::Context* c, Node n );
  ~GMatchNode(){}

  void addParent( GMatchNode* g );
  int getNumParents() { return d_parents.size(); }
  GMatchNode* getParent( int i ) { return d_parents[i]; }

  void addObligation( Node n );
  int getNumObligations() { return d_obligations.size(); }
  Node getObligation( int i ) { return d_obligations[i]; }

  Node getNode() { return d_node; }

  void setMatch( Node n ) { d_match = n; }
  Node getMatch() { return d_match; }
};


class InstantiatorTheoryUf : public Instantiator{
protected:
  typedef context::CDMap<Node, bool, NodeHashFunction> BoolMap;
  typedef context::CDMap<Node, int, NodeHashFunction> IntMap;
  typedef context::CDList<Node, context::ContextMemoryAllocator<Node> > NodeList;
  typedef context::CDMap<Node, NodeList*, NodeHashFunction> NodeLists;
  typedef context::CDMap<Node, GMatchNode*, NodeHashFunction > GMatchMap;

  std::map< Node, int > d_choice_counter;
  int d_numChoices;

  GMatchMap d_gmatches;
  IntMap d_gmatch_size;
  void buildGMatches( Node n );
  GMatchNode* getGMatch( Node n );
  NodeLists d_obligations;
  void addObligation( Node n1, Node n2 );
  void initializeObligationList( Node f );

  //AJR-hack
  Node getConcreteTerm( Node rep );

  //typedef context::CDMap<Node, bool, NodeHashFunction> BoolMap;
  /** reference to the theory that it looks at */
  Theory* d_th;
  //EMatchMap d_ematch;
  //EMatchListMap d_ematch_list;
  BoolMap d_inst_terms;
  BoolMap d_concrete_terms;
  BoolMap d_active_ic;
  /** map from (representative) nodes to list of nodes in their eq class */
  //NodeLists d_equivalence_class;
  /** map from (representative) nodes to list of representative nodes they are disequal from */
  //NodeLists d_disequality;

  ///** used eq classes */
  //std::map< Node, std::vector< Node > > d_emap;
  //std::map< Node, std::vector< Node > > d_dmap;
  ////std::map< Node, Node > d_eq_find;
  //void refreshMaps();
  //bool decideEqual( Node a, Node b );
  bool areEqual( Node a, Node b );
  bool areDisequal( Node a, Node b );
  Node getRepresentative( Node a );
  void debugPrint();
public:
  InstantiatorTheoryUf(context::Context* c, CVC4::theory::InstantiationEngine* ie, Theory* th);
  ~InstantiatorTheoryUf() {}

  Theory* getTheory();
  void check( Node assertion );
  void assertEqual( Node a, Node b );
  bool prepareInstantiation();
private:
  void registerTerm( Node n );
  void setInstTerms( Node n );
  void setConcreteTerms( Node n );
  //void buildEMatchTree( Node n, std::vector< EMatchTreeNode* >& active );
  //void initializeEqClass( Node t );
  //void initializeDisequalityList( Node t );

  //class Counterexample 
  //{
  //public:
  //  //node that it is for
  //  Node d_formula;
  //  //nodes that are currently involved in counterexample
  //  std::vector< Node > d_ce;
  //  //nodes that are interior
  //  std::map< Node, bool > d_interior;
  //  //model requirements
  //  std::map< Node, std::vector< Node > > d_model;
  //  //current graphs to process
  //  std::vector< GMatchNode* > d_curr;

  //  void addToCounterexample( Node i, Node c );
  //  void removeFromCounterexample( Node i );
  //};

  std::map< Node, bool > d_interior;
  std::map< Node, std::vector< Node > > d_model;
  std::map< Node, std::vector< Node > > d_failed_suggestions;
  void calculateBestMatch( Node f );
  void addToCounterexample( Node i, Node c, Node f, std::vector< Node >& ce, 
                            std::vector< GMatchNode* >& curr );
  void removeFromCounterexample( Node i, Node f, std::vector< Node >& ce, 
                                 std::vector< GMatchNode* >& curr );
  Node getValueInCounterexample( Node i, Node f, std::vector< Node >& ce );
  Node getValueInCounterexample( Node i, Node f, std::vector< Node >& ce, std::map< Node, Node >& curr_tasks );
  bool isConsistent( Node i, Node c, Node f, std::vector< Node >& ce,
                     int& numEntailed, int& numNEntailed );
  bool isConsistent( Node i, Node c, Node f, std::vector< Node >& ce, std::map< Node, Node >& curr_tasks,
                     int& numEntailed, int& numNEntailed );

  void propagateCounterexample( Node f, std::vector< Node >& ce, std::vector< GMatchNode* >& curr );

  bool refineCounterexample( Node f, std::vector< Node >& ce, std::vector< GMatchNode* >& curr );
  bool getSuggestion( Node& is, Node& cs, Node f, std::vector< Node >& ce, std::vector< GMatchNode* >& curr, 
                      std::map< Node, Node >& curr_tasks );

  bool decideCounterexample( Node f, std::vector< Node >& ce, std::vector< GMatchNode* >& curr );


  std::map< Node, std::vector< Node > > d_matches;
  std::map< Node, std::map< Node, std::vector< Node > > > d_model_req;
  void getMatches( Node i );

  double d_heuristic;
  Node d_best;

  bool hasInstantiationConstantsFrom( Node i, Node f );
};/* class InstantiatorTheoryUf */

}
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY_UF_INSTANTIATOR_H */
