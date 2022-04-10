int main()
{
    int x = 0;
    int y = 0;
    while (x < 10)
    {
        x += 1;
        if (x == 5)
        {
            continue;
        }
        y += 1;
    }
    return y;
}