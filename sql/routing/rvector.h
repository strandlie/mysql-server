#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include "boost/pending/container_traits.hpp"
#include "my_dbug.h"
#include "routing_iterator.h"
#include "sql/current_thd.h"
#include "sql/sql_class.h"

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

typedef unsigned long long int ulonglong;

namespace routing {

template <typename T>
class RVector {
 private:
  boost::uuids::uuid id;
  std::vector<T> vec_;
  ulonglong ram_limit_;
  bool onDisk;
  ulonglong onDiskSize;

  inline static ulonglong total_count = 0;

 public:
  /*
   * STL CONTAINER REQUIRED
   */

  // TYPEDEFS

  typedef unsigned long size_type;
  typedef int32_t difference_type;
  typedef T &reference;
  typedef T *pointer;
  typedef const T &const_reference;
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
      : id(r_vector.id)
      , vec_(r_vector.vec_)
      , ram_limit_(r_vector.ram_limit_)
      , onDisk(r_vector.onDisk)
      , onDiskSize(r_vector.onDiskSize) {}

  // Move constructor
  // This doesn't actually move anything, since all members are values
  RVector(RVector &&source) noexcept
      : id(source.id)
      , vec_(source.vec_)
      , ram_limit_(source.ram_limit_)
      , onDisk(source.onDisk)
      , onDiskSize(source.onDiskSize) {
    /*
    source.ram_limit_ = nullptr;
    source.onDisk = nullptr;
    source.vec_ = nullptr;
    source.onDiskSize = nullptr;
     */
  }

  explicit RVector(ulonglong ram_limit)
      : id(boost::uuids::random_generator()())
      , vec_()
      , ram_limit_(ram_limit)
      , onDisk(false)
      , onDiskSize{0} {}

  // Default constructor
  RVector() : id(boost::uuids::random_generator()()), vec_() {
    if (current_thd) {
      ulonglong ram_limitation =
          fmin(current_thd->variables.tmp_table_size,
               current_thd->variables.max_heap_table_size);
      ram_limit_ = ram_limitation;
    } else {
      // Fallback
      ram_limit_ = 32000;  // 32kB
    }
    // current_thd->variables.routing_total_size = 0;
    onDisk = false;
    onDiskSize = 0;
    DBUG_LOG("Routing", "Ram-limit set to: " << ram_limit_);
  }

  // Destructor
  ~RVector() {
    RVector::decrMemFtpr(vec_.size());
    routing_file_handler<T>::deleteFileForId(id);
  }
  /*
   *** END *** CONSTRUCTORS
   */

  /*
   *** SERIALIZATION
   */

  template <class Archive>
  void serialize(Archive &ar,
                 __attribute__((unused)) const unsigned int version) {
    ar &vec_;
    ar &ram_limit_;
    ar &onDisk;
    ar &onDiskSize;
  }

  /*
   *** END *** SERIALIZATION
   */

  /*
   * METHODS
   */

  /**
   * routing_iterator<T> does not need AllocT, since the allocator
   * is already passed into the underlying std::vector vec_ when
   * constructing RVector
   * @tparam T The type of the value in the vector
   * @tparam AllocT The type of the alllocator in the underlying vector
   * @return The new routing iterator routing_iterator
   */
  routing_iterator<T> begin() { return routing_iterator<T>(vec_, onDisk, id); }

  /**
   * const_routing_iterator<T> does not need AllocT, since the allocator
   * is already passed into the underlying std::vector vec_ when
   * constructing RVector
   *
   * @tparam T The type of the value contained in the vector
   * @tparam AllocT The type of the allocator in the underlying vector
   * @return the new const routing iterator const_routing_iterator
   */
  const_routing_iterator<T> begin() const {
    return const_routing_iterator<T>(vec_, onDisk, id);
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
    return onDisk ? routing_iterator<T>(vec_, true, onDiskSize, id)
                  : routing_iterator<T>(vec_, false, vec_.size(), id);
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
    return onDisk ? const_routing_iterator<T>(vec_, true, onDiskSize, id)
                  : const_routing_iterator<T>(vec_, false, vec_.size(), id);
  }

  // THESE ARE NOT IMPLEMENTED BECAUSE I DON'T THINK THEY ARE REQUIRED FOR THIS
  // APPLICATION reverse_iterator rbegin(); const_reverse_iterator rbegin()
  // const; reverse_iterator rend(); const_reverse_iterator rend() const;

  size_type size() const { return onDisk ? onDiskSize : vec_.size(); }

  size_type max_size() const {
    return onDisk
               ? INFINITY
               : vec_.max_size();  // TODO: Maybe an estimate of disk size here?
                                   // Any relevant MYSQL system variables?
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
    return onDiskSize < 1;
  }

  /**
   *
   * @param position
   * @return An iterator pointing to the element that follows the last deleted element
   */
  routing_iterator<T> erase(iterator position) {
    if (RVector::total_count <= 0) {
      RVector::total_count = 0;
    } else {
      RVector::decrMemFtpr(1);
    }

    if (!onDisk) {
      return vec_.erase(position);
    }
    routing_file_handler<T>::deleteNth(position.get_pointer(), onDiskSize, id);
    onDiskSize--;
    return position;
  }

  /**
   *
   * @param first
   * @param last
   * @return An iterator pointing to the element that follows the last deleted element
   */
  routing_iterator<T> erase(iterator first, iterator last) {
    auto diff = last.get_pointer() - first.get_pointer();
    if (!onDisk) {
      RVector::decrMemFtpr(diff);
      return vec_.erase(first, last);
    }
    routing_file_handler<T>::deleteBetweenMandN(first.get_pointer(), last.get_pointer(), onDiskSize, id);
    last.setTo(last.get_pointer() - diff);
    return last;
  }

  void clear() { erase(begin(), end()); }

  void swap(RVector &right) {
    auto tmp = new RVector<T>(right);
    right.vec_.swap(this->vec_);
    right.onDiskLocation = this->onDiskLocation;
    right.onDisk = this->onDisk;
    right.onDiskSize = this->onDiskSize;

    this->vec_.swap(tmp->vec_);
    this->onDiskLocation = tmp->onDiskLocation;
    this->onDisk = tmp->onDisk;
    this->onDiskSize = tmp->onDiskSize;
  }

  /*
   *** END *** STL CONTAINER REQUIRED
   */

  /*
   * VECTOR REQUIRED
   */

  void erase(RVector<T> &container, const T &value) {
    container.erase(container.begin(), container.end(), value);
  }

  std::pair<typename RVector<T>::iterator, bool> push(RVector<T> &container,
                                                      const T &value) {
    container.push_back(value);
    return std::make_pair(boost::prior(container.end()), true);
  }

  void push_back(T item) {
    const std::size_t current_size = RVector::total_count * sizeof(T);
    if (current_size + sizeof(item) > ram_limit_ && !onDisk) {
      DBUG_LOG("Routing", "Too large. Spilling to disk");
      // Spill over to disk
      moveExistingToDisk();
      routing_file_handler<T>::push(item, id);
      onDiskSize++;
    } else if (onDisk) {
      // Add to existing file on disk
      routing_file_handler<T>::push(item, id);
      onDiskSize++;
    } else {
      // Add to memory
      vec_.push_back(item);
      RVector::incrMemFtpr(1); // Only increase memory footprint when actually in memory
    }
  }

  /*
  void resize(size_type n, const T& val) {
    doResize(n, val);
  }
  */

  void resize(size_type n) {
    doResize(n);//, T());
  }

  reference operator[](size_type n) {
    if (!onDisk) {
      DBUG_LOG("Routing", "Return from internal vector: ");
      return vec_[n];
    }

    DBUG_LOG("Routing", "Return from disk: ");
    auto t = routing_file_handler<T>::readNth(n, id);
    return *t;
  }

  const_reference operator[](size_type n) const {
    if (!onDisk) {
      DBUG_LOG("Routing", "Return from internal vector: ");
      const T *result = const_cast<const T *>(&vec_[n]);
      return *result;
    }

    DBUG_LOG("Routing", "Return from disk: ");
    auto t = routing_file_handler<T>::readNth(n, id);
    return *t;
  }

  /*
   *** END *** VECTOR REQUIRED
   */
 private:
  static void incrMemFtpr(ulonglong n) {
    RVector::total_count += n;

  }

  static void decrMemFtpr(ulonglong n) {
    if ((RVector::total_count - n) <= 0) {
      RVector::total_count = 0;
    } else {
      RVector::total_count -= n;
    }
  }

  void moveExistingToDisk() {
    typedef typename std::vector<T>::iterator Iter;
    bool first = true;
    for (Iter it = vec_.begin(); it != vec_.end(); ++it) {
      routing_file_handler<T>::push(*it, id, first);
      first = false;
    }
    onDisk = true;
    onDiskSize = vec_.size();

  }

  /**
   * One nuance to this function is that it may be called for one instance after
   * another instance has caused the total size of RVectors to go past the limit.
   *
   * To work toward honoring the ram_limit_ this RVector will then
   * resize and move to disk.
   *
   * @param n The new size
   * @param val The value to fill any new spots with
   */
  void doResize(size_type n) { //, const T& val) {
    const std::size_t total_mem_footprint = RVector::total_count * sizeof(T);
    long difference = n - vec_.size();
    if (total_mem_footprint + (difference * sizeof(T)) > ram_limit_ && !onDisk) {
      // If the RVectors are currently in memory, and resizing will make
      // the memory footprint larger than the limit; move to disk.
      // This will happen for each existing RVector as it is pushed
      // to or resized.

      if (n > vec_.size()) {
        // If we resize to larger than current size, move to disk immediately
        moveExistingToDisk();
        for (ulong i = 0; i < n - vec_.size(); i++) {
          routing_file_handler<T>::push(T()/*val*/, id);
        }
      } else {
        // If we resize to smaller than or equal to the current size,
        // resize first and then move to disk
        vec_.resize(n);//, val);
        moveExistingToDisk();

      }
    } else if (!onDisk) {
      // If the RVectors are currently in memory, and resizing will still be
      // within the limit. Resize in memory.
      vec_.resize(n);//,val);
      if (difference > 0) {
        RVector::incrMemFtpr(difference);
      } else if (difference < 0) {
        RVector::decrMemFtpr(difference);
      }

    } else {
      // The RVector is guaranteed onDisk already. Resize appropriately
      if (n > onDiskSize) {
        for (unsigned long i = 0; i < (n - onDiskSize); i++) {
          routing_file_handler<T>::push(T()/*val*/, id);
        }
      } else if (n < onDiskSize) {
        routing_file_handler<T>::deleteAfterN(n, id);
      }
      // Else n == onDiskSize. Don't write or delete anyting
      onDiskSize = n;
    }


  }


  /*
   *** END *** METHODS
   */

};

}  // namespace routing

namespace boost {

struct vecS_profiled {};

template <typename ValueType>
struct container_gen<vecS_profiled, ValueType> {
  // typedef routing::RVector<ValueType, Routing_allocator<ValueType>> type;
  typedef routing::RVector<ValueType> type;
};

template <>
struct parallel_edge_traits<vecS_profiled> {
  typedef allow_parallel_edge_tag type;
};

namespace detail {
template <>
struct is_random_access<vecS_profiled> {
  enum { value = true };
  typedef mpl::true_ type;
};
}  // namespace detail

// Non Intrusive Serialization
namespace serialization {

typedef boost::detail::stored_edge_iter<
    unsigned long,
    std::__1::__list_iterator<
        list_edge<unsigned long, property<edge_weight_t, double, no_property>>,
        void *>,
    property<edge_weight_t, double, no_property>>
    s_e_iter_t;

template <class Archive>
void serialize(Archive &ar, s_e_iter_t data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_target;
  ar &data.s_prop;
}

typedef boost::detail::adj_list_gen<
    adjacency_list<vecS_profiled, vecS_profiled, undirectedS, no_property,
                   property<edge_weight_t, double, no_property>, no_property,
                   listS>,
    vecS_profiled, vecS_profiled, undirectedS, no_property,
    property<edge_weight_t, double, no_property>, no_property,
    listS>::config::stored_vertex stored_vertex_t;

template <class Archive>
void serialize(Archive &ar, stored_vertex_t data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_out_edges;
  ar &data.m_property;
}

template <class Archive>
void serialize(Archive &ar, char data,
               __attribute__((unused)) const unsigned int version) {
  ar &data;
}

template <class Archive>
void serialize(Archive &ar, boost::no_property data,
               __attribute__((unused)) const unsigned int version) {
  //ar &data;
}

}  // namespace serialization

template <typename T>
boost::graph_detail::vector_tag container_category(
    const routing::RVector<T> &) {
  return boost::graph_detail::vector_tag();
}
}  // namespace boost
