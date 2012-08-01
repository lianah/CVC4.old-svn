/*********************                                                        */
/*! \file literal_match_mode.cpp
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009-2012  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Add one-line brief description here ]]
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

#include <iostream>
#include "theory/quantifiers/literal_match_mode.h"

namespace CVC4 {

std::ostream& operator<<(std::ostream& out, theory::quantifiers::LiteralMatchMode mode) {
  switch(mode) {
  case theory::quantifiers::LITERAL_MATCH_NONE:
    out << "LITERAL_MATCH_NONE";
    break;
  case theory::quantifiers::LITERAL_MATCH_PREDICATE:
    out << "LITERAL_MATCH_PREDICATE";
    break;
  case theory::quantifiers::LITERAL_MATCH_EQUALITY:
    out << "LITERAL_MATCH_EQUALITY";
    break;
  default:
    out << "LiteralMatchMode!UNKNOWN";
  }

  return out;
}

}/* CVC4 namespace */
