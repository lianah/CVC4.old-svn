/*********************                                                        */
/*! \file quantifiers_rewriter.h
 ** \verbatim
 ** Original author: ajreynol
 ** Major contributors: mdeters
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Rewriter for the theory of inductive quantifiers
 **
 ** Rewriter for the theory of inductive quantifiers.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__QUANTIFIERS__QUANTIFIERS_REWRITER_H
#define __CVC4__THEORY__QUANTIFIERS__QUANTIFIERS_REWRITER_H

#include "theory/rewriter.h"

namespace CVC4 {
namespace theory {
namespace quantifiers {

// attribute for "contains instantiation constants from"
struct NestedQuantAttributeId {};
typedef expr::Attribute<NestedQuantAttributeId, Node> NestedQuantAttribute;

class QuantifiersRewriter {
private:
  static bool isClause( Node n );
  static bool isLiteral( Node n );
  static bool isCube( Node n );

  static void computeArgs( std::map< Node, bool >& active, Node n );
  static void computeArgs( std::vector< Node >& args, std::vector< Node >& activeArgs, Node n );
  static Node mkForAll( std::vector< Node >& args, Node n );

  static void setNestedQuantifiers( Node n, Node q );
public:

  static RewriteResponse postRewrite(TNode in) {
    Debug("quantifiers-rewrite") << "post-rewriting " << in << std::endl;
#if 1
#else
    if( in.getKind()==kind::EXISTS || in.getKind()==kind::FORALL ){
      if( in.hasAttribute(NestedQuantAttribute()) ){
        Debug("quantifiers-rewrite") << "It is a nested quantifier." << std::endl;
        if( in.getKind()==kind::EXISTS ){
          std::vector< Node > children;
          for( int i=0; i<(int)in.getNumChildren(); i++ ){
            if( i==(int)in.getNumChildren()-1 ){
              children.push_back( in[i].notNode() );
            }else{
              children.push_back( in[i] );
            }
          }
          Node n = NodeManager::currentNM()->mkNode(kind::FORALL, children );
          Debug("quantifiers-rewrite") << "Rewrite " << in << " to " << n.notNode() << std::endl;
          return RewriteResponse(REWRITE_DONE, n.notNode() );
        }
      }else{
        std::vector< Node > args;
        for( int i=0; i<(int)(in.getNumChildren()-1); i++ ){
          args.push_back( in[i] );
        }
        Node n = rewriteQuant( args, in[ in.getNumChildren()-1 ], in.getKind()==kind::EXISTS );
        Debug("quantifiers-rewrite") << "Rewrite " << in << " to " << n << std::endl;
        return RewriteResponse(REWRITE_DONE, n );
      }
    }
#endif
    return RewriteResponse(REWRITE_DONE, in);
  }

  static RewriteResponse preRewrite(TNode in) {
    Debug("quantifiers-rewrite") << "pre-rewriting " << in << std::endl;
#if 1
    if( in.getKind()==kind::EXISTS ){
      std::vector< Node > children;
      for( int i=0; i<(int)in.getNumChildren(); i++ ){
        if( i==(int)in.getNumChildren()-1 ){
          children.push_back( in[i].notNode() );
        }else{
          children.push_back( in[i] );
        }
      }
      Node n = NodeManager::currentNM()->mkNode(kind::FORALL, children );
      Debug("quantifiers-rewrite") << "Rewrite " << in << " to " << n.notNode() << std::endl;
      return RewriteResponse(REWRITE_DONE, n.notNode() );
    }
#else
    if( in.getKind()==kind::EXISTS || in.getKind()==kind::FORALL ){
      if( !in.hasAttribute(NestedQuantAttribute()) ){
        setNestedQuantifiers( in[ in.getNumChildren()-1 ], in );
      }
    }
#endif
    return RewriteResponse(REWRITE_DONE, in);
  }

  static Node rewriteEquality(TNode equality) {
    return postRewrite(equality).node;
  }

  static inline void init() {}
  static inline void shutdown() {}

  /** returns a literal, writes new quantifier definitions into nb */
  static Node rewriteQuant( std::vector< Node >& args, Node body, NodeBuilder<>& nb );

  static Node rewriteQuant( std::vector< Node >& args, Node body, bool isExists = false );

};/* class QuantifiersRewriter */

}/* CVC4::theory::quantifiers namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__QUANTIFIERS__QUANTIFIERS_REWRITER_H */

