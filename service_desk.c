#include "service_desk.h"
#include "patient.h"
#include "utils.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
	Patient patient = {"default", "#fim\n", 0};
	char speciality[40] = "";
	char command[40] = "";
	char pfifo[15] = "";
	int p1[2] = {-1, -1};
	int p2[2] = {-1, -1};
	struct timeval time;
	int bytes = -1;
	int pid = -1;
	int fd = -1;
	int fdp = -1;
	fd_set fds;

	if (serviceDeskIsRunning((int)getpid()) == true)
	{
		putString("There is already a service desk running!\n", STDERR_FILENO);
		exit(0);
	}
	if (access(SERV_FIFO, F_OK) == 0)
	{
		putString("There is already a service desk FIFO open\n", STDERR_FILENO);
		exit(0);
	}
	if (mkfifo(SERV_FIFO, 0600) == -1)
	{
		putString("An error occured while trying to make FIFO\n", STDERR_FILENO);
		exit(0);
	}
	if (pipe(p1) == -1)
		exit(0);
	if (pipe(p2) == -1)
		exit(0);
	pid = fork();
	if (pid == -1)
		exit(0);
	else if (pid == 0)
	{
		close(p1[1]);
		close(p2[0]);
		close(0);
		dup(p1[0]);
		close(1);
		dup(p2[1]);
		close(p1[0]);
		close(p2[1]);
		execl("classifier", "classifier", NULL);
		putString("An error occured while attempting to execute the classifier\n",
			STDERR_FILENO);
		exit(0);
	}
	close(p1[0]);
	close(p2[1]);
	fd = open(SERV_FIFO, O_RDWR);
	putString("[admin] command: ", STDOUT_FILENO);
	do {
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(fd, &fds);
		time.tv_sec = 1;
		time.tv_usec = 0;
		bytes = select(fd + 1, &fds, NULL, NULL, &time);
		if (bytes > 0 && FD_ISSET(0, &fds))
		{
			bytes = read(0, command, sizeof(command) - 1);
			if (bytes == -1)
			{
				putString("An error occured while trying to read command!\n",
					STDERR_FILENO);
				exit(0);
			}
			command[bytes - 1] = '\0';
			if (strcmp(command, "exit") == 0)
				break;
			putString("[admin] command: ", STDOUT_FILENO);
		}
		else if (bytes > 0 && FD_ISSET(fd, &fds))
		{
			if ((bytes = read(fd, &patient, sizeof(Patient))) == -1)
			{
				putString("An error occured while trying to read patient's details\n",
					STDERR_FILENO);
				exit(0);
			}
			if (bytes == sizeof(Patient))
			{
				if (write(p1[1], patient.symptoms, strlen(patient.symptoms)) == -1)
				{
					putString("An error occured while trying to write symptoms\n",
						STDERR_FILENO);
					exit(0);
				}
				if (strcmp(patient.symptoms, "#fim\n") != 0)
				{
					if ((bytes = read(p2[0], speciality, sizeof(speciality) - 1)) == -1)
					{
						putString("An error occured while trying to read speciality\n",
							STDERR_FILENO);
						exit(0);
					}
					speciality[bytes] = '\0';
					sprintf(pfifo, "/tmp/p%d", patient.pid);
					if ((fdp = open(pfifo, O_WRONLY)) == -1)
					{
						putString("An error occured while trying to open patient FIFO\n",
							STDERR_FILENO);
					}
					if (write(fdp, speciality, sizeof(speciality)) == -1)
					{
						putString("An error occured while trying to write speciality\n",
							STDERR_FILENO);
						exit(0);
					}
					putString(patient.name, STDOUT_FILENO);
					putString("'s speciality was sent\n", STDOUT_FILENO);
					close(fdp);
				}
			}
		}
	} while (true);
	close(fd);
	unlink(SERV_FIFO);
	close(p1[1]);
	close(p2[0]);
	return (0);
}
