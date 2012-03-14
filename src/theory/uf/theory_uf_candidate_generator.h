/*********************                                                        */
/*! \file theory_uf_candidate generator.h
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
 ** \brief Theory uf candidate generator
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY_UF_CANDIDATE_GENERATOR_H
#define __CVC4__THEORY_UF_CANDIDATE_GENERATOR_H

#include "theory/quantifiers_engine.h"
#include "theory/uf/theory_uf_instantiator.h"

namespace CVC4 {
namespace theory {
namespace uf {

class CandidateGeneratorTheoryUfDisequal;

class CandidateGeneratorTheoryUf : public CandidateGenerator
{
  friend class CandidateGeneratorTheoryUfDisequal;
private:
  //operator you are looking for (null means just return the representative)
  Node d_op;
  //instantiator pointer
  InstantiatorTheoryUf* d_ith;
  //the equality class iterator
  EqClassIterator d_eqc;
  int d_term_iter;
  int d_term_iter_limit;
public:
  CandidateGeneratorTheoryUf( InstantiatorTheoryUf* ith, Node op ) : 
    d_op( op ), d_ith( ith ), d_term_iter( -2 ){}
  ~CandidateGeneratorTheoryUf(){}

  void resetInstantiationRound();
  void reset( Node eqc );
  Node getNextCandidate();
};

class CandidateGeneratorTheoryUfDisequal : public CandidateGenerator
{
private:
  //equivalence class iterator
  EqClassInfo::BoolMap::const_iterator d_eqci_iter;
  //equivalence class info
  EqClassInfo* d_eci;
  //candidate generator
  CandidateGeneratorTheoryUf* d_cg;
  //instantiator pointer
  InstantiatorTheoryUf* d_ith;
public:
  CandidateGeneratorTheoryUfDisequal( InstantiatorTheoryUf* ith, Node op );
  ~CandidateGeneratorTheoryUfDisequal(){}

  void resetInstantiationRound();
  void reset( Node eqc );   //should be what you want to be disequal from
  Node getNextCandidate();
};

class CandidateGeneratorTheoryUfEq : public CandidateGenerator
{
private:
  //the equality classes iterator
  EqClassesIterator d_eq;
  //equality or disequality you are trying to match for
  Node d_pattern;
  //this is the node that our pattern will be used for matching
  Node d_match_pattern;
  //einstantiator pointer
  InstantiatorTheoryUf* d_ith;
public:
  CandidateGeneratorTheoryUfEq( InstantiatorTheoryUf* ith, Node pat, Node mpat );
  ~CandidateGeneratorTheoryUfEq(){}

  void resetInstantiationRound();
  void reset( Node eqc );
  Node getNextCandidate();
};

}
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY_UF_INSTANTIATOR_H */