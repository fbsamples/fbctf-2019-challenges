Category
---
Pwnable

Vulnerabilities
---
```c
void get_key(void) {
  read(0, otp_key, sizeof(otp_key)); <-----not null terminated
}

void get_user_message(void) {
  read(0, input_buffer, sizeof(input_buffer)); <----- not null terminated
}       
```

```c
  memcpy(message, &entropy, sizeof(entropy));
  length = snprintf(message + sizeof(entropy), OTP_SIZE - sizeof(entropy), "%s", input_buffer); <----
  memcpy(message + sizeof(entropy) + length, &entropy, sizeof(entropy));

  otp_cipher(message);
  output_encrypted_message(message, length + sizeof(entropy) + sizeof(entropy));
}

```

Memcpy uses the return value of snprintf as an offset to write to. Which leads to a 8 byte OOB write on the stack and a 256 byte OOB read on the stack. This can only be triggered if the length of the input buffer is 256 bytes then snprintf will read into otp key during snprintf.

TODO
---
xinted script and docker
