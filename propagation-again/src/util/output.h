/*********************                                                        */
/*! \file output.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): cconway, taking, dejan
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Output utility classes and functions.
 **
 ** Output utility classes and functions.
 **/

#include "cvc4_public.h"

#ifndef __CVC4__OUTPUT_H
#define __CVC4__OUTPUT_H

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <set>

namespace CVC4 {

template <class T, class U>
std::ostream& operator<<(std::ostream& out, const std::pair<T, U>& p) CVC4_PUBLIC;

template <class T, class U>
std::ostream& operator<<(std::ostream& out, const std::pair<T, U>& p) {
  return out << "[" << p.first << "," << p.second << "]";
}

/**
 * A utility class to provide (essentially) a "/dev/null" streambuf.
 * If debugging support is compiled in, but debugging for
 * e.g. "parser" is off, then Debug("parser") returns a stream
 * attached to a null_streambuf instance so that output is directed to
 * the bit bucket.
 */
class CVC4_PUBLIC null_streambuf : public std::streambuf {
public:
  /* Overriding overflow() just ensures that EOF isn't returned on the
   * stream.  Perhaps this is not so critical, but recommended; this
   * way the output stream looks like it's functioning, in a non-error
   * state. */
  int overflow(int c) { return c; }
};/* class null_streambuf */

/** A null stream-buffer singleton */
extern null_streambuf null_sb;
/** A null output stream singleton */
extern std::ostream null_os CVC4_PUBLIC;

#ifndef CVC4_MUZZLE

class CVC4_PUBLIC CVC4ostream {
  std::ostream* d_os;
  // Current indentation
  unsigned d_indent;

  std::ostream& (*d_endl)(std::ostream&);

public:
  CVC4ostream() : d_os(NULL) {}
  explicit CVC4ostream(std::ostream* os) : d_os(os), d_indent(0) {
    d_endl = &std::endl;
  }

  void pushIndent() { d_indent ++; }
  void popIndent()  { if (d_indent > 0) -- d_indent; }

  void flush() {
    if(d_os != NULL) {
      d_os->flush();
    }
  }

  bool isConnected() { return d_os != NULL; }
  operator std::ostream&() { return isConnected() ? *d_os : null_os; }

  template <class T>
  CVC4ostream& operator<<(T const& t) {
    if(d_os != NULL) {
      d_os = &(*d_os << t);
    }
    return *this;
  }

  // support manipulators, endl, etc..
  CVC4ostream& operator<<(std::ostream& (*pf)(std::ostream&)) {
    if(d_os != NULL) {
      d_os = &(*d_os << pf);

      if (pf == d_endl) {
        for (unsigned i = 0; i < d_indent; ++ i) {
          d_os = &(*d_os << '\t');
        }
      }
    }
    return *this;
  }
  CVC4ostream& operator<<(std::ios& (*pf)(std::ios&)) {
    if(d_os != NULL) {
      d_os = &(*d_os << pf);
    }
    return *this;
  }
  CVC4ostream& operator<<(std::ios_base& (*pf)(std::ios_base&)) {
    if(d_os != NULL) {
      d_os = &(*d_os << pf);
    }
    return *this;
  }
  CVC4ostream& operator<<(CVC4ostream& (*pf)(CVC4ostream&)) {
    return pf(*this);
  }
};/* class CVC4ostream */

inline CVC4ostream& push(CVC4ostream& stream) {
  stream.pushIndent();
  return stream;
}

inline CVC4ostream& pop(CVC4ostream& stream) {
  stream.popIndent();
  return stream;
}

/** The debug output class */
class CVC4_PUBLIC DebugC {
  std::set<std::string> d_tags;
  std::ostream* d_os;

public:
  explicit DebugC(std::ostream* os) : d_os(os) {}

  int printf(const char* tag, const char* fmt, ...) __attribute__ ((format(printf, 3, 4)));
  int printf(std::string tag, const char* fmt, ...) __attribute__ ((format(printf, 3, 4)));

  CVC4ostream operator()(const char* tag) {
    if(!d_tags.empty() && d_tags.find(std::string(tag)) != d_tags.end()) {
      return CVC4ostream(d_os);
    } else {
      return CVC4ostream();
    }
  }
  CVC4ostream operator()(std::string tag) {
    if(!d_tags.empty() && d_tags.find(tag) != d_tags.end()) {
      return CVC4ostream(d_os);
    } else {
      return CVC4ostream();
    }
  }

  bool on (const char* tag) { d_tags.insert(std::string(tag)); return true; }
  bool on (std::string tag) { d_tags.insert(tag); return true; }
  bool off(const char* tag) { d_tags.erase (std::string(tag)); return false; }
  bool off(std::string tag) { d_tags.erase (tag); return false; }

  bool isOn(const char* tag) { return d_tags.find(std::string(tag)) != d_tags.end(); }
  bool isOn(std::string tag) { return d_tags.find(tag) != d_tags.end(); }

  std::ostream& setStream(std::ostream& os) { d_os = &os; return os; }
  std::ostream& getStream() { return *d_os; }
};/* class DebugC */

/** The warning output class */
class CVC4_PUBLIC WarningC {
  std::ostream* d_os;

public:
  explicit WarningC(std::ostream* os) : d_os(os) {}

  int printf(const char* fmt, ...) __attribute__ ((format(printf, 2, 3)));

  CVC4ostream operator()() { return CVC4ostream(d_os); }

  std::ostream& setStream(std::ostream& os) { d_os = &os; return os; }
  std::ostream& getStream() { return *d_os; }
};/* class WarningC */

/** The message output class */
class CVC4_PUBLIC MessageC {
  std::ostream* d_os;

public:
  explicit MessageC(std::ostream* os) : d_os(os) {}

  int printf(const char* fmt, ...) __attribute__ ((format(printf, 2, 3)));

  CVC4ostream operator()() { return CVC4ostream(d_os); }

  std::ostream& setStream(std::ostream& os) { d_os = &os; return os; }
  std::ostream& getStream() { return *d_os; }
};/* class MessageC */

/** The notice output class */
class CVC4_PUBLIC NoticeC {
  std::ostream* d_os;

public:
  explicit NoticeC(std::ostream* os) : d_os(os) {}

  int printf(const char* fmt, ...) __attribute__ ((format(printf, 2, 3)));

  CVC4ostream operator()() { return CVC4ostream(d_os); }

  std::ostream& setStream(std::ostream& os) { d_os = &os; return os; }
  std::ostream& getStream() { return *d_os; }
};/* class NoticeC */

/** The chat output class */
class CVC4_PUBLIC ChatC {
  std::ostream* d_os;

public:
  explicit ChatC(std::ostream* os) : d_os(os) {}

  int printf(const char* fmt, ...) __attribute__ ((format(printf, 2, 3)));

  CVC4ostream operator()() { return CVC4ostream(d_os); }

  std::ostream& setStream(std::ostream& os) { d_os = &os; return os; }
  std::ostream& getStream() { return *d_os; }
};/* class ChatC */

/** The trace output class */
class CVC4_PUBLIC TraceC {
  std::ostream* d_os;
  std::set<std::string> d_tags;

public:
  explicit TraceC(std::ostream* os) : d_os(os) {}

  int printf(const char* tag, const char* fmt, ...) __attribute__ ((format(printf, 3, 4)));
  int printf(std::string tag, const char* fmt, ...) __attribute__ ((format(printf, 3, 4)));

  CVC4ostream operator()(const char* tag) {
    if(!d_tags.empty() && d_tags.find(tag) != d_tags.end()) {
      return CVC4ostream(d_os);
    } else {
      return CVC4ostream();
    }
  }

  CVC4ostream operator()(std::string tag) {
    if(!d_tags.empty() && d_tags.find(tag) != d_tags.end()) {
      return CVC4ostream(d_os);
    } else {
      return CVC4ostream();
    }
  }

  bool on (const char* tag) { d_tags.insert(std::string(tag)); return true; }
  bool on (std::string tag) { d_tags.insert(tag); return true; }
  bool off(const char* tag) { d_tags.erase (std::string(tag)); return false; }
  bool off(std::string tag) { d_tags.erase (tag); return false; }

  bool isOn(const char* tag) { return d_tags.find(std::string(tag)) != d_tags.end(); }
  bool isOn(std::string tag) { return d_tags.find(tag) != d_tags.end(); }

  std::ostream& setStream(std::ostream& os) { d_os = &os; return os; }
  std::ostream& getStream() { return *d_os; }
};/* class TraceC */

/** The debug output singleton */
extern DebugC DebugChannel CVC4_PUBLIC;
#ifdef CVC4_DEBUG
#  define Debug DebugChannel
#else /* CVC4_DEBUG */
#  define Debug CVC4::__cvc4_true() ? CVC4::debugNullCvc4Stream : CVC4::DebugChannel
#endif /* CVC4_DEBUG */

/** The warning output singleton */
extern WarningC Warning CVC4_PUBLIC;
/** The message output singleton */
extern MessageC Message CVC4_PUBLIC;
/** The notice output singleton */
extern NoticeC Notice CVC4_PUBLIC;
/** The chat output singleton */
extern ChatC Chat CVC4_PUBLIC;

/** The trace output singleton */
extern TraceC TraceChannel CVC4_PUBLIC;
#ifdef CVC4_TRACING
#  define Trace TraceChannel
#else /* CVC4_TRACING */
#  define Trace CVC4::__cvc4_true() ? CVC4::debugNullCvc4Stream : CVC4::TraceChannel
#endif /* CVC4_TRACING */

// Disallow e.g. !Debug("foo").isOn() forms
// because the ! will apply before the ? .
// If a compiler error has directed you here,
// just parenthesize it e.g. !(Debug("foo").isOn())
class __cvc4_true {
  void operator!() CVC4_UNUSED;
  void operator~() CVC4_UNUSED;
  void operator-() CVC4_UNUSED;
  void operator+() CVC4_UNUSED;
public:
  inline operator bool() { return true; }
};/* __cvc4_true */

#ifdef CVC4_DEBUG

class CVC4_PUBLIC ScopedDebug {
  std::string d_tag;
  bool d_oldSetting;

public:

  ScopedDebug(std::string tag, bool newSetting = true) :
    d_tag(tag) {
    d_oldSetting = Debug.isOn(d_tag);
    if(newSetting) {
      Debug.on(d_tag);
    } else {
      Debug.off(d_tag);
    }
  }

  ScopedDebug(const char* tag, bool newSetting = true) :
    d_tag(tag) {
    d_oldSetting = Debug.isOn(d_tag);
    if(newSetting) {
      Debug.on(d_tag);
    } else {
      Debug.off(d_tag);
    }
  }

  ~ScopedDebug() {
    if(d_oldSetting) {
      Debug.on(d_tag);
    } else {
      Debug.off(d_tag);
    }
  }
};/* class ScopedDebug */

#else /* CVC4_DEBUG */

class CVC4_PUBLIC ScopedDebug {
public:
  ScopedDebug(std::string tag, bool newSetting = true) {}
  ScopedDebug(const char* tag, bool newSetting = true) {}
};/* class ScopedDebug */

#endif /* CVC4_DEBUG */

#ifdef CVC4_TRACING

class CVC4_PUBLIC ScopedTrace {
  std::string d_tag;
  bool d_oldSetting;

public:

  ScopedTrace(std::string tag, bool newSetting = true) :
    d_tag(tag) {
    d_oldSetting = Trace.isOn(d_tag);
    if(newSetting) {
      Trace.on(d_tag);
    } else {
      Trace.off(d_tag);
    }
  }

  ScopedTrace(const char* tag, bool newSetting = true) :
    d_tag(tag) {
    d_oldSetting = Trace.isOn(d_tag);
    if(newSetting) {
      Trace.on(d_tag);
    } else {
      Trace.off(d_tag);
    }
  }

  ~ScopedTrace() {
    if(d_oldSetting) {
      Trace.on(d_tag);
    } else {
      Trace.off(d_tag);
    }
  }
};/* class ScopedTrace */

#else /* CVC4_TRACING */

class CVC4_PUBLIC ScopedTrace {
public:
  ScopedTrace(std::string tag, bool newSetting = true) {}
  ScopedTrace(const char* tag, bool newSetting = true) {}
};/* class ScopedTrace */

#endif /* CVC4_TRACING */

#else /* ! CVC4_MUZZLE */

class CVC4_PUBLIC ScopedDebug {
public:
  ScopedDebug(std::string tag, bool newSetting = true) {}
  ScopedDebug(const char* tag, bool newSetting = true) {}
};/* class ScopedDebug */

class CVC4_PUBLIC ScopedTrace {
public:
  ScopedTrace(std::string tag, bool newSetting = true) {}
  ScopedTrace(const char* tag, bool newSetting = true) {}
};/* class ScopedTrace */

extern NullDebugC Debug CVC4_PUBLIC;
extern NullC Warning CVC4_PUBLIC;
extern NullC Message CVC4_PUBLIC;
extern NullC Notice CVC4_PUBLIC;
extern NullC Chat CVC4_PUBLIC;
extern NullDebugC Trace CVC4_PUBLIC;

#endif /* ! CVC4_MUZZLE */

/**
 * Same shape as DebugC/TraceC, but does nothing; designed for
 * compilation of non-debug/non-trace builds.  None of these should ever be called
 * in such builds, but we offer this to the compiler so it doesn't complain.
 */
class CVC4_PUBLIC NullDebugC {
public:
/*
  NullDebugC() {}
  NullDebugC(std::ostream* os) {}

  void operator()(const char* tag, const char*) {}
  void operator()(const char* tag, std::string) {}
  void operator()(std::string tag, const char*) {}
  void operator()(std::string tag, std::string) {}

  int printf(const char* tag, const char* fmt, ...) __attribute__ ((format(printf, 3, 4))) {}
  int printf(std::string tag, const char* fmt, ...) __attribute__ ((format(printf, 3, 4))) {}

  std::ostream& operator()(const char* tag) { return null_os; }
  std::ostream& operator()(std::string tag) { return null_os; }

  void on (const char* tag) {}
  void on (std::string tag) {}
  void off(const char* tag) {}
  void off(std::string tag) {}

  bool isOn(const char* tag) { return false; }
  bool isOn(std::string tag) { return false; }

  void setStream(std::ostream& os) {}
  std::ostream& getStream() { return null_os; }

*/
  operator bool() { return false; }
  operator CVC4ostream() { return CVC4ostream(); }
  operator std::ostream&() { return null_os; }
};/* class NullDebugC */

/**
 * Same shape as WarningC/etc., but does nothing; designed for
 * compilation of muzzled builds.  None of these should ever be called
 * in muzzled builds, but we offer this to the compiler so it doesn't
 * complain. */
class CVC4_PUBLIC NullC {
public:
  NullC() {}
  NullC(std::ostream* os) {}

  int printf(const char* fmt, ...) __attribute__ ((format(printf, 2, 3))) { return 0; }

  std::ostream& operator()() { return null_os; }

  std::ostream& setStream(std::ostream& os) { return null_os; }
  std::ostream& getStream() { return null_os; }
};/* class NullC */

extern NullDebugC debugNullCvc4Stream CVC4_PUBLIC;
extern NullC nullCvc4Stream CVC4_PUBLIC;

}/* CVC4 namespace */

#endif /* __CVC4__OUTPUT_H */