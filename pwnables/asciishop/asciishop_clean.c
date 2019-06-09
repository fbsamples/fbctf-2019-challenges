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
#define MAX_POOL_COUNT 256
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

// Allocator

struct active_chunk_t {
  char id[ID_LENGTH];
  uint8_t in_use;
  struct image_t image;
};


struct image_pool_t {
  int refcnt;
  struct active_chunk_t *pool;
};

struct image_pool_t image_pools[MAX_POOL_COUNT];

struct coordinates_t {
  int x;
  int y;
  char c;
};

int pool_count = 0;
char io_buffer[IO_LENGTH];
struct coordinates_t coordinates;


////////////////////////////////////////
// Basic prompts
////////////////////////////////////////

int readn(void *buf, size_t buf_len) {
  while (buf_len) {
    int retval = fread(buf, 1, buf_len, stdin);

    if (retval < 0)
      return -EFAIL;

    buf += retval;
    buf_len -= retval;
  }

  return buf_len;
}

int writen(void *buf, size_t buf_len) {
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

int read_line(char *buf, size_t buf_len) {
  if (!fgets(buf, buf_len, stdin)) {
    return -EFAIL;
  }

  buf[strcspn(buf, "\n")] = 0;

  return 1;
}

int read_id(void) {
  printf("Ascii id: ");
  memset(io_buffer, 0, IO_LENGTH);
  return read_line(io_buffer, ID_LENGTH);
}

int read_coordinates(void) {
  puts("(X, Y) char");
  printf("pixel: ");
  memset(io_buffer, 0, IO_LENGTH);
  memset(&coordinates, 0, sizeof(struct coordinates_t));

  return read_line(io_buffer, IO_LENGTH);
}

int read_input(void) {
  memset(io_buffer, 0, IO_LENGTH);

  return read_line(io_buffer, IO_LENGTH);
}

////////////////////////////////////////
// Pool allocator for images
////////////////////////////////////////

void init_pools(void) {
  for (pool_count = 0; pool_count < INIT_POOL_COUNT; pool_count++) {
    image_pools[pool_count].refcnt = 0;
    image_pools[pool_count].pool = mmap(NULL, sizeof(struct active_chunk_t) * CHUNKS_PER_POOL, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  }
}

int create_new_pool(void) {
  if (pool_count >= MAX_POOL_COUNT) {
    return -EFAIL;
  }

  image_pools[pool_count].refcnt = 0;
  image_pools[pool_count].pool = mmap(NULL, sizeof(struct active_chunk_t) * CHUNKS_PER_POOL, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);;

  return pool_count++;
}

int is_pool_full(struct image_pool_t *pool) {
  return pool->refcnt >= CHUNKS_PER_POOL;
}

int find_available_pool(void) {
  int i = 0;

  for (i = 0; i < pool_count; i++) {
    if (!is_pool_full(&image_pools[i])) {
      return i;
    }
  }

  return -EFAIL;
}

struct active_chunk_t *find_free_chunk_in_pool(int pool_index) {
  int i = 0;
  struct active_chunk_t *head = NULL;
  struct active_chunk_t *current = NULL;

  head = image_pools[pool_index].pool;
  for (i = 0; i < CHUNKS_PER_POOL; i++) {
    current = head + i;

    if (current->in_use == 0) {
      return current;
    }
  }

  return NULL;
}

struct active_chunk_t *allocate_chunk(void) {
  int empty_pool_index = 0;
  struct active_chunk_t *new_chunk = NULL;

  empty_pool_index = find_available_pool();

  if (empty_pool_index == -EFAIL) {
    empty_pool_index = create_new_pool();

    if (empty_pool_index == -EFAIL)  {
      puts("allocator OOM: no pools left");
      return NULL;
    }
  }

  new_chunk = find_free_chunk_in_pool(empty_pool_index);

  if (new_chunk == NULL) {
    puts("allocator OOM: no chunks left");
    return NULL;
  }

  memset(new_chunk, 0, sizeof(struct active_chunk_t));
  new_chunk->in_use = 1;
  image_pools[empty_pool_index].refcnt += 1;

  return new_chunk;
}


struct active_chunk_t *find_chunk_by_id(char *id, int *pool_index) {
  int i, j;
  struct active_chunk_t *head;
  struct active_chunk_t *current;

  for (i = 0; i < pool_count; i++) {
    head = image_pools[i].pool;

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


void deallocate_ascii_chunk(struct active_chunk_t *doomed_chunk) {
  memset(doomed_chunk, 0, sizeof(struct active_chunk_t));
}


void deallocate_chunk(struct active_chunk_t *doomed_chunk) {
  memset(doomed_chunk, 0, sizeof(struct active_chunk_t));
}

int deallocate_chunk_by_id(char *id) {
  struct active_chunk_t *doomed_chunk;
  int pool_index = 0;

  doomed_chunk = find_chunk_by_id(id, &pool_index);

  if (doomed_chunk == NULL)
    return -EFAIL;

  deallocate_chunk(doomed_chunk);
  image_pools[pool_index].refcnt -= 1;

  return pool_index;
}


////////////////////////////////////////
// Validation
////////////////////////////////////////

int validate_ascii_header(struct image_header_t *image_header) {
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

struct active_chunk_t *prompt_user_for_image(void) {
  int retval = 0;
  struct active_chunk_t *active_chunk = NULL;

  retval = read_id();

  if (retval == -EFAIL) {
    puts("no bytes read");
    return NULL;
  }

  active_chunk = find_chunk_by_id(io_buffer, NULL);

  if (active_chunk == NULL) {
    printf("no image found with id %s\n", io_buffer);
    return NULL;
  }

  return active_chunk;
}


////////////////////////////////////////
// Chunk Manipulation
////////////////////////////////////////

struct image_header_t *get_chunk_header(struct active_chunk_t *active_chunk) {
  return &active_chunk->image.header;
}


uint8_t *get_chunk_ascii(struct active_chunk_t *active_chunk) {
  return (uint8_t *) &active_chunk->image.ascii;
}


////////////////////////////////////////
// Handle uploads, downloads, deletes
////////////////////////////////////////

void handle_upload_ascii(void) {
  struct image_header_t *header = NULL;
  struct active_chunk_t *active_chunk = NULL;
  int retval;

  active_chunk = allocate_chunk();
  if (active_chunk == NULL)
    return;

  printf("Ascii id: ");
  read_line(active_chunk->id, ID_LENGTH);

  puts("Upload ascii");

  retval = readn(&active_chunk->image, sizeof(struct image_t));

  if (retval == -EFAIL) {
    puts("Invalid image");
    deallocate_chunk(active_chunk);
  }

  header = get_chunk_header(active_chunk);
  retval = validate_ascii_header(header);

  if (retval == -EFAIL) {
    puts("Invalid image");
    deallocate_chunk(active_chunk);
  }
}

void handle_download_ascii(void) {
  struct active_chunk_t *active_chunk;
  struct image_header_t *header;
  uint8_t *ascii;

  active_chunk = prompt_user_for_image();

  if (active_chunk == NULL) {
    return;
  }

  ascii  = get_chunk_ascii(active_chunk);
  header = get_chunk_header(active_chunk);

  writen(ascii, header->width * header->height);
  writen("<<<EOF\n", 7);
}

void handle_delete_ascii(void) {
  int retval = 0;

  retval = read_id();

  if (retval == -EFAIL) {
    printf("no image found with id %s\n", io_buffer);
    return;
  }

  retval = deallocate_chunk_by_id(io_buffer);

  if (retval == -EFAIL) {
    printf("no image found with id %s\n", io_buffer);
    return;
  }

  printf("Ascii Image %s was successfully deleted\n", io_buffer);
}


////////////////////////////////////////
// handle asciishop operations
////////////////////////////////////////

void handle_asciishop_filter(void) {
  int retval = 0;
  int i = 0;
  struct active_chunk_t *active_chunk = 0;
  uint8_t *ascii;
  char ascii_filter[1024];

  active_chunk = prompt_user_for_image();

  if (active_chunk == NULL) {
    return;
  }

  puts("Upload filter");

  retval = readn(ascii_filter, 1024);

  if (retval == -EFAIL) {
    puts("no bytes read");
    return;
  }

  ascii = get_chunk_ascii(active_chunk);

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
void print_ascii_grid(struct active_chunk_t *active_chunk) {
  int x, y, position = 0;
  char c;
  struct image_t *image;
  struct image_header_t *header;
  uint8_t *ascii;

  ascii = get_chunk_ascii(active_chunk);
  header = get_chunk_header(active_chunk);

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

void touchup_change_byte(struct active_chunk_t *active_chunk) {
  int retval = 0, position = 0;

  struct image_header_t *header = NULL;
  uint8_t *ascii = NULL;


  ascii = get_chunk_ascii(active_chunk);
  header = get_chunk_header(active_chunk);

  retval = read_coordinates();

  if (retval == -EFAIL) {
    return;
  }

  retval = sscanf(io_buffer, "(%d, %d) %c", &coordinates.x, &coordinates.y, &coordinates.c);

  if (retval != 3) {
    return;
  }

  if (coordinates.x < 0) {
    printf("%d is invalid for x\n", coordinates.x);
    return;
  }

  if (coordinates.y < 0) {
    printf("%d is invalid for y\n", coordinates.y);
    return;
  }

  if (coordinates.y * header->width < coordinates.y) {
    puts("integer overflow detected!");
    return;
  }

  position = coordinates.y * header->width;

  if (coordinates.x + position < coordinates.x) {
    puts("integer overflow detected!");
    return;
  }

  position += coordinates.x;

  if (header->offset + position < header->offset) {
    puts("integer overflow detected!");
    return;
  }

  position += header->offset;

  if (position >= header->width * header->height) {
    printf("index %d out of bounds!\n", position);
    return;
  }

  ascii[position & 0xffff] = coordinates.c;
}

void handle_asciishop_touchup(void) {
  char option = 0;
  int retval = 0;
  struct active_chunk_t *active_chunk = NULL;

  retval = read_id();

  if (retval == -EFAIL) {
    puts("no bytes read");
    return;
  }

  active_chunk = find_chunk_by_id(io_buffer, NULL);

  if (active_chunk == NULL) {
    printf("no image found with id %s\n", io_buffer);
    return;
  }


  while (1) {

    puts("1. Change Pixel");
    puts("2. Print Grid");
    puts("3. Help");
    puts("4. back");
    printf(">>> ");

    retval = read_input();

    if (retval == -EFAIL) {
      puts("no bytes read");
      return;
    }

    option = io_buffer[0] - '0';

    if (option == 1) {
      touchup_change_byte(active_chunk);
    } else if (option == 2) {
      print_ascii_grid(active_chunk);
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


void asciishop_menu(void) {
  char option = 0;

  while(1) {
    puts("1. Touchup Ascii Art");
    puts("2. Add filter to Ascii Art");
    puts("3. Help");
    puts("4. back");
    printf(">>> ");

    read_input();
    option = io_buffer[0] - '0';

    if (option == 1) {
      handle_asciishop_touchup();
    } else if (option == 2) {
      handle_asciishop_filter();
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

void banner(void) {
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

void setup(void) {
  alarm(60);
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  banner();
  init_pools();
}


int main(void) {
  char option = 0;

  setup();

  while(1) {
    puts("1. Upload ascii art");
    puts("2. Download ascii art");
    puts("3. Delete ascii art");
    puts("4. Asciishop");
    puts("5. Exit");
    printf(">>> ");

    read_input();
    option = io_buffer[0] - '0';

    if (option == 1) {
      handle_upload_ascii();
    } else if (option == 2) {
      handle_download_ascii();
    } else if (option == 3) {
      handle_delete_ascii();
    } else if (option == 4) {
      asciishop_menu();
    } else if (option == 5) {
      break;
    } else {
      puts("Invalid option.");
    }
  }
}
