/*********************                                                        */
/*! \file instantiation_engine.cpp
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
 ** \brief Implementation of instantiation engine class
 **/

#include "theory/quantifiers_engine.h"
#include "theory/theory_engine.h"
#include "theory/uf/theory_uf_instantiator.h"

using namespace std;
using namespace CVC4;
using namespace CVC4::kind;
using namespace CVC4::context;
using namespace CVC4::theory;

Instantiator::Instantiator(context::Context* c, QuantifiersEngine* qe, Theory* th) : 
d_status( InstStrategy::STATUS_UNFINISHED ),
d_quantEngine( qe ),
d_th( th ){

}

Instantiator::~Instantiator(){
}

void Instantiator::doInstantiation( int effort ){
  d_status = InstStrategy::STATUS_SAT;
  for( int q=0; q<d_quantEngine->getNumQuantifiers(); q++ ){
    Node f = d_quantEngine->getQuantifier( q );
    if( d_quantEngine->getActive( f ) && hasConstraintsFrom( f ) ){
      int d_quantStatus = process( f, effort );
      InstStrategy::updateStatus( d_status, d_quantStatus );
      for( int i=0; i<(int)d_instStrategies.size(); i++ ){
        if( isActiveStrategy( d_instStrategies[i] ) ){
          Debug("inst-engine-inst") << d_instStrategies[i]->identify() << " process " << effort << std::endl;
          //call the instantiation strategy's process method
          d_quantStatus = d_instStrategies[i]->process( f, effort );
          Debug("inst-engine-inst") << "  -> status is " << d_quantStatus << std::endl;
          InstStrategy::updateStatus( d_status, d_quantStatus );
        }
      }
    }
  }
}

void Instantiator::resetInstantiationStrategies(){
  for( int i=0; i<(int)d_instStrategies.size(); i++ ){
    if( isActiveStrategy( d_instStrategies[i] ) ){
      d_instStrategies[i]->resetInstantiationRound();
    }
  }
}

void Instantiator::setHasConstraintsFrom( Node f ){
  d_hasConstraints[f] = true;
  if( !d_quantEngine->hasOwner( f ) ){
    d_quantEngine->setOwner( f, getTheory() );
  }else if( d_quantEngine->getOwner( f )!=getTheory() ){
    d_quantEngine->setOwner( f, NULL );
  }
}

bool Instantiator::hasConstraintsFrom( Node f ) { 
  return d_hasConstraints.find( f )!=d_hasConstraints.end() && d_hasConstraints[f]; 
}

bool Instantiator::isOwnerOf( Node f ){
  return d_quantEngine->hasOwner( f ) && d_quantEngine->getOwner( f )==getTheory();
}

QuantifiersEngine::QuantifiersEngine(context::Context* c, TheoryEngine* te):
d_te( te ),
d_active( c ){
  d_eq_query = NULL;
}

Instantiator* QuantifiersEngine::getInstantiator( int id ){
  return d_te->getTheory( id )->getInstantiator();
}

void QuantifiersEngine::check( Theory::Effort e ){
  if( e==Theory::FULL_EFFORT ){
    ++(d_statistics.d_instantiation_rounds);
  }
  //std::cout << "Instantiation Round" << std::endl;
  for( int i=0; i<(int)d_modules.size(); i++ ){
    d_modules[i]->check( e );
  }
}
void QuantifiersEngine::registerQuantifier( Node f ){
  if( std::find( d_quants.begin(), d_quants.end(), f )==d_quants.end() ){
    ++(d_statistics.d_num_quant);
    d_quants.push_back( f );
    for( int i=0; i<(int)f[0].getNumChildren(); i++ ){
      d_vars[f].push_back( f[0][i] );
      //make instantiation constants
      Node ic = NodeManager::currentNM()->mkInstConstant( f[0][i].getType() );
      d_inst_constants_map[ic] = f;
      d_inst_constants[ f ].push_back( ic );
      ////set the var number attribute
      //InstVarNumAttribute ivna;
      //ic.setAttribute(ivna,i);
    }
    Assert( f.getKind()==FORALL );
    for( int i=0; i<(int)d_modules.size(); i++ ){
      d_modules[i]->registerQuantifier( f );
    }
  }
}

void QuantifiersEngine::assertNode( Node f ){
  Assert( f.getKind()==FORALL );
  for( int i=0; i<(int)d_modules.size(); i++ ){
    d_modules[i]->assertNode( f );
  }
}

bool QuantifiersEngine::addLemma( Node lem ){
  //AJR: the following check is necessary until FULL_CHECK is guarenteed after d_out->lemma(...)
  Debug("inst-engine-debug") << "Adding lemma : " << lem << std::endl;
  lem = Rewriter::rewrite(lem);
  if( d_lemmas_produced.find( lem )==d_lemmas_produced.end() ){
    //d_curr_out->lemma( lem );
    d_lemmas_produced[ lem ] = true;
    d_lemmas_waiting.push_back( lem );
    Debug("inst-engine-debug") << "Added lemma : " << lem << std::endl;
    return true;
  }else{
    Debug("inst-engine-debug") << "Duplicate." << std::endl;
    return false;
  }
}

bool QuantifiersEngine::addInstantiation( Node f, std::vector< Node >& terms )
{
  Assert( f.getKind()==FORALL );
  Assert( !f.hasAttribute(InstConstantAttribute()) );
  Assert( d_vars[f].size()==terms.size() && d_vars[f].size()==f[0].getNumChildren() );
  Node body = f[ 1 ].substitute( d_vars[f].begin(), d_vars[f].end(), 
                                 terms.begin(), terms.end() ); 
  NodeBuilder<> nb(kind::OR);
  nb << f.notNode() << body;
  Node lem = nb;
  if( addLemma( lem ) ){
    //std::cout << "***& Instantiate " << f << " with " << std::endl;
    //for( int i=0; i<(int)terms.size(); i++ ){
    //  std::cout << "   " << terms[i] << std::endl;
    //}

    //std::cout << "**INST" << std::endl;
    Debug("inst-engine") << "*** Instantiate " << f << " with " << std::endl;
    //std::cout << "*** Instantiate " << f << " with " << std::endl;
    uint64_t maxInstLevel = 0;
    for( int i=0; i<(int)terms.size(); i++ ){
      if( terms[i].hasAttribute(InstConstantAttribute()) ){
        std::cout << "***& Instantiate " << f << " with " << std::endl;
        for( int i=0; i<(int)terms.size(); i++ ){
          std::cout << "   " << terms[i] << std::endl;
        }
        std::cout << "unknown ";
        exit( 19 );
      }else{
        Debug("inst-engine") << "   " << terms[i];
        //std::cout << "   " << terms[i] << std::endl;
        //Debug("inst-engine") << " " << terms[i].getAttribute(InstLevelAttribute());
        Debug("inst-engine") << std::endl;
        if( terms[i].hasAttribute(InstLevelAttribute()) ){
          if( terms[i].getAttribute(InstLevelAttribute())>maxInstLevel ){
            maxInstLevel = terms[i].getAttribute(InstLevelAttribute()); 
          }
        }else{
          setInstantiationLevelAttr( terms[i], 0 );
        }
      }
    }
    setInstantiationLevelAttr( body, maxInstLevel+1 );
    ++(d_statistics.d_instantiations);
    d_statistics.d_total_inst_var.setData( d_statistics.d_total_inst_var.getData() + (int)terms.size() );
    d_statistics.d_max_instantiation_level.maxAssign( maxInstLevel+1 );
    return true;
  }else{
    ++(d_statistics.d_inst_duplicate);
    return false;
  }
}

bool QuantifiersEngine::addInstantiation( Node f, InstMatch* m, bool addSplits ){
  std::vector< Node > vars;
  getInstantiationConstantsFor( f, vars );
  std::vector< Node > match;
  m->computeTermVec( this, vars, match );
  //std::cout << "done" << std::endl;
  //std::cout << "m's quant is " << m->getQuantifier() << std::endl;
  //std::cout << "*** Instantiate " << m->getQuantifier() << " with " << std::endl;
  //for( int i=0; i<(int)m->d_match.size(); i++ ){
  //  std::cout << "   " << m->d_match[i] << std::endl;
  //}

  if( addInstantiation( f, match ) ){
    d_statistics.d_total_inst_var_unspec.setData( d_statistics.d_total_inst_var_unspec.getData() + (int)vars.size() - m->d_map.size() );
    if( (int)vars.size()!=m->d_map.size() ){
      //std::cout << "Unspec. " << std::endl;
      //std::cout << "*** Instantiate " << m->getQuantifier() << " with " << std::endl;
      //for( int i=0; i<(int)m->d_match.size(); i++ ){
      //  std::cout << "   " << m->d_match[i] << std::endl;
      //}
      ++(d_statistics.d_inst_unspec);
    }
    //if( addSplits ){
    //  for( std::map< Node, Node >::iterator it = m->d_splits.begin(); it != m->d_splits.end(); ++it ){
    //    addSplitEquality( it->first, it->second, true, true );
    //  }
    //}
    return true;
  }
  return false;
}

bool QuantifiersEngine::addSplit( Node n, bool reqPhase, bool reqPhasePol ){
  n = Rewriter::rewrite( n );
  Node lem = NodeManager::currentNM()->mkNode( OR, n, n.notNode() );
  if( addLemma( lem ) ){
    ++(d_statistics.d_splits);
    Debug("inst-engine") << "*** Add split " << n<< std::endl;
    //if( reqPhase ){
    //  d_curr_out->requirePhase( n, reqPhasePol );
    //}
    return true;
  }
  return false;
}

bool QuantifiersEngine::addSplitEquality( Node n1, Node n2, bool reqPhase, bool reqPhasePol ){
  //Assert( !n1.hasAttribute(InstConstantAttribute()) );
  //Assert( !n2.hasAttribute(InstConstantAttribute()) );
  //Assert( !areEqual( n1, n2 ) );
  //Assert( !areDisequal( n1, n2 ) );
  Kind knd = n1.getType()==NodeManager::currentNM()->booleanType() ? IFF : EQUAL;
  Node fm = NodeManager::currentNM()->mkNode( knd, n1, n2 );
  return addSplit( fm );
}

void QuantifiersEngine::flushLemmas( OutputChannel* out ){
  for( int i=0; i<(int)d_lemmas_waiting.size(); i++ ){
    out->lemma( d_lemmas_waiting[i] );
  }
  d_lemmas_waiting.clear();
}

Node QuantifiersEngine::getSkolemizedBody( Node f ){
  Assert( f.getKind()==FORALL );
  if( d_skolem_body.find( f )==d_skolem_body.end() ){
    for( int i=0; i<(int)f[0].getNumChildren(); i++ ){
      Node skv = NodeManager::currentNM()->mkSkolem( f[0][i].getType() );
      d_skolem_constants[ f ].push_back( skv );
    }
    d_skolem_body[ f ] = f[ 1 ].substitute( d_vars[f].begin(), d_vars[f].end(), 
                                            d_skolem_constants[ f ].begin(), d_skolem_constants[ f ].end() );
    if( f.hasAttribute(InstLevelAttribute()) ){
      setInstantiationLevelAttr( d_skolem_body[ f ], f.getAttribute(InstLevelAttribute()) );
    }
  }
  return d_skolem_body[ f ];
}

Node QuantifiersEngine::getSubstitutedNode( Node n, Node f ){
  Node n2 = n.substitute( d_vars[f].begin(), d_vars[f].end(), 
                          d_inst_constants[ f ].begin(), d_inst_constants[ f ].end() ); 
  setInstantiationConstantAttr( n2, f );
  return n2;
}

void QuantifiersEngine::setInstantiationLevelAttr( Node n, uint64_t level ){
  if( !n.hasAttribute(InstLevelAttribute()) ){
    InstLevelAttribute ila;
    n.setAttribute(ila,level);
  }
  for( int i=0; i<(int)n.getNumChildren(); i++ ){
    setInstantiationLevelAttr( n[i], level );
  }
}


void QuantifiersEngine::setInstantiationConstantAttr( Node n, Node f ){
  if( !n.hasAttribute(InstConstantAttribute()) ){
    bool setAttr = false;
    if( n.getKind()==INST_CONSTANT ){
      setAttr = true;
    }else{
      for( int i=0; i<(int)n.getNumChildren(); i++ ){
        setInstantiationConstantAttr( n[i], f );
        if( n[i].hasAttribute(InstConstantAttribute()) ){
          setAttr = true;
        }
      }
    }
    if( setAttr ){
      InstConstantAttribute ica;
      n.setAttribute(ica,f);
    }
  }
}

QuantifiersEngine::Statistics::Statistics():
  d_num_quant("QuantifiersEngine::Num_Quantifiers", 0),
  d_instantiation_rounds("QuantifiersEngine::Instantiation_Rounds", 0),
  d_instantiations("QuantifiersEngine::Total_Instantiations", 0),
  d_max_instantiation_level("QuantifiersEngine::Max_Instantiation_Level", 0),
  d_splits("QuantifiersEngine::Total_Splits", 0),
  d_total_inst_var("QuantifiersEngine::Inst_Vars", 0),
  d_total_inst_var_unspec("QuantifiersEngine::Inst_Vars_Unspecified", 0),
  d_inst_unspec("QuantifiersEngine::Inst_Unspecified", 0),
  d_inst_duplicate("QuantifiersEngine::Inst_Duplicate", 0),
  d_lit_phase_req("QuantifiersEngine::lit_phase_req", 0),
  d_lit_phase_nreq("QuantifiersEngine::lit_phase_nreq", 0)
{
  StatisticsRegistry::registerStat(&d_num_quant);
  StatisticsRegistry::registerStat(&d_instantiation_rounds);
  StatisticsRegistry::registerStat(&d_instantiations);
  StatisticsRegistry::registerStat(&d_max_instantiation_level);
  StatisticsRegistry::registerStat(&d_splits);
  StatisticsRegistry::registerStat(&d_total_inst_var);
  StatisticsRegistry::registerStat(&d_total_inst_var_unspec);
  StatisticsRegistry::registerStat(&d_inst_unspec);
  StatisticsRegistry::registerStat(&d_inst_duplicate);
  StatisticsRegistry::registerStat(&d_lit_phase_req);
  StatisticsRegistry::registerStat(&d_lit_phase_nreq);
}

QuantifiersEngine::Statistics::~Statistics(){
  StatisticsRegistry::unregisterStat(&d_num_quant);
  StatisticsRegistry::unregisterStat(&d_instantiation_rounds);
  StatisticsRegistry::unregisterStat(&d_instantiations);
  StatisticsRegistry::unregisterStat(&d_max_instantiation_level);
  StatisticsRegistry::unregisterStat(&d_splits);
  StatisticsRegistry::unregisterStat(&d_total_inst_var);
  StatisticsRegistry::unregisterStat(&d_total_inst_var_unspec);
  StatisticsRegistry::unregisterStat(&d_inst_unspec);
  StatisticsRegistry::unregisterStat(&d_inst_duplicate);
  StatisticsRegistry::unregisterStat(&d_lit_phase_req);
  StatisticsRegistry::unregisterStat(&d_lit_phase_nreq);
}

Node QuantifiersEngine::getFreeVariableForInstConstant( Node n ){
  if( d_free_vars.find( n )==d_free_vars.end() ){
    //if integer or real, make zero
    TypeNode tn = n.getType();
    if( tn==NodeManager::currentNM()->integerType() || tn==NodeManager::currentNM()->realType() ){
      Rational z(0);
      d_free_vars[n] = NodeManager::currentNM()->mkConst( z );
    }else{
      d_free_vars[n] = NodeManager::currentNM()->mkVar( tn );
    }
  }
  return d_free_vars[n];
}