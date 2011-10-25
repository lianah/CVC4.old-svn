/*********************                                                        */
/*! \file configuration.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): acsys
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Interface to a public class that provides compile-time information
 ** about the CVC4 library.
 **
 ** Interface to a public class that provides compile-time information
 ** about the CVC4 library.
 **/

#include "cvc4_public.h"

#ifndef __CVC4__CONFIGURATION_H
#define __CVC4__CONFIGURATION_H

#include <string>

namespace CVC4 {

/**
 * Represents the (static) configuration of CVC4.
 */
class CVC4_PUBLIC Configuration {

  /** Private default ctor: Disallow construction of this class */
  Configuration();

  // these constants are filled in by the build system
  static const bool IS_SUBVERSION_BUILD;
  static const char* const SUBVERSION_BRANCH_NAME;
  static const unsigned SUBVERSION_REVISION;
  static const bool SUBVERSION_HAS_MODIFICATIONS;

public:

  static std::string getName();

  static bool isDebugBuild();

  static bool isStatisticsBuild();

  static bool isReplayBuild();

  static bool isTracingBuild();

  static bool isDumpingBuild();

  static bool isMuzzledBuild();

  static bool isAssertionBuild();

  static bool isCoverageBuild();

  static bool isProfilingBuild();

  static bool isCompetitionBuild();

  static std::string getPackageName();

  static std::string getVersionString();

  static unsigned getVersionMajor();

  static unsigned getVersionMinor();

  static unsigned getVersionRelease();

  static std::string getVersionExtra();

  static std::string about();

  static bool isBuiltWithGmp();

  static bool isBuiltWithCln();

  static bool isBuiltWithCudd();

  static bool isBuiltWithTlsSupport();

  static unsigned getNumDebugTags();
  static char const* const* getDebugTags();

  static unsigned getNumTraceTags();
  static char const* const* getTraceTags();

  static bool isSubversionBuild();
  static const char* getSubversionBranchName();
  static unsigned getSubversionRevision();
  static bool hasSubversionModifications();
  static std::string getSubversionId();

  static std::string getCompiler();
  static std::string getCompiledDateTime();

};/* class Configuration */

}/* CVC4 namespace */

#endif /* __CVC4__CONFIGURATION_H */
