int main()
{
    int a = 2;
    while (a < 20)
    {
        if (a == 15)
        {
            return -14;
        }

        a += 1;
    }

    return a;
}