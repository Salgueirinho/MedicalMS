#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "doctor.h"
#include "service_desk.h"
#include "utils.h"

int	main(int argc, char *argv[])
{
	Doctor	me = {"", "", getpid(), false};
	//char		dfifo[20] = "";
	int			fd;

	if (serviceDeskIsRunning(0) == false)
	{
		putString("The service desk isn't running\n", STDERR_FILENO);
		exit(0);
	}
	if (argc < 3)
	{
		putString(argv[0], STDERR_FILENO);
		putString(" <name> <speciality>\n", STDERR_FILENO);
		exit(0);
	}
	/*
	sprintf(dfifo, DFIFO, me.pid);
	if (mkfifo(dfifo, 0600) == -1)
	{
		putString("Error occured while trying to make FIFO\n", STDERR_FILENO);
		exit(0);
	}
	*/
	if (access(SFIFO, F_OK) != 0)
	{
		putString("Error, service desk FIFO doesn't exist\n", STDERR_FILENO);
		//unlink(dfifo);
		exit(0);
	}
	strncpy(me.name, argv[1], sizeof(me.name));
	strncpy(me.speciality, argv[2], sizeof(me.speciality));
	if ((fd = open(SFIFO, O_WRONLY)) == -1)
	{
		putString("Couldn't open named pipe file\n", STDERR_FILENO);
		//unlink(dfifo);
		exit(0);
	}
	if (write(fd, "D", 1) == -1)
	{
		putString("Couldn't write to named pipe\n", STDERR_FILENO);
		close(fd);
		//unlink(dfifo);
		exit(0);
	}
	if (write(fd, &me, sizeof(Doctor)) == -1)
	{
		putString("Couldn't write to named pipe\n", STDERR_FILENO);
		close(fd);
		//unlink(dfifo);
		exit(0);
	}
	close(fd);
	return (0);
}
