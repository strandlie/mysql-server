
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>
#include "boost/pending/container_traits.hpp"
#include "my_dbug.h"
#include "sql/current_thd.h"
#include "sql/sql_class.h"

#include "sql/routing/routing_iterator.h"
#include "sql/routing/routing_stats.h"

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#ifndef MYSQL_RVECTOR_H
#define MYSQL_RVECTOR_H

typedef unsigned long long int ulonglong;

namespace routing {

template <typename T>
class RVector {
 private:
  boost::uuids::uuid id;

  // Contains the current working set
  std::vector<T, Routing_allocator<T>> vec_;
  routing_file_handler<T> fh;
  ulonglong ram_limit_;
  ulonglong totalSize;
  size_t currentFileIdxInMem;

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
      : id(r_vector.id),
        vec_(r_vector.vec_),
        ram_limit_(r_vector.ram_limit_),
        totalSize(r_vector.totalSize),
        currentFileIdxInMem(r_vector.currentFileIdxInMem) {
    fh = routing_file_handler<T>(id);
  }

  // Move constructor
  // This doesn't actually move anything, since all members are values
  RVector(RVector &&source) noexcept
      : id(source.id),
        vec_(source.vec_),
        ram_limit_(source.ram_limit_),
        totalSize(source.totalSize),
        currentFileIdxInMem(source.currentFileIdxInMem) {
    fh = routing_file_handler<T>(id);
    /*
    source.ram_limit_ = nullptr;
    source.onDisk = nullptr;
    source.vec_ = nullptr;
    source.totalSize = nullptr;
     */
  }

  explicit RVector(ulonglong ram_limit)
      : id(boost::uuids::random_generator()()),
        vec_(),
        ram_limit_(ram_limit),
        totalSize{0},
        currentFileIdxInMem(0) {
    fh = routing_file_handler<T>(id);
  }

  // Default constructor
  RVector() : id(boost::uuids::random_generator()()), vec_(), fh(id) {
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
    totalSize = 0;
    currentFileIdxInMem = 0;
    DBUG_LOG("Routing", "Ram-limit set to: " << ram_limit_);
  }

  // Destructor
  ~RVector() {
    /*
     * Don't delete for experiment
    try {
      fh.deleteFiles();
    } catch (std::exception e) {
      DBUG_LOG("Routing", e.what());
    }
     */
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
    ar &id;
    ar &vec_;
    ar &ram_limit_;
    ar &totalSize;
    ar &currentFileIdxInMem;
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
  routing_iterator<T> begin() {
    return routing_iterator<T>(vec_, id, currentFileIdxInMem, totalSize,
                               ram_limit_);
  }

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
    return const_routing_iterator<T>(vec_, id, currentFileIdxInMem, totalSize,
                                     ram_limit_);
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
    return routing_iterator<T>(vec_, totalSize, id, currentFileIdxInMem,
                               totalSize, ram_limit_);
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
    return const_routing_iterator<T>(vec_, id, totalSize, currentFileIdxInMem,
                                     totalSize, ram_limit_);
  }

  // THESE ARE NOT IMPLEMENTED BECAUSE I DON'T THINK THEY ARE REQUIRED FOR THIS
  // APPLICATION reverse_iterator rbegin(); const_reverse_iterator rbegin()
  // const; reverse_iterator rend(); const_reverse_iterator rend() const;

  size_type size() const { return totalSize; }

  size_type max_size() const {
    return INFINITY;  // TODO: Maybe an estimate of disk size here?
                      // Any relevant MYSQL system variables?
  }

  bool empty() const { return totalSize < 1; }

  /**
   *
   * @param position
   * @return An iterator pointing to the element that follows the last deleted
   * element
   */
  routing_iterator<T> erase(iterator position) {
    size_t file_idx = getFileIndex(position.get_pointer());
    changeWorkingSet(file_idx);

    size_t element_idx = getElementIndex(position.get_pointer());
    vec_.erase(vec_.begin() + element_idx);

    totalSize--;
    return position;
  }

  /**
   *
   * @param first
   * @param last
   * @return An iterator pointing to the element that follows the last deleted
   * element
   */
  routing_iterator<T> erase(iterator first, iterator last) {
    auto diff = last.get_pointer() - first.get_pointer();
    for (auto it = first; it != last; it++) {
      erase(it);
    }
    last.setTo(last.get_pointer() - diff);
    return last;
  }

  void clear() { erase(begin(), end()); }

  void swap(RVector &right) {
    auto tmp = new RVector<T>(right);
    right.vec_.swap(this->vec_);
    right.onDiskLocation = this->onDiskLocation;
    right.totalSize = this->totalSize;
    right.currentFileIdxInMem = this->currentFileIdxInMem;

    this->vec_.swap(tmp->vec_);
    this->onDiskLocation = tmp->onDiskLocation;
    this->totalSize = tmp->totalSize;
    this->currentFileIdxInMem = tmp->currentFileIdxInMem;
  }

  RVector &operator=(RVector other) {
    std::swap(vec_, other.vec_);
    std::swap(ram_limit_, other.ram_limit_);
    std::swap(totalSize, other.totalSize);
    std::swap(currentFileIdxInMem, other.currentFileIdxInMem);
    return *this;
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
    size_t new_size = totalSize + 1;
    size_t file_idx = getFileIndex(new_size);
    changeWorkingSet(file_idx);
    vec_.push_back(item);
    totalSize = new_size;
  }

  /*
  void resize(size_type n, const T& val) {
    doResize(n, val);
  }
  */

  void resize(size_type n) {
    if (n > totalSize) {
      // Add the elements beyond the current totalSize
      for (size_t i = totalSize; i < n; i++) {
        size_t file_idx = getFileIndex(i);
        changeWorkingSet(file_idx);
        vec_.push_back(T());
        totalSize++;
      }
      return;
    } else if (n < totalSize) {
      iterator it = this->begin();
      it.setTo(n);
      for (; it != this->end(); it++) {
        erase(it);
        totalSize--;
      }
      return;
    }

    // Else n == totalSize. Don't write or delete anyting
    totalSize = n;
  }

  reference operator[](size_type n) {
    size_t file_idx = getFileIndex(n);
    changeWorkingSet(file_idx);
    size_t element_idx = getElementIndex(n);
    return vec_[element_idx];
  }

  const_reference operator[](size_type n) const {
    auto file_idx = getFileIndex(n);
    (const_cast<RVector *>(this))->changeWorkingSet(file_idx);
    size_t element_idx = getElementIndex(n);

    return const_cast<const_reference>(vec_.at(element_idx));
  }

  /*
   *** END *** VECTOR REQUIRED
   */
 private:
  void moveExistingToDisk() {
    typedef typename std::vector<T>::iterator Iter;

    if (getFileIndex(vec_.size()) > 0) {
      throw std::invalid_argument("Vector is too large when moving to disk");
    }

    routing_file_handler<T>::pushVectorWithIdx(0, vec_, id);
    currentFileIdxInMem = 0;
    totalSize = vec_.size();
    vec_.clear();
  }

  void changeWorkingSet(size_t new_idx) {
    if (new_idx == currentFileIdxInMem) {
      return;
    }
    RoutingStats::numSwaps += 1;
    fh.pushVectorWithIdx(currentFileIdxInMem, vec_);
    vec_ = std::vector<T, Routing_allocator<T>>();
    vec_ = fh.readVectorWithNumber(new_idx);
    currentFileIdxInMem = new_idx;
  }

  size_type getFileIndex(size_type n) const {
    const size_type max_vec_size = (ram_limit_ / 2) / sizeof(T);
    return n / max_vec_size;
  }

  size_type getElementIndex(size_type n) const {
    const size_type max_vec_size = (ram_limit_ / 2) / sizeof(T);
    return n % max_vec_size;
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
void serialize(Archive &ar, s_e_iter_t &data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_target;
  ar &data.s_prop;
  ar &data.m_iter;
}

typedef std::__1::__list_iterator<
    boost::list_edge<
        unsigned long,
        boost::property<boost::edge_weight_t, double, boost::no_property>>,
    void *>
    list_edge_iter;

template <class Archive>
void serialize(Archive &ar, list_edge_iter &data,
               __attribute__((unused)) const unsigned int version) {
  ar & data->m_property;
  ar & data->m_source;
  ar & data->m_target;
}

typedef boost::property<boost::edge_weight_t, double, boost::no_property>
    edge_w_prop;

template <class Archive>
void serialize(Archive &ar, edge_w_prop &data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_base;
  ar &data.m_value;
}

typedef boost::detail::adj_list_gen<
    adjacency_list<vecS_profiled, vecS_profiled, undirectedS, no_property,
                   property<edge_weight_t, double, no_property>, no_property,
                   listS>,
    vecS_profiled, vecS_profiled, undirectedS, no_property,
    property<edge_weight_t, double, no_property>, no_property,
    listS>::config::stored_vertex stored_vertex_t;

typedef boost::detail::adj_list_gen<
    adjacency_list<vecS, vecS_profiled, undirectedS, no_property,
                   property<edge_weight_t, double, no_property>, no_property,
                   listS>,
    vecS_profiled, vecS, undirectedS, no_property,
    property<edge_weight_t, double, no_property>, no_property,
    listS>::config::stored_vertex stored_vertex_t_vec_s;

template <class Archive>
void serialize(Archive &ar, stored_vertex_t &data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_out_edges;
  ar &data.m_property;
}

template <typename Archive>
void serialize(Archive &ar, stored_vertex_t_vec_s &data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_out_edges;
  ar &data.m_property;
  // split_free(ar, data, version);
}

/*
template <class Archive>
void save(Archive &ar, const stored_vertex_t_vec_s data,
          __attribute__((unused)) const unsigned int version) {
  size_t dist = std::distance(data.m_out_edges.begin(), )
}

template <class Archive>
void load(Archive &ar, const stored_vertex_t_vec_s data,
          __attribute__((unused)) const unsigned int version) {

}
*/
template <class Archive>
void serialize(Archive &ar, char &data,
               __attribute__((unused)) const unsigned int version) {
  ar &data;
}

template <class Archive>
void serialize(Archive &ar, boost::no_property &data,
               __attribute__((unused)) const unsigned int version) {
  // ar &data;
}

/*
 * After changing to vecS for EdgeList:
 */

typedef boost::detail::adj_list_gen<
    boost::adjacency_list<
        boost::vecS_profiled, boost::vecS_profiled, boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double, boost::no_property>,
        boost::no_property, boost::vecS>,
    boost::vecS_profiled, boost::vecS_profiled, boost::undirectedS,
    boost::no_property,
    boost::property<boost::edge_weight_t, double, boost::no_property>,
    boost::no_property, boost::vecS>::config::stored_vertex stored_vertex_t_2;

template <class Archive>
void serialize(Archive &ar, stored_vertex_t_2 &data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_out_edges;
  ar &data.m_property;
}

typedef boost::detail::stored_ra_edge_iter<
    unsigned long,
    std::__1::vector<
        boost::list_edge<
            unsigned long,
            boost::property<boost::edge_weight_t, double, boost::no_property>>,
        std::__1::allocator<boost::list_edge<
            unsigned long, boost::property<boost::edge_weight_t, double,
                                           boost::no_property>>>>,
    boost::property<boost::edge_weight_t, double, boost::no_property>>
    stored_ra_iter_t;

template <class Archive>
void serialize(Archive &ar, stored_ra_iter_t &data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_target;
  ar &data.m_i;
  //ar &data.m_vec;
}

typedef boost::list_edge<
    unsigned long,
    boost::property<boost::edge_weight_t, double, boost::no_property>>
    list_edge_t;

template <class Archive>
void serialize(Archive &ar, list_edge_t &data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_property;
  ar &data.m_target;
  ar &data.m_source;
}

typedef boost::detail::adj_list_gen<
    boost::adjacency_list<
        boost::vecS, boost::vecS_profiled, boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double, boost::no_property>,
        boost::no_property, boost::vecS>,
    boost::vecS_profiled, boost::vecS, boost::undirectedS, boost::no_property,
    boost::property<boost::edge_weight_t, double, boost::no_property>,
    boost::no_property, boost::vecS>::config::stored_vertex stored_vertex_t_3;

template <class Archive>
void serialize(Archive &ar, stored_vertex_t_3 &data,
               __attribute__((unused)) const unsigned int version) {
  ar &data.m_property;
  ar &data.m_out_edges;
}
}  // namespace serialization

template <typename T>
boost::graph_detail::vector_tag container_category(
    const routing::RVector<T> &) {
  return boost::graph_detail::vector_tag();
}

}  // namespace boost

typedef boost::adjacency_list<
    boost::vecS, boost::vecS_profiled, boost::undirectedS, boost::no_property,
    boost::property<boost::edge_weight_t, double>, boost::no_property,
    boost::vecS>  //, b::no_property, b::vecS_profiled>
    Graph;

typedef boost::detail::adj_list_gen<
    Graph, boost::vecS_profiled, boost::vecS, boost::undirectedS,
    boost::no_property, boost::property<boost::edge_weight_t, double>,
    boost::no_property, boost::vecS>
    alg;

template <>
inline void routing::RVector<alg::config::stored_vertex>::changeWorkingSet(
    size_t new_idx) {
  if (new_idx == currentFileIdxInMem) {
    return;
  }
  RoutingStats::numSwaps += 1;
  std::vector<boost::list_edge<unsigned long,
                               boost::property<boost::edge_weight_t, double>>>
      *edge_vec_ptr = nullptr;
  bool done = false;
  for (auto & it : vec_) {
    if(done) {
      break;
    }
    for (auto & m_out_edge : it.m_out_edges) {
      if (done) {
        break;
      }
      edge_vec_ptr = m_out_edge.m_vec;
      done = true;
    }
  }
  fh.pushVectorWithIdx(currentFileIdxInMem, vec_);
  vec_.clear();
  fh.readVectorWithNumber(new_idx, vec_);
  DBUG_ASSERT(edge_vec_ptr != nullptr);
  for(auto & it : vec_) {
    for(auto & m_out_edge : it.m_out_edges) {
      //delete m_out_edge.m_vec;
      m_out_edge.m_vec = edge_vec_ptr;
    }
  }
  currentFileIdxInMem = new_idx;
}

#endif  // MYSQL_RVECTOR_H
