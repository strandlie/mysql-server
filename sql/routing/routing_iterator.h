//
// Created by Håkon Strandlie on 24/02/2021.
//

#include <boost/iterator/iterator_facade.hpp>
#include <boost/uuid/uuid.hpp>
#include <fstream>
#include "routing_file_handler.h"

#ifndef MYSQL_ROUTING_ITERATOR_H
#define MYSQL_ROUTING_ITERATOR_H

typedef unsigned long long ulonglong;

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

  routing_iterator(std::vector<T> &vec, boost::uuids::uuid rvector_id,
                   size_t currentFileIdxInMem, size_t total_size,
                   size_t ram_limit)
      : vec_(vec),
        pointer_(0),
        rvector_id_(rvector_id),
        currentFileIdxInMem(0),
        totalSize(total_size),
        ram_limit_(ram_limit) {}

  routing_iterator(std::vector<T> &vec, size_t size,
                   boost::uuids::uuid rvector_id, size_t currentFileIdxInMem,
                   size_t total_size, size_t ram_limit)
      : vec_(vec),
        pointer_(size),
        rvector_id_(rvector_id),
        currentFileIdxInMem(currentFileIdxInMem),
        totalSize(total_size),
        ram_limit_(ram_limit) {}

  routing_iterator(const self_type &iter)
      : vec_(iter.vec_),
        pointer_(iter.pointer_),
        rvector_id_(iter.rvector_id_),
        currentFileIdxInMem(iter.currentFileIdxInMem),
        totalSize(iter.totalSize),
        ram_limit_(iter.ram_limit_) {}

  routing_iterator<T> &operator=(self_type other) {
    std::swap(vec_, other.vec_);
    std::swap(pointer_, other.pointer_);
    std::swap(rvector_id_, other.rvector_id_);
    std::swap(currentFileIdxInMem, other.currentFileIdxInMem);
    std::swap(totalSize, other.totalSize);
    std::swap(ram_limit_, other.ram_limit_);
    return *this;
  }

 private:
  friend class boost::iterator_core_access;

  std::vector<T> vec_;
  size_t pointer_;
  boost::uuids::uuid rvector_id_;
  size_t currentFileIdxInMem;
  size_t totalSize;
  size_t ram_limit_;

 public:
  void increment() { pointer_++; }

  void decrement() { pointer_--; }

  void setTo(size_t n) { pointer_ = n; }

  size_t get_pointer() const { return pointer_; }

  bool equal(self_type const &other) const {
    return this->pointer_ == other.pointer_;
  }

  reference dereference() const {
    size_t file_idx = getFileIndex(pointer_);
    (const_cast<routing_iterator *> (this))->changeWorkingSet(file_idx);
    size_t element_idx = getElementIndex(pointer_);
    return const_cast<reference>(vec_.at(element_idx));
  }

  void changeWorkingSet(size_t new_idx) {
    if (new_idx == currentFileIdxInMem) {
      return;
    }
    routing_file_handler<T>::pushVectorWithIdx(currentFileIdxInMem, vec_, rvector_id_);
    vec_ = routing_file_handler<T>::readVectorWithNumber(new_idx, rvector_id_);
    currentFileIdxInMem = new_idx;
  }

  size_t getFileIndex(size_t n) const {
    const size_t max_vec_size = (ram_limit_ / 2) / sizeof(T);
    return n / max_vec_size;
  }

  size_t getElementIndex(size_t n) const {
    const size_t max_vec_size = (ram_limit_ / 2) / sizeof(T);
    return n % max_vec_size;
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

  const_routing_iterator(std::vector<T> &vec, boost::uuids::uuid rvector_id,
                         size_t currentFileIdxInMem, size_t total_size,
                         size_t ram_limit)
      : vec_(vec),
        pointer_(0),
        rvector_id_(rvector_id),
        currentFileIdxInMem(currentFileIdxInMem),
        totalSize(total_size),
        ram_limit_(ram_limit) {}

  const_routing_iterator(std::vector<T> &vec, size_t size,
                         boost::uuids::uuid rvector_id,
                         size_t currentFileIdxInMem, size_t total_size,
                         size_t ram_limit)
      : vec_(vec),
        pointer_(size),
        rvector_id_(rvector_id),
        currentFileIdxInMem(currentFileIdxInMem),
        totalSize(total_size),
        ram_limit_(ram_limit) {}

  const_routing_iterator(const self_type &iter)
      : vec_(iter.vec_),
        pointer_(iter.pointer_),
        rvector_id_(iter.rvector_id_),
        currentFileIdxInMem(iter.currentFileIdxInMem),
        totalSize(iter.totalSize),
        ram_limit_(iter.ram_limit_) {}

  const_routing_iterator<T> &operator=(self_type other) {
    std::swap(vec_, other.vec_);
    std::swap(pointer_, other.pointer_);
    std::swap(rvector_id_, other.rvector_id_);
    std::swap(currentFileIdxInMem, other.currentFileIdxInMem);
    std::swap(totalSize, other.totalSize);
    std::swap(ram_limit_, other.ram_limit_);
    return *this;
  }

 private:
  friend class boost::iterator_core_access;

  std::vector<T> const vec_;
  int pointer_;
  boost::uuids::uuid rvector_id_;
  size_t currentFileIdxInMem;
  size_t totalSize;
  size_t ram_limit_;

  void increment() { pointer_++; }

  void decrement() { pointer_--; }

  void setTo(size_t n) { pointer_ = n; }

  size_t get_pointer() const { return pointer_; }

  bool equal(self_type const &other) const {
    return this->pointer_ == other.pointer_;
  }
  reference dereference() const {
    size_t file_idx = getFileIndex(pointer_);
    changeWorkingSet(file_idx);
    size_t element_idx = getElementIndex(pointer_);
    return const_cast<reference>(vec_.at(element_idx));
  }

  void changeWorkingSet(size_t new_idx) {
    if (new_idx == currentFileIdxInMem) {
      return;
    }
    routing_file_handler<T>::pushVectorWithIdx(currentFileIdxInMem, vec_,
                                               rvector_id_);
    vec_ = routing_file_handler<T>::readVectorWithNumber(new_idx, rvector_id_);
    currentFileIdxInMem = new_idx;
  }

  size_t getFileIndex(size_t n) {
    const size_t max_vec_size = ram_limit_ / sizeof(T);
    return n / max_vec_size;
  }

  size_t getElementIndex(size_t n) {
    const size_t max_vec_size = ram_limit_ / sizeof(T);
    return n % max_vec_size;
  }
};

}  // namespace routing

#endif  // MYSQL_ROUTING_ITERATOR_H
