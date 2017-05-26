#!/bin/sh
set -x
rm *.o *.so
gcc -O3 -c geany_prettify.c -fPIC `pkg-config --cflags geany` 
gcc -O3 geany_prettify.o -o geany_prettify.so -shared `pkg-config --libs geany`
