#include "routing_container.h"
#include "my_dbug.h"

namespace routing {
// RVector implementation

/*
 * STL CONTAINER REQUIRED (IMPLEMENTATION)
 */

/**
 * routing_iterator<T> does not need AllocT, since the allocator
 * is already passed into the underlying std::vector vec_ when
 * constructing RVector
 * @tparam T The type of the value in the vector
 * @tparam AllocT The type of the alllocator in the underlying vector
 * @return The new routing iterator routing_iterator
 */
template <typename T, typename AllocT>
typename RVector<T, AllocT>::iterator RVector<T, AllocT>::begin() {
  return routing_iterator<T>(vec_, onDisk);
}

/**
 * const_routing_iterator<T> does not need AllocT, since the allocator
 * is already passed into the underlying std::vector vec_ when
 * constructing RVector
 *
 * @tparam T The type of the value contained in the vector
 * @tparam AllocT The type of the allocator in the undelrying vector
 * @return the new const routing iterator const_routing_iterator
 */
template<typename T, typename AllocT>
typename RVector<T, AllocT>::const_iterator RVector<T, AllocT>::begin() const {
  return const_routing_iterator<T>(vec_, onDisk);
}



/**
 * routing_iterator<T> does not need AllocT, since the allocator
 * is already passed into the underlying std::vector vec_ when
 * constructing RVector
 *
 * @tparam T The type of the value in the vector
 * @tparam AllocT The type of the alllocator in the underlying vector
 * @return The routing iterator routing_iterator
 */
template <typename T, typename AllocT>
typename RVector<T, AllocT>::iterator RVector<T, AllocT>::end() {
  return onDisk ? routing_iterator<T>(vec_, true, onDiskSize)
                : routing_iterator<T>(vec_, false, vec_.size());
}

/**
 * const_routing_iterator<T> does not need AllocT, since the allocator
 * is already passed into the underlying std::vector vec_ when
 * constructing RVector
 * @tparam T  The type of the value in the vector
 * @tparam AllocT The type of the allocator in the underlying vector
 * @return The const routing iterator const_routing_iterator
 */
template<typename T, typename AllocT>
typename RVector<T, AllocT>::const_iterator RVector<T, AllocT>::end() const {
  return onDisk ? const_routing_iterator<T>(vec_, true, onDiskSize)
                : const_routing_iterator<T>(vec_, false, vec_.size());
}

template <typename T, typename AllocT>
typename RVector<T, AllocT>::size_type RVector<T, AllocT>::size() const {
  return onDisk ? onDiskSize : vec_.size();
}

template <typename T, typename AllocT>
typename RVector<T, AllocT>::size_type RVector<T, AllocT>::max_size() const {
  return onDisk ? INFINITY : vec_.max_size(); // TODO: Maybe an estimate of disk size here? Any relevant MYSQL system variables?
}


/*
 * TODO: Handle edge-cases where
 *  1) file has been written to and closed
 *  2) Pointer is advanced past the last position
 *  3) More?
 */
template <typename T, typename AllocT>
bool RVector<T, AllocT>::empty() const {
  if (!onDisk) {
    return vec_.empty();
  }

  if (file && file.is_open()) {
    T *t = new T;
    file >> t;
    return t == nullptr;
  }
  return true;
}

template <typename T, typename AllocT>
typename RVector<T, AllocT>::iterator RVector<T, AllocT>::erase(routing_iterator<T> where) {
  if (!onDisk) {
    return vec_.erase(where);
  }

  // TODO: Copy the file, except the relevant line, to a new file. Set the size -1
  return routing_iterator<T>(this);
}

template <typename T, typename AllocT>
typename RVector<T, AllocT>::iterator RVector<T, AllocT>::erase(routing_iterator<T> first, routing_iterator<T> last) {
  if (!onDisk) {
    return vec_.erase(first, last);
  }

  // TODO: Copy the file, except the relevant line, to a new file. Set the size correctly
  return routing_iterator<T>(this);
}

template <typename T, typename AllocT>
void RVector<T, AllocT>::swap(RVector<T, AllocT> &right) {
  if (!onDisk) {
    vec_.swap(right);
    return;
  }
  const char *oldOnDiskLocation = onDiskLocation;
  onDiskLocation = right.onDiskLocation;
  right.onDiskLocation = oldOnDiskLocation;
  return;
}



/*
 *** END ***  STL CONTAINER REQUIRED (IMPLEMENTATION)
 */



/*
 * VECTOR REQUIRED (IMPLEMENTATION)
 */

template <typename T, typename AllocT>
typename RVector<T, AllocT>::reference RVector<T, AllocT>::operator[](size_t n) {
  if (!onDisk) {
    DBUG_LOG("Routing", "Return from internal vector: ");
    return vec_[n];
  }

  /*
   * Make sure iteration starts from the beginning of
   * file each time
   */
  if (file && file.is_open()) {
    file.close();
  }
  file.open(onDiskLocation, std::ios::in);
  if (file && file.is_open()) {
    T *t = new T;
    file >> t;
    for (int i = 0; i < n; i++) {
      file >> t;
    }
    DBUG_LOG("Routing", "Return from disk: ");
    return t;
  }
}

template <typename T, typename AllocT>
void RVector<T, AllocT>::push_back(T item) {
  typedef typename std::vector<T>::iterator Iter;
  const std::size_t current_size = vec_.size() * sizeof(T);
  DBUG_LOG("Routing", "Number of elements: " << vec_.size());
  DBUG_LOG("Routing", "Current size: " << current_size);
  if (current_size + sizeof(item) > ram_limit_ && !onDisk) {
    DBUG_LOG("Routing", "Too large. Spilling to disk");
    // Spill to disk
    if (!file.is_open()) {
      remove(onDiskLocation);
      file.open(onDiskLocation, std::ios::out | std::ios::app);
    }
    for (Iter it = vec_.begin(); it != vec_.end(); ++it) {
      file << *it << std::endl;
    }
    file << item << std::endl;
    file.close();
    onDisk = true;
    onDiskSize = vec_.size() + 1;  // +1 since the first element too large triggers the conversion

  } else if (onDisk) {
    if (!file.is_open()) {
      file.open(onDiskLocation, std::ios::out | std::ios::app);
    }
    file << item << std::endl;
    file.close();
    onDiskSize++;
  } else {
    vec_.push_back(item);
  }
}

template <typename T, typename AllocT>
void RVector<T, AllocT>::resize(size_type n, T val) {
  if (onDisk) {
    vec_.resize(n, val);
  } else {
    // Open file, fill with val? Or do nothing?
  }
}


template <typename T, typename AllocT>
void RVector<T, AllocT>::resize(size_type n) {
  if (onDisk) {
    vec_.resize(n);
  } else {
    // Open file, fill with val? Or do nothing?
  }
}

/*
 *** END *** VECTOR REQUIRED (IMPLEMENTATION)
 */

}