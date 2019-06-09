#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAX_MESSAGE 256
#define OTP_SIZE MAX_MESSAGE + 8 // message length + padding

FILE *urandom_fp;

char input_buffer[MAX_MESSAGE];
char otp_key[OTP_SIZE];

char encrypted_message_header[40] = "----- BEGIN ROP ENCRYPTED MESSAGE -----\n";
char encrypted_message_footer[39] = "\n----- END ROP ENCRYPTED MESSAGE -----\n";

void banner(void) {
  puts("Test our new OTP probocol: Randomly Over Padding");
  puts("Spec: cipher((4 byte nonce) | message | (4 byte nonce))\n");
  fflush(stdout);
}

// Sets up the randomness stream
void setup(void) {
  banner();
  urandom_fp = fopen("/dev/urandom", "r");

  if (!urandom_fp) {
    puts("/dev/urandom was not initialized - reach out to admin");
    exit(1);
  }
}

uint32_t gen_nonce(void) {
  uint32_t e = 0;

  if (!urandom_fp) {
    puts("/dev/urandom was not initialized - reach out to admin");
    exit(1);
  }

  fread(&e, sizeof(e), 1, urandom_fp);

  return e;
}

void get_key(void) {
  read(0, otp_key, sizeof(otp_key));
}

void get_user_message(void) {
  read(0, input_buffer, sizeof(input_buffer));
}

void output_encrypted_message(char *message, size_t length) {
  write(1, encrypted_message_header, sizeof(encrypted_message_header));
  write(1, message, length);
  write(1, encrypted_message_footer, sizeof(encrypted_message_footer));
}

void otp_cipher(char *buffer) {
  size_t i = 0;

  for (i = 0; i < OTP_SIZE; i++) {
    buffer[i] ^= otp_key[i];
  }
}

void rop_protocol(char *message) {
  size_t length = 0;  
  uint32_t entropy = 0;
  
  entropy = gen_nonce();

  memcpy(message, &entropy, sizeof(entropy));
  length = snprintf(message + sizeof(entropy), OTP_SIZE - sizeof(entropy), "%s", input_buffer);
  memcpy(message + sizeof(entropy) + length, &entropy, sizeof(entropy));

  otp_cipher(message);
  output_encrypted_message(message, length + sizeof(entropy) + sizeof(entropy));
}

void encryption_menu(char *message) {
  char input[4];
  char option = 0;
  memset(input, 0, sizeof(input)); 

  while (1) {
    puts("1. Set Key");
    puts("2. Encrypt message");
    puts("3. Exit");
    printf(">>> ");

    fflush(stdout);

    read(0, input, 4);
    option = input[0] - '0';

    if (option == 1) {
      puts("Enter key:");
      fflush(stdout);

      get_key();
    } else if (option == 2) {
      puts("Enter message to encrypt:");
      fflush(stdout);

      get_user_message();
      rop_protocol(message);
    } else {
      break;
    }
  }
}


void cleanup(void) {
  if (urandom_fp) 
    fclose(urandom_fp);
}

int main(void) {
  char buffer[OTP_SIZE];
  setup();

  encryption_menu(buffer);

  cleanup();
}
