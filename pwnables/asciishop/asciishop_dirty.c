// Pixel Art

#include <assert.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>
#include <ctype.h>

// Stamp constants
#define __AUTHOR__ "tisx5ak"

// input length constants
#define ID_LENGTH 8
#define IO_LENGTH 64

// custom allocator constants
#define POOL_MAX_SIZE 256
#define INIT_POOL_COUNT 2
#define MAX_POOL_COUNT 32
#define CHUNKS_PER_POOL 16


// image Constants
#define MAX_W 32
#define MAX_H 32
#define MAX_LENGTH (MAX_W * MAX_H)


#define EFAIL (0x1)


struct image_header_t {
  char magic[4];
  int width;
  int height;
  int offset;
};

struct image_t {
  struct image_header_t header;
  uint8_t ascii[MAX_W][MAX_H];
};

struct active_chunk_t {
  char id[ID_LENGTH];
  uint8_t in_use;
  struct image_t image;
};

struct image_pool_t {
  int refcnt;
  struct active_chunk_t *pool;
};

struct image_pool_t data_0x46eaf0[MAX_POOL_COUNT];

struct coordinates_t {
  int x;
  int y;
  char c;
};

int data_0x46eae0 = 0;
char data_0x46ecf0[IO_LENGTH];
struct coordinates_t sub_46ed30;


////////////////////////////////////////
// Basic prompts
////////////////////////////////////////

int sub_12180(void *buf, size_t buf_len) {
  while (buf_len) {
    int retval = fread(buf, 1, buf_len, stdin);

    if (retval < 0)
      return -EFAIL;

    buf += retval;
    buf_len -= retval;
  }

  return buf_len;
}

int sub_12210(void *buf, size_t buf_len) {
  while (buf_len) {
    int retval = fwrite(buf, 1, buf_len, stdout);

    if (retval == 0)
      return -EFAIL;

    if (retval < 0)
      return -EFAIL;

    buf += retval;
    buf_len -= retval;
  }

  return buf_len;
}

int sub_122c0(char *buf, size_t buf_len) {
  if (!fgets(buf, buf_len, stdin)) {
    return -EFAIL;
  }

  buf[strcspn(buf, "\n")] = 0;

  return 1;
}

int sub_12340(void) {
  printf("Ascii id: ");
  memset(data_0x46ecf0, 0, IO_LENGTH);
  return sub_122c0(data_0x46ecf0, ID_LENGTH);
}

int sub_12390(void) {
  puts("(X, Y) char");
  printf("pixel: ");
  memset(data_0x46ecf0, 0, IO_LENGTH);
  memset(&sub_46ed30, 0, sizeof(struct coordinates_t));

  return sub_122c0(data_0x46ecf0, IO_LENGTH);
}

int sub_12420(void) {
  memset(data_0x46ecf0, 0, IO_LENGTH);

  return sub_122c0(data_0x46ecf0, IO_LENGTH);
}

////////////////////////////////////////
// Pool allocator for images
////////////////////////////////////////

void sub_12460(void) {
  for (data_0x46eae0 = 0; data_0x46eae0 < INIT_POOL_COUNT; data_0x46eae0++) {
    data_0x46eaf0[data_0x46eae0].refcnt = 0;
    data_0x46eaf0[data_0x46eae0].pool = mmap(NULL, sizeof(struct active_chunk_t) * CHUNKS_PER_POOL, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  }
}

int sub_12510(void) {
  if (data_0x46eae0 >= MAX_POOL_COUNT) {
    return -EFAIL;
  }

  data_0x46eaf0[data_0x46eae0].refcnt = 0;
  data_0x46eaf0[data_0x46eae0].pool = mmap(NULL, sizeof(struct active_chunk_t) * CHUNKS_PER_POOL, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);;

  return data_0x46eae0++;
}

int sub_125b0(struct image_pool_t *pool) {
  return pool->refcnt >= CHUNKS_PER_POOL;
}

int sub_125d0(void) {
  int i = 0;

  for (i = 0; i < data_0x46eae0; i++) {
    if (!sub_125b0(&data_0x46eaf0[i])) {
      return i;
    }
  }

  return -EFAIL;
}

struct active_chunk_t *sub_12650(int pool_index) {
  int i = 0;
  struct active_chunk_t *head = NULL;
  struct active_chunk_t *current = NULL;

  head = data_0x46eaf0[pool_index].pool;
  for (i = 0; i < CHUNKS_PER_POOL; i++) {
    current = head + i;

    if (current->in_use == 0) {
      return current;
    }
  }

  return NULL;
}

struct active_chunk_t *sub_126f0(void) {
  int empty_pool_index = 0;
  struct active_chunk_t *new_chunk = NULL;

  empty_pool_index = sub_125d0();

  if (empty_pool_index == -EFAIL) {
    empty_pool_index = sub_12510();

    if (empty_pool_index == -EFAIL)  {
      puts("allocator OOM: no pools left");
      return NULL;
    }
  }

  new_chunk = sub_12650(empty_pool_index);

  if (new_chunk == NULL) {
    puts("allocator OOM: no chunks left");
    return NULL;
  }

  memset(new_chunk, 0, sizeof(struct active_chunk_t));
  new_chunk->in_use = 1;
  data_0x46eaf0[empty_pool_index].refcnt += 1;

  return new_chunk;
}


struct active_chunk_t *sub_127d0(char *id, int *pool_index) {
  int i = 0, j = 0;
  struct active_chunk_t *head = NULL;
  struct active_chunk_t *current = NULL;

  for (i = 0; i < data_0x46eae0; i++) {
    head = data_0x46eaf0[i].pool;

    for (j = 0; j < CHUNKS_PER_POOL; j++) {
      current = head + j;

      if (strncmp(current->id, id, ID_LENGTH) == 0) {

        if (pool_index != NULL) {
          *pool_index = i;
        }

        return current;
      }
    }
  }

  return NULL;
}


void sub_128d0(struct active_chunk_t *doomed_chunk) {
  memset(doomed_chunk, 0, sizeof(struct active_chunk_t));
}

int sub_12900(char *id) {
  struct active_chunk_t *doomed_chunk = NULL;
  int pool_index = 0;

  doomed_chunk = sub_127d0(id, &pool_index);

  if (doomed_chunk == NULL)
    return -EFAIL;

  sub_128d0(doomed_chunk);
  data_0x46eaf0[pool_index].refcnt -= 1;

  return pool_index;
}


////////////////////////////////////////
// Validation
////////////////////////////////////////

int sub_129b0(struct image_header_t *image_header) {
  if (memcmp(image_header->magic, "ASCI", 4) != 0) {
    return -EFAIL;
  }

  if (image_header->width < 0)
    image_header->width = -image_header->width;

  if (image_header->height < 0)
    image_header->height = -image_header->height;

  if (image_header->offset < 0)
    image_header->offset = -image_header->offset;

  if (image_header->width > MAX_W) return -EFAIL;
  if (image_header->height > MAX_H) return -EFAIL;
  if (image_header->offset > MAX_LENGTH) return -EFAIL;

  return 1;
}


////////////////////////////////////////
// Prompts
////////////////////////////////////////

struct active_chunk_t *sub_12aa0(void) {
  int retval = 0;
  struct active_chunk_t *active_chunk = NULL;

  retval = sub_12340();

  if (retval == -EFAIL) {
    puts("no bytes read");
    return NULL;
  }

  active_chunk = sub_127d0(data_0x46ecf0, NULL);

  if (active_chunk == NULL) {
    printf("no image found with id %s\n", data_0x46ecf0);
    return NULL;
  }

  return active_chunk;
}


////////////////////////////////////////
// Chunk Manipulation
////////////////////////////////////////

struct image_header_t *sub_12b40(struct active_chunk_t *active_chunk) {
  return &active_chunk->image.header;
}


uint8_t *sub_12b60(struct active_chunk_t *active_chunk) {
  return (uint8_t *) &active_chunk->image.ascii;
}


////////////////////////////////////////
// Handle uploads, downloads, deletes
////////////////////////////////////////

void sub_12b80(void) {
  struct image_header_t *header = NULL;
  struct active_chunk_t *active_chunk = NULL;
  int retval = 0;

  active_chunk = sub_126f0();
  if (active_chunk == NULL)
    return;

  printf("Ascii id: ");
  sub_122c0(active_chunk->id, ID_LENGTH);

  puts("Upload ascii");

  retval = sub_12180(&active_chunk->image, sizeof(struct image_t));

  if (retval == -EFAIL) {
    puts("Invalid image");
    sub_128d0(active_chunk);
  }

  header = sub_12b40(active_chunk);
  retval = sub_129b0(header);

  if (retval == -EFAIL) {
    puts("Invalid image");
    sub_128d0(active_chunk);
  }
}

void sub_12c70(void) {
  struct active_chunk_t *active_chunk = NULL;
  struct image_header_t *header = NULL;
  uint8_t *ascii = NULL;

  active_chunk = sub_12aa0();

  if (active_chunk == NULL) {
    return;
  }

  ascii  = sub_12b60(active_chunk);
  header = sub_12b40(active_chunk);

  sub_12210(ascii, header->width * header->height);
  sub_12210("<<<EOF\n", 7);
}

void sub_12d00(void) {
  int retval = 0;

  retval = sub_12340();

  if (retval == -EFAIL) {
    printf("no image found with id %s\n", data_0x46ecf0);
    return;
  }

  retval = sub_12900(data_0x46ecf0);

  if (retval == -EFAIL) {
    printf("no image found with id %s\n", data_0x46ecf0);
    return;
  }

  printf("Ascii Image %s was successfully deleted\n", data_0x46ecf0);
}


////////////////////////////////////////
// handle asciishop operations
////////////////////////////////////////

void sub_12da0(void) {
  int retval = 0;
  int i = 0;
  struct active_chunk_t *active_chunk = 0;
  uint8_t *ascii = NULL;
  char ascii_filter[1024];

  active_chunk = sub_12aa0();

  if (active_chunk == NULL) {
    return;
  }

  puts("Upload filter");

  retval = sub_12180(ascii_filter, 1024);

  if (retval == -EFAIL) {
    puts("no bytes read");
    return;
  }

  ascii = sub_12b60(active_chunk);

  for (i = 0; i < retval; i++) {
    ascii[i] |= ascii_filter[i];
  }
}

/*

 * Grid looks like this
 * Prints X when the pixel is OOB

* 4x3 Photo example
id: foobar

4| X X X X X
3| D D D D D
2| C C C C C
1| B B B B B
0| A A A A A
XX _ _ _ _ _
XX 0 1 2 3 4


[
00         02
  [1, 2, 3],
  [4, 5, 6],
  [7, 8, 9],
  [a, b, c],
  [d, e, f],
40         43
]

4  d e f X X
3  a b c X X
2  7 8 9 X X
1  4 5 6 X X
0  1 2 3 X X
XX _ _ _ _ _
XX 0 1 2 3 4

QQQQ
*/
void sub_12eb0(struct active_chunk_t *active_chunk) {
  int x, y, position = 0;
  char c = 0;
  struct image_t *image = NULL;
  struct image_header_t *header = NULL;
  uint8_t *ascii = NULL;

  ascii = sub_12b60(active_chunk);
  header = sub_12b40(active_chunk);

  for (y = MAX_H - 1; y >= 0; y--) {
    for (x = 0; x < MAX_W; x++) {
      position = header->offset + (x + (y * header->width));

      if (x == 0) {
        printf("%2d |", y);
      }

      c = ascii[position & 0xffff];

      if (x >= header->width) {
        printf("   ");
      } else {
        if (c == 0) {
          printf("   ");
        } else if (isalnum(c)) {
          printf(" %2c", c);
        } else {
          printf(" %2x", c);
        }
      }

      if (x == MAX_W - 1) {
        printf("\n");
      }
    }
  }

  printf("     ");

  for (x = 0; x < MAX_W; x++) {
    printf("__ ");
  }

  printf("\n     ");

  for (x = 0; x < MAX_W; x++) {
    printf("%2d ", x);
  }
  puts("");
}

/*
Vulnerability
X: 0
Y: 0
New Character: Q

if (x is negative) fail
if (y is negative) fail

if positive
  position = add to offset

if position < (image->width * image->height):
  Negative OOB write

RRR
*/

void sub_130f0(struct active_chunk_t *active_chunk) {
  int retval = 0, position = 0;

  struct image_header_t *header = NULL;
  uint8_t *ascii = NULL;


  ascii = sub_12b60(active_chunk);
  header = sub_12b40(active_chunk);

  retval = sub_12390();

  if (retval == -EFAIL) {
    return;
  }

  retval = sscanf(data_0x46ecf0, "(%d, %d) %c", &sub_46ed30.x, &sub_46ed30.y, &sub_46ed30.c);

  if (retval != 3) {
    return;
  }

  if (sub_46ed30.x < 0) {
    printf("%d is invalid for x\n", sub_46ed30.x);
    return;
  }

  if (sub_46ed30.y < 0) {
    printf("%d is invalid for y\n", sub_46ed30.y);
    return;
  }

  position = sub_46ed30.y * header->width;

  if (sub_46ed30.y != position / header->width)  {
    puts("integer overflow detected!");
    return;
  }

  if (sub_46ed30.x + position < sub_46ed30.x) {
    puts("integer overflow detected!");
    return;
  }

  position += sub_46ed30.x;

  if (header->offset + position < header->offset) {
    puts("integer overflow detected!");
    return;
  }

  position += header->offset;

  if (position >= header->width * header->height) {
    printf("index %d out of bounds!\n", position);
    return;
  }

  ascii[position & 0xffff] = sub_46ed30.c;
}

void sub_13300(void) {
  char option = 0;
  int retval = 0;
  struct active_chunk_t *active_chunk = NULL;

  retval = sub_12340();

  if (retval == -EFAIL) {
    puts("no bytes read");
    return;
  }

  active_chunk = sub_127d0(data_0x46ecf0, NULL);

  if (active_chunk == NULL) {
    printf("no image found with id %s\n", data_0x46ecf0);
    return;
  }


  while (1) {

    puts("1. Change Pixel");
    puts("2. Print Grid");
    puts("3. Help");
    puts("4. back");
    printf(">>> ");

    retval = sub_12420();

    if (retval == -EFAIL) {
      puts("no bytes read");
      return;
    }

    option = data_0x46ecf0[0] - '0';

    if (option == 1) {
      sub_130f0(active_chunk);
    } else if (option == 2) {
      sub_12eb0(active_chunk);
    } else if (option == 3) {
      puts("(X, Y) char\nExample: (16, 10) A");
      continue;
    } else if (option == 4) {
      break;
    } else {
      puts("Invalid input.");
    }
  }
}


void sub_134a0(void) {
  char option = 0;

  while(1) {
    puts("1. Touchup Ascii Art");
    puts("2. Add filter to Ascii Art");
    puts("3. Help");
    puts("4. back");
    printf(">>> ");

    sub_12420();
    option = data_0x46ecf0[0] - '0';

    if (option == 1) {
      sub_13300();
    } else if (option == 2) {
      sub_12da0();
    } else if (option == 3) {
      puts("Welcome to AsciiShop");
      puts("\tTouchup your photo with option 1");
      puts("\tAdd a filter to your photo with option 2");
    } else if (option == 4) {
      break;
    } else {
      puts("Invalid option.");
    }
  }
}

////////////////////////////////////////
// Setup and main
////////////////////////////////////////

void sub_135c0(void) {
  puts("++++++++++++++++++++++++++++++++++++++++++");
  puts("+WWWWWWNNNXkc,'..   ....   ....,,;;;oKWWW+");
  puts("+WWWWWNNXx;...',,'.......      ..,;;;oXWW+");
  puts("+WWWWNWXl.     ................. ..,:ckNW+");
  puts("+WWWWWNo.                ............:kNW+");
  puts("+WWWWWK;                 .............lKW+");
  puts("+NXXXKO;      .......'',,;;;.......''.'xW+");
  puts("+K000Ok;    ..';;;,;looddddo:',,..':c'.;K+");
  puts("+KOOOkd:. .......';codxkxdodlldd,..;ll,,O+");
  puts("+XOO0kxd'.',.......;oxo;'..,:cdd:. .'ccc0+");
  puts("+Kxdkolo;.;c:;;:c:;lO0xc;,';cloxl.  .;cxN+");
  puts("+XkxOkdx:.;lddddolcxKK0kxddxkO00d..,loONW+");
  puts("+NXXXXXXk;':ldxxdlo0XXXK0000000Od,;k0KNWW+");
  puts("+NXXXKKOd'.;ldxxdcoO00K0O0K0OOOko,lOkOXWW+");
  puts("+odkkxdo, .,cdddl:;codx000000OOk;..:xKNNW+");
  puts("+:odddol;...:olc;,:ldxkOOO00OOOx:,,;;cxXN+");
  puts("+..;ldoll;..';c:'',:ccldxxkOOOkocll:,',l0+");
  puts("+.  :xdolc'...,;;,,:cloxkkkkkxc,.':c;,''c+");
  puts("+   ;xdoll;....,cccldxxOOkxdxd,..';;;;'.'+");
  puts("+   :kxdlc;..'..'',;:ccllodkxl,..,:,;;...+");
  puts("+   ;l:,...  .'...     .'o0k:;,'.,l,',...+");
  puts("++                                      ++");
  puts("++++++++++++++++++++++++++++++++++++++++++");
  puts("++++++++++      AsciiShop       ++++++++++");
  puts("++++++++++++++++++++++++++++++++++++++++++");
  puts("++++++++++   Reimagine Ascii.   ++++++++++");
}

void sub_13760(void) {
  alarm(60);
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  sub_135c0();
  sub_12460();
}


int main(void) {
  char option = 0;

  sub_13760();

  while(1) {
    puts("1. Upload ascii art");
    puts("2. Download ascii art");
    puts("3. Delete ascii art");
    puts("4. Asciishop");
    puts("5. Exit");
    printf(">>> ");

    sub_12420();
    option = data_0x46ecf0[0] - '0';

    if (option == 1) {
      sub_12b80();
    } else if (option == 2) {
      sub_12c70();
    } else if (option == 3) {
      sub_12d00();
    } else if (option == 4) {
      sub_134a0();
    } else if (option == 5) {
      break;
    } else {
      puts("Invalid option.");
    }
  }
}
