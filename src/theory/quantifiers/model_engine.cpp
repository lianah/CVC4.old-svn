/*********************                                                        */
/*! \file model_engine.cpp
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
 ** \brief Implementation of model engine class
 **/

#include "theory/quantifiers/model_engine.h"
#include "theory/theory_engine.h"
#include "theory/uf/equality_engine.h"
#include "theory/uf/theory_uf.h"
#include "theory/uf/theory_uf_strong_solver.h"
#include "theory/uf/theory_uf_instantiator.h"

//#define ME_PRINT_WARNINGS

//#define DISABLE_EVAL_SKIP_MULTIPLE

#define USE_INDEX_ORDERING
#define EVAL_FAIL_SKIP_MULTIPLE
//#define USE_RELEVANT_DOMAIN
//#define ONE_QUANT_PER_ROUND_INST_GEN
//#define ONE_QUANT_PER_ROUND

using namespace std;
using namespace CVC4;
using namespace CVC4::kind;
using namespace CVC4::context;
using namespace CVC4::theory;
using namespace CVC4::theory::quantifiers;

RepSetIterator::RepSetIterator( QuantifiersEngine* qe, Node f, ModelEngine* model ) : d_f( f ), d_model( model ){
  //store instantiation constants
  for( size_t i=0; i<f[0].getNumChildren(); i++ ){
    d_ic.push_back( qe->getInstantiationConstant( d_f, i ) );
    d_index.push_back( 0 );
  }
  for( size_t i=0; i<f[0].getNumChildren(); i++ ){
    //store default index order
    d_index_order.push_back( i );
    d_var_order[i] = i;
    //store default domain
    d_domain.push_back( RepDomain() );
    for( int j=0; j<(int)d_model->getReps()->d_type_reps[d_f[0][i].getType()].size(); j++ ){
      d_domain[i].push_back( j );
    }
  }
}

void RepSetIterator::setIndexOrder( std::vector< int >& indexOrder ){
  d_index_order.clear();
  d_index_order.insert( d_index_order.begin(), indexOrder.begin(), indexOrder.end() );
  //make the d_var_order mapping
  for( int i=0; i<(int)d_index_order.size(); i++ ){
    d_var_order[d_index_order[i]] = i;
  }
}

void RepSetIterator::setDomain( std::vector< RepDomain >& domain ){
  d_domain.clear();
  d_domain.insert( d_domain.begin(), domain.begin(), domain.end() );
  //we are done if a domain is empty
  for( int i=0; i<(int)d_domain.size(); i++ ){
    if( d_domain[i].empty() ){
      d_index.clear();
    }
  }
}

void RepSetIterator::increment2( QuantifiersEngine* qe, int counter ){
  Assert( !isFinished() );
#ifdef DISABLE_EVAL_SKIP_MULTIPLE
  counter = (int)d_index.size()-1;
#endif
  //increment d_index
  while( counter>=0 && d_index[counter]==(int)(d_domain[counter].size()-1) ){
    counter--;
  }
  if( counter==-1 ){
    d_index.clear();
  }else{
    for( int i=(int)d_index.size()-1; i>counter; i-- ){
      d_index[i] = 0;
      d_model->clearEvalFailed( i );
    }
    d_index[counter]++;
    d_model->clearEvalFailed( counter );
  }
}

void RepSetIterator::increment( QuantifiersEngine* qe ){
  if( !isFinished() ){
    increment2( qe, (int)d_index.size()-1 );
  }
}

bool RepSetIterator::isFinished(){
  return d_index.empty();
}

void RepSetIterator::getMatch( QuantifiersEngine* ie, InstMatch& m ){
  for( int i=0; i<(int)d_index.size(); i++ ){
    m.d_map[ ie->getInstantiationConstant( d_f, d_index_order[i] ) ] = getTerm( i );
  }
}

Node RepSetIterator::getTerm( int i ){
  TypeNode tn = d_f[0][d_index_order[i]].getType();
  Assert( d_model->getReps()->d_type_reps.find( tn )!=d_model->getReps()->d_type_reps.end() );
  int index = d_index_order[i];
  return d_model->getReps()->d_type_reps[tn][d_domain[index][d_index[index]]];
}

void RepSetIterator::calculateTerms( QuantifiersEngine* qe ){
  d_terms.clear();
  for( int i=0; i<qe->getNumInstantiationConstants( d_f ); i++ ){
    d_terms.push_back( getTerm( i ) );
  }
}

void RepSetIterator::debugPrint( const char* c ){
  for( int i=0; i<(int)d_index.size(); i++ ){
    Debug( c ) << i << ": " << d_index[i] << ", (" << getTerm( i ) << " / " << d_ic[ i ] << std::endl;
  }
}

void RepSetIterator::debugPrintSmall( const char* c ){
  Debug( c ) << "RI: ";
  for( int i=0; i<(int)d_index.size(); i++ ){
    Debug( c ) << d_index[i] << ": " << getTerm( i ) << " ";
  }
  Debug( c ) << std::endl;
}

ExtModel::ExtModel( context::Context* c ) : Model( c ){

}

Node ExtModel::getValue( TNode n ){
  return n;
}

//Model Engine constructor
ModelEngine::ModelEngine( TheoryQuantifiers* th ) :
d_builder(*this){
  d_th = th;
  d_quantEngine = th->getQuantifiersEngine();
  d_ss = ((uf::TheoryUF*)d_quantEngine->getTheoryEngine()->getTheory( THEORY_UF ))->getStrongSolver();
  d_true = NodeManager::currentNM()->mkConst( true );
  d_false = NodeManager::currentNM()->mkConst( false );
}

void ModelEngine::check( Theory::Effort e ){
  if( e==Theory::EFFORT_LAST_CALL && !d_quantEngine->hasAddedLemma() ){
    //first, check if we can minimize the model further
    if( !d_ss->minimize() ){
      return;
    }
    //the following will attempt to build a model and test that it satisfies all asserted universal quantifiers
    int addedLemmas = 0;
    if( optUseModel() ){
      //check if any quantifiers are un-initialized
      for( int i=0; i<d_quantEngine->getNumAssertedQuantifiers(); i++ ){
        Node f = d_quantEngine->getAssertedQuantifier( i );
        addedLemmas += initializeQuantifier( f );
      }
    }
    if( addedLemmas==0 ){
      //quantifiers are initialized, we begin an instantiation round
      double clSet = 0;
      if( Options::current()->printModelEngine ){
        clSet = double(clock())/double(CLOCKS_PER_SEC);
        Message() << "---Model Engine Round---" << std::endl;
      }
      Debug("fmf-model-debug") << "---Begin Instantiation Round---" << std::endl;
      ++(d_statistics.d_inst_rounds);
      //reset the quantifiers engine
      d_quantEngine->resetInstantiationRound( e );
      //build the representatives
      Debug("fmf-model-debug") << "Building representatives..." << std::endl;
      buildRepresentatives();
      //construct model if optUseModel() is true
      if( optUseModel() ){
        //initialize the model
        Debug("fmf-model-debug") << "Initializing model..." << std::endl;
        initializeModel();
        //analyze the quantifiers
        Debug("fmf-model-debug") << "Analyzing quantifiers..." << std::endl;
        analyzeQuantifiers();
        //if applicable, find exceptions
        if( optInstGen() ){
          //now, see if we know that any exceptions via InstGen exist
          Debug("fmf-model-debug") << "Perform InstGen techniques for quantifiers..." << std::endl;
          for( int i=0; i<d_quantEngine->getNumAssertedQuantifiers(); i++ ){
            Node f = d_quantEngine->getAssertedQuantifier( i );
            if( d_quant_sat.find( f )==d_quant_sat.end() ){
              addedLemmas += doInstGen( f );
              if( optOneQuantPerRoundInstGen() && addedLemmas>0 ){
                break;
              }
            }
          }
          if( Options::current()->printModelEngine ){
            if( addedLemmas>0 ){
              Message() << "InstGen, added lemmas = " << addedLemmas << std::endl;
            }else{
              Message() << "No InstGen lemmas..." << std::endl;
            }
          }
          Debug("fmf-model-debug") << "---> Added lemmas = " << addedLemmas << std::endl;
        }
        if( addedLemmas==0 ){
          //if no immediate exceptions, build the model
          //  this model will be an approximation that will need to be tested via exhaustive instantiation
          Debug("fmf-model-debug") << "Building model..." << std::endl;
          for( std::map< Node, uf::UfModel >::iterator it = d_uf_model.begin(); it != d_uf_model.end(); ++it ){
            it->second.buildModel();
          }
          Debug("fmf-model-debug") << "Done building models." << std::endl;
          //print debug
          Debug("fmf-model-complete") << std::endl;
          debugPrint("fmf-model-complete");
        }
      }
      if( addedLemmas==0 ){
        //verify we are SAT by trying exhaustive instantiation
        d_triedLemmas = 0;
        d_testLemmas = 0;
        d_totalLemmas = 0;
        Debug("fmf-model-debug") << "Do exhaustive instantiation..." << std::endl;
        for( int i=0; i<d_quantEngine->getNumAssertedQuantifiers(); i++ ){
          Node f = d_quantEngine->getAssertedQuantifier( i );
          if( d_quant_sat.find( f )==d_quant_sat.end() ){
            addedLemmas += exhaustiveInstantiate( f );
            if( optOneQuantPerRound() && addedLemmas>0 ){
              break;
            }
          }
#ifdef ME_PRINT_WARNINGS
          if( addedLemmas>10000 ){
            break;
          }
#endif
        }
        Debug("fmf-model-debug") << "---> Added lemmas = " << addedLemmas << " / " << d_triedLemmas << " / ";
        Debug("fmf-model-debug") << d_testLemmas << " / " << d_totalLemmas << std::endl;
        if( Options::current()->printModelEngine ){
          Message() << "Added Lemmas = " << addedLemmas << " / " << d_triedLemmas << " / ";
          Message() << d_testLemmas << " / " << d_totalLemmas << std::endl;
          double clSet2 = double(clock())/double(CLOCKS_PER_SEC);
          Message() << "Finished model engine, time = " << (clSet2-clSet) << std::endl;
        }
#ifdef ME_PRINT_WARNINGS
        if( addedLemmas>10000 ){
          Debug("fmf-exit") << std::endl;
          debugPrint("fmf-exit");
          exit( 0 );
        }
#endif
      }
    }
    if( addedLemmas==0 ){
      //CVC4 will answer SAT
      Debug("fmf-consistent") << std::endl;
      debugPrint("fmf-consistent");
    }else{
      //otherwise, the search will continue
      d_quantEngine->flushLemmas( &d_th->getOutputChannel() );
    }
  }
}

void ModelEngine::registerQuantifier( Node f ){

}

void ModelEngine::assertNode( Node f ){

}

bool ModelEngine::optUseModel() {
  return Options::current()->fmfModelBasedInst;
}

bool ModelEngine::optOneInstPerQuantRound(){
  return Options::current()->fmfOneInstPerRound;
}

bool ModelEngine::optInstGen(){
  return Options::current()->fmfInstGen;
}

bool ModelEngine::optUseRelevantDomain(){
#ifdef USE_RELEVANT_DOMAIN
  return true;
#else
  return false;
#endif
}

bool ModelEngine::optOneQuantPerRoundInstGen(){
#ifdef ONE_QUANT_PER_ROUND_INST_GEN
  return true;
#else
  return false;
#endif
}

bool ModelEngine::optOneQuantPerRound(){
#ifdef ONE_QUANT_PER_ROUND
  return true;
#else
  return false;
#endif
}

void ModelEngine::buildModel( Model& m ){

}

int ModelEngine::initializeQuantifier( Node f ){
  if( d_quant_init.find( f )==d_quant_init.end() ){
    d_quant_init[f] = true;
    Debug("inst-fmf-init") << "Initialize " << f << std::endl;
    //add the model basis instantiation
    // This will help produce the necessary information for model completion.
    // We do this by extending distinguish ground assertions (those
    //   containing terms with "model basis" attribute) to hold for all cases.

    ////first, check if any variables are required to be equal
    //for( std::map< Node, bool >::iterator it = d_quantEngine->d_phase_reqs[f].begin();
    //    it != d_quantEngine->d_phase_reqs[f].end(); ++it ){
    //  Node n = it->first;
    //  if( n.getKind()==EQUAL && n[0].getKind()==INST_CONSTANT && n[1].getKind()==INST_CONSTANT ){
    //    Notice() << "Unhandled phase req: " << n << std::endl;
    //  }
    //}
    std::vector< Node > ics;
    std::vector< Node > terms;
    for( int j=0; j<(int)f[0].getNumChildren(); j++ ){
      Node ic = d_quantEngine->getInstantiationConstant( f, j );
      Node t = getModelBasisTerm( ic.getType() );
      ics.push_back( ic );
      terms.push_back( t );
      //calculate the basis match for f
      d_quant_basis_match[f].d_map[ ic ] = t;
    }
    ++(d_statistics.d_num_quants_init);
    //register model basis body
    Node n = d_quantEngine->getCounterexampleBody( f );
    Node gn = n.substitute( ics.begin(), ics.end(), terms.begin(), terms.end() );
    registerModelBasis( n, gn );
    //add model basis instantiation
    if( d_quantEngine->addInstantiation( f, terms ) ){
      return 1;
    }else{
      //shouldn't happen usually, but will occur if x != y is a required literal for f.
      //Notice() << "No model basis for " << f << std::endl;
      ++(d_statistics.d_num_quants_init_fail);
    }
  }
  return 0;
}

void ModelEngine::buildRepresentatives(){
  d_ra.clear();
  //collect all representatives for all types and store as representative alphabet
  for( int i=0; i<d_ss->getNumCardinalityTypes(); i++ ){
    TypeNode tn = d_ss->getCardinalityType( i );
    std::vector< Node > reps;
    d_ss->getRepresentatives( tn, reps );
    Assert( !reps.empty() );
    Debug("fmf-model-debug") << "   " << tn << " -> " << reps.size() << std::endl;
    Debug("fmf-model-debug") << "      ";
    for( int i=0; i<(int)reps.size(); i++ ){
      //if( reps[i].getAttribute(ModelBasisAttribute()) ){
      //  reps[i] = d_quantEngine->getEqualityQuery()->getInternalRepresentative( reps[i] );
      //}
      //Assert( !reps[i].getAttribute(ModelBasisAttribute()) );
      Debug("fmf-model-debug") << reps[i] << " ";
    }
    Debug("fmf-model-debug") << std::endl;
    //set them in the alphabet
    d_ra.set( tn, reps );
    if( Options::current()->printModelEngine ){
      Message() << "Cardinality( " << tn << " )" << " = " << reps.size() << std::endl;
      //Message() << d_quantEngine->getEqualityQuery()->getRepresentative( NodeManager::currentNM()->mkConst( true ) ) << std::endl;
    }
  }
}

void ModelEngine::initializeModel(){
  d_uf_model.clear();
  d_quant_model_lits.clear();
  d_quant_sat.clear();
  d_quant_uf_terms.clear();
  d_quant_inst_domain.clear();

  for( int i=0; i<(int)d_quantEngine->getNumAssertedQuantifiers(); i++ ){
    Node f = d_quantEngine->getAssertedQuantifier( i );
    //collect uf terms and initialize uf models
    collectUfTerms( f[1], d_quant_uf_terms[f] );
    for( int j=0; j<(int)d_quant_uf_terms[f].size(); j++ ){
      initializeUfModel( d_quant_uf_terms[f][j].getOperator() );
    }
    d_quant_inst_domain[f].resize( f[0].getNumChildren() );
  }
  if( optUseRelevantDomain() ){
    //add ground terms to domain (rule 1 of complete instantiation essentially uf fragment)
    for( std::map< Node, uf::UfModel >::iterator it = d_uf_model.begin(); it != d_uf_model.end(); ++it ){
      it->second.computeRelevantDomain();
    }
    //find fixed point for relevant domain computation
    bool success;
    do{
      success = true;
      for( int i=0; i<(int)d_quantEngine->getNumAssertedQuantifiers(); i++ ){
        Node f = d_quantEngine->getAssertedQuantifier( i );
        //compute the domain of relevant instantiations (rule 3 of complete instantiation, essentially uf fragment)
        if( computeRelevantInstantiationDomain( d_quantEngine->getCounterexampleBody( f ), Node::null(), -1, d_quant_inst_domain[f] ) ){
          success = false;
        }
        //extend the possible domain for functions (rule 2 of complete instantiation, essentially uf fragment)
        RepDomain range;
        if( extendFunctionDomains( d_quantEngine->getCounterexampleBody( f ), range ) ){
          success = false;
        }
      }
    }while( !success );
  }
}

void ModelEngine::analyzeQuantifiers(){
  int quantSatInit = 0;
  int nquantSatInit = 0;
  //analyze the preferences of each quantifier
  for( int i=0; i<(int)d_quantEngine->getNumAssertedQuantifiers(); i++ ){
    Node f = d_quantEngine->getAssertedQuantifier( i );
    Debug("fmf-model-prefs") << "Analyze quantifier " << f << std::endl;
    std::vector< Node > pro_con[2];
    std::vector< Node > constantSatOps;
    bool constantSatReconsider;
    //for each asserted quantifier f,
    // - determine which literals form model basis for each quantifier
    // - check which function/predicates have good and bad definitions according to f
    for( std::map< Node, bool >::iterator it = d_quantEngine->d_phase_reqs[f].begin();
         it != d_quantEngine->d_phase_reqs[f].end(); ++it ){
      Node n = it->first;
      Node gn = d_model_basis[n];
      Debug("fmf-model-req") << "   Req: " << n << " -> " << it->second << std::endl;
      //calculate preference
      int pref = 0;
      bool value;
      if( d_th->getValuation().hasSatValue( gn, value ) ){
        if( value!=it->second ){
          //store this literal as a model basis literal
          //  this literal will force a default values in model that (modulo exceptions) shows
          //  that f is satisfied by the model
          d_quant_model_lits[f].push_back( value ? n : n.notNode() );
          pref = 1;
        }else{
          pref = -1;
        }
      }
      if( pref!=0 ){
        //Store preferences for UF
        bool isConst = !n.hasAttribute(InstConstantAttribute());
        std::vector< Node > uf_terms;
        if( gn.getKind()==APPLY_UF ){
          uf_terms.push_back( gn );
          isConst = d_uf_model[gn.getOperator()].isConstant();
        }else if( gn.getKind()==EQUAL ){
          isConst = true;
          for( int j=0; j<2; j++ ){
            if( n[j].hasAttribute(InstConstantAttribute()) ){
              if( n[j].getKind()==APPLY_UF ){
                Node op = gn[j].getOperator();
                if( d_uf_model.find( op )!=d_uf_model.end() ){
                  uf_terms.push_back( gn[j] );
                  isConst = isConst && d_uf_model[op].isConstant();
                }else{
                  isConst = false;
                }
              }else{
                isConst = false;
              }
            }
          }
        }
        Debug("fmf-model-prefs") << "  It is " << ( pref==1 ? "pro" : "con" );
        Debug("fmf-model-prefs") << " the definition of " << n << std::endl;
        if( pref==1 && isConst ){
          d_quant_sat[f] = true;
          //instead, just note to the model for each uf term that f is pro its definition
          constantSatReconsider = false;
          constantSatOps.clear();
          for( int j=0; j<(int)uf_terms.size(); j++ ){
            Node op = uf_terms[j].getOperator();
            constantSatOps.push_back( op );
            if( d_uf_model[op].d_reconsider_model ){
              constantSatReconsider = true;
            }
          }
          if( !constantSatReconsider ){
            break;
          }
	      }else{
          int pcIndex = pref==1 ? 0 : 1;
          for( int j=0; j<(int)uf_terms.size(); j++ ){
            pro_con[pcIndex].push_back( uf_terms[j] );
          }
        }
      }
    }
    if( d_quant_sat.find( f )!=d_quant_sat.end() ){
      Debug("fmf-model-prefs") << "  * Constant SAT due to definition of ops: ";
      for( int i=0; i<(int)constantSatOps.size(); i++ ){
        Debug("fmf-model-prefs") << constantSatOps[i] << " ";
        d_uf_model[constantSatOps[i]].d_reconsider_model = false;
      }
      Debug("fmf-model-prefs") << std::endl;
      quantSatInit++;
      d_statistics.d_pre_sat_quant += quantSatInit;
    }else{
      nquantSatInit++;
      d_statistics.d_pre_nsat_quant += quantSatInit;
      //note quantifier's value preferences to models
      for( int k=0; k<2; k++ ){
        for( int j=0; j<(int)pro_con[k].size(); j++ ){
          Node op = pro_con[k][j].getOperator();
          d_uf_model[op].setValuePreference( f, pro_con[k][j], k==0 );
        }
      }
    }
  }
  Debug("fmf-model-prefs") << "Pre-Model Completion: Quantifiers SAT: " << quantSatInit << " / " << (quantSatInit+nquantSatInit) << std::endl;
}

int ModelEngine::doInstGen( Node f ){
  //we wish to add all known exceptions to our model basis literal(s)
  //  this will help to refine our current model.
  //This step is advantageous over exhaustive instantiation, since we are adding instantiations that involve model basis terms,
  //  effectively acting as partial instantiations instead of pointwise instantiations.
  int addedLemmas = 0;
  for( int i=0; i<(int)d_quant_model_lits[f].size(); i++ ){
    bool phase = d_quant_model_lits[f][i].getKind()!=NOT;
    Node lit = d_quant_model_lits[f][i].getKind()==NOT ? d_quant_model_lits[f][i][0] : d_quant_model_lits[f][i];
    Assert( lit.hasAttribute(InstConstantAttribute()) );
    std::vector< Node > tr_terms;
    if( lit.getKind()==APPLY_UF ){
      //only match predicates that are contrary to this one, use literal matching
      Node eq = NodeManager::currentNM()->mkNode( IFF, lit, !phase ? d_true : d_false );
      d_quantEngine->setInstantiationConstantAttr( eq, f );
      tr_terms.push_back( eq );
    }else if( lit.getKind()==EQUAL ){
      //collect trigger terms
      for( int j=0; j<2; j++ ){
        if( lit[j].hasAttribute(InstConstantAttribute()) ){
          if( lit[j].getKind()==APPLY_UF ){
            tr_terms.push_back( lit[j] );
          }else{
            tr_terms.clear();
            break;
          }
        }
      }
      if( tr_terms.size()==1 && !phase ){
        //equality between a function and a ground term, use literal matching
        tr_terms.clear();
        tr_terms.push_back( lit );
      }
    }
    //if applicable, try to add exceptions here
    if( !tr_terms.empty() ){
      //make a trigger for these terms, add instantiations
      Trigger* tr = Trigger::mkTrigger( d_quantEngine, f, tr_terms );
      //Notice() << "Trigger = " << (*tr) << std::endl;
      tr->resetInstantiationRound();
      tr->reset( Node::null() );
      //d_quantEngine->d_optInstMakeRepresentative = false;
      //d_quantEngine->d_optMatchIgnoreModelBasis = true;
      addedLemmas += tr->addInstantiations( d_quant_basis_match[f] );
    }
  }
  return addedLemmas;
}

int ModelEngine::exhaustiveInstantiate( Node f, bool useRelInstDomain ){
  int tests = 0;
  int addedLemmas = 0;
  int triedLemmas = 0;
  Debug("inst-fmf-ei") << "Add matches for " << f << "..." << std::endl;
  Debug("inst-fmf-ei") << "   Instantiation Constants: ";
  for( size_t i=0; i<f[0].getNumChildren(); i++ ){
    Debug("inst-fmf-ei") << d_quantEngine->getInstantiationConstant( f, i ) << " ";
  }
  Debug("inst-fmf-ei") << std::endl;
  if( d_quant_model_lits[f].empty() ){
    Debug("inst-fmf-ei") << "WARNING: " << f << " has no model literal definitions (is f clausified?)" << std::endl;
#ifdef ME_PRINT_WARNINGS
    Message() << "WARNING: " << f << " has no model literal definitions (is f clausified?)" << std::endl;
#endif
  }else{
    Debug("inst-fmf-ei") << "  Model literal definitions:" << std::endl;
    for( int i=0; i<(int)d_quant_model_lits[f].size(); i++ ){
      Debug("inst-fmf-ei") << "    " << d_quant_model_lits[f][i] << std::endl;
    }
  }
  d_eval_failed_lits.clear();
  d_eval_failed.clear();
  d_eval_term_model.clear();
  RepSetIterator riter( d_quantEngine, f, this );
  //set the domain for the iterator (the sufficient set of instantiations to try)
  if( useRelInstDomain ){
    riter.setDomain( d_quant_inst_domain[f] );
  }
  while( !riter.isFinished() && ( addedLemmas==0 || !optOneInstPerQuantRound() ) ){
    d_testLemmas++;
    if( optUseModel() ){
      //see if instantiation is already true in current model
      Debug("fmf-model-eval") << "Evaluating ";
      riter.debugPrintSmall("fmf-model-eval");
      //calculate represenative terms we are currently considering
      riter.calculateTerms( d_quantEngine );
      Debug("fmf-model-eval") << "Done calculating terms." << std::endl;
      tests++;
      //if evaluate(...)==1, then the instantiation is already true in the model
      //  depIndex is the index of the least significant variable that this evaluation relies upon
      int depIndex = riter.getNumTerms()-1;
      int eval = evaluate( &riter, d_quantEngine->getCounterexampleBody( f ), depIndex );
      if( eval==1 ){
        Debug("fmf-model-eval") << "  Returned success with depIndex = " << depIndex << std::endl;
        riter.increment2( d_quantEngine, depIndex );
      }else{
        Debug("fmf-model-eval") << "  Returned " << (eval==-1 ? "failure" : "unknown") << ", depIndex = " << depIndex << std::endl;
        InstMatch m;
        riter.getMatch( d_quantEngine, m );
        Debug("fmf-model-eval") << "* Add instantiation " << m << std::endl;
        triedLemmas++;
        d_triedLemmas++;
        if( d_quantEngine->addInstantiation( f, m ) ){
          addedLemmas++;
#ifdef EVAL_FAIL_SKIP_MULTIPLE
          if( eval==-1 ){
            riter.increment2( d_quantEngine, depIndex );
          }else{
            riter.increment( d_quantEngine );
          }
#else
          riter.increment( d_quantEngine );
#endif
        }else{
          Debug("ajr-temp") << "* Failed Add instantiation " << m << std::endl;
          riter.increment( d_quantEngine );
        }
      }
    }else{
      InstMatch m;
      riter.getMatch( d_quantEngine, m );
      Debug("fmf-model-eval") << "* Add instantiation " << std::endl;
      triedLemmas++;
      d_triedLemmas++;
      if( d_quantEngine->addInstantiation( f, m ) ){
        addedLemmas++;
      }
      riter.increment( d_quantEngine );
    }
  }
  int totalInst = 1;
  for( int i=0; i<(int)f[0].getNumChildren(); i++ ){
    totalInst = totalInst * (int)getReps()->d_type_reps[ f[0][i].getType() ].size();
  }
  d_totalLemmas += totalInst;
  Debug("inst-fmf-ei") << "Finished: " << std::endl;
  Debug("inst-fmf-ei") << "   Inst Skipped: " << (totalInst-triedLemmas) << std::endl;
  Debug("inst-fmf-ei") << "   Inst Tried: " << triedLemmas << std::endl;
  Debug("inst-fmf-ei") << "   Inst Added: " << addedLemmas << std::endl;
  Debug("inst-fmf-ei") << "   # Tests: " << tests << std::endl;
///-----------
#ifdef ME_PRINT_WARNINGS
  if( addedLemmas>1000 ){
    Notice() << "WARNING: many instantiations produced for " << f << ": " << std::endl;
    Notice() << "   Inst Skipped: " << (totalInst-triedLemmas) << std::endl;
    Notice() << "   Inst Tried: " << triedLemmas << std::endl;
    Notice() << "   Inst Added: " << addedLemmas << std::endl;
    Notice() << "   # Tests: " << tests << std::endl;
    Notice() << std::endl;
    if( !d_quant_model_lits[f].empty() ){
      Notice() << "  Model literal definitions:" << std::endl;
      for( int i=0; i<(int)d_quant_model_lits[f].size(); i++ ){
        Notice() << "    " << d_quant_model_lits[f][i] << std::endl;
      }
      Notice() << std::endl;
    }
  }
#endif
///-----------
  return addedLemmas;
}

void ModelEngine::registerModelBasis( Node n, Node gn ){
  if( d_model_basis.find( n )==d_model_basis.end() ){
    d_model_basis[n] = gn;
    for( int i=0; i<(int)n.getNumChildren(); i++ ){
      registerModelBasis( n[i], gn[i] );
    }
  }
}

Node ModelEngine::getArbitraryElement( TypeNode tn, std::vector< Node >& exclude ){
  Node retVal;
  if( tn==NodeManager::currentNM()->booleanType() ){
    if( exclude.empty() ){
      retVal = d_false;
    }else if( exclude.size()==1 ){
      retVal = NodeManager::currentNM()->mkConst( areEqual( exclude[0], d_false ) );
    }
  }else if( d_ra.d_type_reps.find( tn )!=d_ra.d_type_reps.end() ){
    for( int i=0; i<(int)d_ra.d_type_reps[tn].size(); i++ ){
      if( std::find( exclude.begin(), exclude.end(), d_ra.d_type_reps[tn][i] )==exclude.end() ){
        retVal = d_ra.d_type_reps[tn][i];
        break;
      }
    }
  }
  if( !retVal.isNull() ){
    return d_quantEngine->getEqualityQuery()->getRepresentative( retVal );
  }else{
    return Node::null();
  }
}

Node ModelEngine::getModelBasisTerm( TypeNode tn, int i ){
  if( d_model_basis_term.find( tn )==d_model_basis_term.end() ){
    std::stringstream ss;
    ss << Expr::setlanguage(Options::current()->outputLanguage);
    ss << "e_" << tn;
    d_model_basis_term[tn] = NodeManager::currentNM()->mkVar( ss.str(), tn );
    ModelBasisAttribute mba;
    d_model_basis_term[tn].setAttribute(mba,true);
  }
  return d_model_basis_term[tn];
}

Node ModelEngine::getModelBasisOpTerm( Node op ){
  if( d_model_basis_op_term.find( op )==d_model_basis_op_term.end() ){
    TypeNode t = op.getType();
    std::vector< Node > children;
    children.push_back( op );
    for( int i=0; i<(int)t.getNumChildren()-1; i++ ){
      children.push_back( getModelBasisTerm( t[i] ) );
    }
    d_model_basis_op_term[op] = NodeManager::currentNM()->mkNode( APPLY_UF, children );
  }
  return d_model_basis_op_term[op];
}

void ModelEngine::collectUfTerms( Node n, std::vector< Node >& terms ){
  if( n.getKind()==APPLY_UF ){
    terms.push_back( n );
  }
  for( int i=0; i<(int)n.getNumChildren(); i++ ){
    collectUfTerms( n[i], terms );
  }
}

void ModelEngine::initializeUfModel( Node op ){
  if( d_uf_model.find( op )==d_uf_model.end() ){
    TypeNode tn = op.getType();
    tn = tn[ (int)tn.getNumChildren()-1 ];
    if( tn==NodeManager::currentNM()->booleanType() || uf::StrongSolverTheoryUf::isRelevantType( tn ) ){
      d_uf_model[ op ] = uf::UfModel( op, this );
    }
  }
}

bool ModelEngine::computeRelevantInstantiationDomain( Node n, Node parent, int arg, std::vector< RepDomain >& rd ){
  bool domainChanged = false;
  if( n.getKind()==INST_CONSTANT ){
    bool domainSet = false;
    int vi = n.getAttribute(InstVarNumAttribute());
    Assert( !parent.isNull() );
    if( parent.getKind()==APPLY_UF ){
      //if the child of APPLY_UF term f( ... ), only consider the active domain of f at given argument
      Node op = parent.getOperator();
      if( d_uf_model.find( op )!=d_uf_model.end() ){
        for( int i=0; i<(int)d_uf_model[op].d_active_domain[arg].size(); i++ ){
          int d = d_uf_model[op].d_active_domain[arg][i];
          if( std::find( rd[vi].begin(), rd[vi].end(), d )==rd[vi].end() ){
            rd[vi].push_back( d );
            domainChanged = true;
          }
        }
        domainSet = true;
      }
    }
    if( !domainSet ){
      //otherwise, we must consider the entire domain
      TypeNode tn = n.getType();
      Assert( d_ra.hasType( tn ) );
      if( rd[vi].size()!=d_ra.d_type_reps[tn].size() ){
        rd[vi].clear();
        for( int i=0; i<(int)d_ra.d_type_reps[tn].size(); i++ ){
          rd[vi].push_back( i );
          domainChanged = true;
        }
      }
    }
  }else{
    for( int i=0; i<(int)n.getNumChildren(); i++ ){
      if( computeRelevantInstantiationDomain( n[i], n, i, rd ) ){
        domainChanged = true;
      }
    }
  }
  return domainChanged;
}

bool ModelEngine::extendFunctionDomains( Node n, RepDomain& range ){
  if( n.getKind()==INST_CONSTANT ){
    Node f = n.getAttribute(InstConstantAttribute());
    int index = n.getAttribute(InstVarNumAttribute());
    range.insert( range.begin(), d_quant_inst_domain[f][index].begin(), d_quant_inst_domain[f][index].end() );
    return false;
  }else{
    Node op;
    if( n.getKind()==APPLY_UF ){
      op = n.getOperator();
    }
    bool domainChanged = false;
    for( int i=0; i<(int)n.getNumChildren(); i++ ){
      RepDomain childRange;
      if( extendFunctionDomains( n[i], childRange ) ){
        domainChanged = true;
      }
      if( n.getKind()==APPLY_UF ){
        for( int j=0; j<(int)childRange.size(); j++ ){
          int v = childRange[j];
          if( std::find( d_uf_model[op].d_active_domain[i].begin(), d_uf_model[op].d_active_domain[i].end(), v )==
              d_uf_model[op].d_active_domain[i].end() ){
            d_uf_model[op].d_active_domain[i].push_back( v );
            domainChanged = true;
          }
        }
      }
    }
    //get the range
    if( n.hasAttribute(InstConstantAttribute()) ){
      if( n.getKind()==APPLY_UF ){
        range.insert( range.end(), d_uf_model[op].d_active_range.begin(), d_uf_model[op].d_active_range.end() );
      }
    }else{
      Node r = d_quantEngine->getEqualityQuery()->getRepresentative( n );
      range.push_back( getReps()->getIndexFor( r ) );
    }
    return domainChanged;
  }
}

void ModelEngine::makeEvalTermModel( Node n ){
  if( d_eval_term_model.find( n )==d_eval_term_model.end() ){
    makeEvalTermIndexOrder( n );
    if( !d_eval_term_use_default_model[n] ){
      Node op = n.getOperator();
      d_eval_term_model[n] = uf::UfModelTreeOrdered( op, d_eval_term_index_order[n] );
      d_uf_model[op].makeModel( d_quantEngine, d_eval_term_model[n] );
      //Debug("fmf-model-index-order") << "Make model for " << n << " : " << std::endl;
      //d_eval_term_model[n].debugPrint( "fmf-model-index-order", d_quantEngine, 2 );
    }
  }
}

struct sortGetMaxVariableNum {
  std::map< Node, int > d_max_var_num;
  int computeMaxVariableNum( Node n ){
    if( n.getKind()==INST_CONSTANT ){
      return n.getAttribute(InstVarNumAttribute());
    }else if( n.hasAttribute(InstConstantAttribute()) ){
      int maxVal = -1;
      for( int i=0; i<(int)n.getNumChildren(); i++ ){
        int val = getMaxVariableNum( n[i] );
        if( val>maxVal ){
          maxVal = val;
        }
      }
      return maxVal;
    }else{
      return -1;
    }
  }
  int getMaxVariableNum( Node n ){
    std::map< Node, int >::iterator it = d_max_var_num.find( n );
    if( it==d_max_var_num.end() ){
      int num = computeMaxVariableNum( n );
      d_max_var_num[n] = num;
      return num;
    }else{
      return it->second;
    }
  }
  bool operator() (Node i,Node j) { return (getMaxVariableNum(i)<getMaxVariableNum(j));}
};

void ModelEngine::makeEvalTermIndexOrder( Node n ){
  if( d_eval_term_index_order.find( n )==d_eval_term_index_order.end() ){
#ifdef USE_INDEX_ORDERING
    //sort arguments in order of least significant vs. most significant variable in default ordering
    std::map< Node, std::vector< int > > argIndex;
    std::vector< Node > args;
    for( int i=0; i<(int)n.getNumChildren(); i++ ){
      if( argIndex.find( n[i] )==argIndex.end() ){
        args.push_back( n[i] );
      }
      argIndex[n[i]].push_back( i );
    }
    sortGetMaxVariableNum sgmvn;
    std::sort( args.begin(), args.end(), sgmvn );
    for( int i=0; i<(int)args.size(); i++ ){
      for( int j=0; j<(int)argIndex[ args[i] ].size(); j++ ){
        d_eval_term_index_order[n].push_back( argIndex[ args[i] ][j] );
      }
    }
    bool useDefault = true;
    for( int i=0; i<(int)d_eval_term_index_order[n].size(); i++ ){
      if( i!=d_eval_term_index_order[n][i] ){
        useDefault = false;
        break;
      }
    }
    d_eval_term_use_default_model[n] = useDefault;
    Debug("fmf-model-index-order") << "Will consider the following index ordering for " << n << " : ";
    for( int i=0; i<(int)d_eval_term_index_order[n].size(); i++ ){
      Debug("fmf-model-index-order") << d_eval_term_index_order[n][i] << " ";
    }
    Debug("fmf-model-index-order") << std::endl;
#else
    d_eval_term_use_default_model[n] = true;
#endif
  }
}

//if evaluate( rai, n, phaseReq ) = eVal,
// if eVal = rai->d_index.size()
//   then the formula n instantiated with rai cannot be proven to be equal to phaseReq
// otherwise,
//   each n{rai->d_index[0]/x_0...rai->d_index[eVal]/x_eVal, */x_(eVal+1) ... */x_n } is equal to phaseReq in the current model
int ModelEngine::evaluate( RepSetIterator* rai, Node n, int& depIndex ){
  ++(d_statistics.d_eval_formulas);
  //Debug("fmf-model-eval-debug") << "Evaluate " << n << " " << phaseReq << std::endl;
  //Notice() << "Eval " << n << std::endl;
  if( n.getKind()==NOT ){
    int val = evaluate( rai, n[0], depIndex );
    return val==1 ? -1 : ( val==-1 ? 1 : 0 );
  }else if( n.getKind()==OR || n.getKind()==AND || n.getKind()==IMPLIES ){
    int baseVal = n.getKind()==AND ? 1 : -1;
    int eVal = baseVal;
    int posDepIndex = rai->getNumTerms();
    int negDepIndex = -1;
    for( int i=0; i<(int)n.getNumChildren(); i++ ){
      //evaluate subterm
      int childDepIndex;
      Node nn = ( i==0 && n.getKind()==IMPLIES ) ? n[i].notNode() : n[i];
      int eValT = evaluate( rai, nn, childDepIndex );
      if( eValT==baseVal ){
        if( eVal==baseVal ){
          if( childDepIndex>negDepIndex ){
            negDepIndex = childDepIndex;
          }
        }
      }else if( eValT==-baseVal ){
        eVal = -baseVal;
        if( childDepIndex<posDepIndex ){
          posDepIndex = childDepIndex;
          if( posDepIndex==-1 ){
            break;
          }
        }
      }else if( eValT==0 ){
        if( eVal==baseVal ){
          eVal = 0;
        }
      }
    }
    if( eVal!=0 ){
      depIndex = eVal==-baseVal ? posDepIndex : negDepIndex;
      return eVal;
    }else{
      return 0;
    }
  }else if( n.getKind()==IFF || n.getKind()==XOR ){
    int depIndex1;
    int eVal = evaluate( rai, n[0], depIndex1 );
    if( eVal!=0 ){
      int depIndex2;
      int eVal2 = evaluate( rai, n.getKind()==XOR ? n[1].notNode() : n[1], depIndex2 );
      if( eVal2!=0 ){
        depIndex = depIndex1>depIndex2 ? depIndex1 : depIndex2;
        return eVal==eVal2 ? 1 : -1;
      }
    }
    return 0;
  }else if( n.getKind()==ITE ){
    int depIndex1;
    int eVal = evaluate( rai, n[0], depIndex1 );
    if( eVal==0 ){
      //DO_THIS: evaluate children to see if they are the same value?
      return 0;
    }else{
      int depIndex2;
      int eValT = evaluate( rai, n[eVal==1 ? 1 : 2], depIndex2 );
      depIndex = depIndex1>depIndex2 ? depIndex1 : depIndex2;
      return eValT;
    }
  }else if( n.getKind()==FORALL ){
    return 0;
  }else{
    ////if we know we will fail again, immediately return
    //if( d_eval_failed.find( n )!=d_eval_failed.end() ){
    //  if( d_eval_failed[n] ){
    //    return -1;
    //  }
    //}
    //Debug("fmf-model-eval-debug") << "Evaluate literal " << n << std::endl;
    //this should be a literal
    Node gn = n.substitute( rai->d_ic.begin(), rai->d_ic.end(), rai->d_terms.begin(), rai->d_terms.end() );
    //Debug("fmf-model-eval-debug") << "  Ground version = " << gn << std::endl;
    int retVal = 0;
    depIndex = rai->getNumTerms()-1;
    if( n.getKind()==APPLY_UF ){
      //case for boolean predicates
      Node val = evaluateTerm( rai, n, gn, depIndex );
      if( d_quantEngine->getEqualityQuery()->hasTerm( val ) ){
        if( areEqual( val, d_true ) ){
          retVal = 1;
        }else{
          retVal = -1;
        }
      }
    }else if( n.getKind()==EQUAL ){
      //case for equality
      retVal = evaluateEquality( rai, n[0], n[1], gn[0], gn[1], depIndex );
    }else if( n.getKind()==APPLY_TESTER ){
      //case for datatype tester predicate
    }
    if( retVal!=0 ){
      Debug("fmf-model-eval-debug") << "Evaluate literal: return " << retVal << ", depends = " << depIndex << std::endl;
    }
    return retVal;
  }
}

int ModelEngine::evaluateEquality( RepSetIterator* rai, Node n1, Node n2, Node gn1, Node gn2, int& depIndex ){
  ++(d_statistics.d_eval_eqs);
  //Notice() << "Eval eq " << n1 << " " << n2 << std::endl;
  Debug("fmf-model-eval-debug") << "Evaluate equality: " << std::endl;
  Debug("fmf-model-eval-debug") << "   " << n1 << " = " << n2 << std::endl;
  Debug("fmf-model-eval-debug") << "   " << gn1 << " = " << gn2 << std::endl;
  int depIndex1, depIndex2;
  Node val1 = evaluateTerm( rai, n1, gn1, depIndex1 );
  Node val2 = evaluateTerm( rai, n2, gn2, depIndex2 );
  Debug("fmf-model-eval-debug") << "   Values :  ";
  Model::printRepresentative( "fmf-model-eval-debug", d_quantEngine, val1 );
  Debug("fmf-model-eval-debug") <<  " = ";
  Model::printRepresentative( "fmf-model-eval-debug", d_quantEngine, val2 );
  Debug("fmf-model-eval-debug") << std::endl;
  int retVal = 0;
  if( areEqual( val1, val2 ) ){
    retVal = 1;
  }else if( areDisequal( val1, val2 ) ){
    retVal = -1;
  }else{
    //std::cout << "Neither equal nor disequal " << val1.getKind() << " " << val2.getKind() << " : " << val1.getType() << std::endl;
    //std::cout << "                           " << d_quantEngine->getEqualityQuery()->hasTerm( val1 ) << " " << d_quantEngine->getEqualityQuery()->hasTerm( val2 ) << std::endl;
    //std::cout << "                           " << val1 << " " << val2 << std::endl;
  }
  if( retVal!=0 ){
    Debug("fmf-model-eval-debug") << "   ---> Success, value = " << (retVal==1) << std::endl;
    depIndex = depIndex1>depIndex2 ? depIndex1 : depIndex2;
  }else{
    Debug("fmf-model-eval-debug") << "   ---> Failed" << std::endl;
    depIndex = rai->getNumTerms()-1;
  }
  return retVal;
}

Node ModelEngine::evaluateTerm( RepSetIterator* rai, Node n, Node gn, int& depIndex ){
  //Notice() << "Eval term " << n << std::endl;
  if( n.hasAttribute(InstConstantAttribute()) ){
    Node val;
    depIndex = rai->getNumTerms()-1;
    //check the type of n
    if( n.getKind()==INST_CONSTANT ){
      depIndex = rai->d_var_order[ n.getAttribute(InstVarNumAttribute()) ];
      val = gn;
    }else if( n.getKind()==ITE ){
      int condDepIndex = -1;
      int eval = evaluate( rai, n[0], condDepIndex );
      if( eval==0 ){
        //DO_THIS: evaluate children to see if they are the same?
      }else{
        int index = eval==1 ? 1 : 2;
        int valDepIndex = -1;
        val = evaluateTerm( rai, n[index], gn[index], valDepIndex );
        depIndex = condDepIndex>valDepIndex ? condDepIndex : valDepIndex;
      }
    }else if( n.getKind()==APPLY_UF ){
      //Debug("fmf-model-eval-debug") << "Evaluate term " << n << " (" << gn << ")" << std::endl;
      //Notice() << "e " << n << std::endl;
      //first we must evaluate the arguments
      Node op = n.getOperator();
      //if it is a defined UF, then consult the interpretation
      Node gnn = gn;
      ++(d_statistics.d_eval_uf_terms);
      //first we must evaluate the arguments
      bool childrenChanged = false;
      std::vector< Node > children;
      children.push_back( op );
      std::vector< int > children_depIndex;
      //for each argument, calculate its value, and the variables its value depends upon
      for( int i=0; i<(int)n.getNumChildren(); i++ ){
        children_depIndex.push_back( -1 );
        Node nn = evaluateTerm( rai, n[i], gn[i], children_depIndex[i] );
        children.push_back( nn );
        childrenChanged = childrenChanged || nn!=gn[i];
      }
      //remake gn if changed
      if( childrenChanged ){
        gnn = NodeManager::currentNM()->mkNode( APPLY_UF, children );
      }
      int argDepIndex = 0;
      if( d_uf_model.find( op )!=d_uf_model.end() ){
        //Notice() << "make eval" << std::endl;
        //make the term model specifically for n
        makeEvalTermModel( n );
        //Notice() << "done " << d_eval_term_use_default_model[n] << std::endl;
        //now, consult the model
        if( d_eval_term_use_default_model[n] ){
          val = d_uf_model[op].d_tree.getValue( d_quantEngine, gnn, argDepIndex );
        }else{
          val = d_eval_term_model[ n ].getValue( d_quantEngine, gnn, argDepIndex );
        }
        //Notice() << "done get value " << val << std::endl;
        //Debug("fmf-model-eval-debug") << "Evaluate term " << n << " (" << gn << ", " << gnn << ")" << std::endl;
        //d_eval_term_model[ n ].debugPrint("fmf-model-eval-debug", d_quantEngine );
        Assert( !val.isNull() );
      }else{
        d_eval_term_use_default_model[n] = true;
        val = gnn;
        argDepIndex = (int)n.getNumChildren();
      }
      Debug("fmf-model-eval-debug") << "Evaluate term " << n << " = " << gn << " = " << gnn << " = ";
      Model::printRepresentative( "fmf-model-eval-debug", d_quantEngine, val );
      Debug("fmf-model-eval-debug") << ", depIndex = " << depIndex << std::endl;
      //Notice() << n << " = " << gn << " = " << gnn << " = " << val << std::endl;
      depIndex = -1;
      for( int i=0; i<argDepIndex; i++ ){
        int index = d_eval_term_use_default_model[n] ? i : d_eval_term_index_order[n][i];
        Debug("fmf-model-eval-debug") << "Add variables from " << index << "..." << std::endl;
        if( children_depIndex[index]>depIndex ){
          depIndex = children_depIndex[index];
        }
      }
      ////cache the result
      //d_eval_term_vals[gn] = val;
      //d_eval_term_fv_deps[n][gn].insert( d_eval_term_fv_deps[n][gn].end(), fv_deps.begin(), fv_deps.end() )
    }else if( n.getKind()==SELECT ){
      //DO_THIS?
    }else if( n.getKind()==STORE ){
      //DO_THIS?
    }else if( n.getKind()==PLUS ){
      //DO_THIS?
    }else if( n.getKind()==APPLY_SELECTOR ){

    }
    if( val.isNull() ){
      val = gn;
      //DOTHIS: theories?
      //std::cout << "Unevaluated term " << n.getKind() << " : " << n.getType() << " (" << d_quantEngine->getEqualityQuery()->hasTerm( gn ) << ")" << std::endl;
      //std::cout << "                 " << n << std::endl;
      //must collect free variables for dependencies
      std::vector< Node > fv_deps;
      Trigger::getVarContainsNode( n.getAttribute(InstConstantAttribute()), n, fv_deps );
      depIndex = -1;
      for( int i=0; i<(int)fv_deps.size(); i++ ){
        int index = rai->d_var_order[ fv_deps[i].getAttribute(InstVarNumAttribute()) ];
        if( index>depIndex ){
          depIndex = index;
        }
      }
    }
    return val;
  }else{
    depIndex = -1;
    return n;
  }
}

void ModelEngine::clearEvalFailed( int index ){
  for( int i=0; i<(int)d_eval_failed_lits[index].size(); i++ ){
    d_eval_failed[ d_eval_failed_lits[index][i] ] = false;
  }
  d_eval_failed_lits[index].clear();
}

bool ModelEngine::areEqual( Node a, Node b ){
  return d_quantEngine->getEqualityQuery()->areEqual( a, b );
}

bool ModelEngine::areDisequal( Node a, Node b ){
  return d_quantEngine->getEqualityQuery()->areDisequal( a, b );
}

void ModelEngine::debugPrint( const char* c ){
  Debug( c ) << "---Current Model---" << std::endl;
  Debug( c ) << "Representatives: " << std::endl;
  d_ra.debugPrint( c, d_quantEngine );
  Debug( c ) << "Quantifiers: " << std::endl;
  for( int i=0; i<(int)d_quantEngine->getNumAssertedQuantifiers(); i++ ){
    Node f = d_quantEngine->getAssertedQuantifier( i );
    Debug( c ) << "   ";
    if( d_quant_sat.find( f )!=d_quant_sat.end() ){
      Debug( c ) << "*SAT* ";
    }else{
      Debug( c ) << "      ";
    }
    Debug( c ) << f << std::endl;
  }
  Debug( c ) << "Functions: " << std::endl;
  for( std::map< Node, uf::UfModel >::iterator it = d_uf_model.begin(); it != d_uf_model.end(); ++it ){
    it->second.debugPrint( c );
    Debug( c ) << std::endl;
  }
}

ModelEngine::Statistics::Statistics():
  d_inst_rounds("ModelEngine::Inst_Rounds", 0),
  d_pre_sat_quant("ModelEngine::Status_quant_pre_sat", 0),
  d_pre_nsat_quant("ModelEngine::Status_quant_pre_non_sat", 0),
  d_eval_formulas("ModelEngine::Eval_Formulas", 0 ),
  d_eval_eqs("ModelEngine::Eval_Equalities", 0 ),
  d_eval_uf_terms("ModelEngine::Eval_Uf_Terms", 0 ),
  d_num_quants_init("ModelEngine::Num_Quants", 0 ),
  d_num_quants_init_fail("ModelEngine::Num_Quants_No_Basis", 0 )
{
  StatisticsRegistry::registerStat(&d_inst_rounds);
  StatisticsRegistry::registerStat(&d_pre_sat_quant);
  StatisticsRegistry::registerStat(&d_pre_nsat_quant);
  StatisticsRegistry::registerStat(&d_eval_formulas);
  StatisticsRegistry::registerStat(&d_eval_eqs);
  StatisticsRegistry::registerStat(&d_eval_uf_terms);
  StatisticsRegistry::registerStat(&d_num_quants_init);
  StatisticsRegistry::registerStat(&d_num_quants_init_fail);
}

ModelEngine::Statistics::~Statistics(){
  StatisticsRegistry::unregisterStat(&d_inst_rounds);
  StatisticsRegistry::unregisterStat(&d_pre_sat_quant);
  StatisticsRegistry::unregisterStat(&d_pre_nsat_quant);
  StatisticsRegistry::unregisterStat(&d_eval_formulas);
  StatisticsRegistry::unregisterStat(&d_eval_eqs);
  StatisticsRegistry::unregisterStat(&d_eval_uf_terms);
  StatisticsRegistry::unregisterStat(&d_num_quants_init);
  StatisticsRegistry::unregisterStat(&d_num_quants_init_fail);
}
