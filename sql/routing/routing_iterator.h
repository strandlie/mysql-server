//
// Created by Håkon Strandlie on 24/02/2021.
//

#include <fstream>

#ifndef MYSQL_ROUTING_ITERATOR_H
#define MYSQL_ROUTING_ITERATOR_H


typedef unsigned long long int ulonglong;

namespace routing {
template<typename T>
class routing_iterator {

 public:

  typedef routing_iterator<T> self_type;
  typedef T value_type;
  typedef T &reference;
  typedef T *pointer;
  typedef std::forward_iterator_tag iterator_category;
  typedef size_t difference_type;

 private:

  char *const onDiskLocation = "/var/tmp/mysql_routing_spillfile";
  std::vector<T> &vec_;
  bool onDisk;
  std::ifstream inFile;
  int pointer_;

 public:

  routing_iterator(std::vector<T> &vec, bool onDisk)
      : vec_(vec), onDisk(onDisk) {
    if (onDisk) {
      inFile.open(onDiskLocation, std::ios::in);
    }
  }

  routing_iterator(std::vector<T> &vec, bool onDisk, int size)
      : vec_(vec), onDisk(onDisk), pointer_(size) {
    if (onDisk) {
      inFile.open(onDiskLocation, std::ios::in);
    }
  }

  routing_iterator(const self_type &iter)
      : vec_(iter.vec_), pointer_(iter.pointer_), onDisk(iter.onDisk) {
    if (onDisk) {
      inFile.open(onDiskLocation, std::ios::in);
    }
  }

  routing_iterator() : vec_(), pointer_{0}, onDisk{false} {}

  ~routing_iterator() {
    if (inFile.is_open()) {
      inFile.close();
    }
  }

  /*
   * Internally implemented methods
   */
  self_type operator++() { self_type i = *this; pointer_++; return i; }
  self_type operator++(int junk) { pointer_++; return *this; }
  pointer operator->() { return &vec_.at(pointer_); }
  bool operator==(const self_type &right) const { return pointer_ == right.pointer_; }
  bool operator!=(const self_type &right) const { return !(this == right); }

  /*
   * Externally implemented methods
   */
  reference operator*();

};

template<typename T>
class const_routing_iterator {

 public:

  typedef const_routing_iterator self_type;
  typedef T value_type;
  typedef T& reference;
  typedef T* pointer;
  typedef size_t difference_type;
  typedef std::forward_iterator_tag iterator_category;


 private:

  char *const onDiskLocation = "/var/tmp/mysql_routing_spillfile";
  std::vector<T>& vec_;
  int pointer_;
  bool onDisk;
  std::ifstream inFile;

 public:

  const_routing_iterator(std::vector<T> &vec, bool onDisk) : vec_(vec), onDisk(onDisk) {
    if (onDisk) {
      inFile.open(onDiskLocation, std::ios::in);
    }
  }

  const_routing_iterator(std::vector<T> &vec, bool onDisk, int size) : vec_(vec), onDisk(onDisk), pointer_(size) {
    if (onDisk) {
      inFile.open(onDiskLocation, std::ios::in);
    }
  }

  const_routing_iterator(const self_type &iter) : vec_(iter.vec_), pointer_(iter.pointer_), onDisk(iter.onDisk) {
    if (onDisk) {
      inFile.open(onDiskLocation, std::ios::in);
    }
  }

  const_routing_iterator() : vec_(), pointer_{0}, onDisk{false} {}

  ~const_routing_iterator() {
    if (inFile.is_open()) {
      inFile.close();
    }
  }

  /*
   * Internally implemented methods
   */
  self_type operator++() { self_type i = *this; pointer_++; return i; }
  self_type operator++(int junk) { pointer_++; return *this; }
  const pointer operator->() { return &vec_.at(pointer_); }
  bool operator==(const self_type& right) { return pointer_ == right.pointer_; }
  bool operator !=(const self_type& right) { return !(this == right);}

  /*
   * Externally implemented methods
   */
  reference operator*();
};


};

#endif  // MYSQL_ROUTING_ITERATOR_H
