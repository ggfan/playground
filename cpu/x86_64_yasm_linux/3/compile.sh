#!/bin/bash
rm *.o
yasm -f elf -o add.o  add_with_subfunc.s
gcc -m32 -c main.c -o main.o

gcc -m32 -o test.out   *.o


# compile directly with ld
# There are more into this than just providing those options: does not work yet
#echo   "building with ld..."
#rm *.o
#yasm -f elf -o add.o  add.s
#gcc -m32 -c main.c  -o main.o
#ld --entry=main --verbose -m elf_i386 -o test_ld.out   *.o -lc /usr/lib/gcc/x86_64-linux-gnu/4.8/32/crtend.o /usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../lib32/crtn.o

