int main()
{
    int x = 45, y = -57, z = 8;
    int z = x & y;
    x = z | y;
    z = x ^ y;
    y = ~z;
    return y;
}