/*********************                                                        */
/*! \file node_visitor.h
 ** \verbatim
 ** Original author: dejan
 ** Major contributors: 
 ** Minor contributors (to current version):
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **/

#include "cvc4_private.h"

#pragma once

#include "context/context.h"
#include "theory/shared_terms_database.h"

#include <ext/hash_map>

namespace CVC4 {

class TheoryEngine;

/**
 * Visitor that calls the appropriate theory to pre-register the term. The visitor also keeps track
 * of the sets of theories that are involved in the terms, so that it can say if there are multiple
 * theories involved.
 *
 * A sub-term has been visited if the theories of both the parent and the term itself have already
 * visited this term.
 *
 * Computation of the set of theories in the original term are computed in the alreadyVisited method
 * so as no to skip any theories.
 */
class PreRegisterVisitor {

  /** The engine */
  TheoryEngine* d_engine;

  typedef context::CDHashMap<TNode, theory::Theory::Set, TNodeHashFunction> TNodeToTheorySetMap;

  /**
   * Map from terms to the theories that have already had this term pre-registered.
   */
  TNodeToTheorySetMap d_visited;

  /**
   * A set of all theories in the term
   */
  theory::Theory::Set d_theories;

  /**
   * Is true if the term we're traversing involves multiple theories.
   */
  bool d_multipleTheories;

  /**
   * String representation of the visited map, for debugging purposes.
   */
  std::string toString() const;

public:

  /** Return type tells us if there are more than one theory or not */
  typedef bool return_type;
  
  PreRegisterVisitor(TheoryEngine* engine, context::Context* context)
  : d_engine(engine)
  , d_visited(context)
  , d_theories(0)
  , d_multipleTheories(false)
  {}

  /**
   * Returns true is current has already been pre-registered with both current and parent theories.
   */
  bool alreadyVisited(TNode current, TNode parent);
  
  /**
   * Pre-registeres current with any of the current and parent theories that haven't seen the term yet.
   */
  void visit(TNode current, TNode parent);
  
  /**
   * Marks the node as the starting literal.
   */
  void start(TNode node);

  /**
   * Notifies the engine of all the theories used.
   */
  bool done(TNode node);

};


/**
 * The reason why we need to make this outside of the pre-registration loop is because we need a shared term x to 
 * be associated with every atom that contains it. For example, if given f(x) >= 0 and f(x) + 1 >= 0, although f(x) has
 * been visited already, we need to visit it again, since we need to associate it with both atoms.
 */
class SharedTermsVisitor {

  /** The shared terms database */
  SharedTermsDatabase& d_sharedTerms;

  /**
   * Cache from proprocessing of atoms.
   */
  typedef std::hash_map<TNode, theory::Theory::Set, TNodeHashFunction> TNodeVisitedMap;
  TNodeVisitedMap d_visited;

  /**
   * String representation of the visited map, for debugging purposes.
   */
  std::string toString() const;

  /** 
   * The initial atom.
   */
  TNode d_atom; 
    
public:

  typedef void return_type;

  SharedTermsVisitor(SharedTermsDatabase& sharedTerms)
  : d_sharedTerms(sharedTerms) {}

  /**
   * Returns true is current has already been pre-registered with both current and parent theories.
   */
  bool alreadyVisited(TNode current, TNode parent) const;
  
  /**
   * Pre-registeres current with any of the current and parent theories that haven't seen the term yet.
   */
  void visit(TNode current, TNode parent);
  
  /**
   * Marks the node as the starting literal.
   */
  void start(TNode node);

  /**
   * Just clears the state.
   */
  void done(TNode node);

  /**
   * Clears the internal state.
   */   
  void clear();
};


}
