//
// Created by Håkon Strandlie on 08/03/2021.
//

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <fstream>
#include <iostream>

namespace routing {
template <typename T>
class routing_file_handler {
 public:
  static T &readNth(int n);

  static void push(T element);
};

template <typename T>
T &routing_file_handler<T>::readNth(int n) {
  char const *onDiskLocation = "/var/tmp/mysql_routing_spillfile";
  std::ifstream file;
  file.open(onDiskLocation, std::ios::in);
  boost::archive::text_iarchive ia(file);
  auto t = new T;

  if (file && file.is_open()) {
    ia >> t;
    for (int i = 0; i < n; i++) {
      ia >> t;
    }
  }
  return *t;
}

template <typename T>
void routing_file_handler<T>::push(T element) {
  char const *onDiskLocation = "/var/tmp/mysql_routing_spillfile";
  std::ofstream file;
  file.open(onDiskLocation, std::ios::out);

  boost::archive::text_oarchive oa(file);
  oa << element;
}
}  // namespace routing