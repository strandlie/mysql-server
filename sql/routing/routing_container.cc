#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include "boost/pending/container_traits.hpp"
#include "routing_iterator.cc"
#include "sql/current_thd.h"
//#include "sql/sql_class.h"
#include "my_dbug.h"

typedef unsigned long long int ulonglong;


namespace routing {

template <typename T>
struct RVector {
 private:

  char const *onDiskLocation = "/var/tmp/mysql_routing_spillfile";
  std::vector<T> vec_;
  ulonglong ram_limit_;
  std::fstream file;
  bool onDisk;
  ulonglong onDiskSize;

 public:
  /*
   * STL CONTAINER REQUIRED
   */

  // Typedefs
  typedef unsigned long size_type;
  typedef int32_t difference_type;
  typedef T& reference;
  typedef T* pointer;
  typedef const T& const_reference;
  typedef T value_type;
  typedef routing_iterator<T> iterator;
  typedef const_routing_iterator<T> const_iterator;
  // typedef T7 reverse_iterator;
  // typedef T8 const_reverse_iterator;

  /*
   * CONSTRUCTORS
   */
  // Copy constructor
  RVector(RVector<T> &r_vector)
      : vec_(r_vector.vec_),
        ram_limit_(r_vector.ram_limit_),
        onDisk(r_vector.onDisk),
        onDiskSize(r_vector.onDiskSize) {}

  // Move constructor
  // This doesn't actually move anything, since all members are values
  RVector(RVector&& source) :
                              vec_(source.vec_),
                              ram_limit_(source.ram_limit_),
                              onDisk(source.onDisk),
                              onDiskSize(source.onDiskSize) {
    /*
    source.ram_limit_ = nullptr;
    source.onDisk = nullptr;
    source.vec_ = nullptr;
    source.onDiskSize = nullptr;
     */
  }


  RVector(ulonglong ram_limit) : ram_limit_(ram_limit), onDisk(false) {}

  // Default constructor
  RVector() {
    if (false) {
      //ulonglong ram_limitation =
      //    fmin(current_thd->variables.tmp_table_size, current_thd->variables.max_heap_table_size);
      //ram_limit_ = ram_limitation;
    } else {
      // Fallback
      ram_limit_ = 32000; // 32kB
    }
    //DBUG_LOG("Routing", "Ram-limit set to: " << ram_limit_);
  }

  // Destructor
  ~RVector() {
    if (file && file.is_open()) {
      file.close();
    }
    remove(onDiskLocation);
  }
  /*
   *** END *** CONSTRUCTORS
   */

  /*
   * METHOD DEFINITIONS
   */

 /**
  * routing_iterator<T> does not need AllocT, since the allocator
  * is already passed into the underlying std::vector vec_ when
  * constructing RVector
  * @tparam T The type of the value in the vector
  * @tparam AllocT The type of the alllocator in the underlying vector
  * @return The new routing iterator routing_iterator
  */
  routing_iterator<T> begin() {
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
  const_routing_iterator<T> begin() const {
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
  routing_iterator<T> end() {
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
  const_routing_iterator<T> end() const {
    return onDisk ? const_routing_iterator<T>(vec_, true, onDiskSize)
                  : const_routing_iterator<T>(vec_, false, vec_.size());
  }

  // THESE ARE NOT IMPLEMENTED BECAUSE I DON'T THINK THEY ARE REQUIRED FOR THIS APPLICATION
  // reverse_iterator rbegin();
  // const_reverse_iterator rbegin() const;
  // reverse_iterator rend();
  // const_reverse_iterator rend() const;


  size_type size() const {
    return onDisk ? onDiskSize : vec_.size();
  }

  size_type max_size() const {
    return onDisk ? INFINITY : vec_.max_size(); // TODO: Maybe an estimate of disk size here? Any relevant MYSQL system variables?
  }

 /*
  * TODO: Handle edge-cases where
  *  1) file has been written to and closed
  *  2) Pointer is advanced past the last position
  *  3) More?
  */
  bool empty() const {
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

  routing_iterator<T> erase(iterator where) {
    if (!onDisk) {
      return vec_.erase(where);
    }

    // TODO: Copy the file, except the relevant line, to a new file. Set the size -1
    return routing_iterator<T>(this);
  }

  routing_iterator<T> erase(iterator first,
                            iterator last) {
    if (!onDisk) {
      return vec_.erase(first, last);
    }

    // TODO: Copy the file, except the relevant line, to a new file. Set the size correctly
    return routing_iterator<T>(this);
  }


  routing_iterator<T> erase(iterator first,
                            iterator last,
                            const T& value) {
    if (!onDisk) {
      return vec_.erase(first, last, value);
    }
    // TODO: Copy the file, except the relevant line, to a new file. Set the size correctly
    return routing_iterator<T>(this);

  }
  void clear() { erase(begin(), end()); }

  void swap(RVector &right) {
    RVector<T> *tmp = new RVector<T>(right);
    right.vec_.swap(this->vec_);
    right.file = this->file;
    right.onDiskLocation = this->onDiskLocation;
    right.onDisk = this->onDisk;
    right.onDiskSize = this->onDiskSize;

    this->vec_.swap(tmp->vec_);
    this->file = tmp->file;
    this->onDiskLocation = tmp->onDiskLocation;
    this->onDisk = tmp->onDisk;
    this->onDiskSize = tmp->onDiskSize;

    return;

  }

  /*
   *** END *** STL CONTAINER REQUIRED
   */

  /*
   * VECTOR REQUIRED
   */


  void erase(RVector<T>& container, const T& value) {
    container.erase(container.begin(), container.end(), value);
  }

  std::pair<typename RVector<T>::iterator, bool> push(RVector<T>& container, const T& value) {
    container.push_back(value);
    return std::make_pair(boost::prior(container.end()), true);
  }

  void push_back(T item) {
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
        //file << *it << std::endl; // Does this work with reading?
      }
      //file << item << std::endl;  // Does this work with reading?
      file.close();
      onDisk = true;
      onDiskSize = vec_.size() + 1;  // +1 since the first element too large triggers the conversion

    } else if (onDisk) {
      if (!file.is_open()) {
        file.open(onDiskLocation, std::ios::out | std::ios::app);
      }
      //file << item << std::endl;
      file.close();
      onDiskSize++;
    } else {
      vec_.push_back(item);
    }
  }

  void resize(size_type n, T val) {
    if (onDisk) {
      vec_.resize(n, val);
    }
    // Else, don't do anything. A file does not need to be resized.
  }

  void resize(size_type n) {
    if (onDisk) {
      vec_.resize(n);
    }
    // Else, don't do anything. A file does not need to be resized.
  }

  reference operator[](size_type n) {
    if (!onDisk) {
      DBUG_LOG("Routing", "Return from internal vector: ");
      return vec_[n];
    }

    /*
     * Make sure iteration starts from the beginning of
     * file each time
     */
    /*
    std::ifstream loc_file;
    if (loc_file && loc_file.is_open()) {
      loc_file.close();
    }
    loc_file.open(onDiskLocation, std::ios::in);
    if (loc_file && loc_file.is_open()) {
      T *t = new T;
      loc_file >> t;
      for (int i = 0; i < n; i++) {
        loc_file >> t;
      }
      DBUG_LOG("Routing", "Return from disk: ");
      return t;
    }
    */
    DBUG_LOG("Routing", "Return from disk: ");
    T t = routing_file_handler<T>::readNth(n);
    return t;
  }

  const_reference operator[](size_type n) const {
    if (!onDisk) {
      DBUG_LOG("Routing", "Return from internal vector: ");
      const T* result = const_cast<T*>(&vec_[n]);
      return *result;
    }

    DBUG_LOG("Routing", "Return from disk: ");
    T t = routing_file_handler<T>::readNth(n);
    return t;
  }

  /*
   *** END *** VECTOR REQUIRED
   */


  /*
   *** END *** METHOD DEFINITIONS
   */

};

template<typename T>
boost::graph_detail::vector_tag container_category(const RVector<T>&) { return boost::graph_detail::vector_tag(); }
}