#include "medical_os.h"

void	setSIGINT(void)
{
	if (signal(SIGINT, handleSIGINT) == SIG_ERR)
	{
		fprintf(stderr, "It wasn't possible to configure SIGINT\n");
		exit(-1);
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

bool serviceDeskIsRunning(int pid)
{
	struct dirent	*ent = NULL;
	char buf[50] = "\0";
	DIR* dir = NULL;
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

void	*sendLifeSignal(void *ptr)
{
	LifeSignal	*lifesignal = (LifeSignal *) ptr;
	int					i;

	while(true)
	{
		i = 0;
		while (i < 15)
		{
			sleep(1);
			if (*lifesignal->exit == true)
				return (NULL);
			i++;
		}
		if(write(lifesignal->service_desk_fd, "N", 1) == -1)
		{
			fprintf(stderr, "Couldn't write control character \"N\" to FIFO\n");
			close(lifesignal->service_desk_fd);
			exit(0);
		}
		if ((write(lifesignal->service_desk_fd, &lifesignal->pid, sizeof(int)) == -1))
		{
			fprintf(stderr, "Couldn't write life signal's PID to FIFO\n");
			close(lifesignal->service_desk_fd);
			exit(0);
		}
	}
	return (NULL);
}
