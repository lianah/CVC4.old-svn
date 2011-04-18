/*********************                                                        */
/*! \file cdlist.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): barrett, taking
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Context-dependent list class.
 **
 ** Context-dependent list class.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__CONTEXT__CDLIST_H
#define __CVC4__CONTEXT__CDLIST_H

#include <iterator>
#include <memory>
#include <string>
#include <sstream>

#include "context/context.h"
#include "context/context_mm.h"
#include "context/cdlist_forward.h"
#include "util/Assert.h"

namespace CVC4 {
namespace context {

/**
 * Generic context-dependent dynamic array.  Note that for efficiency,
 * this implementation makes the following assumptions:
 *
 * 1. Over time, objects are only added to the list.  Objects are only
 *    removed when a pop restores the list to a previous state.
 *
 * 2. T objects can safely be copied using their copy constructor,
 *    operator=, and memcpy.
 */
template <class T, class AllocatorT>
class CDList : public ContextObj {
public:

  /** The value type with which this CDList<> was instantiated. */
  typedef T value_type;
  /** The allocator type with which this CDList<> was instantiated. */
  typedef AllocatorT Allocator;

protected:

  static const size_t INITIAL_SIZE = 10;
  static const size_t GROWTH_FACTOR = 2;

  /**
   * d_list is a dynamic array of objects of type T.
   */
  T* d_list;

  /**
   * Whether to call the destructor when items are popped from the
   * list.  True by default, but can be set to false by setting the
   * second argument in the constructor to false.
   */
  bool d_callDestructor;

  /**
   * Number of objects in d_list
   */
  size_t d_size;

  /**
   * Allocated size of d_list.
   */
  size_t d_sizeAlloc;

  /**
   * Our allocator.
   */
  Allocator d_allocator;

  /**
   * Private copy constructor used only by save().  d_list and
   * d_sizeAlloc are not copied: only the base class information and
   * d_size are needed in restore.
   */
  CDList(const CDList<T, Allocator>& l) :
    ContextObj(l),
    d_list(NULL),
    d_callDestructor(false),
    d_size(l.d_size),
    d_sizeAlloc(0),
    d_allocator(l.d_allocator) {
    Debug("cdlist") << "copy ctor: " << this
                    << " from " << &l
                    << " size " << d_size << std::endl;
  }

  /**
   * Reallocate the array with more space.
   * Throws bad_alloc if memory allocation fails.
   */
  void grow() {
    if(d_list == NULL) {
      // Allocate an initial list if one does not yet exist
      d_sizeAlloc = INITIAL_SIZE;
      Debug("cdlist") << "initial grow of cdlist " << this
                      << " level " << getContext()->getLevel()
                      << " to " << d_sizeAlloc << std::endl;
      if(d_sizeAlloc > d_allocator.max_size()) {
        d_sizeAlloc = d_allocator.max_size();
      }
      d_list = d_allocator.allocate(d_sizeAlloc);
      if(d_list == NULL) {
        throw std::bad_alloc();
      }
    } else {
      // Allocate a new array with double the size
      size_t newSize = GROWTH_FACTOR * d_sizeAlloc;
      if(newSize > d_allocator.max_size()) {
        newSize = d_allocator.max_size();
        Assert(newSize > d_sizeAlloc,
               "cannot request larger list due to allocator limits");
      }
      T* newList = d_allocator.allocate(newSize);
      Debug("cdlist") << "2x grow of cdlist " << this
                      << " level " << getContext()->getLevel()
                      << " to " << newSize
                      << " (from " << d_list
                      << " to " << newList << ")" << std::endl;
      if(newList == NULL) {
        throw std::bad_alloc();
      }
      std::memcpy(newList, d_list, sizeof(T) * d_sizeAlloc);
      d_allocator.deallocate(d_list, d_sizeAlloc);
      d_list = newList;
      d_sizeAlloc = newSize;
    }
  }

  /**
   * Implementation of mandatory ContextObj method save: simply copies
   * the current size to a copy using the copy constructor (the
   * pointer and the allocated size are *not* copied as they are not
   * restored on a pop).  The saved information is allocated using the
   * ContextMemoryManager.
   */
  ContextObj* save(ContextMemoryManager* pCMM) {
    ContextObj* data = new(pCMM) CDList<T, Allocator>(*this);
    Debug("cdlist") << "save " << this
                    << " at level " << this->getContext()->getLevel()
                    << " size at " << this->d_size
                    << " sizeAlloc at " << this->d_sizeAlloc
                    << " d_list is " << this->d_list
                    << " data:" << data << std::endl;
    return data;
  }

  /**
   * Implementation of mandatory ContextObj method restore: simply
   * restores the previous size.  Note that the list pointer and the
   * allocated size are not changed.
   */
  void restore(ContextObj* data) {
    Debug("cdlist") << "restore " << this
                    << " level " << this->getContext()->getLevel()
                    << " data == " << data
                    << " call dtor == " << this->d_callDestructor
                    << " d_list == " << this->d_list << std::endl;
    if(this->d_callDestructor) {
      const size_t size = ((CDList<T, Allocator>*)data)->d_size;
      while(this->d_size != size) {
        --this->d_size;
        this->d_allocator.destroy(&this->d_list[this->d_size]);
      }
    } else {
      this->d_size = ((CDList<T, Allocator>*)data)->d_size;
    }
    Debug("cdlist") << "restore " << this
                    << " level " << this->getContext()->getLevel()
                    << " size back to " << this->d_size
                    << " sizeAlloc at " << this->d_sizeAlloc << std::endl;
  }

public:

  /**
   * Main constructor: d_list starts as NULL, size is 0
   */
  CDList(Context* context, bool callDestructor = true,
         const Allocator& alloc = Allocator()) :
    ContextObj(context),
    d_list(NULL),
    d_callDestructor(callDestructor),
    d_size(0),
    d_sizeAlloc(0),
    d_allocator(alloc) {
  }

  /**
   * Main constructor: d_list starts as NULL, size is 0
   */
  CDList(bool allocatedInCMM, Context* context, bool callDestructor = true,
         const Allocator& alloc = Allocator()) :
    ContextObj(allocatedInCMM, context),
    d_list(NULL),
    d_callDestructor(callDestructor),
    d_size(0),
    d_sizeAlloc(0),
    d_allocator(alloc) {
  }

  /**
   * Destructor: delete the list
   */
  ~CDList() throw(AssertionException) {
    this->destroy();

    if(this->d_callDestructor) {
      for(size_t i = 0; i < this->d_size; ++i) {
        this->d_allocator.destroy(&this->d_list[i]);
      }
    }

    this->d_allocator.deallocate(this->d_list, this->d_sizeAlloc);
  }

  /**
   * Return the current size of (i.e. valid number of objects in) the
   * list.
   */
  size_t size() const {
    return d_size;
  }

  /**
   * Return true iff there are no valid objects in the list.
   */
  bool empty() const {
    return d_size == 0;
  }

  /**
   * Add an item to the end of the list.
   */
  void push_back(const T& data) {
    Debug("cdlist") << "push_back " << this
                    << " " << getContext()->getLevel()
                    << ": make-current, "
                    << "d_list == " << d_list << std::endl;
    makeCurrent();

    Debug("cdlist") << "push_back " << this
                    << " " << getContext()->getLevel()
                    << ": grow? " << d_size
                    << " " << d_sizeAlloc << std::endl;
    if(d_size == d_sizeAlloc) {
      Debug("cdlist") << "push_back " << this
                      << " " << getContext()->getLevel()
                      << ": grow!" << std::endl;
      grow();
    }
    Assert(d_size < d_sizeAlloc);

    Debug("cdlist") << "push_back " << this
                    << " " << getContext()->getLevel()
                    << ": construct! at " << d_list
                    << "[" << d_size << "] == " << &d_list[d_size]
                    << std::endl;
    d_allocator.construct(&d_list[d_size], data);
    Debug("cdlist") << "push_back " << this
                    << " " << getContext()->getLevel()
                    << ": done..." << std::endl;
    ++d_size;
    Debug("cdlist") << "push_back " << this
                    << " " << getContext()->getLevel()
                    << ": size now " << d_size << std::endl;
  }

  /**
   * Access to the ith item in the list.
   */
  const T& operator[](size_t i) const {
    Assert(i < d_size, "index out of bounds in CDList::operator[]");
    return d_list[i];
  }

  /**
   * Returns the most recent item added to the list.
   */
  const T& back() const {
    Assert(d_size > 0, "CDList::back() called on empty list");
    return d_list[d_size - 1];
  }

  /**
   * Iterator for CDList class.  It has to be const because we don't
   * allow items in the list to be changed.  It's a straightforward
   * wrapper around a pointer.  Note that for efficiency, we implement
   * only prefix increment and decrement.  Also note that it's OK to
   * create an iterator from an empty, uninitialized list, as begin()
   * and end() will have the same value (NULL).
   */
  class const_iterator {
    T const* d_it;

    const_iterator(T const* it) : d_it(it) {}

    friend class CDList<T, Allocator>;

  public:

    // FIXME we support operator--, but do we satisfy all the
    // requirements of a bidirectional iterator ?
    typedef std::input_iterator_tag iterator_category;
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef const T* pointer;
    typedef const T& reference;

    const_iterator() : d_it(NULL) {}

    inline bool operator==(const const_iterator& i) const {
      return d_it == i.d_it;
    }

    inline bool operator!=(const const_iterator& i) const {
      return d_it != i.d_it;
    }

    inline const T& operator*() const {
      return *d_it;
    }

    /** Prefix increment */
    const_iterator& operator++() {
      ++d_it;
      return *this;
    }

    /** Prefix decrement */
    const_iterator& operator--() { --d_it; return *this; }

    // Postfix operations on iterators: requires a Proxy object to
    // hold the intermediate value for dereferencing
    class Proxy {
      const T* d_t;

    public:

      Proxy(const T& p): d_t(&p) {}

      T& operator*() {
        return *d_t;
      }
    };/* class CDList<>::const_iterator::Proxy */

    /** Postfix increment: returns Proxy with the old value. */
    Proxy operator++(int) {
      Proxy e(*(*this));
      ++(*this);
      return e;
    }

    /** Postfix decrement: returns Proxy with the old value. */
    Proxy operator--(int) {
      Proxy e(*(*this));
      --(*this);
      return e;
    }

  };/* class CDList<>::const_iterator */

  /**
   * Returns an iterator pointing to the first item in the list.
   */
  const_iterator begin() const {
    return const_iterator(static_cast<T const*>(d_list));
  }

  /**
   * Returns an iterator pointing one past the last item in the list.
   */
  const_iterator end() const {
    return const_iterator(static_cast<T const*>(d_list) + d_size);
  }
};/* class CDList<> */

}/* CVC4::context namespace */
}/* CVC4 namespace */

#endif /* __CVC4__CONTEXT__CDLIST_H */
