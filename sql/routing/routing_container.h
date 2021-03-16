#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include "boost/pending/container_traits.hpp"
#include "routing_iterator.cc"
#include "sql/current_thd.h"
#include "sql/sql_class.h"

typedef unsigned long long int ulonglong;


namespace routing {

template <typename T, typename AllocT>
struct RVector {
 private:

  char const *onDiskLocation = "/var/tmp/mysql_routing_spillfile";
  std::vector<T, AllocT> vec_;
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
  typedef typename AllocT::difference_type difference_type;
  typedef T& reference;
  typedef T* pointer;
  typedef typename AllocT::const_reference const_reference;
  typedef T value_type;
  typedef routing_iterator<T> iterator;
  typedef const_routing_iterator<T> const_iterator;
  // typedef T7 reverse_iterator;
  // typedef T8 const_reverse_iterator;

  /*
   * CONSTRUCTORS
   */
  // Copy constructor
  RVector(RVector<T, AllocT> &r_vector)
      : ram_limit_(r_vector.ram_limit_),
        onDisk(r_vector.onDisk),
        vec_(r_vector.vec_),
        onDiskSize(r_vector.onDiskSize) {
    file.open(onDiskLocation);
  }
  RVector(ulonglong ram_limit) : ram_limit_(ram_limit), onDisk(false) {}
  RVector() {
    if (current_thd) {
      ulonglong ram_limitation =
          fmin(current_thd->variables.tmp_table_size, current_thd->variables.max_heap_table_size);
      ram_limit_ = ram_limitation;
    } else {
      // Fallback
      ram_limit_ = 32000; // 32kB
    }
    DBUG_LOG("Routing", "Ram-limit set to: " << ram_limit_);
  }
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
  routing_iterator<T> begin();
  const_routing_iterator<T> begin() const;
  routing_iterator<T> end();
  const_routing_iterator<T> end() const;
  // reverse_iterator rbegin();
  // const_reverse_iterator rbegin() const;
  // reverse_iterator rend();
  // const_reverse_iterator rend() const;
  size_type size() const;
  size_type max_size() const;
  bool empty() const;
  routing_iterator<T> erase(iterator where);
  routing_iterator<T> erase(iterator first,
                            iterator last);
  routing_iterator<T> erase(iterator first,
                            iterator last,
                            const T& value);
  void clear() { erase(begin(), end()); }
  void swap(RVector &right);

  /*
   *** END *** STL CONTAINER REQUIRED
   */

  /*
   * VECTOR REQUIRED
   */

  std::pair<typename RVector<T, AllocT>::iterator, bool> push(RVector<T, AllocT>& container, const T& value);
  void erase(RVector<T, AllocT>& container, const T& value);
  void push_back(T item);
  void resize(size_type n, T val);
  void resize(size_type n);
  reference operator[](size_type n);
  reference operator[](size_type n) const;

  /*
   *** END *** VECTOR REQUIRED
   */


  /*
   *** END *** METHOD DEFINITIONS
   */

};

template<typename T, typename AllocT>
boost::graph_detail::vector_tag container_category(const RVector<T, AllocT>&) { return boost::graph_detail::vector_tag(); }
}