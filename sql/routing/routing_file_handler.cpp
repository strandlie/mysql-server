//
// Created by Håkon Strandlie on 08/03/2021.
//

#include "routing_file_handler.h"

namespace routing {
template<typename T>
static T &routing_file_handler<T>::readNth(int n, std::ifstream file) {
  auto t = new T;
  if (file && file.is_open()) {
    file >> *t;
    for (int i = 0; i < n; i++) {
      file >> *t;
    }
  }
  return *t;
}

}
