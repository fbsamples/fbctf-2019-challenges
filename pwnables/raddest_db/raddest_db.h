#ifndef ARRAY_DB
#define ARRAY_DB

#include "raddest_db_database.h"

void repl(const std::string & prompt, std::function<void(std::vector<std::string> &)> callback);
void getter_repl(const std::string & prompt, RaddestDBDatabase *raddest_db, uint32_t key, int count);
#endif
