int main()
{
    int x = 3, y = 8, z = 56, w = -3;
    x = y * w + z * w;
    w = z / y;
    z = x % y;
    return 8 * x - 48 * z + w % y;
}