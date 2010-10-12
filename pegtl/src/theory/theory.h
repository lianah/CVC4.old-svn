/*********************                                                        */
/** theory.h
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): dejan, taking
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.
 **
 ** Base of the theory interface.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__THEORY_H
#define __CVC4__THEORY__THEORY_H

#include "expr/node.h"
#include "expr/attribute.h"
#include "theory/output_channel.h"
#include "context/context.h"

#include <queue>
#include <list>

#include <typeinfo>

namespace CVC4 {
namespace theory {

template <class T>
class TheoryImpl;

/**
 * Base class for T-solvers.  Abstract DPLL(T).
 *
 * This is essentially an interface class.  The TheoryEngine has
 * pointers to Theory.  But each individual theory implementation T
 * should inherit from TheoryImpl<T>, which specializes a few things
 * for that theory.
 */
class Theory {
private:

  template <class T>
  friend class TheoryImpl;

  /**
   * Construct a Theory.
   */
  Theory(context::Context* ctxt, OutputChannel& out) throw() :
    d_context(ctxt),
    d_out(&out) {
  }

  /**
   * Disallow default construction.
   */
  Theory();

protected:

  /**
   * The output channel for the Theory.
   */
  context::Context* d_context;

  /**
   * The output channel for the Theory.
   */
  OutputChannel* d_out;

  /**
   * The assertFact() queue.
   */
  // FIXME CD: on backjump we clear the facts IFF the queue gets
  // emptied on every DL.  In general I guess we need a CDQueue<>?
  // Perhaps one that asserts it's empty at each push?
  std::queue<Node> d_facts;

  /**
   * Return whether a node is shared or not.  Used by setup().
   */
  bool isShared(TNode n) throw();

  /**
   * Returns true if the assertFact queue is empty
   */
  bool done() throw() {
    return d_facts.empty();
  }

public:

  /**
   * Destructs a Theory.  This implementation does nothing, but we
   * need a virtual destructor for safety in case subclasses have a
   * destructor.
   */
  virtual ~Theory() {
  }

  /**
   * Subclasses of Theory may add additional efforts.  DO NOT CHECK
   * equality with one of these values (e.g. if STANDARD xxx) but
   * rather use range checks (or use the helper functions below).
   * Normally we call QUICK_CHECK or STANDARD; at the leaves we call
   * with MAX_EFFORT.
   */
  enum Effort {
    MIN_EFFORT = 0,
    QUICK_CHECK = 10,
    STANDARD = 50,
    FULL_EFFORT = 100
  };/* enum Effort */

  // TODO add compiler annotation "constant function" here
  static bool minEffortOnly(Effort e)        { return e == MIN_EFFORT; }
  static bool quickCheckOrMore(Effort e)     { return e >= QUICK_CHECK; }
  static bool quickCheckOnly(Effort e)       { return e >= QUICK_CHECK && e < STANDARD; }
  static bool standardEffortOrMore(Effort e) { return e >= STANDARD; }
  static bool standardEffortOnly(Effort e)   { return e >= STANDARD && e < FULL_EFFORT; }
  static bool fullEffort(Effort e)           { return e >= FULL_EFFORT; }

  /**
   * Set the output channel associated to this theory.
   */
  void setOutputChannel(OutputChannel& out) {
    d_out = &out;
  }

  /**
   * Get the output channel associated to this theory.
   */
  OutputChannel& getOutputChannel() {
    return *d_out;
  }

  /**
   * Get the output channel associated to this theory [const].
   */
  const OutputChannel& getOutputChannel() const {
    return *d_out;
  }

  /**
   * Pre-register a term.  Done one time for a Node, ever.
   *
   */
  virtual void preRegisterTerm(TNode) = 0;

  /**
   * Register a term.
   *
   * When get() is called to get the next thing off the theory queue,
   * setup() is called on its subterms (in TheoryEngine).  Then setup()
   * is called on this node.
   *
   * This is done in a "context escape" -- that is, at context level 0.
   * setup() MUST NOT MODIFY context-dependent objects that it hasn't
   * itself just created.
   */
  virtual void registerTerm(TNode) = 0;

  /**
   * Assert a fact in the current context.
   */
  void assertFact(TNode n) {
    d_facts.push(n);
  }

  /**
   * Check the current assignment's consistency.
   */
  virtual void check(Effort level = FULL_EFFORT) = 0;

  /**
   * T-propagate new literal assignments in the current context.
   */
  virtual void propagate(Effort level = FULL_EFFORT) = 0;

  /**
   * Return an explanation for the literal represented by parameter n
   * (which was previously propagated by this theory).  Report
   * explanations to an output channel.
   */
  virtual void explain(TNode n, Effort level = FULL_EFFORT) = 0;

  //
  // CODE INVARIANT CHECKING (used only with CVC4_ASSERTIONS)
  //

  /**
   * Different states at which invariants are checked.
   */
  enum ReadyState {
    ABOUT_TO_PUSH,
    ABOUT_TO_POP
  };/* enum ReadyState */

  /**
   * Public invariant checker.  Assert that this theory is in a valid
   * state for the (external) system state.  This implementation
   * checks base invariants and then calls theoryReady().  This
   * function may abort in the case of a failed assert, or return
   * false (the caller should assert that the return value is true).
   *
   * @param s the state about which to check invariants
   */
  bool ready(ReadyState s) {
    if(s == ABOUT_TO_PUSH) {
      Assert(d_facts.empty(), "Theory base code invariant broken: "
             "fact queue is nonempty on context push");
    }

    return theoryReady(s);
  }

protected:

  /**
   * Check any invariants at the ReadyState given.  Only called by
   * Theory class, and then only with CVC4_ASSERTIONS enabled.  This
   * function may abort in the case of a failed assert, or return
   * false (the caller should assert that the return value is true).
   *
   * @param s the state about which to check invariants
   */
  virtual bool theoryReady(ReadyState s) {
    return true;
  }

};/* class Theory */


/**
 * Base class for T-solver implementations.  Each individual
 * implementation T of the Theory interface should inherit from
 * TheoryImpl<T>.  This class specializes some things for a particular
 * theory implementation.
 *
 * The problem with this is that Theory implementations cannot be
 * further subclassed without designing all non-children in the type
 * DAG to play the same trick as here (be template-polymorphic in their
 * most-derived child), linearizing the inheritance hierarchy (viewing
 * each instantiation separately).
 */
template <class T>
class TheoryImpl : public Theory {

protected:

  /**
   * Construct a Theory.
   */
  TheoryImpl(context::Context* ctxt, OutputChannel& out) :
    Theory(ctxt, out) {
    /* FIXME: assert here that a TheoryImpl<T> doesn't already exist
     * for this NodeManager??  If it does, we're hosed because they'll
     * share per-theory node attributes. */
  }

  /** Tag for the "registerTerm()-has-been-called" flag on Nodes */
  struct Registered {};
  /** The "registerTerm()-has-been-called" flag on Nodes */
  typedef CVC4::expr::CDAttribute<Registered, bool> RegisteredAttr;

  /** Tag for the "preRegisterTerm()-has-been-called" flag on Nodes */
  struct PreRegistered {};
  /** The "preRegisterTerm()-has-been-called" flag on Nodes */
  typedef CVC4::expr::Attribute<PreRegistered, bool> PreRegisteredAttr;

  /**
   * Returns the next atom in the assertFact() queue.  Guarantees that
   * registerTerm() has been called on the theory specific subterms.
   *
   * @return the next atom in the assertFact() queue.
   */
  Node get();
};/* class TheoryImpl<T> */

template <class T>
Node TheoryImpl<T>::get() {
  Assert(typeid(*this) == typeid(T),
         "Improper Theory inheritance chain detected.");

  Assert( !d_facts.empty(),
          "Theory::get() called with assertion queue empty!" );

  Node fact = d_facts.front();
  d_facts.pop();

  if(! fact.getAttribute(RegisteredAttr())) {
    std::list<TNode> toReg;
    toReg.push_back(fact);

    /* Essentially this is doing a breadth-first numbering of
     * non-registered subterms with children.  Any non-registered
     * leaves are immediately registered. */
    for(std::list<TNode>::iterator workp = toReg.begin();
        workp != toReg.end();
        ++workp) {

      TNode n = *workp;

      for(TNode::iterator i = n.begin(); i != n.end(); ++i) {
        TNode c = *i;

        if(! c.getAttribute(RegisteredAttr())) {
          if(c.getNumChildren() == 0) {
            c.setAttribute(RegisteredAttr(), true);
            registerTerm(c);
          } else {
            toReg.push_back(c);
          }
        }
      }
    }

    /* Now register the list of terms in reverse order.  Between this
     * and the above registration of leaves, this should ensure that
     * all subterms in the entire tree were registered in
     * reverse-topological order. */
    for(std::list<TNode>::reverse_iterator i = toReg.rend();
        i != toReg.rbegin();
        ++i) {

      TNode n = *i;

      /* Note that a shared TNode in the DAG rooted at "fact" could
       * appear twice on the list, so we have to avoid hitting it
       * twice. */
      // FIXME when ExprSets are online, use one of those to avoid
      // duplicates in the above?
      if(! n.getAttribute(RegisteredAttr())) {
        n.setAttribute(RegisteredAttr(), true);
        registerTerm(n);
      }
    }
  }

  return fact;
}

}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__THEORY_H */
