#!/bin/bash
# 1) to compile with yasm for 32 bit binary
rm *.o
yasm -f elf hello.s
ld -m elf_i386 -s -o hello.out *.o


# 2) to compile with yasm for 64 bit binary
rm -f *.o
yasm -f elf64 hello.s
ld -m elf_x86_64 -s -o hello64.out  *.o


# 3) to use gcc for 32 bit
rm  -f  *.o
yasm  -f elf hello_gcc.s
gcc -m32  -o hello_gcc32.out  *.o


# 4) to use gcc for 64 bit
rm -f *.o
yasm -f elf64 hello_gcc.s
gcc -m64 -o hello_gcc64.out hello_gcc.o

