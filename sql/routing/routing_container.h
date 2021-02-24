#include <iostream>
#include <vector>
#include <fstream>
#include "routing_iterator.h"

typedef unsigned long long int ulonglong;

namespace routing {


template <typename T, typename AllocT>
struct RVector {
 private:

  char *const onDiskLocation = "/var/tmp/mysql_routing_spillfile";
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
  typedef size_t size_type;
  typedef typename AllocT::difference_type difference_type;
  typedef T& reference;
  typedef T* pointer;
  typedef typename AllocT::const_reference const_reference;
  typedef T value_type;
  typedef routing_iterator<T> iterator;
  typedef const_routing_iterator<T> const_iterator;
  // typedef T7 reverse_iterator;
  // typedef T8 const_reverse_iterator;

  // Method definitions
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
  routing_iterator<T> erase(routing_iterator<T> where);
  routing_iterator<T> erase(routing_iterator<T> first,
                            routing_iterator<T> last);
  void clear() { erase(begin(), end()); };
  void swap(RVector &right);

  /*
   * STL CONTAINER REQUIRED *** END
   */

  /*
   * VECTOR REQUIRED
   */

  void push_back(T item);
  void resize(size_type n, T val);
  void resize(size_type n);
  T &operator[](size_t n);

  /*
   * VECTOR REQUIRED *** END
   */

  // Copy constructor
  RVector(RVector<T, AllocT> &r_vector)
      : ram_limit_(r_vector.ram_limit_),
        onDisk(r_vector.onDisk),
        vec_(r_vector.vec_),
        onDiskSize(r_vector.onDiskSize) {
    file.open(onDiskLocation);
  };
  RVector(ulonglong ram_limit) : ram_limit_(ram_limit), onDisk(false) {}
  ~RVector() {
    if (file && file.is_open()) {
      file.close();
    }
    remove(onDiskLocation);
  }
};
};