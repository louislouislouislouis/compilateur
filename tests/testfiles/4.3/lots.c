#include "stdio.h"

int main()
/* main */
{
    int x = 4, y; // Multiple declarations
    int z = 2;
    y = z, z = x; // Multiple assignments
    y = x;
    return y;
}
