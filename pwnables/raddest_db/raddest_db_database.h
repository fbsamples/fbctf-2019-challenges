#ifndef ARRAY_DATABASE
#define ARRAY_DATABASE

#define MAX_FAST_INDEX 64
#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include <functional>

#include "elements.h"

enum DatabaseType {
  DOUBLE = 1,
  INTEGER = 2,
  STRING = 3,
  MIXED = 4
};

class RaddestDBDatabase {
  public:
    RaddestDBDatabase();
    ~RaddestDBDatabase();

    void Set(std::string type, uint32_t key, std::string value);
    void Get(uint32_t key);
    void Delete(uint32_t key);
    void Empty(void);
    void Print(void);

    void SetCallback(uint32_t key, std::string value, std::string type) {
      Set(type, key, value);
    }

    void GetCallback(uint32_t key, std::string value, std::string type) {
      Get(key);
    }

    void DeleteCallback(uint32_t key, std::string value, std::string type) {
      Delete(key);
    }

    void EmptyCallback(uint32_t key, std::string value, std::string type) {
      Empty();
    }

    void PrintCallback(uint32_t key, std::string value, std::string type) {
      Print();
    }

    void AddGetter(uint32_t key, std::vector<std::string> & tokens);
    void CallGetters(uint32_t key);
    size_t GetterSize(uint32_t key) {
      return getters_[key].size();
    }

  private:


    void StoreStringElement(uint32_t key, const char *value);
    void StoreIntegerElement(uint32_t key, uint64_t value);
    void StoreDoubleElement(uint32_t key, double value);

    void StoreElement(uint32_t key, ArrayElement *value);

    ArrayElement *GetArrayElement(uint32_t key);

    void PrintElements(void);

    std::unordered_map<uint32_t , std::vector<std::function<void(uint32_t, std::string, std::string)> > > getters_;

    std::unordered_map<uint32_t, ArrayElement *> *backing_store_;
};

void EchoCallback(uint32_t key, std::string value, std::string type);
void Echo(std::string value);

#endif
