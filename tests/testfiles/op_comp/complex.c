int main()
{
    int x = 1 && 0;
    int y = 1 && 1;
    int z = 0 && 1;
    int a = 0 && 0;
    return !(!x && y || z && !a);
}