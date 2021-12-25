#include "patient.h"
#include "service_desk.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void registerPatient(Patient *me)
{
	int bytes = -1;

	putString("Your symptoms: ", STDOUT_FILENO);
	if ((bytes = read(0, me->symptoms, sizeof(me->symptoms) - 1)) == -1)
	{
		putString("Couldn't read symptoms\n", STDERR_FILENO);
		exit(0);
	}
	me->symptoms[bytes - 1] = '\n';
	me->symptoms[bytes] = '\0';
}

int main(int argc, char *argv[])
{
	Patient	me = {"", "", getpid(), ""};
	char	pfifo[20] = "";
	int		fdp;
	int		fd;

	if (serviceDeskIsRunning(0) == false)
	{
		putString("The service desk isn't running\n", STDERR_FILENO);
		exit(0);
	}
	if (argc < 2)
	{
		putString(argv[0], STDERR_FILENO);
		putString(" <name>\n", STDERR_FILENO);
		exit(0);
	}
	sprintf(pfifo, PFIFO, me.pid);
	if (mkfifo(pfifo, 0600) == -1)
	{
		putString("Error occured while trying to make FIFO\n", STDERR_FILENO);
		exit(0);
	}
	if (access(SFIFO, F_OK) != 0)
	{
		putString("Error, service desk FIFO doesn't exist\n", STDERR_FILENO);
		unlink(pfifo);
		exit(0);
	}
	strncpy(me.name, argv[1], sizeof(me.name));
	registerPatient(&me);
	if ((fd = open(SFIFO, O_WRONLY)) == -1)
	{
		putString("Couldn't open named pipe file\n", STDERR_FILENO);
		unlink(pfifo);
		exit(0);
	}
	if (write(fd, "P", 1) == -1)
	{
		putString("Couldn't write to named pipe\n", STDERR_FILENO);
		close(fd);
		unlink(pfifo);
		exit(0);
	}
	if (write(fd, &me, sizeof(Patient)) == -1)
	{
		putString("Couldn't write to named pipe\n", STDERR_FILENO);
		close(fd);
		unlink(pfifo);
		exit(0);
	}
	close(fd);
	if ((fdp = open(pfifo, O_RDONLY)) == -1)
	{
		putString("Couldn't open FIFO\n", STDERR_FILENO);
		close(fd);
		unlink(pfifo);
		exit(0);
	}
	if (read(fdp, me.speciality, sizeof(me.speciality) - 1) == -1)
	{
		close(fd);
		unlink(pfifo);
		putString("Couldn't read from named pipe\n", STDERR_FILENO);
		exit(0);
	}
	close(fdp);
	if (putString(me.speciality, STDOUT_FILENO) == -1)
	{
		close(fd);
		unlink(pfifo);
		putString("Couldn't putString\n", STDERR_FILENO);
		exit(0);
	}
	unlink(pfifo);
	exit(0);
}
