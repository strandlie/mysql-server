//
// Created by Håkon Strandlie on 24/02/2021.
//

#include "routing_iterator.h"

// routing_iterator implementation
namespace routing {

template <typename T>
typename routing_iterator<T>::reference routing_iterator<T>::operator*() {
  if (onDisk) {
    pointer t = new T;
    if (inFile && inFile.is_open()) {
      inFile >> *t;
      for (int i = 0; i < pointer_; i++) {
        inFile >> *t;
      }
    }
    return *t;
  }
  return vec_.at(pointer_);
}

template<typename T>
const typename const_routing_iterator<T>::reference const_routing_iterator<T>::operator*() {
  if (onDisk) {
    pointer t = new T;
    if (inFile && inFile.is_open()) {
      inFile >> *t;
      for (int i = 0; i < pointer_; i++) {
        inFile >> *t;
      }
    }
    return *t;
  }
}

}


