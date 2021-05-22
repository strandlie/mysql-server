//
// Created by Håkon Strandlie on 08/03/2021.
//

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "routing_stats.h"

namespace routing {
#include <filesystem>
template <typename T>
class routing_file_handler {
  typedef unsigned long ulong;

 public:
  static std::string onDiskLocation;
  std::vector<std::ofstream> outFiles;
  std::vector<std::ifstream> inFiles;
  boost::uuids::uuid id;

  routing_file_handler(boost::uuids::uuid id) : id(id) {}

  void readVectorWithNumber(size_t, std::vector<T, Routing_allocator<T>>&);
  void pushVectorWithIdx(size_t, std::vector<T, Routing_allocator<T>>);
  void deleteFiles();

  std::ifstream &getInfileForFileNr(size_t);
  std::ofstream &getOutfileForFileNr(size_t);
};

template <typename T>
std::string routing_file_handler<T>::onDiskLocation = "/var/tmp/mysql_routing/";

template <typename T>
void routing_file_handler<T>::readVectorWithNumber(size_t file_nr, std::vector<T, Routing_allocator<T>>& vec) {
  std::ifstream &file = getInfileForFileNr(file_nr);
  file.seekg(0);
  if(file.is_open()) {
    boost::archive::text_iarchive ia(file);
    ia >> vec;
  }
  RoutingStats::numBytesRead += file.tellg();
}

template <typename T>
void routing_file_handler<T>::pushVectorWithIdx(
    size_t file_nr, std::vector<T, Routing_allocator<T>> vec) {
  std::ofstream &file = getOutfileForFileNr(file_nr);
  file.seekp(0);
  boost::archive::text_oarchive oa(file);
  oa << vec;
  RoutingStats::numBytesWritten += file.tellp();
  file.flush();

  // Archive closes when it goes out of scope
}

template <typename T>
void routing_file_handler<T>::deleteFiles() {
  for (int i = 0; i < fmax(inFiles.size(), outFiles.size()); i++) {
    auto filePath = onDiskLocation + to_string(id) + "-" + std::to_string(i);
    remove(filePath.c_str());
  }
}

template <typename T>
std::ifstream &routing_file_handler<T>::getInfileForFileNr(size_t file_nr) {
  if (file_nr < inFiles.size()) {
    std::ifstream& inFile = inFiles[file_nr];
    if(!inFile.is_open()) {
      // Try again to open it, since it may have
      // been created since last time
      auto location =
          onDiskLocation + to_string(id) + "-" + std::to_string(file_nr);
      inFile.open(location, std::ios::in);
    }
    return inFile;
  }

  ulong old_size = inFiles.size();
  inFiles.resize(file_nr + 1);
  for(ulong i = old_size; i < inFiles.size(); i++) {
    auto location =
        onDiskLocation + to_string(id) + "-" + std::to_string(i);
    std::ifstream& inFile = inFiles[i];
    inFile.open(location, std::ios::in);
  }
  return inFiles[file_nr];
}

template <typename T>
std::ofstream &routing_file_handler<T>::getOutfileForFileNr(size_t file_nr) {
  if (file_nr < outFiles.size()) {
    return outFiles[file_nr];
  }

  ulong old_size = outFiles.size();
  outFiles.resize(file_nr + 1);
  for(ulong i = old_size; i < outFiles.size(); i++) {
    std::ofstream& file = outFiles[i];
    auto location =
        onDiskLocation + to_string(id) + "-" + std::to_string(i);
    file.open(location, std::ios::out);
  }
  return outFiles[file_nr];
}
}  // namespace routing
