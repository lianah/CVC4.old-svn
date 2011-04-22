/*********************                                                        */
/*! \file theory_arith.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: taking
 ** Minor contributors (to current version): barrett
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Arithmetic theory.
 **
 ** Arithmetic theory.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__ARITH__THEORY_ARITH_H
#define __CVC4__THEORY__ARITH__THEORY_ARITH_H

#include "theory/theory.h"
#include "context/context.h"
#include "context/cdlist.h"
#include "context/cdset.h"
#include "expr/node.h"

#include "theory/arith/arith_utilities.h"
#include "theory/arith/arithvar_dense_set.h"
#include "theory/arith/delta_rational.h"
#include "theory/arith/tableau.h"
#include "theory/arith/arith_rewriter.h"
#include "theory/arith/partial_model.h"
#include "theory/arith/unate_propagator.h"
#include "theory/arith/simplex.h"

#include "util/stats.h"

#include <vector>
#include <queue>
#include <map>
#include <set>

namespace CVC4 {
namespace theory {
namespace arith {

/**
 * Implementation of QF_LRA.
 * Based upon:
 * http://research.microsoft.com/en-us/um/people/leonardo/cav06.pdf
 */
class TheoryArith : public Theory {
private:

  /* TODO Everything in the chopping block needs to be killed. */
  /* Chopping block begins */

  std::vector<Node> d_splits;
  //This stores the eager splits sent out of the theory.

  /* Chopping block ends */

  std::vector<Node> d_variables;
  std::vector<Node> d_atoms;

  bool d_setupOnline;

  /**
   * Priority Queue of the basic variables that may be inconsistent.
   *
   * This is required to contain at least 1 instance of every inconsistent
   * basic variable. This is only required to be a superset though so its
   * contents must be checked to still be basic and inconsistent.
   */
  std::priority_queue<ArithVar> d_possiblyInconsistent;

  /** Stores system wide constants to avoid unnessecary reconstruction. */
  ArithConstants d_constants;

  /**
   * Manages information about the assignment and upper and lower bounds on
   * variables.
   */
  ArithPartialModel d_partialModel;

  ArithVarDenseSet d_basicManager;
  ActivityMonitor d_activityMonitor;

  /**
   * List of all of the inequalities asserted in the current context.
   */
  context::CDSet<Node, NodeHashFunction> d_diseq;

  /**
   * The tableau for all of the constraints seen thus far in the system.
   */
  Tableau d_tableau;

  /**
   * The rewriter module for arithmetic.
   */
  ArithRewriter d_rewriter;

  ArithUnatePropagator d_propagator;
  SimplexDecisionProcedure d_simplex;

  ArithVar d_oneVar;
  Node d_oneVarIsOne;

public:
  TheoryArith(int id, context::Context* c, OutputChannel& out);
  ~TheoryArith();

  /**
   * Rewriting optimizations.
   */
  RewriteResponse preRewrite(TNode n, bool topLevel);

  /**
   * Plug in old rewrite to the new (pre,post)rewrite interface.
   */
  RewriteResponse postRewrite(TNode n, bool topLevel) {
    return d_rewriter.postRewrite(n);
  }

  /**
   * Does non-context dependent setup for a node connected to a theory.
   */
  void preRegisterTerm(TNode n);

  /** CD setup for a node. Currently does nothing. */
  void registerTerm(TNode n);

  void check(Effort e);
  void propagate(Effort e);
  void explain(TNode n, Effort e);

  Node getValue(TNode n, TheoryEngine* engine);

  void shutdown(){ }

  void presolve();

  std::string identify() const { return std::string("TheoryArith"); }

private:
  /** Added */
  void setupAtom(TNode n);
  void setupAtomList(const vector<Node>& atoms);
  void setupOneVar();
  Node simplify(const std::map<Node, Node>& simpMap, Node arithNode);
  Node simplifyFP(const std::map<Node, Node>& simpMap, Node arithNode);

  std::vector<Node> d_simplifiedAtoms;
  std::set<Node> d_setupAtoms;

  //THIS IS NOT CONTEXT DEPENDENT, BUT IT GETS AWAY WITH IT BECAUSE IT IS USED ONLY
  //BY PRESOLVE
  //THIS SHOULD NEVER BE USED ANYWHERE EXCEPT PRESOLVE
  std::queue<Node> d_toPropagate;

  std::map<Node,Node> detectSimplifications(const std::set<Node>& inputAsserted);
  void simplifyAtoms(const vector<Node>& atoms, const map<Node, Node>& simplifications);
  bool detectConflicts(set<Node>& asserted, const map<Node, Node>& simplifications,
                       Node simpJustification);

  void propagateSimplifiedAtoms(const set<Node>& asserted,
                                const vector<Node>& atoms);

  vector<ArithVar> detectUnconstrained();
  void ejectList(const vector<ArithVar>& unconstrained);

  void checkAllDisequalities();
  Node fakeRewrite(Node arithNode);
  /** Added */

  bool isTheoryLeaf(TNode x) const;

  ArithVar determineLeftVariable(TNode assertion, Kind simpleKind);


  /**
   * This requests a new unique ArithVar value for x.
   * This also does initial (not context dependent) set up for a variable,
   * except for setting up the initial.
   */
  ArithVar requestArithVar(TNode x, bool basic);

  /** Initial (not context dependent) sets up for a variable.*/
  void setupInitialValue(ArithVar x);

  /** Initial (not context dependent) sets up for a new slack variable.*/
  void setupSlack(TNode left);




  /**
   * Handles the case splitting for check() for a new assertion.
   * returns true if their is a conflict.
   */
  bool assertionCases(TNode assertion);

  ArithVar findBasicRow(ArithVar variable);

  void asVectors(Polynomial& p,
                 std::vector<Rational>& coeffs,
                 std::vector<ArithVar>& variables) const;


  /** These fields are designed to be accessable to TheoryArith methods. */
  class Statistics {
  public:
    IntStat d_statUserVariables, d_statSlackVariables;
    IntStat d_statDisequalitySplits;
    IntStat d_statDisequalityConflicts;

    Statistics();
    ~Statistics();
  };

  Statistics d_statistics;


};/* class TheoryArith */

}/* CVC4::theory::arith namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__ARITH__THEORY_ARITH_H */