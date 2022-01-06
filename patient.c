#include <signal.h>
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
	printf("Your symptoms: ");
	if (fgets(me->symptoms, sizeof(me->symptoms), stdin) == NULL)
		exit(0);
}

void	handleSIGINT(int i)
{
	(void) i;
	printf("\nCtrl + c is disabled for this session, please use \"exit\" instead!\n");
}

int main(int argc, char *argv[])
{
	Patient	me = {"", "", getpid(), ""};
	char	pfifo[20] = "";
	char  message[255] = "";
	int		queue_size;
	int		fdp;
	int		fd;

	if (signal(SIGINT, handleSIGINT) == SIG_ERR)
	{
		fprintf(stderr, "It wasn't possible to configure SIGINT\n");
		exit(-1);
	}
	if (serviceDeskIsRunning(0) == false)
	{
		fprintf(stderr, "The service desk isn't running\n");
		exit(0);
	}
	if (argc < 2)
	{
		printf("%s <name>\n", argv[0]);
		exit(0);
	}
	sprintf(pfifo, PFIFO, me.pid);
	if (mkfifo(pfifo, 0600) == -1)
	{
		fprintf(stderr, "Error occured while trying to make FIFO\n");
		exit(0);
	}
	if (access(SFIFO, F_OK) != 0)
	{
		fprintf(stderr, "Error, service desk FIFO doesn't exist\n");
		unlink(pfifo);
		exit(0);
	}
	strncpy(me.name, argv[1], sizeof(me.name) - 1);
	registerPatient(&me);
	if ((fd = open(SFIFO, O_WRONLY)) == -1)
	{
		fprintf(stderr, "Couldn't open named pipe file\n");
		unlink(pfifo);
		exit(0);
	}
	if (write(fd, "P", 1) == -1)
	{
		fprintf(stderr, "Couldn't write to named pipe\n");
		close(fd);
		unlink(pfifo);
		exit(0);
	}
	if (write(fd, &me, sizeof(Patient)) == -1)
	{
		fprintf(stderr, "Couldn't write to named pipe\n");
		close(fd);
		unlink(pfifo);
		exit(0);
	}
	close(fd);
	if ((fdp = open(pfifo, O_RDONLY)) == -1)
	{
		fprintf(stderr, "Couldn't open FIFO\n");
		close(fd);
		unlink(pfifo);
		exit(0);
	}
	if (read(fdp, me.speciality, sizeof(me.speciality) - 1) == -1)
	{
		fprintf(stderr, "Couldn't read from named pipe\n");
		close(fd);
		unlink(pfifo);
		exit(0);
	}
	if (read(fdp, &queue_size, sizeof(int)) == -1)
	{
		fprintf(stderr, "Couldn't read from named pipe\n");
		close(fd);
		unlink(pfifo);
		exit(0);
	}
	close(fdp);
	printf("Your speciality: %s", me.speciality);
	printf("Patients in front of you: %d\n", queue_size);

	while(strcmp(message, "exit\n") != 0)
	{
		printf("Command: ");
		if (fgets(message, sizeof(message) - 1, stdin) == NULL)
			exit(0);
	}

	unlink(pfifo);
	exit(0);
}
