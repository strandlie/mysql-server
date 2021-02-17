#include "routing_container.h"

// rout_It implementation

template<typename T>
bool rout_It<T>::operator!=(const rout_It<T> &other) const {
  return *this != other;
}

template<typename T>
bool rout_It<T>::operator==(const rout_It<T> &other) const {
  return pointer_ == other.pointer_;
}

template<typename T>
rout_It<T>& rout_It<T>::operator++() {
  ++pointer_;
  return *this;
}

template<typename T>
T& rout_It<T>::operator*() {
  if (onDisk) {
    std::string line;
    if (inFile && inFile.is_open()) {
      // inFile saves state for file pointer
      std::getline(inFile, line);
    } else {
      line = "-1";
    }
    T result = std::stod(line);
    return result;
  }
  return vec_.at(pointer_);
}

// RVector implementation

template<typename T>
void RVector<T>::push_back(T item) {
  typedef typename std::vector<T>::iterator Iter;
  const std::size_t current_size = vec_.size() * sizeof(T);
  std::cout << "Number of elements: " << vec_.size() << std::endl;
  std::cout << "Current size: " << current_size << std::endl;
  std::cout << std::endl;
  if (current_size + sizeof(item) > ram_limit_ && !onDisk) {
    std::cout << "Too large. Spilling to disk" << std::endl;
    // Spill to disk
    if (!outFile.is_open()) {
      remove(onDiskLocation);
      outFile.open(onDiskLocation, std::ios::out | std::ios::app);

    }
    for (Iter it = vec_.begin(); it != vec_.end(); ++it) {
      outFile << *it << std::endl;
    }
    outFile << item << std::endl;
    outFile.close();
    onDisk = true;
    onDiskSize = vec_.size() + 1;   // +1 since the first element too large triggers the conversion

  } else if (onDisk)  {
    if (!outFile.is_open()) {
      outFile.open(onDiskLocation, std::ios::out | std::ios::app);
    }
    outFile << item << std::endl;
    outFile.close();
    onDiskSize++;
  } else {
    vec_.push_back(item);
  }
}

template<typename T>
rout_It<T> RVector<T>::begin() {
  return rout_It<T>(vec_, onDisk);
}

template<typename T>
rout_It<T> RVector<T>::end() {
  return onDisk ? rout_It<T>(vec_, true, onDiskSize) : rout_It<T>(vec_, false, vec_.size());
}
