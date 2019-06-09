; WARNING: FOR MACOS ONLY
; nasm -f macho64 rot13.asm -o rot13.o && ld -o rot13.macho -macosx_version_min 10.7 -e start rot13.o && ./rot13.macho

BITS 64

section .text
global start

start:
  push   rbp
  mov    rbp,rsp

; PRINT OUTPUT
  push   0xa203a
  mov    DWORD [rsp+0x4],0x0
  push   0x76207965
  mov    DWORD [rsp+0x4],0x65756c61
  push   0x7475706e
  mov    DWORD [rsp+0x4],0x6b206120
  push   0x61656c50
  mov    DWORD [rsp+0x4],0x69206573
  mov    rax, 0x2000004
  mov    rdi, 0x1
  mov    rsi, rsp
  mov    edx, 0x1b
  syscall
  pop    rax
  pop    rax
  pop    rax
  pop    rax
  ; GET INPUT
  mov    rax, 0x2000003
  mov    rdi, 0
  lea    rsi, [rel key]
  mov    edx, 0x18
  syscall

; START DECRYPTION
  mov    r12, 0 ;              ; int i = 0;
  lea    r14, [rel key]
loop1:
  cmp    r12, 0x17             ; Length of Answer
  jge    verify_rot13
  movzx  ecx, BYTE [r14 + r12] ; c = Answer[i]
  cmp    ecx, 0x41             ; if c >='A'
  jl     goto_lowercase
  cmp    ecx, 0x5a             ; if c <='Z'
  jg     goto_lowercase
  add    ecx, 0xd              ; e = c + ROT
  cmp    ecx, 0x5a             ; if((e = c + ROT) <= 'Z')
  jg     greater_than
  mov    BYTE [r14 + r12], cl  ; Answer[i] = e;
  jmp    goto_increment1
greater_than:
  movzx  ecx, BYTE [r14 + r12]
  sub    ecx, 0xd              ; e = c - ROT
  mov    BYTE [r14 + r12], cl  ; Answer[i] = e;
goto_increment1:
  jmp    increment
goto_lowercase:
  movzx  ecx, BYTE [r14 + r12] ; c = Answer[i]
  cmp    ecx, 0x61 ; 'a'       ; if c >='a'
  jl     neither
  cmp    ecx, 0x7a             ; if c <='z'
  jg     neither
  mov    eax, ecx
  add    eax, 0xd              ; e = c + ROT
  cmp    eax, 0x7a             ; if((e= c + ROT) <= 'z')
  jg     greater_than2
  mov    cl, al
  mov    BYTE [r14+r12], cl    ; Answer[i] = e;
  jmp    goto_increment2
greater_than2:
  movzx  ecx, BYTE [r14 + r12]
  sub    ecx, 0xd              ; e = c - ROT
  mov    BYTE [r14 + r12], cl  ; Answer[i] = e;
goto_increment2:
  jmp    increment
neither:                       ; else
  movzx  ecx, BYTE [r14 + r12] ; c
  mov    BYTE [r14 + r12], cl  ; Answer[i] = c;
increment:
  inc    r12
  jmp    loop1

verify_rot13:
  xor    r8, r8                ; int isSame = 0;
  xor    r12, r12              ; int i = 0
  lea    r13, [rel ROT13]
loop2:
  cmp    r12, 0x17  ; i < 23
  jge    exit_loop
  movzx  ecx, BYTE [r14 + r12] ; Answer[i]
  movzx  edx, BYTE [r13 + r12] ; final[i]
  cmp    ecx, edx              ; if (final[i] == Answer[i])
  jne    not_equal
  mov    r8, 0x1               ; isSame = 1
  jmp    continue_loop
not_equal:
  mov    r8, 0x0               ; isSame = 0
  jmp    exit_loop             ; break;
continue_loop:
  inc    r12
  jmp    loop2
exit_loop:
  cmp    r8d, 0x1               ; if (isSame == 1)
  jne    fail
  mov    eax, 0x2000004 ; write
  mov    rdi, 1         ; std out
  lea    rsi, [rel msg]
  mov    edx, 21
  syscall
  xor    eax, eax
  mov    dx, WORD [r14]
  mov    ax, 0x354C    ; 0x4d34 ^ 0x354C = 0x7878 (offset of next shellcode)
  xor    ax, dx
  pop    rbp
  retn
fail:
  mov    rax, 0x2000004 ; write
  mov    rdi, 1         ; std out
  lea    rsi, [rel msg2]
  mov    edx, 230
  syscall
  xor    eax, eax
  mov    rax, 0x2000001 ; exit
  mov    rdi, 0
  syscall

section .data
key: times 0x20 db 0
msg: db "I believe in you ...", 10
msg2:
  db 32,32,32,32,32,70,65,73,76,32,87,72,65,76,69,33,10,10,87,32,32,32,32,32,87,32,32,32,32,32,32,87,32,32,32,32,32,32,32,32,10,87,32,32,32,32,32,32,32,32,87,32,32,87,32,32,32,32,32,87,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,39,46,32,32,87,32,32,32,32,32,32,10,32,32,46,45,34,34,45,46,95,32,32,32,32,32,92,32,92,46,45,45,124,32,32,10,32,47,32,32,32,32,32,32,32,34,45,46,46,95,95,41,32,46,45,39,32,32,32,10,124,32,32,32,32,32,95,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,32,10,92,39,45,46,95,95,44,32,32,32,46,95,95,46,44,39,32,32,32,32,32,32,32,10,32,96,39,45,45,45,45,39,46,95,92,45,45,39,32,32,32,32,32,32,10,86,86,86,86,86,86,86,86,86,86,86,86,86,86,86,86,86,86,86,86,86, 10
ROT13: db "4ZberYriryf2TbXrrcTbvat", 10
