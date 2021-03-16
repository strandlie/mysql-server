//
// Created by Håkon Strandlie on 24/02/2021.
//

#include <fstream>
#include <boost/iterator/iterator_facade.hpp>
#include "routing_file_handler.cc"

#ifndef MYSQL_ROUTING_ITERATOR_H
#define MYSQL_ROUTING_ITERATOR_H


typedef unsigned long long int ulonglong;

namespace routing {
template<typename T>
class routing_iterator
    : public boost::iterator_facade<
          routing_iterator<T>,
          T,
          boost::bidirectional_traversal_tag,
          T&,
          int32_t
        >
{

 public:

  typedef routing_iterator<T> self_type;
  typedef T value_type;
  typedef T& reference;
  typedef T* pointer;
  typedef std::forward_iterator_tag iterator_category;
  typedef int32_t difference_type;

 public:

  routing_iterator(std::vector<T> &vec, bool onDisk)
      : vec_(vec), onDisk(onDisk) {}

  routing_iterator(std::vector<T> &vec, bool onDisk, size_t size)
      : vec_(vec), onDisk(onDisk), pointer_(size) {}

  routing_iterator(const self_type &iter)
      : vec_(iter.vec_), pointer_(iter.pointer_), onDisk(iter.onDisk) {}

  routing_iterator() : vec_(), pointer_{0}, onDisk{false} {}

 private:
  friend class boost::iterator_core_access;

  std::vector<T> vec_;
  bool onDisk;
  size_t pointer_;

  void increment() {
    pointer_++;
  }

  void decrement() {
    pointer_--;
  }

  bool equal(self_type const& other) const {
    return this->pointer_ == other.pointer_;
  }

  reference const dereference() const {
    if (!onDisk) {
      return const_cast<const reference>(vec_.at(pointer_));
    }
    T t = routing_file_handler<T>::readNth(pointer_);
    return t;
  };


  /*
  void swap(routing_iterator<T> &other) {
    routing_iterator<T> *tmp = new routing_iterator<T>(other);
    other.onDisk = this->onDisk;
    other.vec_.swap(this->vec_);
    other.pointer_ = this->pointer_;

    this->onDisk = tmp->onDisk;
    this->vec_.swap(tmp->vec_);
    this->pointer_ = tmp->pointer_;
  }

  self_type operator++() { self_type i = *this; pointer_++; return i; } // Pre-increment
  self_type operator++(int junk) { pointer_++; return *this; }          // Post-increment
  self_type operator--() { self_type i = *this; pointer_--; return i; } // Pre-decrement
  self_type operator--(int junk) { pointer_--; return *this; }         // Post-decrement
  bool operator==(const self_type &right) const { return pointer_ == right.pointer_; }
  bool operator!=(const self_type &right) const { return !(*this == right); }

  reference operator*() {
    if (onDisk) {
      T t = routing_file_handler<T>::readNth(pointer_);
      return t;
    }
    return vec_.at(pointer_);
  }

  pointer operator->() {
    if (onDisk) {
      T t = routing_file_handler<T>::readNth(pointer_);
      return &t;
    }
    return &vec_.at(pointer_);
  }

  routing_iterator &operator=(const routing_iterator &other) {
    routing_iterator<T>(other).swap(*this);
    return *this;
  }
  */

};

template<typename T>
class const_routing_iterator
    : public boost::iterator_facade<
        routing_iterator<T>,
        T const,
        boost::bidirectional_traversal_tag,
        T&,
        int32_t
    >
{

 public:

  typedef const_routing_iterator self_type;
  typedef T value_type;
  typedef T const& reference;
  typedef T const* pointer;
  typedef int32_t difference_type;
  typedef std::forward_iterator_tag iterator_category;



 public:

  const_routing_iterator(std::vector<T> &vec, bool onDisk) : vec_(vec), onDisk(onDisk) {}

  const_routing_iterator(std::vector<T> &vec, bool onDisk, int size) : vec_(vec), onDisk(onDisk), pointer_(size) {}

  const_routing_iterator(const self_type &iter) : vec_(iter.vec_), pointer_(iter.pointer_), onDisk(iter.onDisk) {}

  const_routing_iterator() : vec_(), pointer_{0}, onDisk{false} {}

 private:

  friend class boost::iterator_core_access;

  std::vector<T> const vec_;
  int pointer_;
  bool onDisk;

  void increment() {
    pointer_++;
  }

  void decrement() {
    pointer_--;
  }

  bool equal(self_type const& other) const {
    return this->pointer_ == other.pointer_;
  }
  reference dereference() const {
    if (!onDisk) {
      return vec_.at(pointer_);
    }
    T t = routing_file_handler<T>::readNth(pointer_);
    return t;
  };

  /*
  void swap(const_routing_iterator<T> &other) {
    const_routing_iterator<T> *tmp = new const_routing_iterator<T>(other);
    other.onDisk = this->onDisk;
    other.vec_.swap(this->vec_);
    other.pointer_ = this->pointer_;

    this->onDisk = tmp->onDisk;
    this->vec_.swap(tmp->vec_);
    this->pointer_ = tmp->pointer_;
  }

  self_type operator++() { self_type i = *this; pointer_++; return i; } // Pre-increment
  self_type operator++(int junk) { pointer_++; return *this; }          // Post-increment
  self_type operator--() { self_type i = *this; pointer_--; return i; } // Pre-decrement
  self_type operator--(int junk) { pointer_--; return *this; }         // Post-decrement
  bool operator==(const self_type& right) { return pointer_ == right.pointer_; }
  bool operator !=(const self_type& right) { return !(this == right);}

  reference operator*() const {
    if (onDisk) {
      T t = routing_file_handler<T>::readNth(pointer_);
      return t;
    }
    return &vec_[pointer_];

  }

  pointer operator->() const {
    if (onDisk) {
      T t = routing_file_handler<T>::readNth(pointer_);
      return &t;
    }
    return &vec_.at(pointer_);
  }

  self_type &operator=(const self_type &other) {
    const_routing_iterator<T>(other).swap(*this);
    return *this;
  }
  */
};


}

#endif  // MYSQL_ROUTING_ITERATOR_H
