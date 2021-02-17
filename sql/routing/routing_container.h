#include <iostream>
#include <vector>

#include <iostream>
#include <vector>
#include <fstream>

typedef unsigned long long int ulonglong;
char *const onDiskLocation = "/var/tmp/mysql_routing_spillfile";

template<typename T>
struct rout_It
{
  std::vector<T>& vec_;
  int pointer_;
  bool onDisk;
  std::ifstream inFile;

  rout_It(std::vector<T>& vec, bool onDisk) :
      vec_(vec), pointer_{0}, onDisk(onDisk) {
    if (onDisk) {
      inFile.open(onDiskLocation, std::ios::in);
    }
  }

  rout_It(std::vector<T>& vec, bool onDisk, int size) :
      vec_(vec), pointer_{size}, onDisk(onDisk) {
    if (onDisk) {
      inFile.open(onDiskLocation, std::ios::in);
    }
  }

  rout_It(const rout_It<T> &ite):
      vec_(ite.vec_), pointer_(ite.pointer_), onDisk(ite.onDisk) {
    if (onDisk) {
      inFile.open(onDiskLocation, std::ios::in);
    }
  }

  ~rout_It() {
    if (inFile.is_open()) {
      inFile.close();
    }
  }



  bool operator!=(const rout_It<T>& other) const;
  bool operator==(const rout_It<T>& other) const;
  rout_It& operator++();
  T& operator*();
};

template<typename T>
struct RVector
{
  std::vector<T> vec_;
  ulonglong ram_limit_;
  std::ofstream outFile;
  bool onDisk;
  ulonglong onDiskSize;


  RVector(ulonglong ram_limit) : ram_limit_(ram_limit), onDisk(false) {}
  ~RVector() {
    if(outFile && outFile.is_open()) {
      outFile.close();
    }
    remove(onDiskLocation);
  }
  void push_back(T item);
  rout_It<T> begin();
  rout_It<T> end();
};
