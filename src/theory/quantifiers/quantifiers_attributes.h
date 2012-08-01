/*********************                                                        */
/*! \file quantifiers_attributes.h
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
 ** \brief Attributes for the theory quantifiers
 **
 ** Attributes for the theory quantifiers.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__QUANTIFIERS__QUANTIFIERS_REWRITER_H
#define __CVC4__THEORY__QUANTIFIERS__QUANTIFIERS_REWRITER_H

#include "theory/rewriter.h"
#include "theory/quantifiers_engine.h"

namespace CVC4 {
namespace theory {
namespace quantifiers {

struct QuantifiersAttributes
{
  /** set user attribute
    *   This function will apply a custom set of attributes to all top-level universal
    *   quantifiers contained in n
    */
  static void setUserAttribute( Node n, std::string& attr, Node attrExpr );
};


}
}
}

#endif
