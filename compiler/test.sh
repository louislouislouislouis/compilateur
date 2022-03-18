./ifcc $1.c>$1.s
as -o $1.o $1.s
gcc -o $1 $1.o
