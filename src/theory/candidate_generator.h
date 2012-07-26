/*********************                                                        */
/*! \file candidate_generator.h
 ** \verbatim
 ** Original author: ajreynol
 ** Major contributors: none
 ** Minor contributors (to current version): mdeters
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009-2012  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Theory uf candidate generator
 **/

#include "cvc4_private.h"

#ifndef __CVC4__CANDIDATE_GENERATOR_H
#define __CVC4__CANDIDATE_GENERATOR_H

#include "theory/theory.h"
#include "theory/uf/equality_engine.h"

namespace CVC4 {
namespace theory {

/** base class for generating candidates for matching */
class CandidateGenerator {
public:
  CandidateGenerator(){}
  ~CandidateGenerator(){}

  /** Get candidates functions.  These set up a context to get all match candidates.
      cg->reset( eqc );
      do{
        Node cand = cg->getNextCandidate();
        //.......
      }while( !cand.isNull() );

      eqc is the equivalence class you are searching in
  */
  virtual void reset( Node eqc ) = 0;
  virtual Node getNextCandidate() = 0;
  /** add candidate to list of nodes returned by this generator */
  virtual void addCandidate( Node n ) {}
  /** call this at the beginning of each instantiation round */
  virtual void resetInstantiationRound() = 0;
public:
  /** legal candidate */
  static bool isLegalCandidate( Node n );
};/* class CandidateGenerator */

/** candidate generator queue (for manual candidate generation) */
class CandidateGeneratorQueue : public CandidateGenerator {
private:
  std::vector< Node > d_candidates;
  int d_candidate_index;
public:
  CandidateGeneratorQueue() : d_candidate_index( 0 ){}
  ~CandidateGeneratorQueue(){}

  void addCandidate( Node n );

  void resetInstantiationRound(){}
  void reset( Node eqc );
  Node getNextCandidate();
};/* class CandidateGeneratorQueue */

class QuantifiersEngine;
class CandidateGeneratorQEDisequal;

#if 0

class CandidateGeneratorQE : public CandidateGenerator
{
  friend class CandidateGeneratorQEDisequal;
private:
  //operator you are looking for
  Node d_op;
  //instantiator pointer
  QuantifiersEngine* d_qe;
  //the equality class iterator
  eq::EqClassIterator d_eqc;
  int d_term_iter;
  int d_term_iter_limit;
private:
  Node d_retNode;
public:
  CandidateGeneratorQE( QuantifiersEngine* qe, Node op );
  ~CandidateGeneratorQE(){}

  void resetInstantiationRound();
  void reset( Node eqc );
  Node getNextCandidate();
};

#else

class CandidateGeneratorQE : public CandidateGenerator
{
  friend class CandidateGeneratorQEDisequal;
private:
  //operator you are looking for
  Node d_op;
  //instantiator pointer
  QuantifiersEngine* d_qe;
  //the equality class iterator
  std::vector< Node > d_eqc;
  int d_term_iter;
  int d_term_iter_limit;
  bool d_using_term_db;
public:
  CandidateGeneratorQE( QuantifiersEngine* qe, Node op );
  ~CandidateGeneratorQE(){}

  void resetInstantiationRound();
  void reset( Node eqc );
  Node getNextCandidate();
};

#endif

//class CandidateGeneratorQEDisequal : public CandidateGenerator
//{
//private:
//  //equivalence class
//  Node d_eq_class;
//  //equivalence class info
//  EqClassInfo* d_eci;
//  //equivalence class iterator
//  EqClassInfo::BoolMap::const_iterator d_eqci_iter;
//  //instantiator pointer
//  QuantifiersEngine* d_qe;
//public:
//  CandidateGeneratorQEDisequal( QuantifiersEngine* qe, Node eqc );
//  ~CandidateGeneratorQEDisequal(){}
//
//  void resetInstantiationRound();
//  void reset( Node eqc );   //should be what you want to be disequal from
//  Node getNextCandidate();
//};

class CandidateGeneratorQELitEq : public CandidateGenerator
{
private:
  //the equality classes iterator
  eq::EqClassesIterator d_eq;
  //equality you are trying to match equalities for
  Node d_match_pattern;
  //einstantiator pointer
  QuantifiersEngine* d_qe;
public:
  CandidateGeneratorQELitEq( QuantifiersEngine* qe, Node mpat );
  ~CandidateGeneratorQELitEq(){}

  void resetInstantiationRound();
  void reset( Node eqc );
  Node getNextCandidate();
};

class CandidateGeneratorQELitDeq : public CandidateGenerator
{
private:
  //the equality class iterator for false
  eq::EqClassIterator d_eqc_false;
  //equality you are trying to match disequalities for
  Node d_match_pattern;
  //einstantiator pointer
  QuantifiersEngine* d_qe;
public:
  CandidateGeneratorQELitDeq( QuantifiersEngine* qe, Node mpat );
  ~CandidateGeneratorQELitDeq(){}

  void resetInstantiationRound();
  void reset( Node eqc );
  Node getNextCandidate();
};


}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY_UF_INSTANTIATOR_H */
