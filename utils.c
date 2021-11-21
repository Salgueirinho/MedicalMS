#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

int		ourStrLen(const char *str)
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

void	ourPutString(const char *str)
{
	/*
		this is our version of putstring:
		- using system function calls
	*/
	write(1, str, ourStrLen(str));
}

static bool	isNumber(const char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (!('0' <= str[i] && str[i] <= '9'))
			return (false);
		i++;
	}
	return (true);
}

bool balcaoIsRunning(const int pid)
{
	DIR* dir;
	struct dirent* ent;
	char buf[50] = "\0";
	int	fd;

	if (!(dir = opendir("/proc")))
    return -1;

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
					if (!strncmp(buf, "./balcao", 8))
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
	return (false);
}
