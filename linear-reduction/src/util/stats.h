/*********************                                                        */
/*! \file stats.h
 ** \verbatim
 ** Original author: taking
 ** Major contributors: mdeters
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
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

#include "cvc4_public.h"

#ifndef __CVC4__STATS_H
#define __CVC4__STATS_H

#include <string>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <ctime>

#include "util/Assert.h"
#include "lib/clock_gettime.h"

namespace CVC4 {


#ifdef CVC4_STATISTICS_ON
#  define __CVC4_USE_STATISTICS true
#else
#  define __CVC4_USE_STATISTICS false
#endif

class CVC4_PUBLIC Stat;

class CVC4_PUBLIC StatisticsRegistry {
private:
  struct StatCmp {
    inline bool operator()(const Stat* s1, const Stat* s2) const;
  };/* class StatisticsRegistry::StatCmp */

  typedef std::set< Stat*, StatCmp > StatSet;
  static StatSet d_registeredStats;

public:

  typedef StatSet::const_iterator const_iterator;

  static void flushStatistics(std::ostream& out);

  static inline void registerStat(Stat* s) throw(AssertionException);
  static inline void unregisterStat(Stat* s) throw(AssertionException);

  static inline const_iterator begin() {
    return d_registeredStats.begin();
  }
  static inline const_iterator end() {
    return d_registeredStats.end();
  }
};/* class StatisticsRegistry */


class CVC4_PUBLIC Stat {
private:
  std::string d_name;

  static std::string s_delim;

public:

  Stat(const std::string& s) throw(CVC4::AssertionException) :
    d_name(s) {
    if(__CVC4_USE_STATISTICS) {
      AlwaysAssert(d_name.find(s_delim) == std::string::npos);
    }
  }
  virtual ~Stat() {}

  virtual void flushInformation(std::ostream& out) const = 0;

  void flushStat(std::ostream& out) const {
    if(__CVC4_USE_STATISTICS) {
      out << d_name << s_delim;
      flushInformation(out);
    }
  }

  const std::string& getName() const {
    return d_name;
  }

  virtual std::string getValue() const = 0;
};/* class Stat */

inline bool StatisticsRegistry::StatCmp::operator()(const Stat* s1,
                                                    const Stat* s2) const {
  return s1->getName() < s2->getName();
}

inline void StatisticsRegistry::registerStat(Stat* s)
  throw(AssertionException) {
  if(__CVC4_USE_STATISTICS) {
    AlwaysAssert(d_registeredStats.find(s) == d_registeredStats.end());
    d_registeredStats.insert(s);
  }
}/* StatisticsRegistry::registerStat() */


inline void StatisticsRegistry::unregisterStat(Stat* s)
  throw(AssertionException) {
  if(__CVC4_USE_STATISTICS) {
    AlwaysAssert(d_registeredStats.find(s) != d_registeredStats.end());
    d_registeredStats.erase(s);
  }
}/* StatisticsRegistry::unregisterStat() */


/**
 * class T must have stream insertion operation defined.
 * std::ostream& operator<<(std::ostream&, const T&);
 */
template <class T>
class ReadOnlyDataStat : public Stat {
public:
  typedef T payload_t;

  ReadOnlyDataStat(const std::string& s) :
    Stat(s) {
  }

  virtual const T& getData() const = 0;

  void flushInformation(std::ostream& out) const {
    if(__CVC4_USE_STATISTICS) {
      out << getData();
    }
  }

  std::string getValue() const {
    std::stringstream ss;
    ss << getData();
    return ss.str();
  }
};/* class DataStat<T> */


template <class T>
class DataStat : public ReadOnlyDataStat<T> {
public:
  DataStat(const std::string& s) :
    ReadOnlyDataStat<T>(s) {
  }

  virtual void setData(const T&) = 0;

};/* class DataStat<T> */


/** T must have an assignment operator=(). */
template <class T>
class ReferenceStat : public DataStat<T> {
private:
  const T* d_data;

public:
  ReferenceStat(const std::string& s) :
    DataStat<T>(s),
    d_data(NULL) {
  }

  ReferenceStat(const std::string& s, const T& data) :
    DataStat<T>(s),
    d_data(NULL) {
    setData(data);
  }

  void setData(const T& t) {
    if(__CVC4_USE_STATISTICS) {
      d_data = &t;
    }
  }

  const T& getData() const {
    if(__CVC4_USE_STATISTICS) {
      return *d_data;
    } else {
      Unreachable();
    }
  }
};/* class ReferenceStat<T> */


/** T must have an operator=() and a copy constructor. */
template <class T>
class BackedStat : public DataStat<T> {
protected:
  T d_data;

public:

  BackedStat(const std::string& s, const T& init) :
    DataStat<T>(s),
    d_data(init) {
  }

  void setData(const T& t) {
    if(__CVC4_USE_STATISTICS) {
      d_data = t;
    }
  }

  BackedStat<T>& operator=(const T& t) {
    if(__CVC4_USE_STATISTICS) {
      d_data = t;
    }
    return *this;
  }

  const T& getData() const {
    if(__CVC4_USE_STATISTICS) {
      return d_data;
    } else {
      Unreachable();
    }
  }
};/* class BackedStat<T> */


/**
 * A wrapper Stat for another Stat.
 *
 * This type of Stat is useful in cases where a module (like the
 * CongruenceClosure module) might keep its own statistics, but might
 * be instantiated in many contexts by many clients.  This makes such
 * a statistic inappopriate to register with the StatisticsRegistry
 * directly, as all would be output with the same name (and may be
 * unregistered too quickly anyway).  A WrappedStat allows the calling
 * client (say, TheoryUF) to wrap the Stat from the client module,
 * giving it a globally unique name.
 */
template <class Stat>
class WrappedStat : public ReadOnlyDataStat<typename Stat::payload_t> {
  typedef typename Stat::payload_t T;

  const ReadOnlyDataStat<T>& d_stat;

  /** Private copy constructor undefined (no copy permitted). */
  WrappedStat(const WrappedStat&) CVC4_UNDEFINED;
  /** Private assignment operator undefined (no copy permitted). */
  WrappedStat<T>& operator=(const WrappedStat&) CVC4_UNDEFINED;

public:

  WrappedStat(const std::string& s, const ReadOnlyDataStat<T>& stat) :
    ReadOnlyDataStat<T>(s),
    d_stat(stat) {
  }

  const T& getData() const {
    if(__CVC4_USE_STATISTICS) {
      return d_stat.getData();
    } else {
      Unreachable();
    }
  }
};/* class WrappedStat<T> */


class IntStat : public BackedStat<int64_t> {
public:

  IntStat(const std::string& s, int64_t init) :
    BackedStat<int64_t>(s, init) {
  }

  IntStat& operator++() {
    if(__CVC4_USE_STATISTICS) {
      ++d_data;
    }
    return *this;
  }

  IntStat& operator+=(int64_t val) {
    if(__CVC4_USE_STATISTICS) {
      d_data+= val;
    }
    return *this;
  }

  void maxAssign(int64_t val) {
    if(__CVC4_USE_STATISTICS) {
      if(d_data < val) {
        d_data = val;
      }
    }
  }

  void minAssign(int64_t val) {
    if(__CVC4_USE_STATISTICS) {
      if(d_data > val) {
        d_data = val;
      }
    }
  }
};/* class IntStat */


/**
 * The value for an AverageStat is the running average of (e1, e_2, ..., e_n),
 *   (e1 + e_2 + ... + e_n)/n,
 * where e_i is an entry added by an addEntry(e_i) call.
 * The value is initially always 0.
 * (This is to avoid making parsers confused.)
 */
class AverageStat : public BackedStat<double> {
private:
  uint32_t n;

public:
  AverageStat(const std::string& s) :
    BackedStat<double>(s, 0.0 ), n(0) {
  }

  void addEntry(double e){
    if(__CVC4_USE_STATISTICS) {
      double oldSum = n*getData();
      ++n;
      double newSum = oldSum + e;
      setData(newSum / n);
    }
  }

};/* class AverageStat */


// some utility functions for ::timespec
inline ::timespec& operator+=(::timespec& a, const ::timespec& b) {
  // assumes a.tv_nsec and b.tv_nsec are in range
  const long nsec_per_sec = 1000000000L; // one thousand million
  a.tv_sec += b.tv_sec;
  long nsec = a.tv_nsec + b.tv_nsec;
  while(nsec < 0) {
    nsec += nsec_per_sec;
    ++a.tv_sec;
  }
  while(nsec >= nsec_per_sec) {
    nsec -= nsec_per_sec;
    --a.tv_sec;
  }
  a.tv_nsec = nsec;
  return a;
}
inline ::timespec& operator-=(::timespec& a, const ::timespec& b) {
  // assumes a.tv_nsec and b.tv_nsec are in range
  const long nsec_per_sec = 1000000000L; // one thousand million
  a.tv_sec -= b.tv_sec;
  long nsec = a.tv_nsec - b.tv_nsec;
  while(nsec < 0) {
    nsec += nsec_per_sec;
    ++a.tv_sec;
  }
  while(nsec >= nsec_per_sec) {
    nsec -= nsec_per_sec;
    --a.tv_sec;
  }
  a.tv_nsec = nsec;
  return a;
}
inline ::timespec operator+(const ::timespec& a, const ::timespec& b) {
  ::timespec result = a;
  return result += b;
}
inline ::timespec operator-(const ::timespec& a, const ::timespec& b) {
  ::timespec result = a;
  return result -= b;
}
inline bool operator==(const ::timespec& a, const ::timespec& b) {
  // assumes a.tv_nsec and b.tv_nsec are in range
  return a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec;
}
inline bool operator!=(const ::timespec& a, const ::timespec& b) {
  // assumes a.tv_nsec and b.tv_nsec are in range
  return !(a == b);
}
inline bool operator<(const ::timespec& a, const ::timespec& b) {
  // assumes a.tv_nsec and b.tv_nsec are in range
  return a.tv_sec < b.tv_sec ||
    (a.tv_sec == b.tv_sec && a.tv_nsec < b.tv_nsec);
}
inline bool operator>(const ::timespec& a, const ::timespec& b) {
  // assumes a.tv_nsec and b.tv_nsec are in range
  return a.tv_sec > b.tv_sec ||
    (a.tv_sec == b.tv_sec && a.tv_nsec > b.tv_nsec);
}
inline bool operator<=(const ::timespec& a, const ::timespec& b) {
  // assumes a.tv_nsec and b.tv_nsec are in range
  return !(a > b);
}
inline bool operator>=(const ::timespec& a, const ::timespec& b) {
  // assumes a.tv_nsec and b.tv_nsec are in range
  return !(a < b);
}
inline std::ostream& operator<<(std::ostream& os, const ::timespec& t) {
  // assumes t.tv_nsec is in range
  return os << t.tv_sec << "."
            << std::setfill('0') << std::setw(8) << std::right << t.tv_nsec;
}


/**
 * A timer statistic.  The timer can be started and stopped
 * arbitrarily, like a stopwatch; the value of the statistic at the
 * end is the accummulated time.
 */
class TimerStat : public BackedStat< ::timespec > {
  // strange: timespec isn't placed in 'std' namespace ?!
  ::timespec d_start;
  bool d_running;

public:

  /**
   * Utility class to make it easier to call stop() at the end of a
   * code block.  When constructed, it starts the timer.  When
   * destructed, it stops the timer.
   */
  class CodeTimer {
    TimerStat& d_timer;

    /** Private copy constructor undefined (no copy permitted). */
    CodeTimer(const CodeTimer& timer) CVC4_UNDEFINED;
    /** Private assignment operator undefined (no copy permitted). */
    CodeTimer& operator=(const CodeTimer& timer) CVC4_UNDEFINED;

  public:
    CodeTimer(TimerStat& timer) : d_timer(timer) {
      d_timer.start();
    }
    ~CodeTimer() {
      d_timer.stop();
    }
  };/* class TimerStat::CodeTimer */

  TimerStat(const std::string& s) :
    BackedStat< ::timespec >(s, ::timespec()),
    d_running(false) {
  }

  void start() {
    if(__CVC4_USE_STATISTICS) {
      AlwaysAssert(!d_running);
      clock_gettime(CLOCK_MONOTONIC, &d_start);
      d_running = true;
    }
  }

  void stop() {
    if(__CVC4_USE_STATISTICS) {
      AlwaysAssert(d_running);
      ::timespec end;
      clock_gettime(CLOCK_MONOTONIC, &end);
      d_data += end - d_start;
      d_running = false;
    }
  }
};/* class TimerStat */

/**
 * To use a statistic, you need to declare it, initialize it in your
 * constructor, register it in your constructor, and deregister it in
 * your destructor.  Instead, this macro does it all for you (and
 * therefore also keeps the statistic type, field name, and output
 * string all in the same place in your class's header.  Its use is
 * like in this example, which takes the place of the declaration of a
 * statistics field "d_checkTimer":
 *
 *   KEEP_STATISTIC(TimerStat, d_checkTimer, "theory::uf::morgan::checkTime");
 *
 * If any args need to be passed to the constructor, you can specify
 * them after the string.
 *
 * The macro works by creating a nested class type, derived from the
 * statistic type you give it, which declares a registry-aware
 * constructor/destructor pair.
 */
#define KEEP_STATISTIC(_StatType, _StatField, _StatName, _CtorArgs...)  \
  struct Statistic_##_StatField : public _StatType {                    \
    Statistic_##_StatField() : _StatType(_StatName, ## _CtorArgs) {     \
      StatisticsRegistry::registerStat(this);                           \
    }                                                                   \
    ~Statistic_##_StatField() {                                         \
      StatisticsRegistry::unregisterStat(this);                         \
    }                                                                   \
  } _StatField

#undef __CVC4_USE_STATISTICS

}/* CVC4 namespace */

#endif /* __CVC4__STATS_H */
