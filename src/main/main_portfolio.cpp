#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "util/stats.h"

using namespace std;
using namespace CVC4;
using namespace CVC4::main;

/**
 * CVC4's main() routine is just an exception-safe wrapper around CVC4.
 * Please don't construct anything here.  Even if the constructor is
 * inside the try { }, an exception during destruction can cause
 * problems.  That's why main() wraps runCvc4() in the first place.
 * Put everything in runCvc4().
 */
int main(int argc, char* argv[]) {
  Options options;
  try {
    return runCvc4Portfolio(2, argc, argv, options);
  } catch(OptionException& e) {
    *options.out << "unknown" << endl;
    cerr << "CVC4 Error:" << endl << e << endl;
    printUsage(options);
    exit(1);
  } catch(Exception& e) {
#ifdef CVC4_COMPETITION_MODE
    *options.out << "unknown" << endl;
#endif
    *options.err << "CVC4 Error:" << endl << e << endl;
    if(options.statistics) {
      pStatistics->flushStatistics(*options.err);
    }
    exit(1);
  } catch(bad_alloc) {
#ifdef CVC4_COMPETITION_MODE
    *options.out << "unknown" << endl;
#endif
    *options.err << "CVC4 ran out of memory." << endl;
    if(options.statistics) {
      pStatistics->flushStatistics(*options.err);
    }
    exit(1);
  } catch(...) {
#ifdef CVC4_COMPETITION_MODE
    *options.out << "unknown" << endl;
#endif
    *options.err << "CVC4 threw an exception of unknown type." << endl;
    exit(1);
  }
}

