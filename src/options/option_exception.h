/*********************                                                        */
/*! \file option_exception.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: taking, cconway
 ** Minor contributors (to current version): dejan
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009-2012  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Options-related exceptions
 **
 ** Options-related exceptions.
 **/

#include "cvc4_public.h"

#ifndef __CVC4__OPTION_EXCEPTION_H
#define __CVC4__OPTION_EXCEPTION_H

#include "util/exception.h"

namespace CVC4 {

/**
 * Class representing an option-parsing exception such as badly-typed
 * or missing arguments, arguments out of bounds, etc.  If an option
 * name is itself unrecognized, a UnrecognizedOptionException (a derived
 * class, below) should be used instead.
 */
class CVC4_PUBLIC OptionException : public CVC4::Exception {
public:
  OptionException(const std::string& s) throw() :
    CVC4::Exception("Error in option parsing: " + s) {
  }
};/* class OptionException */

/**
 * Class representing an exception in option processing due to an
 * unrecognized or unsupported option key.
 */
class CVC4_PUBLIC UnrecognizedOptionException : public CVC4::OptionException {
public:
  UnrecognizedOptionException() :
    CVC4::OptionException("Unrecognized informational or option key or setting") {
  }

  UnrecognizedOptionException(const std::string& msg) :
    CVC4::OptionException(msg) {
  }
};/* class UnrecognizedOptionException */

}/* CVC4 namespace */

#endif /* __CVC4__OPTION_EXCEPTION_H */
