/*********************                                                        */
/*! \file congruence_closure.h
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
 ** \brief The congruence closure module
 **
 ** The congruence closure module.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__UTIL__CONGRUENCE_CLOSURE_H
#define __CVC4__UTIL__CONGRUENCE_CLOSURE_H

#include <sstream>
#include <list>

#include <ext/hash_map>

#include "expr/node_manager.h"
#include "expr/node.h"
#include "expr/kind_map.h"
#include "context/context_mm.h"
#include "context/cdo.h"
#include "context/cdmap.h"
#include "context/cdset.h"
#include "context/cdlist.h"
#include "context/cdcirclist.h"
#include "util/exception.h"
#include "theory/uf/morgan/stacking_map.h"
#include "util/stats.h"
#include "util/hash.h"
#include "util/dynamic_array.h"

namespace CVC4 {

template <class OutputChannel>
class CongruenceClosure;

template <class OutputChannel>
std::ostream& operator<<(std::ostream& out,
                         const CongruenceClosure<OutputChannel>& cc);

/**
 * A CongruenceClosureException is thrown by
 * CongruenceClosure::explain() when that method is asked to explain a
 * congruence that doesn't exist.
 */
class CVC4_PUBLIC CongruenceClosureException : public Exception {
public:
  inline CongruenceClosureException(std::string msg) :
    Exception(std::string("Congruence closure exception: ") + msg) {}
  inline CongruenceClosureException(const char* msg) :
    Exception(std::string("Congruence closure exception: ") + msg) {}
};/* class CongruenceClosureException */

/**
 * Congruence closure module for CVC4.
 *
 * This is a service class for theories.  One uses a CongruenceClosure
 * by adding a number of relevant equality terms with registerTerm() and
 * asserted equalities with addEquality().  It then gets notified
 * (through OutputChannel, below), of new equality terms that are
 * implied by the current set of asserted (and implied) equalities.

 *
 * OutputChannel is a template argument (it's probably a thin layer,
 * and we want to avoid a virtual call hierarchy in this case, and
 * enable aggressive inlining).  It need only implement one method,
 * notifyCongruent():
 *
 *   class MyOutputChannel {
 *   public:
 *     void notifyCongruence(TNode eq) {
 *       // CongruenceClosure is notifying us that "a" is now the EC
 *       // representative for "b" in this context.  After a pop, "a"
 *       // will be its own representative again.  Note that "a" and
 *       // "b" might never have been added with registerTerm().  However,
 *       // something in their equivalence class was (for which a
 *       // previous notifyCongruent() would have notified you of
 *       // their EC representative, which is now "a" or "b").
 *       //
 *       // This call is made while the merge is being done.  If you
 *       // throw any exception here, you'll immediately exit the
 *       // CongruenceClosure call and will miss whatever pending
 *       // merges were being performed, leaving the CongruenceClosure
 *       // module in a bad state.  Therefore if you throw, you should
 *       // only do so if you are going to backjump, re-establishing a
 *       // good (former) state.  Keep this in mind if a
 *       // CVC4::Interrupted that *doesn't* lead to a backjump can
 *       // interrupt you.
 *     }
 *   };
 */
template <class OutputChannel>
class CongruenceClosure {

  /**
   * A Cid is a "Congruence ID", and is a dense, integral
   * representation of a term.  Positive Cids are for individuals, and
   * negative Cids are for application-like things (technically: they
   * are non-nullary applications of operators we have been requested
   * to compute congruence over).
   */
  typedef int32_t Cid;

  struct CidHashFunction {
    inline size_t operator()(Cid c) const { return c; }
  };/* struct CongruenceClosure<>::CidHashFunction */

  template <class T, class UnderlyingHashFunction = std::hash<T> >
  struct VectorHashFunction {
    inline size_t operator()(const std::vector<T>& v) const {
      UnderlyingHashFunction h;
      size_t hash = 0;
      typename std::vector<T>::const_iterator i = v.begin();
      typename std::vector<T>::const_iterator i_end = v.end();
      while(i != i_end) {
        hash ^= h(*i) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        ++i;
      }
      return hash;
    }
  };/* struct CongruenceClosure<>::VectorHashFunction<> */

  std::hash_map<TNode, Cid, TNodeHashFunction> d_cidMap;
  DynamicArray<TNode> d_reverseCidMapIndividuals;
  DynamicArray<TNode> d_reverseCidMapApplications;
  Cid d_nextIndividualCid;
  Cid d_nextApplicationCid;

  inline Cid cid(TNode n) {
    Cid& cid = d_cidMap[n];
    if(cid == 0) {
      if(isCongruenceOperator(n)) {
        cid = d_nextApplicationCid--;
        d_reverseCidMapApplications[-cid] = n;
      } else {
        cid = d_nextIndividualCid++;
        d_reverseCidMapIndividuals[cid] = n;
      }
    }
    return cid;
  }
  inline Cid cid(TNode n) const {
    std::hash_map<TNode, Cid, TNodeHashFunction>::const_iterator i = d_cidMap.find(n);
    Assert(i != d_cidMap.end());
    return (*i).second;
  }
  inline TNode node(Cid cid) const {
    return cid > 0 ?
      d_reverseCidMapIndividuals[cid] :
      d_reverseCidMapApplications[-cid];
  }
  static inline bool isApplication(Cid cid) {
    return cid < 0;
  }
  static inline bool isIndividual(Cid cid) {
    return cid > 0;
  }

  /** The context at play. */
  context::Context* d_context;

  /**
   * The output channel, used for notifying the client of new
   * congruences.  Only terms registered with registerTerm() will
   * generate notifications.
   */
  OutputChannel* d_out;

  /**
   * The bitmap of Kinds over which we are computing congruence.  It
   * doesn't really make sense for this list of operators to change,
   * so we don't allow it to---plus, if it did change, we'd probably
   * have to recompute everything from scratch anyway.
   */
  const KindMap d_congruenceOperatorMap;

  // typedef all of these so that iterators are easy to define, and so
  // experiments can be run easily with different data structures
  typedef context::CDMap<Cid, Cid> RepresentativeMap;
  typedef context::CDCircList<Cid, context::ContextMemoryAllocator<Cid> > ClassList;
  typedef context::CDList<TNode> Uses;
  typedef DynamicArray<Uses> UseList;
  typedef DynamicArray< std::vector<Node> > PropagateList;
  typedef context::CDMap<std::vector<Cid>, Node, VectorHashFunction<Cid, CidHashFunction> > LookupMap;

  //typedef theory::uf::morgan::StackingMap<Node, Node, NodeHashFunction> ProofMap;
  //typedef theory::uf::morgan::StackingMap<Node, Node, NodeHashFunction> ProofLabel;

  // Simple, NON-context-dependent pending list, union find and "seen
  // set" types for constructing explanations and
  // nearestCommonAncestor(); see explain().
  //typedef std::list<std::pair<Node, Node> > PendingProofList_t;
  //typedef __gnu_cxx::hash_map<TNode, TNode, TNodeHashFunction> UnionFind_t;
  //typedef __gnu_cxx::hash_set<TNode, TNodeHashFunction> SeenSet_t;

  RepresentativeMap d_representative;
  ClassList d_classList;
  PropagateList d_propagate;
  UseList d_useList;
  LookupMap d_lookup;

  //ProofMap d_proof;
  //ProofLabel d_proofLabel;

  //ProofMap d_proofRewrite;

  // === STATISTICS ===
  AverageStat d_explanationLength;/**< average explanation length */

  inline bool isCongruenceOperator(TNode n) const {
    // For the datatypes theory, we've removed the invariant that
    // parameterized kinds must have at least one argument.  Consider
    // (CONSTRUCTOR nil) for instance.  So, n here can be an operator
    // that's normally checked for congruence (like CONSTRUCTOR) but
    // shouldn't be treated as a congruence operator if it only has an
    // operator and no children (like CONSTRUCTOR nil), since we can
    // treat that as a simple variable.
    return n.getNumChildren() > 0 && d_congruenceOperatorMap[n.getKind()];
  }

public:
  /** Construct a congruence closure module instance */
  CongruenceClosure(context::Context* ctxt, OutputChannel* out, KindMap kinds)
    throw(AssertionException) :
    d_cidMap(),
    d_reverseCidMapIndividuals(false),
    d_reverseCidMapApplications(false),
    d_nextIndividualCid(1),
    d_nextApplicationCid(-1),
    d_context(ctxt),
    d_out(out),
    d_congruenceOperatorMap(kinds),
    d_representative(ctxt),
    d_classList(ctxt, context::ContextMemoryAllocator<Cid>(ctxt->getCMM())),
    d_useList(true),
    d_lookup(ctxt),
    //d_proof(ctxt),
    //d_proofLabel(ctxt),
    //d_proofRewrite(ctxt),
    d_explanationLength("congruence_closure::AverageExplanationLength") {
    CheckArgument(!kinds.isEmpty(), "cannot construct a CongruenceClosure with an empty KindMap");
  }

  ~CongruenceClosure() {}

  /**
   * Register a term (which should be a node of kind EQUAL or IFF) for
   * CC consideration, so that CC will push out a notification to its
   * output channel should the equality become implied by the current
   * state.  This set of terms is NOT context-dependent, and that's by
   * design: lemmas added at deep context levels can lead to new,
   * pre-registered terms which should still be considered for
   * propagation after that context is popped (since the lemmas, and
   * therefore their constituent terms, don't go away).
   *
   * This function calls OutputChannel::notifyCongruent() for the term
   * if the equality is already implied by the current partial
   * assignment, so it can throw anything that that function can.
   */
  void registerTerm(TNode eq);

  /**
   * Add an equality literal eq into CC consideration (it should be a
   * node of kind EQUAL or IFF), asserting that this equality is now
   * true.  This assertion is context-dependent.  Calls
   * OutputChannel::notifyCongruent() to notify the client of any
   * equalities (registered using registerTerm()) that are now congruent.
   * Therefore, it can throw anything that that function can.
   *
   * Note that equalities asserted via assertEquality() need not have
   * been registered using registerTerm()---the values in those two sets
   * have no requirements---the two sets can be equal, disjoint,
   * overlapping, it doesn't matter.
   */
  void assertEquality(TNode inputEq) {
    Debug("cc") << "CC assertEquality[" << d_context->getLevel() << "]: "
                << inputEq << std::endl;
    AssertArgument(inputEq.getKind() == kind::EQUAL ||
                   inputEq.getKind() == kind::IFF, inputEq);

    merge(find(cid(inputEq[0])), find(cid(inputEq[1])), inputEq);
  }

private:
  void merge(Cid a, Cid b, TNode inputEq);

  /*
  TNode proofRewrite(TNode pfStep) const {
    ProofMap::const_iterator i = d_proofRewrite.find(pfStep);
    if(i == d_proofRewrite.end()) {
      return pfStep;
    } else {
      return (*i).second;
    }
  }
  */

public:
  /**
   * Inquire whether two expressions are congruent in the current
   * context.
   */
  inline bool areCongruent(TNode a, TNode b) const throw(AssertionException) {
    if(Debug.isOn("cc")) {
      Debug("cc") << "CC areCongruent? " << a << "  ==  " << b << std::endl;
      Debug("cc") << "  a  " << a << std::endl;
      Debug("cc") << "  a' " << normalize(a) << std::endl;
      Debug("cc") << "  b  " << b << std::endl;
      Debug("cc") << "  b' " << normalize(b) << std::endl;
    }

    Cid ap = find(cid(a)), bp = find(cid(b));

    return ap == bp || normalize(ap) == normalize(bp);
  }

private:
  /**
   * Find the EC representative for a term t in the current context.
   */
  inline Cid find(Cid c) const throw(AssertionException) {
    RepresentativeMap::const_iterator it = d_representative.find(c);
    if(it == d_representative.end()) {
      return c;
    } else {
      return (*it).second;
    }
  }

  /*
  void explainAlongPath(Cid a, Cid c, PendingProofList_t& pending, UnionFind_t& unionFind, std::list<Node>& pf)
    throw(AssertionException);

  Node highestNode(Cid a, UnionFind_t& unionFind) const
    throw(AssertionException);

  Node nearestCommonAncestor(Cid a, Cid b, UnionFind_t& unionFind)
    throw(AssertionException);
  */

  /**
   * Normalization.  Two terms are congruent iff they have the same
   * normal form.
   */
  Cid normalize(Cid c) const throw(AssertionException);

public:

  Node normalize(TNode n) const throw(AssertionException) {
    return (d_cidMap.find(n) == d_cidMap.end()) ? n : node(normalize(cid(n)));
  }

  /**
   * Request an explanation for why a and b are in the same EC in the
   * current context.  Throws a CongruenceClosureException if they
   * aren't in the same EC.
   */
  Node explain(Node a, Node b)
    throw(CongruenceClosureException, AssertionException);

  /**
   * Request an explanation for why the lhs and rhs of this equality
   * are in the same EC.  Throws a CongruenceClosureException if they
   * aren't in the same EC.
   */
  inline Node explain(TNode eq)
    throw(CongruenceClosureException, AssertionException) {
    AssertArgument(eq.getKind() == kind::EQUAL ||
                   eq.getKind() == kind::IFF, eq);
    return explain(eq[0], eq[1]);
  }

private:

  friend std::ostream& operator<< <>(std::ostream& out,
                                     const CongruenceClosure<OutputChannel>& cc);

  /**
   * Internal propagation of information.  Propagation tends to
   * cascade (this implementation uses an iterative "pending"
   * worklist), and "seed" is the "seeding" propagation to do (the
   * first one).  Calls OutputChannel::notifyCongruent() indirectly,
   * so it can throw anything that that function can.
   */
  void propagate(Cid seed1, Cid seed2);
  void propagate(TNode seed1, TNode seed2) {
#   warning FIXME remove this function
    propagate(cid(seed1), cid(seed2));
  }

  /**
   * Internal lookup mapping from tuples to equalities.
   */
  inline TNode lookup(const std::vector<Cid>& a) const {
    typename LookupMap::const_iterator i = d_lookup.find(a);
    if(i == d_lookup.end()) {
      return TNode::null();
    } else {
      TNode l = (*i).second;
      Assert(l.getKind() == kind::EQUAL ||
             l.getKind() == kind::IFF);
      return l;
    }
  }

  /**
   * Internal lookup mapping.
   */
  inline void setLookup(const std::vector<Cid>& a, TNode b) {
    Assert(b.getKind() == kind::EQUAL ||
           b.getKind() == kind::IFF);
    d_lookup[a] = b;
  }

  std::vector<Cid> buildRepresentativesOfApply(Cid apply)
    throw(AssertionException);

  /**
   * Append equality "eq" to uselist of "of".
   */
  inline void appendToUseList(Cid of, TNode eq) {
    Trace("cc") << "adding " << eq << " to use list of " << of << std::endl;
    Assert(eq.getKind() == kind::EQUAL ||
           eq.getKind() == kind::IFF);
    Assert(of == find(of));
    Uses& uses = d_useList[of];
    uses.push_back(eq);
  }

  /**
   * Merge equivalence class ec1 under ec2.  ec1's new union-find
   * representative becomes ec2.  Calls
   * OutputChannel::notifyCongruent(), so it can throw anything that
   * that function can.
   */
  void ufmerge(TNode ec1, TNode ec2);
  void mergeProof(TNode a, TNode b, TNode e);

public:

  // === STATISTICS ACCESSORS ===

  /**
   * Get access to the explanation-length statistic.  Returns the
   * statistic itself so that reference-statistics can be wrapped
   * around it, useful since CongruenceClosure is a client class and
   * shouldn't be directly registered with the StatisticsRegistry.
   */
  const AverageStat& getExplanationLength() const throw() {
    return d_explanationLength;
  }

};/* class CongruenceClosure */

template <class OutputChannel>
void CongruenceClosure<OutputChannel>::registerTerm(TNode t) {
  Assert(t.getKind() == kind::EQUAL ||
         t.getKind() == kind::IFF);
  TNode a = t[0];
  TNode b = t[1];

  Debug("cc") << "CC registerTerm " << t << std::endl;

  if(areCongruent(a, b)) {
    // we take care to only notify our client once of congruences
    d_out->notifyCongruence(t);
  }

  d_propagate[cid(a)].push_back(t);
  d_propagate[cid(b)].push_back(t);
}

/* From [Nieuwenhuis & Oliveras]
1. Procedure Merge(s=t)
2.   If s and t are constants a and b Then
3.     add a=b to Pending
4.     Propagate()
5.   Else ** s=t is of the form f(a1, a2)=a **
6.     If Lookup(a1', a2') is some f(b1, b2)=b Then
7.       add ( f(a1, a2)=a, f(b1, b2)=b ) to Pending
8.       Propagate()
9.     Else
10.      set Lookup(a1', a2') to f(a1, a2)=a
11.      add f(a1, a2)=a to UseList(a1') and to UseList(a2')
*/

template <class OutputChannel>
void CongruenceClosure<OutputChannel>::merge(Cid s, Cid t, TNode inputEq) {

  Trace("cc") << "CC merge[" << d_context->getLevel() << "]: " << inputEq << std::endl;

  Assert(inputEq.getKind() == kind::EQUAL || inputEq.getKind() == kind::IFF);
  Assert(s == find(s) && t == find(t));

  if(s == t) {
    // redundant
    return;
  }

  if(!isApplication(s) && !isApplication(t)) {
    // s, t are constants
    propagate(s, t);
  } else {
    if(isApplication(s)) {
      std::vector<Cid> ap = buildRepresentativesOfApply(s);

      TNode lup = lookup(ap);
      Trace("cc:detail") << "CC lookup(ap): " << lup << std::endl;
      if(!lup.isNull()) {
        propagate(inputEq, lup);
      } else {
        Trace("cc") << "CC lookup(ap) setting to " << inputEq << std::endl;
        setLookup(ap, inputEq);
        for(std::vector<Cid>::iterator i = ap.begin(); i != ap.end(); ++i) {
          appendToUseList(*i, inputEq);
        }
      }
    }
    if(isApplication(t)) {
      std::vector<Cid> ap = buildRepresentativesOfApply(t);

      TNode lup = lookup(ap);
      Trace("cc:detail") << "CC lookup(ap): " << lup << std::endl;
      if(!lup.isNull()) {
        propagate(inputEq, lup);
      } else {
        Trace("cc") << "CC lookup(ap) setting to " << inputEq << std::endl;
        setLookup(ap, inputEq);
        for(std::vector<Cid>::iterator i = ap.begin(); i != ap.end(); ++i) {
          appendToUseList(*i, inputEq);
        }
      }
    }
  }
}/* merge() */


template <class OutputChannel>
std::vector<typename CongruenceClosure<OutputChannel>::Cid> CongruenceClosure<OutputChannel>::buildRepresentativesOfApply(Cid apply)
  throw(AssertionException) {
  Assert(isApplication(apply));
  TNode n = node(apply);
  Assert(isCongruenceOperator(n));
  std::vector<Cid> argspb;
  Assert(node(find(cid(n.getOperator()))) == n.getOperator(),
         "CongruenceClosure:: bad state: "
         "function symbol (or other congruence operator) merged with another");
  if(n.getMetaKind() == kind::metakind::PARAMETERIZED) {
    argspb.push_back(cid(n.getOperator()));
  }
  for(TNode::iterator i = n.begin(); i != n.end(); ++i) {
    argspb.push_back(cid(*i));
  }
  return argspb;
}/* buildRepresentativesOfApply() */

template <class OutputChannel>
void CongruenceClosure<OutputChannel>::propagate(Cid s, Cid t) {
#if 0
  Trace("cc:detail") << "=== doing a round of propagation ===" << std::endl
                     << "the \"seed\" propagation is: " << seed << std::endl;

  std::list<Cid> pending;

  Trace("cc") << "seed propagation with: " << seed << std::endl;
  pending.push_back(seed);
  do { // while(!pending.empty())
    Node e = pending.front();
    pending.pop_front();

    Trace("cc") << "=== top of propagate loop ===" << std::endl
                << "=== e is " << e << " ===" << std::endl;

    TNode a, b;
    if(e.getKind() == kind::EQUAL ||
       e.getKind() == kind::IFF) {
      // e is an equality a = b (a, b are constants)

      a = e[0];
      b = e[1];

      Trace("cc:detail") << "propagate equality: " << a << " == " << b << std::endl;
    } else {
      // e is a tuple ( apply f A... = a , apply f B... = b )

      Trace("cc") << "propagate tuple: " << e << "\n";

      Assert(e.getKind() == kind::TUPLE);

      Assert(e.getNumChildren() == 2);
      Assert(e[0].getKind() == kind::EQUAL ||
             e[0].getKind() == kind::IFF);
      Assert(e[1].getKind() == kind::EQUAL ||
             e[1].getKind() == kind::IFF);

      // tuple is (eq, lookup)
      a = e[0][1];
      b = e[1][1];

      Assert(!isCongruenceOperator(a));
      Assert(!isCongruenceOperator(b));

      Trace("cc") << "                 ( " << a << " , " << b << " )" << std::endl;
    }

    if(Debug.isOn("cc")) {
      Trace("cc:detail") << "=====at start=====" << std::endl
                         << "a          :" << a << std::endl
                         << "NORMALIZE a:" << normalize(a) << std::endl
                         << "b          :" << b << std::endl
                         << "NORMALIZE b:" << normalize(b) << std::endl
                         << "alreadyCongruent?:" << areCongruent(a,b) << std::endl;
    }

    // change from paper: need to normalize() here since in our
    // implementation, a and b can be applications
    Node ap = find(a), bp = find(b);
    if(ap != bp) {

      Trace("cc:detail") << "EC[a] == " << ap << std::endl
                         << "EC[b] == " << bp << std::endl;

      { // w.l.o.g., |classList ap| <= |classList bp|
        ClassLists::iterator cl_api = d_classList.find(ap);
        ClassLists::iterator cl_bpi = d_classList.find(bp);
        unsigned sizeA = (cl_api == d_classList.end() ? 0 : (*cl_api).second->size());
        unsigned sizeB = (cl_bpi == d_classList.end() ? 0 : (*cl_bpi).second->size());
        Trace("cc") << "sizeA == " << sizeA << "  sizeB == " << sizeB << std::endl;
        if(sizeA > sizeB) {
          Trace("cc") << "swapping..\n";
          TNode tmp = ap; ap = bp; bp = tmp;
          tmp = a; a = b; b = tmp;
        }
      }

      { // class list handling
        ClassLists::const_iterator cl_bpi = d_classList.find(bp);
        ClassList* cl_bp;
        if(cl_bpi == d_classList.end()) {
          cl_bp = new(d_context->getCMM()) ClassList(true, d_context, context::ContextMemoryAllocator<TNode>(d_context->getCMM()));
          d_classList.insertDataFromContextMemory(bp, cl_bp);
          Trace("cc:detail") << "CC in prop alloc classlist for " << bp << std::endl;
        } else {
          cl_bp = (*cl_bpi).second;
        }
        // we don't store 'ap' in its own class list; so process it here
        Trace("cc:detail") << "calling mergeproof/merge1 " << ap
                           << "   AND   " << bp << std::endl;
        mergeProof(a, b, e);
        ufmerge(ap, bp);

        Trace("cc") << " adding ap == " << ap << std::endl
                    << "   to class list of " << bp << std::endl;
        cl_bp->push_back(ap);
        ClassLists::const_iterator cli = d_classList.find(ap);
        if(cli != d_classList.end()) {
          const ClassList* cl = (*cli).second;
          for(ClassList::const_iterator i = cl->begin();
              i != cl->end();
              ++i) {
            TNode c = *i;
            if(Debug.isOn("cc")) {
              Debug("cc") << "c is " << c << "\n"
                          << " from cl of " << ap << std::endl;
              Debug("cc") << " it's find ptr is: " << find(c) << std::endl;
            }
            Assert(find(c) == ap);
            Trace("cc:detail") << "calling merge2 " << c << bp << std::endl;
            ufmerge(c, bp);
            // move c from classList(ap) to classlist(bp);
            //i = cl.erase(i);// difference from paper: don't need to erase
            Trace("cc") << " adding c to class list of " << bp << std::endl;
            cl_bp->push_back(c);
          }
        }
        Trace("cc:detail") << "bottom\n";
      }

      { // use list handling
        if(Debug.isOn("cc:detail")) {
          Debug("cc:detail") << "ap is " << ap << std::endl;
          Debug("cc:detail") << "find(ap) is " << find(ap) << std::endl;
          Debug("cc:detail") << "CC in prop go through useList of " << ap << std::endl;
        }
        UseLists::iterator usei = d_useList.find(ap);
        if(usei != d_useList.end()) {
          UseList* ul = (*usei).second;
          //for( f(c1,c2)=c in UseList(ap) )
          for(UseList::const_iterator i = ul->begin();
              i != ul->end();
              ++i) {
            TNode eq = *i;
            Trace("cc") << "CC  -- useList: " << eq << std::endl;
            Assert(eq.getKind() == kind::EQUAL ||
                   eq.getKind() == kind::IFF);
            // change from paper
            // use list elts can have form (apply c..) = x  OR  x = (apply c..)
            Assert(isCongruenceOperator(eq[0]) ||
                   isCongruenceOperator(eq[1]));
            // do for each side that is an application
            for(int side = 0; side <= 1; ++side) {
              if(!isCongruenceOperator(eq[side])) {
                continue;
              }

              Node cp = buildRepresentativesOfApply(eq[side]);

              // if lookup(c1',c2') is some f(d1,d2)=d then
              TNode n = lookup(cp);

              Trace("cc:detail") << "CC     -- c' is " << cp << std::endl;

              if(!n.isNull()) {
                Trace("cc:detail") << "CC     -- lookup(c') is " << n << std::endl;
                // add (f(c1,c2)=c,f(d1,d2)=d) to pending
                Node tuple = NodeManager::currentNM()->mkNode(kind::TUPLE, eq, n);
                Trace("cc") << "CC add tuple to pending: " << tuple << std::endl;
                pending.push_back(tuple);
                // remove f(c1,c2)=c from UseList(ap)
                Trace("cc:detail") << "supposed to remove " << eq << std::endl
                                   << "  from UseList of " << ap << std::endl;
                //i = ul.erase(i);// difference from paper: don't need to erase
              } else {
                Trace("cc") << "CC     -- lookup(c') is null" << std::endl;
                Trace("cc") << "CC     -- setlookup(c') to " << eq << std::endl;
                // set lookup(c1',c2') to f(c1,c2)=c
                setLookup(cp, eq);
                // move f(c1,c2)=c from UseList(ap) to UseList(b')
                //i = ul.erase(i);// difference from paper: don't need to erase
                appendToUseList(bp, eq);
              }
            }
          }
        }
      }/* use lists */
      Trace("cc:detail") << "CC in prop done with useList of " << ap << std::endl;
    } else {
      Trace("cc:detail") << "CCs the same ( == " << ap << "), do nothing." << std::endl;
    }

    if(Debug.isOn("cc")) {
      Debug("cc") << "=====at end=====" << std::endl
                  << "a          :" << a << std::endl
                  << "NORMALIZE a:" << normalize(a) << std::endl
                  << "b          :" << b << std::endl
                  << "NORMALIZE b:" << normalize(b) << std::endl
                  << "alreadyCongruent?:" << areCongruent(a,b) << std::endl;
    }
    Assert(areCongruent(a, b));
  } while(!pending.empty());
#endif /* 0 */
}/* propagate() */

#if 0

template <class OutputChannel>
void CongruenceClosure<OutputChannel>::ufmerge(TNode ec1, TNode ec2) {
  /*
  if(Debug.isOn("cc:detail")) {
    Debug("cc:detail") << "  -- merging " << ec1
                       << (d_careSet.find(ec1) == d_careSet.end() ?
                           " -- NOT in care set" : " -- IN CARE SET") << std::endl
                       << "         and " << ec2
                       << (d_careSet.find(ec2) == d_careSet.end() ?
                           " -- NOT in care set" : " -- IN CARE SET") << std::endl;
  }
  */

  Trace("cc") << "CC setting rep of " << ec1 << std::endl;
  Trace("cc") << "CC             to " << ec2 << std::endl;

  /* can now be applications
  Assert(!isCongruenceOperator(ec1));
  Assert(!isCongruenceOperator(ec2));
  */

  Assert(find(ec1) != ec2);
  //Assert(find(ec1) == ec1);
  Assert(find(ec2) == ec2);

  d_representative.set(ec1, ec2);

  if(d_careSet.find(ec1) != d_careSet.end()) {
    d_careSet.insert(ec2);
    d_out->notifyCongruence(ec1, ec2);
  }
}/* ufmerge() */


template <class OutputChannel>
void CongruenceClosure<OutputChannel>::mergeProof(TNode a, TNode b, TNode e) {
  Trace("cc") << "  -- merge-proofing " << a << "\n"
              << "                and " << b << "\n"
              << "               with " << e << "\n";

  // proof forest gets a -> b labeled with e

  // first reverse all the edges in proof forest to root of this proof tree
  Trace("cc") << "CC PROOF reversing proof tree\n";
  // c and p are child and parent in (old) proof tree
  Node c = a, p = d_proof[a], edgePf = d_proofLabel[a];
  // when we hit null p, we're at the (former) root
  Trace("cc") << "CC PROOF start at c == " << c << std::endl
              << "                  p == " << p << std::endl
              << "             edgePf == " << edgePf << std::endl;
  while(!p.isNull()) {
    // in proof forest,
    // we have edge   c --edgePf-> p
    // turn it into   c <-edgePf-- p

    Node pParSave = d_proof[p];
    Node pLabelSave = d_proofLabel[p];
    d_proof.set(p, c);
    d_proofLabel.set(p, edgePf);
    c = p;
    p = pParSave;
    edgePf = pLabelSave;
    Trace("cc") << "CC PROOF now   at c == " << c << std::endl
                << "                  p == " << p << std::endl
                << "             edgePf == " << edgePf << std::endl;
  }

  // add an edge from a to b
  d_proof.set(a, b);
  d_proofLabel.set(a, e);
}/* mergeProof() */

#endif /* 0 */

template <class OutputChannel>
typename CongruenceClosure<OutputChannel>::Cid CongruenceClosure<OutputChannel>::normalize(Cid t) const
  throw(AssertionException) {
  Trace("cc:detail") << "normalize " << t << std::endl;
#if 0
  if(!isCongruenceOperator(t)) {// t is a constant
    t = find(t);
    Trace("cc:detail") << "  find " << t << std::endl;
    return t;
  } else {// t is an apply
    NodeBuilder<> apb(kind::TUPLE);
    Assert(normalize(t.getOperator()) == t.getOperator(),
           "CongruenceClosure:: bad state: "
           "function symbol merged with another");
    if(t.getMetaKind() == kind::metakind::PARAMETERIZED) {
      apb << t.getOperator();
    }
    Node n;
    bool allConstants = (!isCongruenceOperator(n));
    for(TNode::iterator i = t.begin(); i != t.end(); ++i) {
      TNode c = *i;
      n = normalize(c);
      apb << n;
      allConstants = (allConstants && !isCongruenceOperator(n));
    }

    Node ap = apb;
    Trace("cc:detail") << "  got ap " << ap << std::endl;

    Node theLookup = lookup(ap);
    if(allConstants && !theLookup.isNull()) {
      Assert(theLookup.getKind() == kind::EQUAL ||
             theLookup.getKind() == kind::IFF);
      Assert(isCongruenceOperator(theLookup[0]));
      Assert(!isCongruenceOperator(theLookup[1]));
      return find(theLookup[1]);
    } else {
      NodeBuilder<> fa(t.getKind());
      for(Node::iterator i = ap.begin(); i != ap.end(); ++i) {
        fa << *i;
      }
      // ensure a hard Node link exists for the return
      Node n = fa;
      return n;
    }
  }
#else /* 0 */
  return t;
#endif /* 0 */
}/* normalize() */

#if 0

// This is the find() operation for the auxiliary union-find.  This
// union-find is not context-dependent, as it's used only during
// explain().  It does path compression.
template <class OutputChannel>
Node CongruenceClosure<OutputChannel>::highestNode(TNode a, UnionFind_t& unionFind) const
  throw(AssertionException) {
  UnionFind_t::iterator i = unionFind.find(a);
  if(i == unionFind.end()) {
    return a;
  } else {
    return unionFind[a] = highestNode((*i).second, unionFind);
  }
}/* highestNode() */


template <class OutputChannel>
void CongruenceClosure<OutputChannel>::explainAlongPath(TNode a, TNode c, PendingProofList_t& pending, UnionFind_t& unionFind, std::list<Node>& pf)
  throw(AssertionException) {

  a = highestNode(a, unionFind);
  Assert(c == highestNode(c, unionFind));

  while(a != c) {
    Node b = d_proof[a];
    Node e = d_proofLabel[a];
    if(e.getKind() == kind::EQUAL ||
       e.getKind() == kind::IFF) {
      pf.push_back(e);
    } else {
      Assert(e.getKind() == kind::TUPLE);
      pf.push_back(e[0]);
      pf.push_back(e[1]);
      Assert(isCongruenceOperator(e[0][0]));
      Assert(!isCongruenceOperator(e[0][1]));
      Assert(isCongruenceOperator(e[1][0]));
      Assert(!isCongruenceOperator(e[1][1]));
      Assert(e[0][0].getNumChildren() == e[1][0].getNumChildren());
      Assert(e[0][0].getOperator() == e[1][0].getOperator(),
             "CongruenceClosure:: bad state: function symbols should be equal");
      // shouldn't have to prove this for operators
      //pending.push_back(std::make_pair(e[0][0].getOperator(), e[1][0].getOperator()));
      for(int i = 0, nc = e[0][0].getNumChildren(); i < nc; ++i) {
        pending.push_back(std::make_pair(e[0][0][i], e[1][0][i]));
      }
    }
    unionFind[a] = b;
    a = highestNode(b, unionFind);
  }
}/* explainAlongPath() */


template <class OutputChannel>
Node CongruenceClosure<OutputChannel>::nearestCommonAncestor(TNode a, TNode b, UnionFind_t& unionFind)
  throw(AssertionException) {
  SeenSet_t seen;

  Assert(find(a) == find(b));

  do {
    a = highestNode(a, unionFind);
    seen.insert(a);
    a = d_proof[a];
  } while(!a.isNull());

  for(;;) {
    b = highestNode(b, unionFind);
    if(seen.find(b) != seen.end()) {
      return b;
    }
    b = d_proof[b];

    Assert(!b.isNull());
  }
}/* nearestCommonAncestor() */

#endif /* 0 */


template <class OutputChannel>
Node CongruenceClosure<OutputChannel>::explain(Node a, Node b)
  throw(CongruenceClosureException, AssertionException) {
#if 0
  Assert(a != b);

  if(!areCongruent(a, b)) {
    Debug("cc") << "explain: " << a << std::endl << " and   : " << b << std::endl;
    throw CongruenceClosureException("asked to explain() congruence of nodes "
                                     "that aren't congruent");
  }

  if(isCongruenceOperator(a)) {
    a = replace(flatten(a));
  }
  if(isCongruenceOperator(b)) {
    b = replace(flatten(b));
  }

  PendingProofList_t pending;
  UnionFind_t unionFind;
  std::list<Node> terms;

  pending.push_back(std::make_pair(a, b));

  Trace("cc") << "CC EXPLAINING " << a << " == " << b << std::endl;

  do {// while(!pending.empty())
    std::pair<Node, Node> eq = pending.front();
    pending.pop_front();

    a = eq.first;
    b = eq.second;

    Node c = nearestCommonAncestor(a, b, unionFind);

    explainAlongPath(a, c, pending, unionFind, terms);
    explainAlongPath(b, c, pending, unionFind, terms);
  } while(!pending.empty());

  if(Trace.isOn("cc")) {
    Trace("cc") << "CC EXPLAIN final proof has size " << terms.size() << std::endl;
  }

  NodeBuilder<> pf(kind::AND);
  while(!terms.empty()) {
    Node p = terms.front();
    terms.pop_front();
    Trace("cc") << "CC EXPLAIN " << p << std::endl;
    p = proofRewrite(p);
    Trace("cc") << "   rewrite " << p << std::endl;
    if(!p.isNull()) {
      pf << p;
    }
  }

  Trace("cc") << "CC EXPLAIN done" << std::endl;

  Assert(pf.getNumChildren() > 0);

  if(pf.getNumChildren() == 1) {
    d_explanationLength.addEntry(1.0);
    return pf[0];
  } else {
    d_explanationLength.addEntry(double(pf.getNumChildren()));
    return pf;
  }
#else /* 0 */
  return Node();
#endif /* 0 */
}/* explain() */


template <class OutputChannel>
std::ostream& operator<<(std::ostream& out,
                         const CongruenceClosure<OutputChannel>& cc) {
  out << "==============================================" << std::endl;

  /*out << "Representatives:" << std::endl;
  for(typename CongruenceClosure<OutputChannel>::RepresentativeMap::const_iterator i = cc.d_representative.begin(); i != cc.d_representative.end(); ++i) {
    out << "  " << (*i).first << " => " << (*i).second << std::endl;
  }*/

  out << "ClassLists:" << std::endl;
  for(typename CongruenceClosure<OutputChannel>::ClassLists::const_iterator i = cc.d_classList.begin(); i != cc.d_classList.end(); ++i) {
    if(cc.find((*i).first) == (*i).first) {
      out << "  " << (*i).first << " =>" << std::endl;
      for(typename CongruenceClosure<OutputChannel>::ClassList::const_iterator j = (*i).second->begin(); j != (*i).second->end(); ++j) {
        out << "      " << *j << std::endl;
      }
    }
  }

  out << "UseLists:" << std::endl;
  for(typename CongruenceClosure<OutputChannel>::UseLists::const_iterator i = cc.d_useList.begin(); i != cc.d_useList.end(); ++i) {
    if(cc.find((*i).first) == (*i).first) {
      out << "  " << (*i).first << " =>" << std::endl;
      for(typename CongruenceClosure<OutputChannel>::UseList::const_iterator j = (*i).second->begin(); j != (*i).second->end(); ++j) {
        out << "      " << *j << std::endl;
      }
    }
  }

  out << "Lookup:" << std::endl;
  for(typename CongruenceClosure<OutputChannel>::LookupMap::const_iterator i = cc.d_lookup.begin(); i != cc.d_lookup.end(); ++i) {
    TNode n = (*i).second;
    out << "  " << (*i).first << " => " << n << std::endl;
  }

  out << "Care set:" << std::endl;
  for(typename CongruenceClosure<OutputChannel>::CareSet::const_iterator i = cc.d_careSet.begin(); i != cc.d_careSet.end(); ++i) {
    out << "  " << *i << std::endl;
  }

  out << "==============================================" << std::endl;

  return out;
}

}/* CVC4 namespace */

#endif /* __CVC4__UTIL__CONGRUENCE_CLOSURE_H */
