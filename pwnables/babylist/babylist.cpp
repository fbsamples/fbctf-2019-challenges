#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <csignal>

using namespace std;

#define NAME_LEN 0x70
#define LISTS_LEN 10

struct list {
  char name[NAME_LEN];
  vector<int> numbers;
};

struct list *lists[LISTS_LEN];

void handler(int s) {
  cout << "Timeout" << endl;
  exit(0);
}

void setup() {
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  alarm(140);
  signal(SIGALRM, handler);
}

void print_banner() {
  cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
  cout << "Welcome to babylist!" << endl;
  cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl << endl;
}

void print_menu() {
  cout << "1. Create a list" << endl;
  cout << "2. Add element to list" << endl;
  cout << "3. View element in list" << endl;
  cout << "4. Duplicate a list" << endl;
  cout << "5. Remove a list" << endl;
  cout << "6. Exit" << endl;
  cout << "> ";
}

int get_index() {
  int idx;
  cout << "Enter index of list:" << endl;
  cin >> idx;
  cin.ignore();

  if (idx < 0 || idx >= LISTS_LEN || !lists[idx]) {
    cout << "Error: Invalid index" << endl;
    exit(-1);
  }
  return idx;
}

void create() {
  int i;
  for (i = 0;i<LISTS_LEN;i++) {
    if (lists[i] == NULL) {
      break;
    }
  }
  if (i == LISTS_LEN) {
    cout << "Sorry, no empty spot available :(" << endl;
    return;
  }
  lists[i] = new struct list;
  cout << "Enter name for list:" << endl;
  cin.getline(lists[i]->name, NAME_LEN);
  cout << "List has been created!" << endl;
}

void add() {
  int number;
  int idx = get_index();
  cout << "Enter number to add:" << endl;
  cin >> number;
  cin.ignore();
  lists[idx]->numbers.push_back(number);
  cout << "Number successfully added to list!" << endl;
}

void view() {
  int numIdx;
  int idx = get_index();
  cout << "Enter index into list:" << endl;
  cin >> numIdx;
  cin.ignore();
  cout << lists[idx]->name << "[" << numIdx<< "] = " <<
    lists[idx]->numbers.at(numIdx) << endl;
}

void duplicate() {
  int i;
  for (i = 0;i<LISTS_LEN;i++) {
    if (lists[i] == NULL) {
      break;
    }
  }
  if (i == LISTS_LEN) {
    cout << "Sorry, no empty spot available :(" << endl;
    return;
  }
  int idx = get_index();
  lists[i] = new struct list;
  memcpy(lists[i], lists[idx], sizeof(struct list));
  cout << "Enter name for new list:" << endl;
  cin.getline(lists[i]->name, NAME_LEN);
  cout << "List has been duplicated!" << endl;
}

void remove() {
  int idx = get_index();
  memset(lists[idx], 0, sizeof(struct list));
  delete lists[idx];
  lists[idx] = 0;
  cout << "List has been deleted!" << endl;
}

int main() {
  int choice;
  setup();
  print_banner();
  while (1) {
    print_menu();
    cin >> choice;
    cin.ignore();
    switch (choice) {
    case 1:
      create();
      break;
    case 2:
      add();
      break;
    case 3:
      view();
      break;
    case 4:
      duplicate();
      break;
    case 5:
      remove();
      break;
    default:
      return 0;
    }
  }
  return 0;
}
