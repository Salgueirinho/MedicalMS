#include <unistd.h>

void	ourPutChar(char c)
{
	write(1, &c, 1);
}

int		ourStrLen(char *str)
{
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
	write(1, str, ourStrLen(str));
}
