int main()
{
	int x = 0;
	if (x < 10)
	{
		x = x + 1;
		if (x < 10)
		{
			x = x + 1;
		}
		else if (x < 20)
		{
			x = x + 1;
		}
		else
		{
			x = x + 1;
		}
	}
	return x;
}