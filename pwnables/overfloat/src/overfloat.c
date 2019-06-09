#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>


/*
 * overflow - baby pwn
 */


void timeout(int s) {
	(void) s;
	puts("Too Slow! Sorry :(");
	exit(0);
}


void chart_course(float *coordinates) {
	float coordinate;
	char input_buffer[100];

	for (int i = 0 ;; i++) {
		if (i % 2 == 0) {
			printf("LAT[%d]: ", (i/2)%10);
		} else {
			printf("LON[%d]: ", (i/2)%10);
		}

		fgets(input_buffer, sizeof(input_buffer), stdin);
		if (!strncmp(input_buffer, "done", 4)) {
			if (i % 2 == 0) {
				break;
			} else {
				puts("WHERES THE LONGITUDE?");
				i--;
				continue;
			}
		}
		coordinate = atof(input_buffer);
		memset(input_buffer, 0, sizeof(input_buffer));
		coordinates[i] = coordinate;
	}
}


int main(int argc, char **argv) {
	(void) argc; (void) argv;
	float coordinates[10];
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);
	alarm(30);
	signal(SIGALRM, timeout);
	puts("                                 _ .--.        \n"
	     "                                ( `    )       \n"
	     "                             .-'      `--,     \n"
	     "                  _..----.. (             )`-. \n"
	     "                .'_|` _|` _|(  .__,           )\n"
	     "               /_|  _|  _|  _(        (_,  .-' \n"
	     "              ;|  _|  _|  _|  '-'__,--'`--'    \n"
	     "              | _|  _|  _|  _| |               \n"
	     "          _   ||  _|  _|  _|  _|               \n"
	     "        _( `--.\\_|  _|  _|  _|/               \n"
	     "     .-'       )--,|  _|  _|.`                 \n"
	     "    (__, (_      ) )_|  _| /                   \n"
	     "      `-.__.\\ _,--'\\|__|__/                  \n"
	     "                    ;____;                     \n"
	     "                     \\YT/                     \n"
	     "                      ||                       \n"
	     "                     |\"\"|                    \n"
	     "                     '=='                      \n"
		 "\n"
	     "WHERE WOULD YOU LIKE TO GO?");
	memset(coordinates, 0, sizeof(coordinates));
	chart_course((float*)coordinates);
	puts("BON VOYAGE!");
}
