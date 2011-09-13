/*********************                                                        */
/*! \file theory_uf_instantiator.cpp
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
 ** \brief Implementation of theory uf instantiator class
 **/

#include "theory/uf/theory_uf_instantiator.h"
#include "theory/theory_engine.h"
#include "theory/uf/theory_uf.h"
#include "theory/uf/equality_engine_impl.h"

using namespace std;
using namespace CVC4;
using namespace CVC4::kind;
using namespace CVC4::context;
using namespace CVC4::theory;
using namespace CVC4::theory::uf;


//SubTermNode::SubTermNode( context::Context* c, Node n ) :
//d_parents( c ),
//d_obligations( c ),
//d_node( n ){
//  
//}
//
//void SubTermNode::addParent( SubTermNode* g ) { 
//  for( GmnList::const_iterator it = d_parents.begin(); it!=d_parents.end(); ++it ){
//    if( *it == g ){
//      return;
//    }
//  }
//  d_parents.push_back( g ); 
//}
//
//void SubTermNode::addObligation( Node n ) { 
//  for( ObList::const_iterator it = d_obligations.begin(); it!=d_obligations.end(); ++it ){
//    if( *it == n ){
//      return;
//    }
//  }
//  d_obligations.push_back( n ); 
//}

InstantiatorTheoryUf::InstantiatorTheoryUf(context::Context* c, CVC4::theory::InstantiationEngine* ie, Theory* th) :
Instantiator( c, ie ),
//d_subterms( c ),
//d_subterm_size( c ),
d_obligations( c ),
d_th(th),
d_terms( c ),
d_terms_full( c ),
d_disequality( c )
//d_ematch_done( c )
{
  registerTerm( ((TheoryUF*)d_th)->d_true );
  registerTerm( ((TheoryUF*)d_th)->d_false );
  Node eq = NodeManager::currentNM()->mkNode( IFF, ((TheoryUF*)d_th)->d_true, ((TheoryUF*)d_th)->d_false );
  d_disequality.push_back( eq );
}

Theory* InstantiatorTheoryUf::getTheory() { 
  return d_th; 
}

void InstantiatorTheoryUf::check( Node assertion )
{
  //TheoryUF* t = (TheoryUF*)d_th;
  //Debug("quant-uf") << "InstantiatorTheoryUf::check: " << assertion << std::endl;
  //switch(assertion.getKind()) {
  //case kind::EQUAL:
  //case kind::IFF:
  //  assertEqual(assertion[0], assertion[1]);
  //  break;
  //case kind::APPLY_UF:
  //  { // assert it's a predicate
  //    Assert(assertion.getOperator().getType().getRangeType().isBoolean());
  //    assertEqual(assertion, t->d_trueNode);
  //  }
  //  break;
  //case kind::NOT:
  //  if(assertion[0].getKind() == kind::EQUAL ||
  //     assertion[0].getKind() == kind::IFF) {
  //    assertDisequal(assertion[0][0], assertion[0][1]);
  //  } else {
  //    // negation of a predicate
  //    Assert(assertion[0].getKind() == kind::APPLY_UF);
  //    // assert it's a predicate
  //    Assert(assertion[0].getOperator().getType().getRangeType().isBoolean());
  //    assertEqual(assertion[0], t->d_falseNode);
  //  }
  //  break;
  //default:
  //  Unhandled(assertion.getKind());
  //}
}

void InstantiatorTheoryUf::assertEqual( Node a, Node b )
{
  Debug("inst-uf") << "InstantiatorTheoryUf::equal: " << a << " == " << b << std::endl;
  if( a.hasAttribute(InstantitionConstantAttribute()) || 
      b.hasAttribute(InstantitionConstantAttribute()) ){
    //add to obligation list
    Node formula;
    Node f;
    if( a.hasAttribute(InstantitionConstantAttribute()) ){
      f = a.getAttribute(InstantitionConstantAttribute());
      Kind knd = a.getType()==NodeManager::currentNM()->booleanType() ? IFF : EQUAL;
      formula = NodeManager::currentNM()->mkNode( knd, a, b );
    }else if( b.hasAttribute(InstantitionConstantAttribute()) ){
      f = b.getAttribute(InstantitionConstantAttribute());
      //swap sides
      Kind knd = a.getType()==NodeManager::currentNM()->booleanType() ? IFF : EQUAL;
      formula = NodeManager::currentNM()->mkNode( knd, b, a );
    }
    //swap sides for a disequality
    if( a.getKind()==EQUAL || a.getKind()==IFF ){
      if( !a[0].hasAttribute(InstantitionConstantAttribute()) ){
        Assert( a[1].hasAttribute(InstantitionConstantAttribute()) );
        a = NodeManager::currentNM()->mkNode( a.getKind(), a[1], a[0] );
        InstantitionConstantAttribute icai;
        a.setAttribute(icai,f);
      }
      formula = NodeManager::currentNM()->mkNode( NOT, a );
    }
    Assert( f!=Node::null() );
    NodeList* ob;
    NodeLists::iterator ob_i = d_obligations.find( f );
    if( ob_i==d_obligations.end() ){
      ob = new(d_th->getContext()->getCMM()) NodeList(true, d_th->getContext(), false,
                                                      ContextMemoryAllocator<Node>(d_th->getContext()->getCMM()));
      d_obligations.insertDataFromContextMemory( f, ob );
    }else{
      ob = (*ob_i).second;
    }
    for( NodeList::const_iterator it = ob->begin(); it != ob->end(); ++it ){
      Assert( *it != formula );
    }
    ob->push_back( formula );
    //DO_THIS: sub quantifiers?  add to their obligation list too?
  }
  if( a.getKind()==EQUAL || a.getKind()==IFF ){
    Assert( b==((TheoryUF*)d_th)->d_false );
    d_disequality.push_back( a );
    registerTerm( a[0] );
    registerTerm( a[1] );
  }else{
    Assert( b.getKind()!=EQUAL && b.getKind()!=IFF );
    registerTerm( a );
    registerTerm( b );
  }
}

void InstantiatorTheoryUf::registerTerm( Node n, bool isTop )
{
  bool recurse = false;
  if( isTop ){
    if( d_terms.find( n )==d_terms.end() ){
      d_terms[n] = true;
      d_terms_full[n] = true;
      recurse = true;
    }
  }else{
    if( d_terms_full.find( n )==d_terms_full.end() ){
      d_terms_full[n] = true;
      recurse = true;
    }
  }
  if( recurse ){
    for( int i=0; i<(int)n.getNumChildren(); i++ ){
      registerTerm( n[i], false );
    }
  }
}

//void InstantiatorTheoryUf::buildSubTerms( Node n )
//{
//  SubTermMap::iterator it = d_subterms.find( n );
//  if( it==d_subterms.end() ){
//    SubTermNode* g = getSubTerm( n );
//    for( int i=0; i<(int)n.getNumChildren(); i++ ){
//      if( n[i].hasAttribute(InstantitionConstantAttribute()) ){
//        buildSubTerms( n[i] );
//        getSubTerm( n[i] )->addParent( g );
//      }
//    }
//  }
//}
//
//SubTermNode* InstantiatorTheoryUf::getSubTerm( Node n )
//{
//  SubTermMap::iterator gm_i = d_subterms.find( n );
//  if( gm_i == d_subterms.end() ) {
//    SubTermNode* g = new SubTermNode( d_th->getContext(), n );
//    d_subterms[n] = g;
//    //add to count for the counterexample of its quantifier
//    for( std::map< Node, std::vector< Node > >::iterator it = d_inst_constants.begin(); 
//          it !=d_inst_constants.end(); ++it ){
//      if( hasInstantiationConstantsFrom( n, it->first ) ){
//        IntMap::iterator gms_i = d_subterm_size.find( it->first );
//        if( gms_i==d_subterm_size.end() ){
//          d_subterm_size[ it->first ] = 0;
//        }
//        d_subterm_size[ it->first ] = d_subterm_size[ it->first ] + 1;
//      }
//    }
//    return g;
//  }else{
//    return (*gm_i).second;
//  }
//}

//void InstantiatorTheoryUf::setActiveInstConstants( Node n ){
//  Assert( n.hasAttribute(InstantitionConstantAttribute()) );
//  if( n.getKind()==INST_CONSTANT ){
//    d_active_ic[ n ] = true;
//  }else{
//    if( d_inst_terms.find( n )==d_inst_terms.end() ){
//      for( int i=0; i<(int)n.getNumChildren(); i++ ){
//        if( n[i].hasAttribute(InstantitionConstantAttribute()) ){
//          setActiveInstConstants( n[i] );
//        }
//      }
//    }
//  }
//}

void InstantiatorTheoryUf::resetInstantiation()
{
  d_does_ematch.clear();
  d_eq_amb.clear();
  for( std::map< Node, std::map< Node, InstMatchGroup > >::iterator it = d_ematch.begin();
      it != d_ematch.end(); ++it ){
    for( std::map< Node, InstMatchGroup >::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2 ){
      it2->second.reset();
    }
  }
  for( std::map< Node, std::map< Node, InstMatchGroup > >::iterator it = d_ematch_mod.begin();
      it != d_ematch_mod.end(); ++it ){
    for( std::map< Node, InstMatchGroup >::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2 ){
      it2->second.reset();
    }
  }
  for( std::map< Node, std::map< Node, InstMatchGroup > >::iterator it = d_partial_ematch.begin();
      it != d_partial_ematch.end(); ++it ){
    for( std::map< Node, InstMatchGroup >::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2 ){
      it2->second.reset();
    }
  }
  for( std::map< Node, std::map< Node, InstMatchGroup > >::iterator it = d_partial_ematch_mod.begin();
      it != d_partial_ematch_mod.end(); ++it ){
    for( std::map< Node, InstMatchGroup >::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2 ){
      it2->second.reset();
    }
  }
  for( std::map< Node, InstMatchGroup >::iterator it = d_ematch_full.begin(); 
       it != d_ematch_full.end(); ++it ){
    it->second.reset();
  }
  //build equivalence classes
  d_emap.clear();
  for( BoolMap::iterator it = d_terms.begin(); it!=d_terms.end(); ++it ){
    Node n = (*it).first;
    Node r = getRepresentative( n );
    if( std::find( d_emap[r].begin(), d_emap[r].end(), n )==d_emap[r].end() ){
      d_emap[r].push_back( n );
    }
  }
  //build disequality lists
  d_dmap.clear();
  for( NodeList::const_iterator it = d_disequality.begin(); it!=d_disequality.end(); ++it ){
    Node n = (*it);
    Node r1 = getRepresentative( n[0] );
    Node r2 = getRepresentative( n[1] );
    if( std::find( d_dmap[r1].begin(), d_dmap[r1].end(), r2 )==d_dmap[r1].end() ){
      d_dmap[r1].push_back( r2 );
    }
    if( std::find( d_dmap[r2].begin(), d_dmap[r2].end(), r1 )==d_dmap[r2].end() ){
      d_dmap[r2].push_back( r1 );
    }
  }
}

bool InstantiatorTheoryUf::prepareInstantiation( Effort e )
{
  if( e==MIN_EFFORT ){
    debugPrint();
  }
  Debug("quant-uf") << "Search (" << e << ") for instantiation for UF: " << std::endl;

  for( std::map< Node, std::vector< Node > >::iterator it = d_instEngine->d_inst_constants.begin(); 
        it != d_instEngine->d_inst_constants.end(); ++it ){
    if( d_instEngine->getActive( it->first ) ){
      calculateMatches( it->first, e );
    }
  }
  //if( d_inst_matches.getNumMatches()>0 ){
  //  Debug("quant-uf") << "*** I produced these matches (" << e << ") : " << std::endl;
  //  d_inst_matches.debugPrint("quant-uf");
  //  Debug("quant-uf") << std::endl;
  //}

  return false;
}

void InstantiatorTheoryUf::debugPrint()
{
  //Debug("quant-uf") << "Terms:" << std::endl;
  //for( BoolMap::const_iterator it = d_terms.begin(); it!=d_terms.end(); ++it ){
  //  Debug("quant-uf") << "   " << (*it).first;
  //  //Debug("quant-uf") << "  ->  " << getRepresentative( (*it).first );
  //  Debug("quant-uf") << std::endl;
  //}
  Debug("quant-uf") << "Equalivalence classes:" << std::endl;
  int counter = 1;
  for( std::map< Node, std::vector< Node > >::iterator it = d_emap.begin(); it!=d_emap.end(); ++it ){
    Debug("quant-uf") << "E" << counter << ": { ";
    for( int i = 0; i<(int)it->second.size(); i++ ){
      if( i!=0 ){
        Debug("quant-uf") << ", ";
      }
      Debug("quant-uf") << it->second[i];
    }
    Debug("quant-uf") << " }";
    Debug("quant-uf") << ", disequal : ";
    std::map< Node, std::vector< Node > >::iterator itd = d_dmap.find( it->first );
    if( itd!=d_dmap.end() ){
      for( int i = 0; i<(int)itd->second.size(); i++ ){
        if( i!=0 ){
          Debug("quant-uf") << ", ";
        }
        int counter2 = 1;
        std::map< Node, std::vector< Node > >::iterator it4 = d_emap.begin();
        while( it4!=d_emap.end() && !areEqual( it4->first, itd->second[i] ) ){
          ++it4;
          ++counter2;
        }
        if( it4==d_emap.end() ){
          Debug("quant-uf") << getRepresentative( itd->second[i] );
        }else{
          Debug("quant-uf") << "E" << counter2;
        }
      }
    }
    ++counter;
    Debug("quant-uf") << std::endl;
  }
  Debug("quant-uf") << std::endl;

  for( std::map< Node, std::vector< Node > >::iterator it = d_instEngine->d_inst_constants.begin(); 
        it != d_instEngine->d_inst_constants.end(); ++it ){
    Node f = it->first;
    Debug("quant-uf") << f << std::endl;
    Debug("quant-uf") << "   Inst constants:" << std::endl;
    Debug("quant-uf") << "      ";
    for( int i=0; i<(int)it->second.size(); i++ ){
      if( i>0 ){
        Debug("quant-uf") << ", ";
      }
      Debug("quant-uf") << it->second[i];
      if(d_terms_full.find( it->second[i] )==d_terms_full.end()){
        Debug("quant-uf") << " (inactive)";
      }
    }
    Debug("quant-uf") << std::endl;
    Debug("quant-uf") << "   Obligations:" << std::endl;
    NodeLists::iterator ob_i = d_obligations.find( f );
    if( ob_i!=d_obligations.end() ){
      NodeList* ob = (*ob_i).second;
      for( NodeList::const_iterator it = ob->begin(); it != ob->end(); ++it ){
        Debug("quant-uf") << "      " << *it << std::endl;
      }
    }
  }

  Debug("quant-uf") << std::endl;
}

Node InstantiatorTheoryUf::getConcreteTermEqualTo( Node n ){
  Node ns = getRepresentative( n );
  if( !ns.hasAttribute(InstantitionConstantAttribute()) ){
    return ns;
  }else{
    if( d_emap.find( ns )!=d_emap.end() ){
      for( int i=0; i<(int)d_emap[ ns ].size(); i++ ){
        if( !d_emap[ns][i].hasAttribute(InstantitionConstantAttribute()) ){
          return d_emap[ns][i];
        }
      }
    }
    return Node::null();
  }
}

bool InstantiatorTheoryUf::areEqual( Node a, Node b ){
  if( ((TheoryUF*)d_th)->d_equalityEngine.hasTerm( a ) &&
      ((TheoryUF*)d_th)->d_equalityEngine.hasTerm( b ) ){
    return ((TheoryUF*)d_th)->d_equalityEngine.areEqual( a, b );
  }else{
    return false;
  }
}

bool InstantiatorTheoryUf::areDisequal( Node a, Node b ){
  a = getRepresentative( a );
  b = getRepresentative( b );
  std::map< Node, std::vector< Node > >::iterator itd = d_dmap.find( a );
  if( itd!=d_dmap.end() ){
    for( int i=0; i<(int)itd->second.size(); i++ ){
      if( getRepresentative( itd->second[i] )==b ){
        return true;
      }
    }
  }
  return false;
}

Node InstantiatorTheoryUf::getRepresentative( Node a ){
  if( ((TheoryUF*)d_th)->d_equalityEngine.hasTerm( a ) ){
    return ((TheoryUF*)d_th)->d_equalityEngine.getRepresentative( a );
  }else{
    return a;
  }
}

bool InstantiatorTheoryUf::hasInstantiationConstantsFrom( Node i, Node f )
{
  //DO_THIS
  return i.getAttribute(InstantitionConstantAttribute())==f;
}

//bool InstantiatorTheoryUf::decideEqual( Node a, Node b )
//{
//  if( !areEqual( a, b ) && !areDisequal( a, b ) ){
//    if( !a.hasAttribute(InstantitionConstantAttribute()) && b.hasAttribute(InstantitionConstantAttribute()) ){
//      Node t = a;
//      a = b;
//      b = t;
//    }
//    a = getRepresentative( a );
//    b = getRepresentative( b );
//
//    if( d_emap.find( a )==d_emap.end() ){
//      d_emap[a].push_back( a );
//    }
//    std::map< Node, std::vector< Node > >::iterator ite = d_emap.find( a );
//    for( int i=0; i<(int)ite->second.size(); i++ ){
//      d_emap[b].push_back( ite->second[i] );
//    }
//    d_emap.erase( ite );
//
//    std::map< Node, std::vector< Node > >::iterator itd = d_dmap.find( a );
//    if( itd!=d_dmap.end() ){
//      for( int i=0; i<(int)itd->second.size(); i++ ){
//        if( std::find( d_dmap[b].begin(), d_dmap[b].end(), itd->second[i] )==d_dmap[b].end() ){
//          d_dmap[b].push_back( itd->second[i] );
//        }
//      }
//      d_dmap.erase( itd );
//    }
//
//    d_eq_find[a] = b;
//    return true;
//  }else{
//    return false;
//  }
//}



void InstantiatorTheoryUf::calculateMatches( Node f, Effort e )
{
  Debug("quant-uf") << "Try to solve (" << e << ") for " << f << "... " << std::endl;
  //calculate base matches
  InstMatch base( f, d_instEngine );
  //check if any instantiation constants are solved for
  for( int j = 0; j<(int)d_instEngine->d_inst_constants[f].size(); j++ ){
    Node i = d_instEngine->d_inst_constants[f][j];
    Node c;
    if( d_terms_full.find( i )==d_terms_full.end() ){
      //i does not exist in a literal in the current context, can use fresh constant
      c = NodeManager::currentNM()->mkVar( i.getType() ); 
      //c = d_instEngine->d_skolem_constants[f][j];  //for convience/performance, use skolem constant?
    }else{
      c = getConcreteTermEqualTo( i );
    }
    if( c!=Node::null() ){
      base.setMatch( i, c );
    }
  }
  if( base.isComplete() ){
    //f is instantiation ready
    d_inst_matches.d_matches.push_back( base );
  }else{
    if( e>MIN_EFFORT ){
      NodeLists::iterator ob_i = d_obligations.find( f );
      if( ob_i!=d_obligations.end() ){
        NodeList* ob = (*ob_i).second;
        std::map< Node, InstMatchGroup > obMatches;
        // for each literal asserted about the negation of the body of f
        for( NodeList::const_iterator it = ob->begin(); it != ob->end(); ++it ){
          Debug("quant-uf-alg") << "Process obligation " << (*it) << std::endl;

          if( (*it).getKind()==NOT ){
            Assert( (*it)[0][0].getAttribute(InstantitionConstantAttribute())==f );
            if( !(*it)[0][1].hasAttribute(InstantitionConstantAttribute()) ){
              //a disequality between a trigger and ground term
              Node i = (*it)[0][0];
              if( e<FULL_EFFORT ){
                Node c = getRepresentative( (*it)[0][1] );
                //match against all equivalence classes disequal from c
                for( int j=0; j<(int)d_dmap[c].size(); j++ ){
                  Node ci = d_dmap[c][j];
                  Assert( ci==getRepresentative( ci ) );
                  if( e==QUICK_EFFORT ){
                    doEMatchMod( i, ci, f );  
                    obMatches[ (*it) ].add( d_ematch_mod[i][ci] );
                  }else{
                    doPartialEMatchMod( i, ci, f );
                    obMatches[ (*it) ].add( d_partial_ematch_mod[i][ci] );
                  }
                }
              }else{
                //find all matches that are not equal to c
                doEMatchFull( i, f );
                obMatches[ (*it) ].add( d_ematch_full[i] );
              }
            }else{
              //a disequality between two triggers
              Node i1 = (*it)[0][0];
              Node i2 = (*it)[0][1];
              if( e<FULL_EFFORT ){
                //for each equivalence class E
                for( std::map< Node, std::vector< Node > > ::iterator it1 = d_emap.begin(); it1!=d_emap.end(); ++it1 ){
                  Node ci1 = (*it1).first;
                  Assert( ci1==getRepresentative( ci1 ) );
                  if( ci1.getType()==i1.getType() ){
                    //for each equivalence class disequal from E
                    for( int j=0; j<(int)d_dmap[ci1].size(); j++ ){
                      Node ci2 = d_dmap[ci1][j];
                      Assert( ci2==getRepresentative( ci2 ) );
                      InstMatchGroup mg;
                      if( e==QUICK_EFFORT ){
                        doEMatchMod( i1, ci1, f );
                        mg.add( d_ematch_mod[i1][ci1] );
                        doEMatchMod( i2, ci2, f );
                        mg.merge( d_ematch_mod[i2][ci2] );
                      }else{
                        doPartialEMatchMod( i1, ci1, f );
                        mg.add( d_partial_ematch_mod[i1][ci1] );
                        doPartialEMatchMod( i2, ci2, f );
                        mg.combine( d_partial_ematch_mod[i2][ci2] );
                      }
                      obMatches[ (*it) ].add( mg );
                    }
                  }
                }
              }else{
                //E-match any two terms
                doEMatchFull( i1, f );
                obMatches[ (*it) ].add( d_ematch_full[ i1 ] );
                doEMatchFull( i2, f );
                obMatches[ (*it) ].combine( d_ematch_full[ i2 ] );
              }
            }
          }else{
            Assert( (*it).getKind()==IFF || (*it).getKind()==EQUAL );
            if( !(*it)[1].hasAttribute(InstantitionConstantAttribute()) ){
              //equality between trigger and concrete ground term
              Node i = (*it)[0];
              if( e<FULL_EFFORT ){
                Node c = getRepresentative( (*it)[1] );
                if( e==QUICK_EFFORT ){
                  //build E-matches with terms in the same equivalence class
                  doEMatchMod( i, c, f );
                  obMatches[ (*it) ].add( d_ematch_mod[i][c] );
                }else if( e==STANDARD_EFFORT ){
                  doPartialEMatchMod( i, c, f );
                  obMatches[ (*it) ].add( d_partial_ematch_mod[i][c] );
                }
              }else{
                doEMatchFull( i, f );
                obMatches[ (*it) ].add( d_ematch_full[i] );
              }
            }else{
              //equality between two triggers
              Node i1 = (*it)[0];
              Node i2 = (*it)[1];
              if( e<FULL_EFFORT ){
                //for each equivalence class
                for( std::map< Node, std::vector< Node > > ::iterator it1 = d_emap.begin(); it1!=d_emap.end(); ++it1 ){
                  Node c = (*it1).first;
                  if( c.getType()==i1.getType() ){
                    InstMatchGroup mg;
                    if( e==QUICK_EFFORT ){
                      doEMatchMod( i1, c, f );
                      mg.add( d_ematch_mod[i1][c] );
                      doEMatchMod( i2, c, f );
                      mg.merge( d_ematch_mod[i2][c] );
                    }else{
                      doPartialEMatchMod( i1, c, f );
                      mg.add( d_partial_ematch_mod[i1][c] );
                      doPartialEMatchMod( i2, c, f );
                      mg.combine( d_partial_ematch_mod[i2][c] );
                    }
                    obMatches[ (*it) ].add( mg );
                  }
                }
              }else{
                //E-match any two terms
                doEMatchFull( i1, f );
                obMatches[ (*it) ].add( d_ematch_full[ i1 ] );
                doEMatchFull( i2, f );
                obMatches[ (*it) ].combine( d_ematch_full[ i2 ] );
              }
            }
          }
          obMatches[ (*it) ].removeDuplicate();
          Debug("quant-uf-alg") << "Finished creating obligation matches." << std::endl;
          if( obMatches[ (*it) ].d_matches.size()>0 ){
            Debug("quant-uf-alg") << "(Partial) matches for " << (*it) << " : " << std::endl;
            obMatches[ (*it) ].debugPrint( "quant-uf-alg" );
          }
        }
        Debug("quant-uf-alg") << "Build combined matches..." << std::endl;
        //build complete matches
        if( e==QUICK_EFFORT ){
          InstMatchGroup combined;
          bool firstTime = true;
          for( std::map< Node, InstMatchGroup >::iterator it = obMatches.begin(); it != obMatches.end(); ++it ){
            if( firstTime ){
              combined.add( it->second );
              firstTime = false;
            }else{
              combined.merge( it->second );
            }
          }
          d_inst_matches.addComplete( combined, &base );
        }else{
          InstMatchGroup combined;
          for( std::map< Node, InstMatchGroup >::iterator it = obMatches.begin(); it != obMatches.end(); ++it ){
            combined.combine( it->second );
            d_inst_matches.addComplete( combined, &base );
          }
        }
        //Debug("quant-uf-alg") << "Combined matches : " << std::endl;
        //combined.debugPrint( "quant-uf-alg" );
      }
    }
  }


  Debug("quant-uf") << std::endl;
}

void InstantiatorTheoryUf::doEMatch( Node i, Node c, Node f ){
  //Node oi = i;
  //Node oc = c;
  //std::cout << "--> " << oi << " " << oc << std::endl;
  Assert( i.getType()==c.getType() );
  Assert( i.getAttribute(InstantitionConstantAttribute())==f );
  Assert( !c.hasAttribute(InstantitionConstantAttribute()) );
  if( !d_ematch[i][c].d_is_set ){
    d_ematch[i][c].d_is_set = true;
    Debug("quant-uf-ematch") << "E-match " << i << " " << c << std::endl;
    if( !areDisequal( i, c ) ){
      //modulo equality
      //if not already generated
      if( i.getKind()==INST_CONSTANT ){
        InstMatch m( f, d_instEngine );
        if( !areEqual( i, c ) ){
          m.setMatch( i, getRepresentative( c ) );
        }
        Debug("quant-uf-ematch") << i << " and " << c << " Ematched." << std::endl;
        d_does_ematch[i][c] = true;
        d_ematch[i][c].d_matches.push_back( m );
      }else if( c.getKind()!=APPLY_UF || i.getOperator()!=c.getOperator() ){
        //equality independent, do nothing
        d_does_ematch[i][c] = false;
        d_eq_amb[i][c] = false;
        Debug("quant-uf-ematch") << i << " and " << c << " FAILED operators." << std::endl;
      }else{
        Assert( i.getNumChildren()==c.getNumChildren() );
        d_does_ematch[i][c] = true;
        d_eq_amb[i][c] = true;
        InstMatch m( f, d_instEngine );
        d_ematch[i][c].d_matches.push_back( m );
        for( int j=0; j<(int)c.getNumChildren(); j++ ){
          if( areDisequal( i[j], c[j] ) ){
            Debug("quant-uf-ematch") << i << " and " << c << " FAILED disequal arg. " << j << std::endl;
            d_does_ematch[i][c] = false;
            d_eq_amb[i][c] = false;
            d_ematch[i][c].clear();
            break;
          }else if( !areEqual( i[j], c[j] ) && d_does_ematch[i][c] && !d_ematch[i][c].empty() ){
            if( i[j].getAttribute(InstantitionConstantAttribute())==f ){
              Node ca = getRepresentative( c[j] );
              doEMatchMod( i[j], ca, f );
              if( !d_ematch[i][c].merge( d_ematch_mod[i[j]][ca] ) ){
                Debug("quant-uf-ematch") << i << " and " << c << " FAILED incompatible match. " << j << std::endl;
                d_does_ematch[i][c] = false;
                d_ematch[i][c].clear();
              }
            }else{
              Debug("quant-uf-ematch") << i << " and " << c << " FAILED unequal arg." << j << std::endl;
              d_does_ematch[i][c] = false;
              d_ematch[i][c].clear();
            }
          }
        }
      }
    }else{
      Debug("quant-uf-ematch") << i << " and " << c << " FAILED disequal." << std::endl;
      d_does_ematch[i][c] = false;
      d_eq_amb[i][c] = false;
    }
    Assert( d_does_ematch.find( i )!=d_does_ematch.end() );
    Assert( d_does_ematch[i].find( c )!=d_does_ematch[i].end() );
    Assert( d_does_ematch[i][c] || ( d_eq_amb.find( i )!=d_eq_amb.end() && d_eq_amb[i].find( c )!=d_eq_amb[i].end() ) );
  }
  //std::cout << "<-- " << oi << " " << oc << " " << moduloEq << std::endl;
}

void InstantiatorTheoryUf::doEMatchMod( Node i, Node c, Node f ){
  Debug("quant-uf-ematch") << "E-match moddd " << i << " " << c << " " << getRepresentative( c ) << std::endl;
  Assert( i.getType()==c.getType() );
  Assert( c==getRepresentative( c ) ); 
  if( !d_ematch_mod[i][c].d_is_set ){
    d_ematch_mod[i][c].d_is_set = true;
    Debug("quant-uf-ematch") << "E-match mod " << i << " " << c << std::endl;
    if( !areDisequal( i, c ) ){
      if( i.getKind()==INST_CONSTANT || d_emap[c].empty() ){
        if( !c.hasAttribute(InstantitionConstantAttribute()) ){
          doEMatch( i, c, f );
          if( d_does_ematch[i][c] ){
            d_ematch_mod[i][c].add( d_ematch[i][c] );
          }
        }
      }else{
        for( int j=0; j<(int)d_emap[c].size(); j++ ){
          Node ca = d_emap[c][j];
          if( !ca.hasAttribute(InstantitionConstantAttribute()) ){
            doEMatch( i, ca, f );
            if( d_does_ematch[i][ca] ){
              d_ematch_mod[i][c].add( d_ematch[i][ca] );
            }
          }
        }
        d_ematch_mod[i][c].removeRedundant();
      }
    }
  }
}


void InstantiatorTheoryUf::doPartialEMatch( Node i, Node c, Node f ){
  Assert( i.getType()==c.getType() );
  Assert( i.getAttribute(InstantitionConstantAttribute())==f );
  Assert( !c.hasAttribute(InstantitionConstantAttribute()) );
  if( !d_partial_ematch[i][c].d_is_set ){
    d_partial_ematch[i][c].d_is_set = true;
    Debug("quant-uf-ematch") << "Partial E-match " << i << " " << c << std::endl;
    doEMatch( i, c, f );
    if( d_does_ematch[i][c] ){
      Debug("quant-uf-ematch") << i << " and " << c << " fully match." << std::endl;
      d_partial_ematch[i][c].add( d_ematch[i][c] );
    }else if( d_eq_amb[i][c] ){
      Debug("quant-uf-ematch") << i << " and " << c << " are ambiguous." << std::endl;
      Assert( i.getKind()==APPLY_UF && c.getKind()==APPLY_UF );
      Assert( i.getOperator()==c.getOperator() );
      Assert( i.getNumChildren()==c.getNumChildren() );
      for( int j=0; j<(int)c.getNumChildren(); j++ ){
        Assert( !areDisequal( i[j], c[j] ) );
        if( !areEqual( i[j], c[j] ) && i[j].getAttribute(InstantitionConstantAttribute())==f ){
          Node cj = getRepresentative( c[j] );
          doPartialEMatchMod( i[j], cj, f );
          d_partial_ematch[i][c].add( d_partial_ematch_mod[i[j]][cj] );
        }
      }
    }
  }
}

void InstantiatorTheoryUf::doPartialEMatchMod( Node i, Node c, Node f ){
  Assert( i.getType()==c.getType() );
  Assert( c==getRepresentative( c ) ); 
  if( !d_partial_ematch_mod[i][c].d_is_set ){
    d_partial_ematch_mod[i][c].d_is_set = true;
    if( !areDisequal( i, c ) ){
      if( i.getKind()==INST_CONSTANT || d_emap[c].empty() ){
        if( !c.hasAttribute(InstantitionConstantAttribute()) ){
          doPartialEMatch( i, c, f );
          d_partial_ematch_mod[i][c].add( d_partial_ematch[i][c] );
        }
      }else{
        for( int j=0; j<(int)d_emap[c].size(); j++ ){
          Node ca = d_emap[c][j];
          if( !ca.hasAttribute(InstantitionConstantAttribute()) ){
            doPartialEMatch( i, ca, f );
            d_partial_ematch_mod[i][c].add( d_partial_ematch[i][ca] );
          }
        }
        d_partial_ematch_mod[i][c].removeDuplicate();
      }
    }
  }
}

void InstantiatorTheoryUf::doEMatchFull( Node i, Node f ){
  Assert( i.getAttribute(InstantitionConstantAttribute())==f );
  if( !d_ematch_full[i].d_is_set ){
    d_ematch_full[i].d_is_set = true;
    for( BoolMap::const_iterator it = d_terms.begin(); it!=d_terms.end(); ++it ){
      Node c = (*it).first;
      if( !c.hasAttribute(InstantitionConstantAttribute()) && i.getType()==c.getType() ){
        doPartialEMatch( i, c, f );
        d_ematch_full[i].add( d_partial_ematch[i][c] );
      }
    }
    d_ematch_full[i].removeDuplicate();
  }
}

//void InstantiatorTheoryUf::partialMerge( InstMatchGroup& m1, InstMatchGroup& m2, 
//                                         std::map< Node, Node >& splits ){
//  splits.clear();
//  bool splitsSet = false;
//  for( int i=0; i<(int)m1.d_matches.size(); i++ ){
//    for( int j=0; j<(int)m2.d_matches.size(); j++ ){
//      std::map< Node, Node > tempSplits;
//      m1.d_matches[i].partialMerge( m2.d_matches[j], tempSplits );
//      if( !splitsSet || tempSplits.size()<splits.size() ){
//        bool consistent = true;
//        for( std::map< Node, Node >::iterator it=tempSplits.begin(); it!=tempSplits.end(); ++it ){
//          if( areDisequal( it->first, it->second ) ){
//            consistent = false;
//            break;
//          }
//        }
//        if( consistent ){
//          splits.clear();
//          for( std::map< Node, Node >::iterator it=tempSplits.begin(); it!=tempSplits.end(); ++it ){
//            splits[it->first] = it->second;
//          }
//          splitsSet = true;
//        }
//      }
//    }
//  }
//}


//void InstantiatorTheoryUf::setEmatchDone( Node i, Node c ){
//  NodeLists::iterator emdi_i = d_ematch_done.find( i );
//  NodeList* emdi;
//  if( emdi_i == d_ematch_done.end() ) {
//    emdi = new(d_th->getContext()->getCMM()) NodeList(true, d_th->getContext(), false,
//                                                      ContextMemoryAllocator<Node>(d_th->getContext()->getCMM()));
//    d_ematch_done.insertDataFromContextMemory(i, emdi);
//  }else{
//    emdi = (*emdi_i).second;
//  }
//  emdi->push_back( c );
//  Assert( isEmatchDone( i, c ) );
//}
//
//bool InstantiatorTheoryUf::isEmatchDone( Node i, Node c ){
//  NodeLists::iterator emdi_i = d_ematch_done.find( i );
//  if( emdi_i != d_ematch_done.end() ){
//    NodeList* emdi = (*emdi_i).second;
//    for( NodeList::const_iterator it = emdi->begin(); it != emdi->end(); it++ ) {
//      if( *it == c ){
//        return true;
//      }
//    }
//  }
//  return false;
//}
//
//bool InstantiatorTheoryUf::isGeneralization( Node i1, Node i2 )
//{
//  if( i1==i2 ){
//    return true;
//  }else if( i1.getKind()==INST_CONSTANT ){
//    return i2.getKind()!=INST_CONSTANT;
//  }else if( i1.getKind()==APPLY_UF ){
//    if( i2.getKind()==APPLY_UF && i1.getOperator()==i2.getOperator() ){
//      for( int i=0; i<(int)i1.getNumChildren(); i++ ){
//        if( !isGeneralization( i1[i], i2[i] ) ){
//          return false;
//        }
//      }
//      return true;
//    }
//  }
//  return false;
//}