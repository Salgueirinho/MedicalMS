#include <stdbool.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

int	putString(const char *s, int fd)
{
	return (write(fd, s, strlen(s)));
}
static int	intlen(int n)
{
	int	len;

	len = 0;
	if (n == 0)
		return (1);
	else
	{
		while (n)
		{
			len++;
			n /= 10;
		}
	}
	return (len);
}

int	putNumber(int n, int fd)
{
	char	n_str[12];
	int		len;
	int		i;

	if (n == -2147483648)
		return (write(fd, "-2147483648", 11));
	else
	{
		len = intlen(n);
		i = 0;
		if (n < 0)
		{
			write(fd, "-", 1);
			n *= -1;
		}
		while (i < len)
		{
			n_str[len - i - 1] = n % 10 + '0';
			n /= 10;
			i++;
		}
		return (write(fd, n_str, len));
	}
}

static bool	isNumber(const char *s)
{
	while (*s)
	{
		if (!isdigit(*s))
			return (false);
		s++;
	}
	return (true);
}

bool serviceDeskIsRunning(const int pid)
{
	DIR* dir = NULL;
	struct dirent* ent = NULL;
	char buf[50] = "\0";
	int	fd = -1;

	if (!(dir = opendir("/proc")))
		exit(1);
	while((ent = readdir(dir)) != NULL)
	{
		if (isNumber(ent->d_name) && atoi(ent->d_name) != pid)
		{
			strcat(buf, "/proc/");
			strcat(buf, ent->d_name);
			strcat(buf, "/cmdline");
			fd = open(buf, O_RDONLY);
			if (fd)
			{
				strcpy(buf, "\0");
				if (read(fd, buf, sizeof(buf)) != -1)
				{
					if (strncmp(buf, "./service_desk", 14) == 0)
					{
						close(fd);
						closedir(dir);
						return (true);
					}
				}
				close(fd);
			}
		}
		strcpy(buf, "");
	}
	closedir(dir);
	return (false);
}
