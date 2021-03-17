//
// Created by Håkon Strandlie on 08/03/2021.
//

#include <fstream>
#include <iostream>
#include <boost/archive/text_oarchive.hpp>

#ifndef MYSQL_ROUTING_FILE_HANDLER_H
#define MYSQL_ROUTING_FILE_HANDLER_H

namespace routing {
template <typename T>
class routing_file_handler {
 public:
  static T &readNth(int n) {
    char const *onDiskLocation = "/var/tmp/mysql_routing_spillfile";
    std::ifstream file;
    file.open(onDiskLocation, std::ios::in);

    auto t = new T;
    if (file && file.is_open()) {
      //file >> t;
      for (int i = 0; i < n; i++) {
        //file >> t;
      }
    }
    return *t;
  }

  static void push(T element) {
    char const *onDiskLocation = "/var/tmp/mysql_routing_spillfile";
    std::ofstream file;
    file.open(onDiskLocation, std::ios::out);

    boost::archive::text_oarchive oa(file);
    oa << element;
  }

};
}


#endif  // MYSQL_ROUTING_FILE_HANDLER_H