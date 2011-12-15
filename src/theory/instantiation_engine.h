/*********************                                                        */
/*! \file instantiation_engine.h
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
 ** \brief Theory instantiator, Instantiation Engine classes
 **/

#include "cvc4_private.h"

#ifndef __CVC4__INSTANTIATION_ENGINE_H
#define __CVC4__INSTANTIATION_ENGINE_H

#include "theory/theory.h"
#include "util/hash.h"
#include "theory/inst_match.h"

#include "util/stats.h"

#include <ext/hash_set>
#include <iostream>
#include <map>

namespace CVC4 {

class TheoryEngine;
class SmtEngine;

// attribute for "contains instantiation constants from"
struct InstConstantAttributeId {};
typedef expr::Attribute<InstConstantAttributeId, Node> InstConstantAttribute;

struct InstLevelAttributeId {};
typedef expr::Attribute<InstLevelAttributeId, uint64_t> InstLevelAttribute;

namespace theory {

class InstStrategyList;
class InstantiationEngine;

class InstStrategy
{
protected:
  /** node this strategy is for */
  Node d_f;
  /** reference to the instantiation engine */
  InstantiationEngine* d_instEngine;
public:
  InstStrategy( Node f, InstantiationEngine* ie ) : d_f( f ), d_instEngine( ie ){}
  virtual ~InstStrategy(){}

  /** reset instantiation */
  virtual void resetInstantiationRound(){}
  /** process method */
  virtual bool process( int effort ){ return false; }
};

class InstStrategyModerator
{
public:
  /** quantifier this moderator is for */
  Node d_f;
public:
  InstStrategyModerator( Node f ) : d_f( f ){}
  ~InstStrategyModerator(){}
  /** vector instantiation strategies */
  std::vector< InstStrategy* > d_strats;
  /** process method */
  bool process( int effort );
};

class Instantiator{
  friend class InstantiationEngine;
protected:
  /** status */
  int d_status;
  /** quant status */
  int d_quantStatus;
  /** reference to the instantiation engine */
  InstantiationEngine* d_instEngine;
  /** reference to the theory that it looks at */
  Theory* d_th;

  /** has constraints from quantifier */
  std::map< Node, bool > d_hasConstraints;
  /** instantiation strategies */
  //std::map< Node, InstStrategyList > d_instStrategies;
  /** process quantifier */
  virtual void process( Node f, int effort ) {}
public:
  enum Status {
    STATUS_UNFINISHED,
    STATUS_UNKNOWN,
    STATUS_SAT,
  };/* enum Effort */
public:
  Instantiator(context::Context* c, InstantiationEngine* ie, Theory* th);
  ~Instantiator();

  /** get corresponding theory for this instantiator */
  Theory* getTheory() { return d_th; }
  /** check function, assertion was asserted to theory */
  virtual void check( Node assertion ){}

  /** reset instantiation */
  virtual void resetInstantiationRound() { d_status = STATUS_UNFINISHED; }
  /** do instantiation method*/
  virtual void doInstantiation( int effort );
  /** identify */
  virtual std::string identify() const { return std::string("Unknown"); }
  /** print debug information */
  virtual void debugPrint( const char* c ) {}

  /** get status */
  int getStatus() { return d_status; }
  /** update status */
  static void updateStatus( int& currStatus, int addStatus );
  /** set has constraints from quantifier f */
  void setHasConstraintsFrom( Node f );
  /** has constraints from */
  bool hasConstraintsFrom( Node f );
  /** is full owner of quantifier f? */
  bool isOwnerOf( Node f );
};/* class Instantiator */

class InstantiatorDefault;
namespace uf {
  class InstantiatorTheoryUf;
}
namespace arith {
  class InstantiatorTheoryArith;
}

class InstantiationEngine
{
  friend class Instantiator;
  friend class ::CVC4::TheoryEngine;
  friend class uf::InstantiatorTheoryUf;
  friend class arith::InstantiatorTheoryArith;
  friend class QuantMatchGenerator;
private:
  typedef context::CDMap< Node, bool, NodeHashFunction > BoolMap;
  /** theory instantiator objects for each theory */
  theory::Instantiator* d_instTable[theory::THEORY_LAST];
  /** reference to theory engine object */
  TheoryEngine* d_te;
  /** map from universal quantifiers to the list of variables */
  std::map< Node, std::vector< Node > > d_vars;
  /** map from universal quantifiers to the list of skolem constants */
  std::map< Node, std::vector< Node > > d_skolem_constants;
  /** map from universal quantifiers to their skolemized body */
  std::map< Node, Node > d_skolem_body;
  /** instantiation constants to universal quantifiers */
  std::map< Node, Node > d_inst_constants_map;
  /** map from universal quantifiers to the list of instantiation constants */
  std::map< Node, std::vector< Node > > d_inst_constants;
  /** map from universal quantifiers to their counterexample body */
  std::map< Node, Node > d_counterexample_body;
  /** map from universal quantifiers to their counterexample literals */
  std::map< Node, Node > d_ce_lit;
  /** is clausal */
  std::map< Node, bool > d_is_clausal;
  /** map from quantifiers to whether they are active */
  BoolMap d_active;
  /** lemmas produced */
  std::map< Node, bool > d_lemmas_produced;
  /** lemmas waiting */
  std::vector< Node > d_lemmas_waiting;
  /** status */
  int d_status;
  /** phase requirements for instantiation literals */
  std::map< Node, bool > d_phase_reqs;
  /** whether a particular quantifier is clausal */
  std::map< Node, bool > d_clausal;
  /** quantifier match generators */
  std::map< Node, QuantMatchGenerator* > d_qmg;
  /** free variable for instantiation constant */
  std::map< Node, Node > d_free_vars;

  /** owner of quantifiers */
  std::map< Node, Theory* > d_owner;
  /** set instantiation level */
  void setInstantiationLevel( Node n, uint64_t level );
public:
  InstantiationEngine(context::Context* c, TheoryEngine* te);
  ~InstantiationEngine();
  
  theory::Instantiator* getInstantiator( Theory* t ) { return d_instTable[t->getId()]; }
  TheoryEngine* getTheoryEngine() { return d_te; }

  /** register quantifier */
  void registerQuantifier( Node f, OutputChannel* out, Valuation& valuation );
  /** register term, f is the quantifier it belongs to */
  void registerTerm( Node n, Node f, OutputChannel* out );
  /** compute phase requirements */
  void computePhaseReqs( Node n, bool polarity );
  /** do a round of instantiation */
  bool doInstantiationRound( OutputChannel* out );

  /** add lemma lem */
  bool addLemma( Node lem );
  /** instantiate f with arguments terms */
  bool addInstantiation( Node f, std::vector< Node >& terms );
  /** do instantiation specified by m */
  bool addInstantiation( InstMatch* m, bool addSplits = false );
  /** split on node n */
  bool addSplit( Node n, bool reqPhase = false, bool reqPhasePol = true );
  /** add split equality */
  bool addSplitEquality( Node n1, Node n2, bool reqPhase = false, bool reqPhasePol = true );

  /** get the ce body f[e/x] */
  Node getCounterexampleBody( Node f ) { return d_counterexample_body[ f ]; }
  /** get the skolemized body f[e/x] */
  Node getSkolemizedBody( Node f );
  /** get the quantified variables for quantifier f */
  void getVariablesFor( Node f, std::vector< Node >& vars );
  /** get instantiation constants */
  void getInstantiationConstantsFor( Node f, std::vector< Node >& ics ) { 
    ics.insert( ics.begin(), d_inst_constants[f].begin(), d_inst_constants[f].end() ); 
  }
  /** get the i^th instantiation constant of f */
  Node getInstantiationConstant( Node f, int i ) { return d_inst_constants[f][i]; }
  /** get number of instantiation constants for f */
  int getNumInstantiationConstants( Node f ) { return (int)d_inst_constants[f].size(); }

  /** get the corresponding counterexample literal for quantified formula node n */
  Node getCounterexampleLiteralFor( Node f ) { return d_ce_lit[ f ]; }
  /** set active */
  void setActive( Node n, bool val ) { d_active[n] = val; }
  /** get active */
  bool getActive( Node n ) { return d_active[n]; }
  /** get status */
  int getStatus() { return d_status; }
  /** has added lemma */
  bool hasAddedLemma() { return !d_lemmas_waiting.empty(); }

  /** get quantifier match generator */
  QuantMatchGenerator* getMatchGenerator( Node f ) { return d_qmg[f]; }
  /** get free variable for instantiation constant */
  Node getFreeVariableForInstConstant( Node n );

  class Statistics {
  public:
    IntStat d_instantiation_rounds;
    IntStat d_instantiations;
    IntStat d_max_instantiation_level;
    IntStat d_splits;
    IntStat d_total_inst_var;
    IntStat d_total_inst_var_unspec;
    IntStat d_inst_unspec;
    IntStat d_inst_duplicate;
    Statistics();
    ~Statistics();
  };
  Statistics d_statistics;

};/* class InstantiationEngine */

}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__INSTANTIATION_ENGINE_H */
