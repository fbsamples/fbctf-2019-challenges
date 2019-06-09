
#include <algorithm>
#include <unordered_map>
#include <limits>
#include <iostream>
#include <stdlib.h>

#include "raddest_db_database.h"
#include "elements.h"

//////////////////////////////
// Constructor/Destructor
//////////////////////////////
using namespace std::placeholders;

RaddestDBDatabase::RaddestDBDatabase() :
  backing_store_(nullptr) {
  backing_store_ = new std::unordered_map<uint32_t, ArrayElement *>();
}

RaddestDBDatabase::~RaddestDBDatabase() {}

//////////////////////////////
// Private Methods
//////////////////////////////

void RaddestDBDatabase::StoreStringElement(uint32_t key, const char *value) {
  auto it = backing_store_->find(key);

  if (it == backing_store_->end()) {
    StringElement *new_element = new StringElement(value);
    StoreElement(key, new_element);
  } else {
    StringElement *old_element = reinterpret_cast<StringElement *>(it->second);

    size_t len = strnlen(value, 1024);
    char *new_value = new char[len + 1];
    memset(new_value, 0, len + 1);
    strncpy(new_value, value, len);

    old_element->element = new_value;
  }
}

void RaddestDBDatabase::StoreIntegerElement(uint32_t key, uint64_t value) {
  auto it = backing_store_->find(key);

  if (it == backing_store_->end()) {
    IntegerElement *new_element = new IntegerElement(value);
    StoreElement(key, new_element);
  } else {
    IntegerElement *old_element = reinterpret_cast<IntegerElement *>(it->second);
    old_element->element = value;
  }
}

void RaddestDBDatabase::StoreDoubleElement(uint32_t key, double value) {
  auto it = backing_store_->find(key);

  if (it == backing_store_->end()) {
    DoubleElement *new_element = new DoubleElement(value);
    StoreElement(key, new_element);
  } else {
    DoubleElement *old_element = reinterpret_cast<DoubleElement *>(it->second);
    old_element->element = value;
  }
}

void RaddestDBDatabase::StoreElement(uint32_t key, ArrayElement *element) {
	(*backing_store_)[key] = element;
}

ArrayElement *RaddestDBDatabase::GetArrayElement(uint32_t key) {
  auto it = backing_store_->find(key);
  if (it == backing_store_->end()) return NULL;
  return it->second;
}

void RaddestDBDatabase::PrintElements(void) {
  int count = 0;
  for (auto it = backing_store_->begin(); it != backing_store_->end(); ++it) {
    it->second->print();
    CallGetters(it->first);
  }
}

//////////////////////////////
// Public Methods
//////////////////////////////
void RaddestDBDatabase::Set(std::string type, uint32_t key, std::string value) {
  if (type == "string") {
    StoreStringElement(key, value.c_str());
  }

  if (type == "int") {
    StoreIntegerElement(key, std::atoi(value.c_str()));
  }

  if (type == "float") {
    std::cout << value << std::endl;
    StoreDoubleElement(key, std::stold(value.c_str()));
  }
}

void RaddestDBDatabase::Get(uint32_t key) {
  ArrayElement *element = GetArrayElement(key);
  if (element == NULL) return;
  element->print();
  CallGetters(key);
}

void RaddestDBDatabase::Delete(uint32_t key) {
  backing_store_->erase(key);
}

void RaddestDBDatabase::Empty(void) {
  backing_store_->clear();
}

void RaddestDBDatabase::Print(void) {
  PrintElements();
}

void RaddestDBDatabase::CallGetters(uint32_t key) {
  std::vector<std::function<void(uint32_t, std::string, std::string)> > callback_vector = getters_[key];

  for ( auto it = callback_vector.begin(); it != callback_vector.end(); it++ ) {
    (*it)(0, std::string("bound0"), std::string("bound1"));
  }
}

void RaddestDBDatabase::AddGetter(uint32_t key, std::vector<std::string> & tokens) {
  if (tokens.size() < 1) return;

  std::string command = tokens[0];

  if (command == "get") {
    if (tokens.size() != 2) return;
    uint32_t get_key = std::atoi(tokens[1].c_str());
    getters_[key].push_back(std::bind(&RaddestDBDatabase::GetCallback, this, get_key, std::string(), std::string()));
  }

  if (command == "store") {
    if (tokens.size() != 4) return;
    uint32_t set_key = std::atoi(tokens[2].c_str());
    getters_[key].push_back(std::bind(&RaddestDBDatabase::SetCallback, this, set_key, tokens[3], tokens[1]));
  }

  if (command == "delete") {
    if (tokens.size() != 2) return;
    uint32_t delete_key = std::atoi(tokens[2].c_str());
    getters_[key].push_back(std::bind(&RaddestDBDatabase::DeleteCallback, this, delete_key, tokens[1], std::string()));
  }

  if (command == "empty") {
    if (tokens.size() != 1) return;
    getters_[key].push_back(std::bind(&RaddestDBDatabase::EmptyCallback, this, 0, std::string(), std::string()));
  }

  if (command == "echo") {
    if (tokens.size() != 2) return;
    getters_[key].push_back(std::bind(EchoCallback, 0, tokens[1], std::string()));
  }

  if (command == "print") {
    if (tokens.size() != 1) return;
    getters_[key].push_back(std::bind(&RaddestDBDatabase::PrintCallback, this, key, std::string(), std::string()));
  }
}

void EchoCallback(uint32_t key, std::string str, std::string str1) {
  std::cout << str << std::endl;
}

void Echo(std::string str) {
  std::cout << str << std::endl;
}

