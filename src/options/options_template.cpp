/*********************                                                        */
/*! \file options_template.cpp
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Contains code for handling command-line options.
 **
 ** Contains code for handling command-line options
 **/

#include <cstdio>
#include <cstdlib>
#include <new>
#include <string>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <getopt.h>

#include "expr/expr.h"
#include "util/configuration.h"
#include "util/exception.h"
#include "util/language.h"

${include_all_option_headers}

#line 38 "${template}"

#include "util/output.h"

#include "options/options_holder.h"

#include "cvc4autoconfig.h"

${option_handler_includes}

#line 48 "${template}"

using namespace std;
using namespace CVC4;

namespace CVC4 {

CVC4_THREADLOCAL(Options*) Options::s_current = NULL;

template <class T> T handleOption(std::string option, std::string optarg) {
  T::unsupported_handleOption_call___please_write_me;
  return *(T*)0;
}

template <> unsigned short handleOption<unsigned short>(std::string option, std::string optarg) {
  int i = atoi(optarg.c_str());
  if(i < 0) {
    throw OptionException(option + " requires a nonnegative argument.");
  }
  return (unsigned short) i;
}

template <> int handleOption<int>(std::string option, std::string optarg) {
  return atoi(optarg.c_str());
}

template <> unsigned handleOption<unsigned>(std::string option, std::string optarg) {
  int i = atoi(optarg.c_str());
  if(i < 0) {
    throw OptionException(option + " requires a nonnegative argument.");
  }
  return (unsigned) i;
}

template <> double handleOption<double>(std::string option, std::string optarg) {
  return atof(optarg.c_str());
}

template <> unsigned long handleOption<unsigned long>(std::string option, std::string optarg) {
  int i = atoi(optarg.c_str());
  if(i < 0) {
    throw OptionException(option + " requires a nonnegative argument.");
  }
  return (unsigned long) i;
}

template <class T>
typename T::type runHandlers(T, std::string option, std::string optarg) {
  // By default, parse the option argument in a way appropriate for its type.
  // E.g., for "unsigned int" options, ensure that the provided argument is
  // a nonnegative integer that fits in the unsigned int type.

  return handleOption<typename T::type>(option, optarg);
}

template <class T>
void runBoolHandlers(T, std::string option, bool b) {
  // By default, nothing to handle with bool.
  // Users override with :handler in options files to
  // provide custom handlers that can throw exceptions.
}

${all_custom_handlers}

#line 112 "${template}"

#ifdef CVC4_DEBUG
#  define USE_EARLY_TYPE_CHECKING_BY_DEFAULT true
#else /* CVC4_DEBUG */
#  define USE_EARLY_TYPE_CHECKING_BY_DEFAULT false
#endif /* CVC4_DEBUG */

#if defined(CVC4_MUZZLED) || defined(CVC4_COMPETITION_MODE)
#  define DO_SEMANTIC_CHECKS_BY_DEFAULT false
#else /* CVC4_MUZZLED || CVC4_COMPETITION_MODE */
#  define DO_SEMANTIC_CHECKS_BY_DEFAULT true
#endif /* CVC4_MUZZLED || CVC4_COMPETITION_MODE */

Options::Options() :
  d_holder(new options::OptionsHolder()) {
}

Options::Options(const Options& options) :
  d_holder(new options::OptionsHolder(*options.d_holder)) {
}

Options::~Options() {
  delete d_holder;
}

options::OptionsHolder::OptionsHolder() : ${all_modules_defaults}
{
}

#line 142 "${template}"

static const string mostCommonOptionsDescription = "\
Most commonly-used CVC4 options:${common_documentation}";

#line 147 "${template}"

static const string optionsDescription = mostCommonOptionsDescription + "\n\
\n\
Additional CVC4 options:${remaining_documentation}";

#line 153 "${template}"

static const string languageDescription = "\
Languages currently supported as arguments to the -L / --lang option:\n\
  auto           attempt to automatically determine the input language\n\
  pl | cvc4      CVC4 presentation language\n\
  smt | smtlib   SMT-LIB format 1.2\n\
  smt2 | smtlib2 SMT-LIB format 2.0\n\
\n\
Languages currently supported as arguments to the --output-lang option:\n\
  auto           match the output language to the input language\n\
  pl | cvc4      CVC4 presentation language\n\
  smt | smtlib   SMT-LIB format 1.2\n\
  smt2 | smtlib2 SMT-LIB format 2.0\n\
  ast            internal format (simple syntax-tree language)\n\
";

string Options::getDescription() const {
  return optionsDescription;
}

void Options::printUsage(const std::string msg, std::ostream& out) {
  out << msg << optionsDescription << endl << flush;
}

void Options::printShortUsage(const std::string msg, std::ostream& out) {
  out << msg << mostCommonOptionsDescription << endl << endl
      << "For full usage, please use --help." << endl << flush;
}

void Options::printLanguageHelp(std::ostream& out) {
  out << languageDescription << flush;
}

/**
 * This is a table of long options.  By policy, each short option
 * should have an equivalent long option (but the reverse isn't the
 * case), so this table should thus contain all command-line options.
 *
 * Each option in this array has four elements:
 *
 * 1. the long option string
 * 2. argument behavior for the option:
 *    no_argument - no argument permitted
 *    required_argument - an argument is expected
 *    optional_argument - an argument is permitted but not required
 * 3. this is a pointer to an int which is set to the 4th entry of the
 *    array if the option is present; or NULL, in which case
 *    getopt_long() returns the 4th entry
 * 4. the return value for getopt_long() when this long option (or the
 *    value to set the 3rd entry to; see #3)
 *
 * If you add something here, you should add it in src/main/usage.h
 * also, to document it.
 *
 * If you add something that has a short option equivalent, you should
 * add it to the getopt_long() call in parseOptions().
 */
static struct option cmdlineOptions[] = {${all_modules_long_options}
  { NULL, no_argument, NULL, '\0' }
};/* cmdlineOptions */

#line 215 "${template}"

static void preemptGetopt(int& argc, char**& argv, const char* opt) {
  const size_t maxoptlen = 128;

  AlwaysAssert(opt != NULL && *opt != '\0');
  AlwaysAssert(strlen(opt) <= maxoptlen);

  ++argc;
  unsigned i = 0;
  while(argv[i] != NULL && argv[i][0] != '\0') {
    ++i;
  }

  if(argv[i] == NULL) {
    argv = (char**) realloc(argv, (i + 6) * sizeof(char*));
    for(unsigned j = i; j < i + 5; ++j) {
      argv[j] = (char*) malloc(sizeof(char) * maxoptlen);
      argv[j][0] = '\0';
    }
    argv[i + 5] = NULL;
  }

  strncpy(argv[i], opt, maxoptlen - 1);
  argv[i][maxoptlen - 1] = '\0'; // ensure NUL-termination even on overflow
}

/** Parse argc/argv and put the result into a CVC4::Options. */
int Options::parseOptions(int argc, char* argv[]) throw(OptionException) {
  s_current = this;

  const char *progName = argv[0];

  // find the base name of the program
  const char *x = strrchr(progName, '/');
  if(x != NULL) {
    progName = x + 1;
  }
  d_holder->binary_name = string(progName);

  int extra_argc = 0;
  char **extra_argv = (char**) malloc(sizeof(char*));
  extra_argv[0] = NULL;

  int extra_optind = 1, main_optind = 1;

  for(;;) {
    int c = -1;
    if(extra_optind < extra_argc) {
#if HAVE_DECL_OPTRESET
      optreset = 1; // on BSD getopt() (e.g. Mac OS), might also need this
#endif /* HAVE_DECL_OPTRESET */
      optind = extra_optind;
      c = getopt_long(extra_argc, extra_argv,
                      ":${all_modules_short_options}",
                      cmdlineOptions, NULL);
      if(extra_optind >= extra_argc) {
        unsigned i = 0;
        while(extra_argv[i] != NULL && extra_argv[i][0] != '\0') {
          extra_argv[i][0] = '\0';
          ++i;
        }
        extra_argc = extra_optind = 0;
      } else {
        extra_optind = optind;
      }
    }
    if(c == -1) {
#if HAVE_DECL_OPTRESET
      optreset = 1; // on BSD getopt() (e.g. Mac OS), might also need this
#endif /* HAVE_DECL_OPTRESET */
      optind = main_optind;
      c = getopt_long(argc, argv,
                      ":${all_modules_short_options}",
                      cmdlineOptions, NULL);
      main_optind = optind;
      if(c == -1) {
        break;
      }
    }

    switch(c) {
${all_modules_option_handlers}

#line 299 "${template}"

    case ':':
      // This can be a long or short option, and the way to get at the name of it is different.
      if(optopt == 0) { // was a long option
        throw OptionException(string("option `") + argv[optind - 1] + "' missing its required argument");
      } else { // was a short option
        throw OptionException(string("option `-") + char(optopt) + "' missing its required argument");
      }

    case '?':
    default:
      // This can be a long or short option, and the way to get at the name of it is different.
      if(optopt == 0) { // was a long option
        throw OptionException(string("can't understand option `") + argv[optind - 1] + "'");
      } else { // was a short option
        throw OptionException(string("can't understand option `-") + char(optopt) + "'");
      }
    }
  }

  if((*this)[options::incrementalSolving] && (*this)[options::proof]) {
    throw OptionException(string("The use of --incremental with --proof is not yet supported"));
  }

  return optind;
}

std::ostream& operator<<(std::ostream& out, SimplificationMode mode) {
  switch(mode) {
  case SIMPLIFICATION_MODE_INCREMENTAL:
    out << "SIMPLIFICATION_MODE_INCREMENTAL";
    break;
  case SIMPLIFICATION_MODE_BATCH:
    out << "SIMPLIFICATION_MODE_BATCH";
    break;
  case SIMPLIFICATION_MODE_NONE:
    out << "SIMPLIFICATION_MODE_NONE";
    break;
  default:
    out << "SimplificationMode:UNKNOWN![" << unsigned(mode) << "]";
  }

  return out;
}

std::ostream& operator<<(std::ostream& out, ArithPivotRule rule) {
  switch(rule) {
  case MINIMUM:
    out << "MINIMUM";
    break;
  case BREAK_TIES:
    out << "BREAK_TIES";
    break;
  case MAXIMUM:
    out << "MAXIMUM";
    break;
  default:
    out << "ArithPivotRule!UNKNOWN";
  }

  return out;
}

#undef USE_EARLY_TYPE_CHECKING_BY_DEFAULT
#undef DO_SEMANTIC_CHECKS_BY_DEFAULT

}/* CVC4 namespace */
