./ifcc $1.c>$1.s
as -o $1.o $1.s
gcc -arch x86_64 -o $1 $1.o
