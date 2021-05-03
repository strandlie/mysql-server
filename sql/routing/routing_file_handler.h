//
// Created by Håkon Strandlie on 08/03/2021.
//

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>
#include <iostream>
#include <memory>

namespace routing {
template <typename T>
class routing_file_handler {
  typedef unsigned long ulong;
 public:
  static std::string onDiskLocation;
  //static std::shared_ptr<T> readNth(ulong, boost::uuids::uuid);
  static std::vector<T, Routing_allocator<T>> readVectorWithNumber(size_t, boost::uuids::uuid);
  static std::vector<std::shared_ptr<T>> readNFirst(ulong,
                                         boost::uuids::uuid);

  //static void push(const T&, boost::uuids::uuid id, bool = false);
  //static void push(std::shared_ptr<T>, boost::uuids::uuid id, bool = false);
  static void pushVectorWithIdx(size_t, std::vector<T, Routing_allocator<T>>, boost::uuids::uuid id);
  static void deleteNth(ulong n, ulong totalN, boost::uuids::uuid);
  static void deleteAfterN(ulong, boost::uuids::uuid);
  static void deleteBetweenMandN(ulong m, ulong n, ulong total, boost::uuids::uuid id);
  static void deleteFileForId(boost::uuids::uuid id);
};

template <typename T>
std::string routing_file_handler<T>::onDiskLocation =
    "/var/tmp/mysql_routing/";

template <typename T>
std::vector<T, Routing_allocator<T>> routing_file_handler<T>::readVectorWithNumber(size_t file_nr,
                                                    boost::uuids::uuid id) {
  std::ifstream file;
  auto location = onDiskLocation + to_string(id) + "-" + std::to_string(file_nr);
  std::vector<T, Routing_allocator<T>> vec;

  file.open(location, std::ios::in);
  if (file.is_open()) {
    boost::archive::text_iarchive ia(file);
    ia >> vec;
  }
  return vec;
}

template <typename T>
void routing_file_handler<T>::pushVectorWithIdx(size_t file_nr, std::vector<T, Routing_allocator<T>> vec, boost::uuids::uuid id) {
  // char const *onDiskLocation = "/var/tmp/mysql_routing_spillfile";
  std::ofstream file;

  auto location = onDiskLocation + to_string(id) + "-" + std::to_string(file_nr);

  remove(location.c_str());

  file.open(location, std::ios::out);

  boost::archive::text_oarchive oa(file);
  oa << vec;

  // File and archive closes when they go out of scope
}

template <typename T>
std::vector<std::shared_ptr<T>> routing_file_handler<T>::readNFirst(
    ulong n, boost::uuids::uuid id) {
  std::vector<std::shared_ptr<T>> vec;
  std::ifstream file;
  file.open(onDiskLocation + to_string(id), std::ios::in);
  boost::archive::text_iarchive ia(file);
  for (int i = 0; i < n; i++)  {
    auto t = std::make_shared<T>();
    ia >> t;
    vec.push_back(t);
  }
  file.close();
  return vec;
}

/*
template <typename T>
void routing_file_handler<T>::push(std::shared_ptr<T> element, boost::uuids::uuid id,
                                   bool removeExisting) {
  // char const *onDiskLocation = "/var/tmp/mysql_routing_spillfile";
  std::ofstream file;

  auto location = onDiskLocation + to_string(id);
  if (removeExisting) {
    remove(location.c_str());
  }
  file.open(location, std::ios::out | std::ios::app);

  boost::archive::text_oarchive oa(file);
  oa << element;

  file.close();
}
*/


template <typename T>
void routing_file_handler<T>::deleteNth(ulong n, ulong totalN, boost::uuids::uuid id) {
  std::ifstream inFile;
  std::ofstream tmpFile;

  auto origFilePath = onDiskLocation + to_string(id);
  auto tmpFilePath = onDiskLocation + to_string(id) + "__TMP";

  inFile.open(origFilePath, std::ios::in);
  tmpFile.open(tmpFilePath, std::ios::out | std::ios::app);
  boost::archive::text_iarchive ia(inFile);
  boost::archive::text_oarchive tmpa(tmpFile);


  T t;
  for (ulong i = 0; i < totalN; i++) {
    ia >> t;
    if (i != n) {
      tmpa << t;
    }
  }

  inFile.close();
  tmpFile.close();

  remove(origFilePath.c_str());
  rename(tmpFilePath.c_str(), origFilePath.c_str());


}

template <typename T>
void routing_file_handler<T>::deleteAfterN(ulong n, boost::uuids::uuid id) {
  std::ifstream inFile;
  std::ofstream tmpFile;

  auto origFilePath = onDiskLocation + to_string(id);
  auto tmpFilePath = onDiskLocation + to_string(id) + "__TMP";

  inFile.open(origFilePath, std::ios::in);
  tmpFile.open(tmpFilePath, std::ios::out | std::ios::app);
  boost::archive::text_iarchive ia(inFile);
  boost::archive::text_oarchive tmpa(tmpFile);


  T t;
  for (ulong i = 0; i < n; i++) {
    ia >> t;
    tmpa << t;
  }
  inFile.close();
  tmpFile.close();

  remove(origFilePath.c_str());
  rename(tmpFilePath.c_str(), origFilePath.c_str());
}

template <typename T>
void routing_file_handler<T>::deleteBetweenMandN(ulong m, ulong n, ulong totalN, boost::uuids::uuid id) {
  std::ifstream inFile;
  std::ofstream tmpFile;

  auto origFilePath = onDiskLocation + to_string(id);
  auto tmpFilePath = onDiskLocation + to_string(id) + "__TMP";

  inFile.open(origFilePath, std::ios::in);
  tmpFile.open(tmpFilePath, std::ios::out | std::ios::app);
  boost::archive::text_iarchive ia(inFile);
  boost::archive::text_oarchive tmpa(tmpFile);

  T t;
  for (ulong i = 0; i < totalN; i++) {
    if (i < m || i > n) {
      ia >> t;
      tmpa << t;
    }
  }

  inFile.close();
  tmpFile.close();

  remove(origFilePath.c_str());
  rename(tmpFilePath.c_str(), origFilePath.c_str());
}

template <typename T>
void routing_file_handler<T>::deleteFileForId(boost::uuids::uuid id) {
  std::ifstream file;
  size_t i = 0;
  auto filePath = onDiskLocation + to_string(id) + "-" + std::to_string(i);
  file.open(filePath, std::ios::in);
  while(file.is_open()) {
    remove(filePath.c_str());
    i++;
    filePath = onDiskLocation + to_string(id) + "-" + std::to_string(i);
    file.open(filePath);
  }
}
}  // namespace routing
