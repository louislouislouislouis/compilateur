int main()
{
    int a = 2;
    int y = 0;
    if (a == 2)
    {
        while (y < 10)
        {
            a = 3;
            y += 1;
        }
    }

    return y % a;
}