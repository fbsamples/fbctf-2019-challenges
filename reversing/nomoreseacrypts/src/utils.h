#pragma once
#include <cstdio>
#include <pwd.h>
#include <unistd.h>
#include <string>
#include <algorithm>

using namespace std;

const string get_current_username();

const string get_home_directory();

bool ends_with(std::string const &value, std::string const &ending);

string random_string(size_t length);

void hexdump(char *s, uint8_t *buf, size_t sz);
