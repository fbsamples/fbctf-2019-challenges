#ifndef ELEMENTS
#define ELEMENTS

#include <iostream>
#include <assert.h>
#include <string.h>


class ArrayElement {
  public:
    ArrayElement(void) {
    }

    virtual void print(void) {
      assert(0);
    }

};

class DoubleElement : public ArrayElement {
  public:
    DoubleElement(double e) {
      element = e;
    }

    void print(void) {
      std::cout << element << std::endl;
    }

    double raw_element(void) {
      return element;
    }
  double element;
};

class IntegerElement : public ArrayElement {
  public:
    IntegerElement(uint64_t e) {
      element = e;
    }

    void print(void) {
      std::cout << element << std::endl;;
    }

    int raw_element(void) {
      return element;
    }

    uint64_t element;
};

class StringElement : public ArrayElement {
  public:
    StringElement(const char *e) {
      size_t len = strnlen(e, 1024);
      element = new char[len + 1];
      memset(element, 0, len + 1);
      strncpy(element, e, len);
    }

    void print(void) {
      std::cout << element << std::endl;
    }

    char *raw_element(void) {
      return element;
    }

    char *element;
};

#endif
