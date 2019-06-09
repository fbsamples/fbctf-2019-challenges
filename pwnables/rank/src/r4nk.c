#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>  
#include <sys/sendfile.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>

//  gcc -O2 -D_FORTIFY_SOURCE=2 -no-pie r4nk.c -o r4nk

#define BUFLEN 128
#define TITLES_LEN 12
#define WHERE "\nt1tl3> "
#define WHAT "r4nk> "

char BUF[BUFLEN];
const char *TITLES[] = {
	"Hackers",
	"Get Shorty",
	"Iron Man",
	"Mad Max",
	"Return of the Jedi",
	"Happy Gilmore",
	"The Boondock Saints",
	"The Net",
	"Sneakers",
	"Short Circuit",
	"Real Genious",
	"The Princess Bride",
};


int geti(){
	char *endptr;
	int i;
	read(STDIN_FILENO, &BUF, BUFLEN);
	i = strtol(BUF, &endptr, 0);
	if (endptr == BUF) {
		dprintf(1, "invalid number\n");
	}
	return i;
}


void print_ranks(long ranks[]){
	int i = 0;
	dprintf(1, "[");
	for (i=0; i < TITLES_LEN; i++){
		dprintf(1, "%li,", ranks[i]);
	}
	dprintf(1,"]");
}


int prompt(char *msg, int len){
	write(STDOUT_FILENO, msg, len);
	return geti();
}



void rank_menu(long ranks[]) {
	int index;
	int value;
	index = prompt(WHERE, sizeof(WHERE));
	value = prompt(WHAT, sizeof(WHAT));
	ranks[index] = value;
}


void print_titles(long ranks[]){
	int i=0;
	long rank = 0;
	for (i=0; i < TITLES_LEN; i++){
		rank = ranks[i];
		dprintf(1, "%i. %s\n", i, TITLES[rank]);
	}

}


int main_menu(){
	int quit = 0;
	int choice = 0;
	long ranks[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};
	
	while (!quit) {	
		dprintf(1, "\n1. Show\n2. Rank\n3. Quit\n> ");
		choice = geti();
		if (choice == 1){
			print_titles(ranks);
		} 
		else if (choice == 2) {
			rank_menu(ranks);
		}
		else if (choice == 3) {
			quit = 1;
			dprintf(1, "g00dBy3\n");
		}
		else {
			dprintf(1, "N1ce try n00b\n");
		}
	}

}

void handler(int s) {
  dprintf(1, "Timeout\n");
  exit(0);
}

int main(int argc, char *argv[]){
  alarm(30);
  signal(SIGALRM, handler);
	dprintf(1, "wE1c0m3 t0 tItLe R4nK\n=====================\n");
	main_menu();
	return 0;
}



