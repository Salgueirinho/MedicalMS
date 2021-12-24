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
