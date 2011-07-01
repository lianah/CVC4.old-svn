/*********************                                                        */
/*! \file theory_engine.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: dejan, taking
 ** Minor contributors (to current version): cconway, barrett
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief The theory engine.
 **
 ** The theory engine.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY_ENGINE_H
#define __CVC4__THEORY_ENGINE_H

#include "expr/node.h"
#include "prop/prop_engine.h"
#include "theory/shared_term_manager.h"
#include "theory/theory.h"
#include "theory/theoryof_table.h"
#include "util/options.h"
#include "util/stats.h"

namespace CVC4 {

// In terms of abstraction, this is below (and provides services to)
// PropEngine.

/**
 * This is essentially an abstraction for a collection of theories.  A
 * TheoryEngine provides services to a PropEngine, making various
 * T-solvers look like a single unit to the propositional part of
 * CVC4.
 */
class TheoryEngine {

  /** Associated PropEngine engine */
  prop::PropEngine* d_propEngine;

  /** A table of Kinds to pointers to Theory */
  theory::TheoryOfTable d_theoryOfTable;

  /** Tag for the "registerTerm()-has-been-called" flag on Nodes */
  struct Registered {};
  /** The "registerTerm()-has-been-called" flag on Nodes */
  typedef CVC4::expr::CDAttribute<Registered, bool> RegisteredAttr;

  /**
   * An output channel for Theory that passes messages
   * back to a TheoryEngine.
   */
  class EngineOutputChannel : public theory::OutputChannel {

    friend class TheoryEngine;

    TheoryEngine* d_engine;
    context::Context* d_context;
    context::CDO<Node> d_conflictNode;
    context::CDO<Node> d_explanationNode;

    /**
     * Literals that are propagated by the theory. Note that these are TNodes.
     * The theory can only propagate nodes that have an assigned literal in the
     * sat solver and are hence referenced in the SAT solver.
     */
    std::vector<TNode> d_propagatedLiterals;

  public:

    EngineOutputChannel(TheoryEngine* engine, context::Context* context) :
      d_engine(engine),
      d_context(context),
      d_conflictNode(context),
      d_explanationNode(context){
    }

    void newFact(TNode n);

    void conflict(TNode conflictNode, bool safe)
      throw(theory::Interrupted, AssertionException) {
      Debug("theory") << "EngineOutputChannel::conflict("
                      << conflictNode << ")" << std::endl;
      d_conflictNode = conflictNode;
      ++(d_engine->d_statistics.d_statConflicts);
      if(safe) {
        throw theory::Interrupted();
      }
    }

    void propagate(TNode lit, bool)
      throw(theory::Interrupted, AssertionException) {
      d_propagatedLiterals.push_back(lit);
      ++(d_engine->d_statistics.d_statPropagate);
    }

    void lemma(TNode node, bool)
      throw(theory::Interrupted, AssertionException) {
      ++(d_engine->d_statistics.d_statLemma);
      d_engine->newLemma(node);
    }

    void explanation(TNode explanationNode, bool)
      throw(theory::Interrupted, AssertionException) {
      d_explanationNode = explanationNode;
      ++(d_engine->d_statistics.d_statExplanation);
    }

    void setIncomplete()
      throw(theory::Interrupted, AssertionException) {
      d_engine->d_incomplete = true;
    }
  };/* class EngineOutputChannel */

  EngineOutputChannel d_theoryOut;

  /** Pointer to Shared Term Manager */
  SharedTermManager* d_sharedTermManager;

  theory::Theory* d_builtin;
  theory::Theory* d_bool;
  theory::Theory* d_uf;
  theory::Theory* d_arith;
  theory::Theory* d_arrays;
  theory::Theory* d_bv;

  /**
   * Whether or not theory registration is on.  May not be safe to
   * turn off with some theories.
   */
  bool d_theoryRegistration;

  /**
   * Debugging flag to ensure that shutdown() is called before the
   * destructor.
   */
  bool d_hasShutDown;

  /**
   * True if a theory has notified us of incompleteness (at this
   * context level or below).
   */
  context::CDO<bool> d_incomplete;

  /**
   * Check whether a node is in the pre-rewrite cache or not.
   */
  static bool inPreRewriteCache(TNode n, bool topLevel) throw() {
    return theory::Theory::inPreRewriteCache(n, topLevel);
  }

  /**
   * Get the value of the pre-rewrite cache (or Node::null()) if there is
   * none).
   */
  static Node getPreRewriteCache(TNode n, bool topLevel) throw() {
    return theory::Theory::getPreRewriteCache(n, topLevel);
  }

  /**
   * Set the value of the pre-rewrite cache.  v cannot be a null Node.
   */
  static void setPreRewriteCache(TNode n, bool topLevel, TNode v) throw() {
    return theory::Theory::setPreRewriteCache(n, topLevel, v);
  }

  /**
   * Check whether a node is in the post-rewrite cache or not.
   */
  static bool inPostRewriteCache(TNode n, bool topLevel) throw() {
    return theory::Theory::inPostRewriteCache(n, topLevel);
  }

  /**
   * Get the value of the post-rewrite cache (or Node::null()) if there is
   * none).
   */
  static Node getPostRewriteCache(TNode n, bool topLevel) throw() {
    return theory::Theory::getPostRewriteCache(n, topLevel);
  }

  /**
   * Set the value of the post-rewrite cache.  v cannot be a null Node.
   */
  static void setPostRewriteCache(TNode n, bool topLevel, TNode v) throw() {
    return theory::Theory::setPostRewriteCache(n, topLevel, v);
  }

  /**
   * This is the top rewrite entry point, called during preprocessing.
   * It dispatches to the proper theories to rewrite the given Node.
   */
  Node rewrite(TNode in, bool topLevel = true);

  /**
   * Replace ITE forms in a node.
   */
  Node removeITEs(TNode t);

public:

  /**
   * Construct a theory engine.
   */
  TheoryEngine(context::Context* ctxt, const Options& opts);

  /**
   * Destroy a theory engine.
   */
  ~TheoryEngine();

  SharedTermManager* getSharedTermManager() {
    return d_sharedTermManager;
  }

  void setPropEngine(prop::PropEngine* propEngine) {
    Assert(d_propEngine == NULL);
    d_propEngine = propEngine;
  }

  /**
   * Return whether or not we are incomplete (in the current context).
   */
  bool isIncomplete() {
    return d_incomplete;
  }

  /**
   * This is called at shutdown time by the SmtEngine, just before
   * destruction.  It is important because there are destruction
   * ordering issues between PropEngine and Theory.
   */
  void shutdown() {
    // Set this first; if a Theory shutdown() throws an exception,
    // at least the destruction of the TheoryEngine won't confound
    // matters.
    d_hasShutDown = true;

    d_builtin->shutdown();
    d_bool->shutdown();
    d_uf->shutdown();
    d_arith->shutdown();
    d_arrays->shutdown();
    d_bv->shutdown();
  }

  /**
   * Get the theory associated to a given TypeNode.
   *
   * @returns the theory owning the type
   */
  theory::Theory* theoryOf(TypeNode t);

  /**
   * Get the theory associated to a given Node.
   *
   * @returns the theory, or NULL if the TNode is
   * of built-in type.
   */
  theory::Theory* theoryOf(TNode n);

  /**
   * Preprocess a node.  This involves theory-specific rewriting, then
   * calling preRegisterTerm() on what's left over.
   * @param n the node to preprocess
   */
  Node preprocess(TNode n);

  /**
   * Assert the formula to the apropriate theory.
   * @param node the assertion
   */
  inline void assertFact(TNode node) {
    Debug("theory") << "TheoryEngine::assertFact(" << node << ")" << std::endl;
    theory::Theory* theory =
      node.getKind() == kind::NOT ? theoryOf(node[0]) : theoryOf(node);
    if(theory != NULL) {
      theory->assertFact(node);
    }
  }

  /**
   * Check all (currently-active) theories for conflicts.
   * @param effort the effort level to use
   */
  inline bool check(theory::Theory::Effort effort)
  {
    d_theoryOut.d_conflictNode = Node::null();
    d_theoryOut.d_propagatedLiterals.clear();
    // Do the checking
    try {
      //d_builtin->check(effort);
      //d_bool->check(effort);
      d_uf->check(effort);
      d_arith->check(effort);
      d_arrays->check(effort);
      d_bv->check(effort);
    } catch(const theory::Interrupted&) {
      Debug("theory") << "TheoryEngine::check() => conflict" << std::endl;
    }

    if(!d_theoryOut.d_conflictNode.get().isNull() && Debug.isOn("minisat::lemmas")) {
      std::cerr << "Asserted facts in arithmetic!" << std::endl;
      theory::Theory::fact_iterator it = d_arith->facts_begin();
      theory::Theory::fact_iterator it_end = d_arith->facts_end();
      for(; it != it_end; ++ it) {
          std::cerr << *it << std::endl;
      }
    }

    // Return whether we have a conflict
    return d_theoryOut.d_conflictNode.get().isNull();
  }

  /**
   * Calls staticLearning() on all active theories, accumulating their
   * combined contributions in the "learned" builder.
   */
  void staticLearning(TNode in, NodeBuilder<>& learned);

  /**
   * Calls presolve() on all active theories and returns true
   * if one of the theories discovers a conflict.
   */
  bool presolve();

  /**
   * Calls notifyRestart() on all active theories.
   */
  void notifyRestart();

  inline const std::vector<TNode>& getPropagatedLiterals() const {
    return d_theoryOut.d_propagatedLiterals;
  }

  void clearPropagatedLiterals() {
    d_theoryOut.d_propagatedLiterals.clear();
  }

  inline void newLemma(TNode node) {
    d_propEngine->assertLemma(preprocess(node));
  }

  /**
   * Returns the last conflict (if any).
   */
  inline Node getConflict() {
    return d_theoryOut.d_conflictNode;
  }

  inline void propagate() {
    d_theoryOut.d_propagatedLiterals.clear();
    // Do the propagation
    //d_builtin->propagate(theory::Theory::FULL_EFFORT);
    //d_bool->propagate(theory::Theory::FULL_EFFORT);
    d_uf->propagate(theory::Theory::FULL_EFFORT);
    d_arith->propagate(theory::Theory::FULL_EFFORT);
    d_arrays->propagate(theory::Theory::FULL_EFFORT);
    //d_bv->propagate(theory::Theory::FULL_EFFORT);
  }

  inline Node getExplanation(TNode node, theory::Theory* theory) {
    theory->explain(node);
    return d_theoryOut.d_explanationNode;
  }

  inline Node getExplanation(TNode node) {
    d_theoryOut.d_explanationNode = Node::null();
    theory::Theory* theory =
              node.getKind() == kind::NOT ? theoryOf(node[0]) : theoryOf(node);
    theory->explain(node);
    return d_theoryOut.d_explanationNode;
  }

  Node getValue(TNode node);

private:
  class Statistics {
  public:
    IntStat d_statConflicts, d_statPropagate, d_statLemma, d_statAugLemma, d_statExplanation;
    Statistics():
      d_statConflicts("theory::conflicts", 0),
      d_statPropagate("theory::propagate", 0),
      d_statLemma("theory::lemma", 0),
      d_statAugLemma("theory::aug_lemma", 0),
      d_statExplanation("theory::explanation", 0)
    {
      StatisticsRegistry::registerStat(&d_statConflicts);
      StatisticsRegistry::registerStat(&d_statPropagate);
      StatisticsRegistry::registerStat(&d_statLemma);
      StatisticsRegistry::registerStat(&d_statAugLemma);
      StatisticsRegistry::registerStat(&d_statExplanation);
    }

    ~Statistics() {
      StatisticsRegistry::unregisterStat(&d_statConflicts);
      StatisticsRegistry::unregisterStat(&d_statPropagate);
      StatisticsRegistry::unregisterStat(&d_statLemma);
      StatisticsRegistry::unregisterStat(&d_statAugLemma);
      StatisticsRegistry::unregisterStat(&d_statExplanation);
    }
  };
  Statistics d_statistics;

};/* class TheoryEngine */

}/* CVC4 namespace */

#endif /* __CVC4__THEORY_ENGINE_H */