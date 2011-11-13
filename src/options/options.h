/*********************                                                        */
/*! \file options.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: taking, cconway
 ** Minor contributors (to current version): dejan
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Global (command-line, set-option, ...) parameters for SMT.
 **
 ** Global (command-line, set-option, ...) parameters for SMT.
 **/

#include "cvc4_public.h"

#ifndef __CVC4__OPTIONS_H
#define __CVC4__OPTIONS_H

#include <iostream>
#include <fstream>
#include <string>

#include "util/exception.h"
#include "util/language.h"
#include "util/tls.h"

namespace CVC4 {

class ExprStream;
class OptionsHolder;
class NodeManager;
class NodeManagerScope;

/** Enumeration of simplification modes (when to simplify). */
typedef enum {
  /** Simplify the assertions as they come in */
  SIMPLIFICATION_MODE_INCREMENTAL,
  /** Simplify the assertions all together once a check is requested */
  SIMPLIFICATION_MODE_BATCH,
  /** Don't do simplification */
  SIMPLIFICATION_MODE_NONE
} SimplificationMode;

typedef enum {
  MINIMUM,
  BREAK_TIES,
  MAXIMUM
} ArithPivotRule;

/** Class representing an option-parsing exception. */
class CVC4_PUBLIC OptionException : public CVC4::Exception {
public:
  OptionException(const std::string& s) throw() :
    CVC4::Exception("Error in option parsing: " + s) {
  }
};/* class OptionException */

class CVC4_PUBLIC OptionsClass {
  OptionsHolder* d_holder;

  /** The current Options in effect */
  static CVC4_THREADLOCAL(OptionsClass*) s_current;

  friend class NodeManager;
  friend class NodeManagerScope;

public:

  /** Get the current Options in effect */
  static inline OptionsClass& current() {
    return *s_current;
  }

  OptionsClass();
  OptionsClass(const OptionsClass& options);
  ~OptionsClass();

  template <class T>
  void set(T, const typename T::type&) {
    T::you_are_trying_to_assign_to_a_read_only_option;
  }

  template <class T>
  const typename T::type& operator[](T) const;

  /**
   * Get a description of the command-line flags accepted by
   * parseOptions.  The returned string will be escaped so that it is
   * suitable as an argument to printf. */
  std::string getDescription() const;

  /**
   * Print overall command-line option usage message, prefixed by
   * "msg"---which could be an error message causing the usage
   * output in the first place, e.g. "no such option --foo"
   */
  static void printUsage(const std::string msg, std::ostream& out);

  /**
   * Print command-line option usage message for only the most-commonly
   * used options.  The message is prefixed by "msg"---which could be
   * an error message causing the usage output in the first place, e.g.
   * "no such option --foo"
   */
  static void printShortUsage(const std::string msg, std::ostream& out);

  /** Print help for the --lang command line option */
  static void printLanguageHelp(std::ostream& out);

  /**
   * Initialize the options based on the given command-line arguments.
   */
  int parseOptions(int argc, char* argv[]) throw(OptionException);

  /**
   * Set the output language based on the given string.
   */
  void setOutputLanguage(const char* str) throw(OptionException);

  /**
   * Set the input language based on the given string.
   */
  void setInputLanguage(const char* str) throw(OptionException);

};/* class OptionsClass */

/**
 * This function just exists to provide a nicer syntax for getting
 * current options: Options[foo]
 */
class CVC4_PUBLIC OptionObject {
public:
  template <class T>
  inline const typename T::type& operator[](T t) const {
    return OptionsClass::current()[t];
  }

  template <class T>
  void set(T t, const typename T::type& v) {
    OptionsClass::current().set(t, v);
  }
} Options;

std::ostream& operator<<(std::ostream& out, SimplificationMode mode) CVC4_PUBLIC;
std::ostream& operator<<(std::ostream& out, ArithPivotRule rule) CVC4_PUBLIC;

}/* CVC4 namespace */

#include "options/base_options.h"

#endif /* __CVC4__OPTIONS_H */