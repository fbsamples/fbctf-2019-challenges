#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <math.h>
#include <csignal>
#include <unistd.h>

#include "raddest_db_database.h"
#include "raddest_db.h"
#include "elements.h"

std::unordered_map<std::string, RaddestDBDatabase *> env;

void menu() {
  std::cout << "§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§" << std::endl ;
  std::cout << "                Raddest DB                " << std::endl;
  std::cout << "§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§" << std::endl ;
  std::cout << "§    Fast database for storing objects    §" << std::endl ;
  std::cout << "§                                         §" << std::endl ;
  std::cout << "§ Database Operations                     §" << std::endl ;
  std::cout << "§    create  db                           §" << std::endl ;
  std::cout << "§    destroy db                           §" << std::endl ;
  std::cout << "§    list    databases                    §" << std::endl ;
  std::cout << "§                                         §" << std::endl ;
  std::cout << "§ key/value operations                    §" << std::endl ;
  std::cout << "§    store   [db] [type] [key] [value]    §" << std::endl;
  std::cout << "§    store   db  string 2 aaa             §" << std::endl ;
  std::cout << "§    store   db  int 0 41                 §" << std::endl ;
  std::cout << "§    store   db  float 2 1.1              §" << std::endl ;
  std::cout << "§                                         §" << std::endl ;
  std::cout << "§    get     [db]  [key]                  §" << std::endl ;
  std::cout << "§    get     db  0                        §" << std::endl ;
  std::cout << "§                                         §" << std::endl ;
  std::cout << "§    delete  db  0                        §" << std::endl ;
  std::cout << "§    empty   db                           §" << std::endl ;
  std::cout << "§    print   db                           §" << std::endl ;
  std::cout << "§    echo    hellothere                   §" << std::endl ;
  std::cout << "§                                         §" << std::endl ;
  std::cout << "§ Add getters that are run after getting  §" << std::endl ;
  std::cout << "§ a key.                                  §" << std::endl ;
  std::cout << "§    getter [database] [key] [ops]        §" << std::endl;
  std::cout << "§                                         §" << std::endl ;
  std::cout << "§    >>> getter db 0 2                    §" << std::endl ;
  std::cout << "§    [getter]: echo test                  §" << std::endl ;
  std::cout << "§    [getter]: store string 2 aaaa        §" << std::endl ;
  std::cout << "§    >>> get db 0                         §" << std::endl ;
  std::cout << "§    42                                   §" << std::endl ;
  std::cout << "§    test                                 §" << std::endl ;
  std::cout << "§    >>>                                  §" << std::endl ;
  std::cout << "§                                         §" << std::endl ;
  std::cout << "§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§" << std::endl ;
}

std::vector<std::string> tokenize(const std::string & str) {

  std::vector<std::string> tokens;
  size_t s = 0;

  while (s < str.size()) {
    if (str.at(s) == ' ') 
      ++s;

    size_t t = s;

    while (t < str.size() && str.at(t) != ' ') {
      t++;
    }

    tokens.push_back(str.substr(s, t - s));

    s = t;
  }

  return tokens;
}


void eval(std::vector<std::string> & tokens) {
  if (tokens.size() < 1) return;

  std::string command = tokens[0];

  if (command == "create") {
    if (tokens.size() != 2) return;
    std::string db_name = tokens[1];

    env[db_name] = new RaddestDBDatabase();
  }

  if (command == "destroy") {
    if (tokens.size() != 2) return;
    std::string db_name = tokens[1];

    auto got = env.find(db_name);
    if ( got == env.end() ) {
      return;
    }

    delete got->second;
    env.erase(db_name);
  }

  if (command == "list") {
    if (tokens.size() != 2) return;
    std::string identifier = tokens[1];

    if (identifier == "databases") {
      for (auto it = env.begin(); it != env.end(); ++it) {
        std::cout << it->first << std::endl;
      }
    }
  }

  //get db string 0
  if (command == "get") {
    if (tokens.size() != 3) return;
    std::string db_name = tokens[1];
    uint32_t key = std::atoi(tokens[2].c_str());

    auto got = env.find(db_name);
    if ( got == env.end() ) {
      return;
    }

    RaddestDBDatabase *raddest_db = got->second;
    raddest_db->Get(key);
  }

  if (command == "store") {
    if (tokens.size() != 5) return;
    std::string db_name = tokens[1];
    std::string type = tokens[2];
    uint32_t key = std::atoi(tokens[3].c_str());
    std::string value = tokens[4];

    auto got = env.find(db_name);
    if ( got == env.end() ) {
      return;
    }

    RaddestDBDatabase *raddest_db = got->second;
    raddest_db->Set(type, key, value);
  }

  if (command == "delete") {
    if (tokens.size() != 3) return;
    std::string db_name = tokens[1];
    uint32_t key = std::atoi(tokens[2].c_str());

    auto got = env.find(db_name);
    if ( got == env.end() ) {
      return;
    }

    RaddestDBDatabase *raddest_db = got->second;
    raddest_db->Delete(key);
  }


  if (command == "print") {
    if (tokens.size() != 2) return;
    std::string db_name = tokens[1];

    auto got = env.find(db_name);
    if ( got == env.end() ) {
      return;
    }

    RaddestDBDatabase *raddest_db = got->second;
    raddest_db->Print();
  }

  if (command == "echo") {
    if (tokens.size() != 2) return;
    std::string str = tokens[1];

    Echo(str);
  }

  if (command == "empty") {
    if (tokens.size() != 2) return;
    std::string db_name = tokens[1];

    auto got = env.find(db_name);
    if ( got == env.end() ) {
      return;
    }

    RaddestDBDatabase *raddest_db = got->second;
    raddest_db->Empty();
  }

  if (command == "getter") {
    if (tokens.size() != 4) return;
    std::string db_name = tokens[1];
    uint32_t key = std::atoi(tokens[2].c_str());
    uint32_t count = std::atoi(tokens[3].c_str());

    auto got = env.find(db_name);
    if ( got == env.end() ) {
      return;
    }

    RaddestDBDatabase *raddest_db = got->second;

    getter_repl("[getter]: ", raddest_db, key, count);
  }
}

void getter_repl(const std::string & prompt, RaddestDBDatabase *raddest_db, uint32_t key, int count) {
  for (int i = 0; i < count; i++) {
    std::cout << prompt;
    std::string line;

    if (!std::getline(std::cin, line)) {
      return;
    }

    std::vector<std::string> tokens = tokenize(line);
    raddest_db->AddGetter(key, tokens);
  }
}

void repl(const std::string & prompt, std::function<void(std::vector<std::string> &)> callback) {
  for (;;) {
    std::cout << prompt;
    std::string line;

    if (!std::getline(std::cin, line)) {
      return;
    }

    std::vector<std::string> tokens = tokenize(line);
    try {
      callback(tokens);
    } catch(...) {
      std::cout << "runtime error" << std::endl;
    }
  }
}

void handler(int s) {
  std::cout << "Timeout" << std::endl;
  exit(0);
}

void setup() {
  menu();
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  alarm(90);
  signal(SIGALRM, handler);
  std::cin.precision(32);
  std::cout.precision(32);
}

int main(void) {
  setup();
  repl(">>> ", eval);
  std::cout << "bye." << std::endl;
}
