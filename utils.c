#include <unistd.h>

void	ourPutChar(char c)
{
	/*
		this is our version of putchar:
		- using system function calls
	*/
	write(1, &c, 1);
}

int		ourStrLen(char *str)
{
	/*
		this is our version of strlen:
		- avoiding library function calls
	*/
	int	len;

	len = 0;
	while (str[len])
	{
		len++;
	}
	return (len);
}

void	ourPutString(char *str)
{
	/*
		this is our version of putstring:
		- using system function calls
	*/
	write(1, str, ourStrLen(str));
}
