//
// Created by Håkon Strandlie on 24/02/2021.
//

#include "routing_iterator.h"
#include "routing_file_handler.h"

// routing_iterator implementation
namespace routing {

template <typename T>
typename routing_iterator<T>::reference routing_iterator<T>::operator*() {
  if (onDisk) {
    T t = routing_file_handler<T>::readNth(pointer_, inFile);
    return t;
  }
  return vec_.at(pointer_);
}

template<typename T>
typename routing_iterator<T>::pointer routing_iterator<T>::operator->() {
  if (onDisk) {
    T t = routing_file_handler<T>::readNth(pointer_, inFile);
    return &t;
  }
  return &vec_.at(pointer_);
}

template<typename T>
typename routing_iterator<T>::self_type &routing_iterator<T>::operator=(const routing_iterator &other) {
  (*this->vec_) = (*other.vec_);
  (*this->onDisk) = (*other.onDisk);
  (*this->pointer_) = (*other.pointer_);
  return *this;
}

template<typename T>
typename const_routing_iterator<T>::reference const_routing_iterator<T>::operator*() const {
  if (onDisk) {
    T t = routing_file_handler<T>::readNth(pointer_, inFile);
    return t;
  }
  return &vec_[pointer_];
}

template<typename T>
typename const_routing_iterator<T>::pointer const_routing_iterator<T>::operator->() const {
  if (onDisk) {
    T t = routing_file_handler<T>::readNth(pointer_, inFile);
    return &t;
  }
  return &vec_.at(pointer_);
}

template<typename T>
typename const_routing_iterator<T>::self_type &const_routing_iterator<T>::operator=(const const_routing_iterator &other) {
  (*this->vec_) = (*other.vec_);
  (*this->onDisk) = (*other.onDisk);
  (*this->pointer_) = (*other.pointer_);
  return *this;
}
}




