//
// Created by Håkon Strandlie on 08/03/2021.
//

#include <fstream>

#ifndef MYSQL_ROUTING_FILE_HANDLER_H
#define MYSQL_ROUTING_FILE_HANDLER_H

namespace routing {
template <typename T>
class routing_file_handler {
 public:
  static T &readNth(int n, std::ifstream file);
};
}


#endif  // MYSQL_ROUTING_FILE_HANDLER_H