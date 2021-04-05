//
// Created by Håkon Strandlie on 24/02/2021.
//

#include <boost/iterator/iterator_facade.hpp>
#include <boost/uuid/uuid.hpp>
#include <fstream>
#include "routing_file_handler.h"

#ifndef MYSQL_ROUTING_ITERATOR_H
#define MYSQL_ROUTING_ITERATOR_H

typedef unsigned long long int ulonglong;

namespace routing {
template <typename T>
class routing_iterator
    : public boost::iterator_facade<routing_iterator<T>, T,
                                    boost::bidirectional_traversal_tag, T &,
                                    int32_t> {
 public:
  typedef routing_iterator<T> self_type;
  typedef T value_type;
  typedef T &reference;
  typedef T *pointer;
  typedef std::forward_iterator_tag iterator_category;
  typedef int32_t difference_type;

 public:
  // This one is needed by the adjacency_list implementation
  routing_iterator() {}

  routing_iterator(std::vector<T> vec, bool onDisk, boost::uuids::uuid rvector_id)
      : vec_(vec), onDisk(onDisk), pointer_{0}, rvector_id_(rvector_id) {}

  routing_iterator(std::vector<T> vec, bool onDisk, size_t size, boost::uuids::uuid rvector_id)
      : vec_(vec), onDisk(onDisk), pointer_(size), rvector_id_(rvector_id) {}

  routing_iterator(const self_type &iter)
      : vec_(iter.vec_), onDisk(iter.onDisk), pointer_(iter.pointer_), rvector_id_(iter.rvector_id_) {}


  routing_iterator<T>& operator=(self_type other) {
    std::swap(vec_, other.vec_);
    std::swap(onDisk, other.onDisk);
    std::swap(pointer_, other.pointer_);
    return *this;
  }

 private:

  friend class boost::iterator_core_access;

  std::vector<T> vec_;
  bool onDisk;
  size_t pointer_;
  boost::uuids::uuid rvector_id_;

 public:

  void increment() { pointer_++; }

  void decrement() { pointer_--; }

  void setTo(size_t n) { pointer_ = n; }

  size_t get_pointer() const {
    return pointer_;
  }

  bool equal(self_type const &other) const {
    return this->pointer_ == other.pointer_;
  }

  reference dereference() const {
    if (!onDisk) {
      return const_cast<reference>(vec_.at(pointer_));
    }
    auto t = routing_file_handler<T>::readNth(pointer_, rvector_id_);
    return *t;
  }
};

template <typename T>
class const_routing_iterator
    : public boost::iterator_facade<const_routing_iterator<T>, T const,
                                    boost::bidirectional_traversal_tag, T &,
                                    int32_t> {
 public:
  typedef const_routing_iterator self_type;
  typedef T value_type;
  typedef const T &reference;
  typedef const T *pointer;
  typedef int32_t difference_type;
  typedef std::forward_iterator_tag iterator_category;

 public:
  // This one is needed by the adjacency_list implementation
  const_routing_iterator() {}

  const_routing_iterator(std::vector<T> &vec, bool onDisk, boost::uuids::uuid rvector_id)
      : vec_(vec), pointer_(0), onDisk(onDisk), rvector_id_(rvector_id)  {}

  const_routing_iterator(std::vector<T> &vec, bool onDisk, size_t size, boost::uuids::uuid rvector_id)
      : vec_(vec), pointer_(size), onDisk(onDisk), rvector_id_(rvector_id) {}

  const_routing_iterator(const self_type &iter)
      : vec_(iter.vec_), pointer_(iter.pointer_), onDisk(iter.onDisk), rvector_id_(iter.rvector_id_) {}


  const_routing_iterator<T>& operator=(self_type other) {
    std::swap(vec_, other.vec_);
    std::swap(onDisk, other.onDisk);
    std::swap(pointer_, other.pointer_);
    return *this;
  }

 private:
  friend class boost::iterator_core_access;

  std::vector<T> const vec_;
  int pointer_;
  bool onDisk;
  boost::uuids::uuid rvector_id_;

  void increment() { pointer_++; }

  void decrement() { pointer_--; }

  void setTo(size_t n) { pointer_ = n; }

  size_t get_pointer() const {
    return pointer_;
  }

  bool equal(self_type const &other) const {
    return this->pointer_ == other.pointer_;
  }
  reference dereference() const {
    if (!onDisk) {
      return vec_.at(pointer_);
    }
    auto t = routing_file_handler<T>::readNth(pointer_, rvector_id_);
    return *t;
  }
};

}  // namespace routing

#endif  // MYSQL_ROUTING_ITERATOR_H
