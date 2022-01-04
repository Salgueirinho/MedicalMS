#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "doctor.h"
#include "service_desk.h"
#include "utils.h"

int	main(int argc, char *argv[])
{
	Doctor	me = {"", "", "", getpid(), false};
	char		dfifo[20] = "";
	char    command[255] = "";
	int			fd;
	struct timeval time;
	int bytes = -1;
	//int pid = -1;
	//int fdp = -1;
	//char	control;
	fd_set fds;

	if (serviceDeskIsRunning(0) == false)
	{
		fprintf(stderr, "The service desk isn't running\n");
		exit(0);
	}
	if (argc < 3)
	{
		fprintf(stderr, "%s <name> <speciality>\n", argv[0]);
		exit(0);
	}
	sprintf(dfifo, DFIFO, me.pid);
	if (mkfifo(dfifo, 0600) == -1)
	{
		fprintf(stderr, "Error occured while trying to make FIFO\n");
		exit(0);
	}
	if (access(SFIFO, F_OK) != 0)
	{
		fprintf(stderr, "Error, service desk FIFO doesn't exist\n");
		unlink(dfifo);
		exit(0);
	}
	strncpy(me.name, argv[1], sizeof(me.name) - 1);
	strncpy(me.speciality, argv[2], sizeof(me.speciality) - 1);
	if ((fd = open(SFIFO, O_WRONLY)) == -1)
	{
		fprintf(stderr, "Couldn't open named pipe file\n");
		unlink(dfifo);
		exit(0);
	}
	if (write(fd, "D", 1) == -1)
	{
		fprintf(stderr, "Couldn't write to named pipe\n");
		close(fd);
		unlink(dfifo);
		exit(0);
	}
	if (write(fd, &me, sizeof(Doctor)) == -1)
	{
		fprintf(stderr, "Couldn't write to named pipe\n");
		close(fd);
		unlink(dfifo);
		exit(0);
	}
	sprintf(me.signal, "M%d still alive\n", me.pid);
	time.tv_sec = 20;
	time.tv_usec = 0;
	do {
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(fd, &fds);
		bytes = select(fd + 1, &fds, NULL, NULL, &time);
		if (write(fd, me.signal, strlen(me.signal)) == -1)
		{
			fprintf(stderr, "Couldn't write to named pipe\n");
			close(fd);
			unlink(dfifo);
			exit(0);
		}
		if (bytes > 0 && FD_ISSET(0, &fds))
		{
			if (fgets(command, sizeof(command), stdin) == NULL)
			{
				fprintf(stderr, "Couldn't get command\n");
				close(fd);
				unlink(dfifo);
				exit(0);
			}
			if (strcmp(command, "exit\n") == 0)
				break;
		}
		else if (bytes > 0 && FD_ISSET(fd, &fds))
		{

		}
	} while (true);
	close(fd);
	return (0);
}
