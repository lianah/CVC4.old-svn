/*********************                                                        */
/*! \file node_manager.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: cconway, dejan
 ** Minor contributors (to current version): taking
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief A manager for Nodes.
 **
 ** A manager for Nodes.
 **
 ** Reviewed by Chris Conway, Apr 5 2010 (bug #65).
 **/

#include "cvc4_private.h"

/* circular dependency; force node.h first */
#include "expr/attribute.h"
#include "expr/node.h"
#include "expr/type_node.h"
#include "expr/expr.h"

#ifndef __CVC4__NODE_MANAGER_H
#define __CVC4__NODE_MANAGER_H

#include <vector>
#include <string>
#include <ext/hash_set>

#include "expr/kind.h"
#include "expr/metakind.h"
#include "expr/node_value.h"
#include "context/context.h"

namespace CVC4 {

namespace expr {

// Definition of an attribute for the variable name.
// TODO: hide this attribute behind a NodeManager interface.
namespace attr {
  struct VarNameTag {};
}/* CVC4::expr::attr namespace */

typedef expr::Attribute<attr::VarNameTag, std::string> VarNameAttr;

}/* CVC4::expr namespace */

class NodeManager {
  template <unsigned nchild_thresh> friend class CVC4::NodeBuilder;
  friend class NodeManagerScope;
  friend class expr::NodeValue;

  /** Predicate for use with STL algorithms */
  struct NodeValueReferenceCountNonZero {
    bool operator()(expr::NodeValue* nv) { return nv->d_rc > 0; }
  };

  typedef __gnu_cxx::hash_set<expr::NodeValue*,
                              expr::NodeValuePoolHashFcn,
                              expr::NodeValuePoolEq> NodeValuePool;
  typedef __gnu_cxx::hash_set<expr::NodeValue*,
                              expr::NodeValueIDHashFunction,
                              expr::NodeValueEq> ZombieSet;

  static __thread NodeManager* s_current;

  NodeValuePool d_nodeValuePool;

  expr::attr::AttributeManager d_attrManager;

  /**
   * The node value we're currently freeing.  This unique node value
   * is permitted to have outstanding TNodes to it (in "soft"
   * contexts, like as a key in attribute tables), even though
   * normally it's an error to have a TNode to a node value with a
   * reference count of 0.  Being "under deletion" also enables
   * assertions that inc() is not called on it.  (A poorly-behaving
   * attribute cleanup function could otherwise create a "Node" that
   * points to the node value that is in the process of being deleted,
   * springing it back to life.)
   */
  expr::NodeValue* d_nodeUnderDeletion;

  /**
   * True iff we are in reclaimZombies().  This avoids unnecessary
   * recursion; a NodeValue being deleted might zombify other
   * NodeValues, but these shouldn't trigger a (recursive) call to
   * reclaimZombies().
   */
  bool d_inReclaimZombies;

  /**
   * Indicates that the NodeManager is in the process of being destroyed.
   * The main purpose of this is to disable certain debugging assertions
   * that might be sensitive to the order in which objects get cleaned up
   * (e.g., TNode-valued attributes that outlive their associated Node).
   * This may be true before or after the actual NodeManager destructor
   * is executing, while other associated cleanup procedures run. E.g.,
   * an object that contains a NodeManager can set
   * <code>d_inDestruction</code> by calling
   * <code>prepareToBeDestroyed</code>.
   */
  bool d_inDestruction;

  /**
   * The set of zombie nodes.  We may want to revisit this design, as
   * we might like to delete nodes in least-recently-used order.  But
   * we also need to avoid processing a zombie twice.
   */
  ZombieSet d_zombies;

  /**
   * A set of operator singletons (w.r.t.  to this NodeManager
   * instance) for operators.  Conceptually, Nodes with kind, say,
   * PLUS, are APPLYs of a PLUS operator to arguments.  This array
   * holds the set of operators for these things.  A PLUS operator is
   * a Node with kind "BUILTIN", and if you call
   * plusOperator->getConst<CVC4::Kind>(), you get kind::PLUS back.
   */
  Node d_operators[kind::LAST_KIND];

  /**
   * Look up a NodeValue in the pool associated to this NodeManager.
   * The NodeValue argument need not be a "completely-constructed"
   * NodeValue.  In particular, "non-inlined" constants are permitted
   * (see below).
   *
   * For non-CONSTANT metakinds, nv's d_kind and d_nchildren should be
   * correctly set, and d_children[0..n-1] should be valid (extant)
   * NodeValues for lookup.
   *
   * For CONSTANT metakinds, nv's d_kind should be set correctly.
   * Normally a CONSTANT would have d_nchildren == 0 and the constant
   * value inlined in the d_children space.  However, here we permit
   * "non-inlined" NodeValues to avoid unnecessary copying.  For
   * these, d_nchildren == 1, and d_nchildren is a pointer to the
   * constant value.
   *
   * The point of this complex design is to permit efficient lookups
   * (without fully constructing a NodeValue).  In the case that the
   * argument is not fully constructed, and this function returns
   * NULL, the caller should fully construct an equivalent one before
   * calling poolInsert().  NON-FULLY-CONSTRUCTED NODEVALUES are not
   * permitted in the pool!
   */
  inline expr::NodeValue* poolLookup(expr::NodeValue* nv) const;

  /**
   * Insert a NodeValue into the NodeManager's pool.
   *
   * It is an error to insert a NodeValue already in the pool.
   * Enquire first with poolLookup().
   */
  inline void poolInsert(expr::NodeValue* nv);

  /**
   * Remove a NodeValue from the NodeManager's pool.
   *
   * It is an error to request the removal of a NodeValue from the
   * pool that is not in the pool.
   */
  inline void poolRemove(expr::NodeValue* nv);

  /**
   * Determine if nv is currently being deleted by the NodeManager.
   */
  inline bool isCurrentlyDeleting(const expr::NodeValue* nv) const {
    return d_nodeUnderDeletion == nv;
  }

  /**
   * Register a NodeValue as a zombie.
   */
  inline void markForDeletion(expr::NodeValue* nv) {
    Assert(nv->d_rc == 0);

    // if d_reclaiming is set, make sure we don't call
    // reclaimZombies(), because it's already running.
    Debug("gc") << "zombifying node value " << nv
                << " [" << nv->d_id << "]: " << *nv
                << (d_inReclaimZombies ? " [CURRENTLY-RECLAIMING]" : "")
                << std::endl;
    d_zombies.insert(nv);// FIXME multithreading

    if(!d_inReclaimZombies) {// FIXME multithreading
      // for now, collect eagerly.  can add heuristic here later..
      if(d_zombies.size() > 5000) {
        reclaimZombies();
      }
    }
  }

  /**
   * Reclaim all zombies.
   */
  void reclaimZombies();

  /**
   * This template gives a mechanism to stack-allocate a NodeValue
   * with enough space for N children (where N is a compile-time
   * constant).  You use it like this:
   *
   *   NVStorage<4> nvStorage;
   *   NodeValue& nvStack = reinterpret_cast<NodeValue&>(nvStorage);
   *
   * ...and then you can use nvStack as a NodeValue that you know has
   * room for 4 children.
   */
  template <size_t N>
  struct NVStorage {
    expr::NodeValue nv;
    expr::NodeValue* child[N];
  };/* struct NodeManager::NVStorage<N> */

  // attribute tags
  struct TypeTag {};

  // NodeManager's attributes.  These aren't exposed outside of this
  // class; use the getters.
  typedef expr::Attribute<TypeTag, TypeNode> TypeAttr;

  /* A note on isAtomic() and isAtomicFormula() (in CVC3 parlance)..
   *
   * It has been decided for now to hold off on implementations of
   * these functions, as they may only be needed in CNF conversion,
   * where it's pointless to do a lazy isAtomic determination by
   * searching through the DAG, and storing it, since the result will
   * only be used once.  For more details see the 4/27/2010 CVC4
   * developer's meeting notes at:
   *
   * http://goedel.cims.nyu.edu/wiki/Meeting_Minutes_-_April_27,_2010#isAtomic.28.29_and_isAtomicFormula.28.29
   */
  // bool containsDecision(TNode); // is "atomic"
  // bool properlyContainsDecision(TNode); // all children are atomic

public:

  NodeManager(context::Context* ctxt);
  ~NodeManager();

  /**
   * Return true if the destructor has been invoked, or
   * <code>prepareToBeDestroyed()</code> has previously been called.
   */
  bool inDestruction() const { return d_inDestruction; }

  /** Signals that this expression manager will soon be destroyed.
   * Turns off debugging assertions that may not hold as the system
   * is being torn down.
   *
   * NOTE: It is *not* required to call this function before destroying
   * the NodeManager.
   */
  void prepareToBeDestroyed() {
    d_inDestruction = true;
  }

  /** The node manager in the current context. */
  static NodeManager* currentNM() { return s_current; }

  // general expression-builders

  /** Create a node with one child. */
  Node mkNode(Kind kind, TNode child1);
  Node* mkNodePtr(Kind kind, TNode child1);

  /** Create a node with two children. */
  Node mkNode(Kind kind, TNode child1, TNode child2);
  Node* mkNodePtr(Kind kind, TNode child1, TNode child2);

  /** Create a node with three children. */
  Node mkNode(Kind kind, TNode child1, TNode child2, TNode child3);
  Node* mkNodePtr(Kind kind, TNode child1, TNode child2, TNode child3);

  /** Create a node with four children. */
  Node mkNode(Kind kind, TNode child1, TNode child2, TNode child3,
              TNode child4);
  Node* mkNodePtr(Kind kind, TNode child1, TNode child2, TNode child3,
              TNode child4);

  /** Create a node with five children. */
  Node mkNode(Kind kind, TNode child1, TNode child2, TNode child3,
              TNode child4, TNode child5);
  Node* mkNodePtr(Kind kind, TNode child1, TNode child2, TNode child3,
              TNode child4, TNode child5);

  /** Create a node with an arbitrary number of children. */
  template <bool ref_count>
  Node mkNode(Kind kind, const std::vector<NodeTemplate<ref_count> >& children);
  template <bool ref_count>
  Node* mkNodePtr(Kind kind, const std::vector<NodeTemplate<ref_count> >& children);

  /** Create a node by applying an operator to the children */
  template <bool ref_count>
  Node mkNode(TNode opNode, const std::vector<NodeTemplate<ref_count> >& children);
  template <bool ref_count>
  Node* mkNodePtr(TNode opNode, const std::vector<NodeTemplate<ref_count> >& children);

  /**
   * Create a variable with the given name and type.  NOTE that no
   * lookup is done on the name.  If you mkVar("a", type) and then
   * mkVar("a", type) again, you have two variables.
   */
  Node mkVar(const std::string& name, const TypeNode& type);
  Node* mkVarPtr(const std::string& name, const TypeNode& type);

  /** Create a variable with the given type. */
  Node mkVar(const TypeNode& type);
  Node* mkVarPtr(const TypeNode& type);

  /** Create a skolem constant with the given type. */
  Node mkSkolem(const TypeNode& type);

  /**
   * Create a constant of type T.  It will have the appropriate
   * CONST_* kind defined for T.
   */
  template <class T>
  Node mkConst(const T&);

  template <class T>
  TypeNode mkTypeConst(const T&);

  template <class NodeClass, class T>
  NodeClass mkConstInternal(const T&);

  /** Create a node with children. */
  TypeNode mkTypeNode(Kind kind, TypeNode child1);
  TypeNode mkTypeNode(Kind kind, TypeNode child1, TypeNode child2);
  TypeNode mkTypeNode(Kind kind, TypeNode child1, TypeNode child2,
                      TypeNode child3);
  TypeNode mkTypeNode(Kind kind, const std::vector<TypeNode>& children);

  /**
   * Determine whether Nodes of a particular Kind have operators.
   * @returns true if Nodes of Kind k have operators.
   */
  static inline bool hasOperator(Kind k);

  /**
   * Get the (singleton) operator of an OPERATOR-kinded kind.  The
   * returned node n will have kind BUILTIN, and calling
   * n.getConst<CVC4::Kind>() will yield k.
   */
  inline TNode operatorOf(Kind k) {
    AssertArgument( kind::metaKindOf(k) == kind::metakind::OPERATOR, k,
                    "Kind is not an OPERATOR-kinded kind "
                    "in NodeManager::operatorOf()" );
    return d_operators[k];
  }

  /**
   * Retrieve an attribute for a node.
   *
   * @param nv the node value
   * @param attr an instance of the attribute kind to retrieve.
   * @returns the attribute, if set, or a default-constructed
   * <code>AttrKind::value_type</code> if not.
   */
  template <class AttrKind>
  inline typename AttrKind::value_type getAttribute(expr::NodeValue* nv,
                                                    const AttrKind& attr) const;

  /**
   * Check whether an attribute is set for a node.
   *
   * @param nv the node value
   * @param attr an instance of the attribute kind to check
   * @returns <code>true</code> iff <code>attr</code> is set for
   * <code>nv</code>.
   */
  template <class AttrKind>
  inline bool hasAttribute(expr::NodeValue* nv,
                           const AttrKind& attr) const;

  /**
   * Check whether an attribute is set for a node, and, if so,
   * retrieve it.
   *
   * @param nv the node value
   * @param attr an instance of the attribute kind to check
   * @param value a reference to an object of the attribute's value type.
   * <code>value</code> will be set to the value of the attribute, if it is
   * set for <code>nv</code>; otherwise, it will be set to the default
   * value of the attribute.
   * @returns <code>true</code> iff <code>attr</code> is set for
   * <code>nv</code>.
   */
  template <class AttrKind>
  inline bool getAttribute(expr::NodeValue* nv,
                           const AttrKind& attr,
                           typename AttrKind::value_type& value) const;

  /**
   * Set an attribute for a node.  If the node doesn't have the
   * attribute, this function assigns one.  If the node has one, this
   * overwrites it.
   *
   * @param nv the node value
   * @param attr an instance of the attribute kind to set
   * @param value the value of <code>attr</code> for <code>nv</code>
   */
  template <class AttrKind>
  inline void setAttribute(expr::NodeValue* nv,
                           const AttrKind& attr,
                           const typename AttrKind::value_type& value);

  /**
   * Retrieve an attribute for a TNode.
   *
   * @param n the node
   * @param attr an instance of the attribute kind to retrieve.
   * @returns the attribute, if set, or a default-constructed
   * <code>AttrKind::value_type</code> if not.
   */
  template <class AttrKind>
  inline typename AttrKind::value_type 
  getAttribute(TNode n, const AttrKind& attr) const;

  /**
   * Check whether an attribute is set for a TNode.
   *
   * @param n the node
   * @param attr an instance of the attribute kind to check
   * @returns <code>true</code> iff <code>attr</code> is set for <code>n</code>.
   */
  template <class AttrKind>
  inline bool hasAttribute(TNode n,
                           const AttrKind& attr) const;

  /**
   * Check whether an attribute is set for a TNode and, if so, retieve
   * it.
   *
   * @param n the node
   * @param attr an instance of the attribute kind to check
   * @param value a reference to an object of the attribute's value type.
   * <code>value</code> will be set to the value of the attribute, if it is
   * set for <code>nv</code>; otherwise, it will be set to the default value of
   * the attribute.
   * @returns <code>true</code> iff <code>attr</code> is set for <code>n</code>.
   */
  template <class AttrKind>
  inline bool getAttribute(TNode n,
                           const AttrKind& attr,
                           typename AttrKind::value_type& value) const;

  /**
   * Set an attribute for a node.  If the node doesn't have the
   * attribute, this function assigns one.  If the node has one, this
   * overwrites it.
   *
   * @param n the node
   * @param attr an instance of the attribute kind to set
   * @param value the value of <code>attr</code> for <code>n</code>
   */
  template <class AttrKind>
  inline void setAttribute(TNode n,
                           const AttrKind& attr,
                           const typename AttrKind::value_type& value);

  /** Get the (singleton) type for Booleans. */
  inline TypeNode booleanType();

  /** Get the (singleton) type for integers. */
  inline TypeNode integerType();

  /** Get the (singleton) type for booleans. */
  inline TypeNode realType();

  /** Get the (singleton) type for sorts. */
  inline TypeNode kindType();

  /**
   * Make a function type from domain to range.
   *
   * @param domain the domain type
   * @param range the range type
   * @returns the functional type domain -> range
   */
  inline TypeNode mkFunctionType(const TypeNode& domain, const TypeNode& range);

  /**
   * Make a function type with input types from
   * argTypes. <code>argTypes</code> must have at least one element.
   *
   * @param argTypes the domain is a tuple (argTypes[0], ..., argTypes[n])
   * @param range the range type
   * @returns the functional type (argTypes[0], ..., argTypes[n]) -> range
   */
  inline TypeNode mkFunctionType(const std::vector<TypeNode>& argTypes,
                                 const TypeNode& range);

  /**
   * Make a function type with input types from
   * <code>sorts[0..sorts.size()-2]</code> and result type
   * <code>sorts[sorts.size()-1]</code>. <code>sorts</code> must have
   * at least 2 elements.
   */
  inline TypeNode mkFunctionType(const std::vector<TypeNode>& sorts);

  /**
   * Make a predicate type with input types from
   * <code>sorts</code>. The result with be a function type with range
   * <code>BOOLEAN</code>. <code>sorts</code> must have at least one
   * element.
   */
  inline TypeNode mkPredicateType(const std::vector<TypeNode>& sorts);

  /** Make the type of bitvectors of size <code>size</code> */
  inline TypeNode mkBitVectorType(unsigned size);

  /** Make the type of arrays with the given parameterization */
  inline TypeNode mkArrayType(TypeNode indexType, TypeNode constituentType);

  /** Make a new sort. */
  inline TypeNode mkSort();

  /** Make a new sort with the given name. */
  inline TypeNode mkSort(const std::string& name);

  /**
   * Get the type for the given node.
   */
  TypeNode getType(TNode n)
    throw (TypeCheckingExceptionPrivate, AssertionException);

};

/**
 * This class changes the "current" thread-global
 * <code>NodeManager</code> when it is created and reinstates the
 * previous thread-global <code>NodeManager</code> when it is
 * destroyed, effectively maintaining a set of nested
 * <code>NodeManager</code> scopes.  This is especially useful on
 * public-interface calls into the CVC4 library, where CVC4's notion
 * of the "current" <code>NodeManager</code> should be set to match
 * the calling context.  See, for example, the implementations of
 * public calls in the <code>ExprManager</code> and
 * <code>SmtEngine</code> classes.
 *
 * The client must be careful to create and destroy
 * <code>NodeManagerScope</code> objects in a well-nested manner (such
 * as on the stack). You may create a <code>NodeManagerScope</code>
 * with <code>new</code> and destroy it with <code>delete</code>, or
 * place it as a data member of an object that is, but if the scope of
 * these <code>new</code>/<code>delete</code> pairs isn't properly
 * maintained, the incorrect "current" <code>NodeManager</code>
 * pointer may be restored after a delete.
 */
class NodeManagerScope {
  /** The old NodeManager, to be restored on destruction. */
  NodeManager *d_oldNodeManager;

public:

  NodeManagerScope(NodeManager* nm) : d_oldNodeManager(NodeManager::s_current) {
    NodeManager::s_current = nm;
    Debug("current") << "node manager scope: "
                     << NodeManager::s_current << "\n";
  }

  ~NodeManagerScope() {
    NodeManager::s_current = d_oldNodeManager;
    Debug("current") << "node manager scope: "
                     << "returning to " << NodeManager::s_current << "\n";
  }
};


template <class AttrKind>
inline typename AttrKind::value_type
NodeManager::getAttribute(expr::NodeValue* nv, const AttrKind&) const {
  return d_attrManager.getAttribute(nv, AttrKind());
}

template <class AttrKind>
inline bool NodeManager::hasAttribute(expr::NodeValue* nv,
                                      const AttrKind&) const {
  return d_attrManager.hasAttribute(nv, AttrKind());
}

template <class AttrKind>
inline bool
NodeManager::getAttribute(expr::NodeValue* nv, const AttrKind&,
                          typename AttrKind::value_type& ret) const {
  return d_attrManager.getAttribute(nv, AttrKind(), ret);
}

template <class AttrKind>
inline void
NodeManager::setAttribute(expr::NodeValue* nv, const AttrKind&,
                          const typename AttrKind::value_type& value) {
  d_attrManager.setAttribute(nv, AttrKind(), value);
}

template <class AttrKind>
inline typename AttrKind::value_type
NodeManager::getAttribute(TNode n, const AttrKind&) const {
  return d_attrManager.getAttribute(n.d_nv, AttrKind());
}

template <class AttrKind>
inline bool
NodeManager::hasAttribute(TNode n, const AttrKind&) const {
  return d_attrManager.hasAttribute(n.d_nv, AttrKind());
}

template <class AttrKind>
inline bool
NodeManager::getAttribute(TNode n, const AttrKind&,
                          typename AttrKind::value_type& ret) const {
  return d_attrManager.getAttribute(n.d_nv, AttrKind(), ret);
}

template <class AttrKind>
inline void
NodeManager::setAttribute(TNode n, const AttrKind&,
                          const typename AttrKind::value_type& value) {
  d_attrManager.setAttribute(n.d_nv, AttrKind(), value);
}


/** Get the (singleton) type for booleans. */
inline TypeNode NodeManager::booleanType() {
  return TypeNode(mkTypeConst<TypeConstant>(BOOLEAN_TYPE));
}

/** Get the (singleton) type for integers. */
inline TypeNode NodeManager::integerType() {
  return TypeNode(mkTypeConst<TypeConstant>(INTEGER_TYPE));
}

/** Get the (singleton) type for reals. */
inline TypeNode NodeManager::realType() {
  return TypeNode(mkTypeConst<TypeConstant>(REAL_TYPE));
}

/** Get the (singleton) type for sorts. */
inline TypeNode NodeManager::kindType() {
  return TypeNode(mkTypeConst<TypeConstant>(KIND_TYPE));
}

/** Make a function type from domain to range. */
inline TypeNode NodeManager::mkFunctionType(const TypeNode& domain, const TypeNode& range) {
  std::vector<TypeNode> sorts;
  sorts.push_back(domain);
  sorts.push_back(range);
  return mkFunctionType(sorts);
}

inline TypeNode NodeManager::mkFunctionType(const std::vector<TypeNode>& argTypes, const TypeNode& range) {
  Assert(argTypes.size() >= 1);
  std::vector<TypeNode> sorts(argTypes);
  sorts.push_back(range);
  return mkFunctionType(sorts);
}

inline TypeNode
NodeManager::mkFunctionType(const std::vector<TypeNode>& sorts) {
  Assert(sorts.size() >= 2);
  std::vector<TypeNode> sortNodes;
  for (unsigned i = 0; i < sorts.size(); ++ i) {
    sortNodes.push_back(sorts[i]);
  }
  return mkTypeNode(kind::FUNCTION_TYPE, sortNodes);
}

inline TypeNode
NodeManager::mkPredicateType(const std::vector<TypeNode>& sorts) {
  Assert(sorts.size() >= 1);
  std::vector<TypeNode> sortNodes;
  for (unsigned i = 0; i < sorts.size(); ++ i) {
    sortNodes.push_back(sorts[i]);
  }
  sortNodes.push_back(booleanType());
  return mkTypeNode(kind::FUNCTION_TYPE, sortNodes);
}

inline TypeNode NodeManager::mkBitVectorType(unsigned size) {
  return TypeNode(mkTypeConst<BitVectorSize>(BitVectorSize(size)));
}

inline TypeNode NodeManager::mkArrayType(TypeNode indexType,
                                         TypeNode constituentType) {
  return mkTypeNode(kind::ARRAY_TYPE, indexType, constituentType);
}

inline expr::NodeValue* NodeManager::poolLookup(expr::NodeValue* nv) const {
  NodeValuePool::const_iterator find = d_nodeValuePool.find(nv);
  if(find == d_nodeValuePool.end()) {
    return NULL;
  } else {
    return *find;
  }
}

inline void NodeManager::poolInsert(expr::NodeValue* nv) {
  Assert(d_nodeValuePool.find(nv) == d_nodeValuePool.end(),
         "NodeValue already in the pool!");
  d_nodeValuePool.insert(nv);// FIXME multithreading
}

inline void NodeManager::poolRemove(expr::NodeValue* nv) {
  Assert(d_nodeValuePool.find(nv) != d_nodeValuePool.end(),
         "NodeValue is not in the pool!");

  d_nodeValuePool.erase(nv);// FIXME multithreading
}

}/* CVC4 namespace */

#define __CVC4__NODE_MANAGER_NEEDS_CONSTANT_MAP
#include "expr/metakind.h"
#undef __CVC4__NODE_MANAGER_NEEDS_CONSTANT_MAP

#include "expr/node_builder.h"

namespace CVC4 {

// general expression-builders

inline bool NodeManager::hasOperator(Kind k) {
  switch(kind::MetaKind mk = kind::metaKindOf(k)) {

  case kind::metakind::INVALID:
  case kind::metakind::VARIABLE:
    return false;

  case kind::metakind::OPERATOR:
  case kind::metakind::PARAMETERIZED:
    return true;

  case kind::metakind::CONSTANT:
    return false;

  default:
    Unhandled(mk);
  }
}

inline TypeNode NodeManager::mkSort() {
  return NodeBuilder<0>(this, kind::SORT_TYPE).constructTypeNode();
}

inline TypeNode NodeManager::mkSort(const std::string& name) {
  TypeNode type = mkSort();
  type.setAttribute(expr::VarNameAttr(), name);
  return type;
}

inline Node NodeManager::mkNode(Kind kind, TNode child1) {
  return NodeBuilder<1>(this, kind) << child1;
}

inline Node* NodeManager::mkNodePtr(Kind kind, TNode child1) {
  NodeBuilder<1> nb(this, kind);
  nb << child1;
  return nb.constructNodePtr();
}

inline Node NodeManager::mkNode(Kind kind, TNode child1, TNode child2) {
  return NodeBuilder<2>(this, kind) << child1 << child2;
}

inline Node* NodeManager::mkNodePtr(Kind kind, TNode child1, TNode child2) {
  NodeBuilder<2> nb(this, kind);
  nb << child1 << child2;
  return nb.constructNodePtr();
}

inline Node NodeManager::mkNode(Kind kind, TNode child1, TNode child2,
                                TNode child3) {
  return NodeBuilder<3>(this, kind) << child1 << child2 << child3;
}

inline Node* NodeManager::mkNodePtr(Kind kind, TNode child1, TNode child2,
                                TNode child3) {
  NodeBuilder<3> nb(this, kind);
  nb << child1 << child2 << child3;
  return nb.constructNodePtr();
}

inline Node NodeManager::mkNode(Kind kind, TNode child1, TNode child2,
                                TNode child3, TNode child4) {
  return NodeBuilder<4>(this, kind) << child1 << child2 << child3 << child4;
}

inline Node* NodeManager::mkNodePtr(Kind kind, TNode child1, TNode child2,
                                TNode child3, TNode child4) {
  NodeBuilder<4> nb(this, kind);
  nb << child1 << child2 << child3 << child4;
  return nb.constructNodePtr();
}

inline Node NodeManager::mkNode(Kind kind, TNode child1, TNode child2,
                                TNode child3, TNode child4, TNode child5) {
  return NodeBuilder<5>(this, kind) << child1 << child2 << child3 << child4
                                   << child5;
}

inline Node* NodeManager::mkNodePtr(Kind kind, TNode child1, TNode child2,
                                TNode child3, TNode child4, TNode child5) {
  NodeBuilder<5> nb(this, kind);
  nb << child1 << child2 << child3 << child4 << child5;
  return nb.constructNodePtr();
}

// N-ary version
template <bool ref_count>
inline Node NodeManager::mkNode(Kind kind,
                                const std::vector<NodeTemplate<ref_count> >&
                                children) {
  return NodeBuilder<>(this, kind).append(children);
}

template <bool ref_count>
inline Node* NodeManager::mkNodePtr(Kind kind,
                                const std::vector<NodeTemplate<ref_count> >&
                                children) {
  return NodeBuilder<>(this, kind).append(children).constructNodePtr();
}

// N-ary version for operators
template <bool ref_count>
inline Node NodeManager::mkNode(TNode opNode,
                                const std::vector<NodeTemplate<ref_count> >&
                                children) {
  Assert(kind::metaKindOf(opNode.getKind()) == kind::metakind::PARAMETERIZED);
  NodeBuilder<> nb(this, kind::operatorKindToKind(opNode.getKind()));
  nb << opNode;
  nb.append(children);
  return nb;
}

template <bool ref_count>
inline Node* NodeManager::mkNodePtr(TNode opNode,
                                const std::vector<NodeTemplate<ref_count> >&
                                children) {
  NodeBuilder<> nb(this, kind::operatorKindToKind(opNode.getKind()));
  nb << opNode;
  nb.append(children);
  return nb.constructNodePtr();
}


inline TypeNode NodeManager::mkTypeNode(Kind kind, TypeNode child1) {
  return (NodeBuilder<1>(this, kind) << child1).constructTypeNode();
}

inline TypeNode NodeManager::mkTypeNode(Kind kind, TypeNode child1,
                                        TypeNode child2) {
  return (NodeBuilder<2>(this, kind) << child1 << child2).constructTypeNode();
}

inline TypeNode NodeManager::mkTypeNode(Kind kind, TypeNode child1,
                                        TypeNode child2, TypeNode child3) {
  return (NodeBuilder<3>(this, kind) << child1 << child2 << child3).constructTypeNode();;
}

// N-ary version for types
inline TypeNode NodeManager::mkTypeNode(Kind kind,
                                        const std::vector<TypeNode>& children) {
  return NodeBuilder<>(this, kind).append(children).constructTypeNode();
}


inline Node NodeManager::mkVar(const std::string& name, const TypeNode& type) {
  Node n = mkVar(type);
  n.setAttribute(expr::VarNameAttr(), name);
  return n;
}

inline Node* NodeManager::mkVarPtr(const std::string& name,
                                   const TypeNode& type) {
  Node* n = mkVarPtr(type);
  n->setAttribute(expr::VarNameAttr(), name);
  return n;
}

inline Node NodeManager::mkVar(const TypeNode& type) {
  Node n = NodeBuilder<0>(this, kind::VARIABLE);
  n.setAttribute(TypeAttr(), type);
  return n;
}

inline Node* NodeManager::mkVarPtr(const TypeNode& type) {
  Node* n = NodeBuilder<0>(this, kind::VARIABLE).constructNodePtr();
  n->setAttribute(TypeAttr(), type);
  return n;
}

inline Node NodeManager::mkSkolem(const TypeNode& type) {
  Node n = NodeBuilder<0>(this, kind::SKOLEM);
  n.setAttribute(TypeAttr(), type);
  return n;
}

template <class T>
Node NodeManager::mkConst(const T& val) {
  return mkConstInternal<Node, T>(val);
}

template <class T>
TypeNode NodeManager::mkTypeConst(const T& val) {
  return mkConstInternal<TypeNode, T>(val);
}

template <class NodeClass, class T>
NodeClass NodeManager::mkConstInternal(const T& val) {

  // typedef typename kind::metakind::constantMap<T>::OwningTheory theory_t;
  NVStorage<1> nvStorage;
  expr::NodeValue& nvStack = reinterpret_cast<expr::NodeValue&>(nvStorage);

  nvStack.d_id = 0;
  nvStack.d_kind = kind::metakind::ConstantMap<T>::kind;
  nvStack.d_rc = 0;
  nvStack.d_nchildren = 1;
  nvStack.d_children[0] =
    const_cast<expr::NodeValue*>(reinterpret_cast<const expr::NodeValue*>(&val));
  expr::NodeValue* nv = poolLookup(&nvStack);

  if(nv != NULL) {
    return NodeClass(nv);
  }

  nv = (expr::NodeValue*)
    std::malloc(sizeof(expr::NodeValue) + sizeof(T));
  if(nv == NULL) {
    throw std::bad_alloc();
  }

  nv->d_nchildren = 0;
  nv->d_kind = kind::metakind::ConstantMap<T>::kind;
  nv->d_id = expr::NodeValue::next_id++;// FIXME multithreading
  nv->d_rc = 0;

  //OwningTheory::mkConst(val);
  new (&nv->d_children) T(val);

  poolInsert(nv);
  Debug("gc") << "creating node value " << nv
              << " [" << nv->d_id << "]: " << *nv << "\n";

  return NodeClass(nv);
}

}/* CVC4 namespace */

#endif /* __CVC4__EXPR_MANAGER_H */
