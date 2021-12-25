#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "service_desk.h"
#include "patient.h"
#include "utils.h"
#include "doctor.h"

static void	executeClassifier(int p1[2], int p2[2]);

int main(void)
{
	Patient	patient = {"default", "", 0, "geral"};
	Doctor	doctor = {"default", "", 0, false};
	char command[40] = "";
	char pfifo[15] = "";
	int p1[2] = {-1, -1};
	int p2[2] = {-1, -1};
	struct timeval time;
	int bytes = -1;
	int pid = -1;
	int fd = -1;
	int fdp = -1;
	char	control;
	fd_set fds;

	if (serviceDeskIsRunning((int)getpid()) == true)
	{
		fprintf(stderr, "There is already a service desk running!\n");
		exit(0);
	}
	if (access(SFIFO, F_OK) == 0)
	{
		fprintf(stderr, "There is already a service desk FIFO open\n");
		exit(0);
	}
	if (mkfifo(SFIFO, 0600) == -1)
	{
		fprintf(stderr, "An error occured while trying to make FIFO\n");
		exit(0);
	}
	if (pipe(p1) == -1)
	{
		unlink(SFIFO);
		fprintf(stderr, "An error occured while trying to make pipe p1\n");
		exit(0);
	}
	if (pipe(p2) == -1)
	{
		close(p1[0]);
		close(p1[1]);
		unlink(SFIFO);
		fprintf(stderr, "An error occured while trying to make pipe p2\n");
		exit(0);
	}
	if ((pid = fork()) == -1)
	{
		close(p2[0]);
		close(p2[1]);
		close(p1[0]);
		close(p1[1]);
		unlink(SFIFO);
		fprintf(stderr, "An error occured while trying to fork\n");
		exit(0);
	}
	if (pid == 0)
		executeClassifier(p1, p2);
	close(p1[0]);
	close(p2[1]);
	if ((fd = open(SFIFO, O_RDWR)) == -1)
	{
		close(p1[1]);
		close(p2[0]);
		unlink(SFIFO);
		fprintf(stderr, "An error occured while trying to open serice desk FIFO\n");
		exit(0);
	}
	time.tv_sec = 1;
	time.tv_usec = 0;
	do {
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(fd, &fds);
		bytes = select(fd + 1, &fds, NULL, NULL, &time);
		if (bytes > 0 && FD_ISSET(0, &fds))
		{
			if ((bytes = read(0, command, sizeof(command) - 1)) == -1)
			{
				fprintf(stderr, "An error occured while trying to read command!\n");
				exit(0);
			}
			command[bytes - 1] = '\0';
			if (strcmp(command, "exit") == 0)
				break;
		}
		else if (bytes > 0 && FD_ISSET(fd, &fds))
		{
			read(fd, &control , 1);
			if (control == 'D')
			{
				if ((bytes = read(fd, &doctor, sizeof(Doctor))) == -1)
				{
					close(fd);
					close(p1[1]);
					close(p2[0]);
					unlink(SFIFO);
					fprintf(stderr, "An error occured while trying to read doctor's details\n");
					exit(0);
				}
				if (bytes == sizeof(Doctor))
				{
					printf("Registered specialist:\n"
								"- name: %s\n"
								"- speciality: %s\n"
								"- pid: %d\n",
								doctor.name, doctor.speciality, doctor.pid);
				}
			}
			else if (control == 'P')
			{
				if ((bytes = read(fd, &patient, sizeof(Patient))) == -1)
				{
					close(fd);
					close(p1[1]);
					close(p2[0]);
					unlink(SFIFO);
					fprintf(stderr, "An error occured while trying to read patient's details\n");
					exit(0);
				}
				if (bytes == sizeof(Patient))
				{
					if (write(p1[1], patient.symptoms, strlen(patient.symptoms)) == -1)
					{
						close(fd);
						close(p1[1]);
						close(p2[0]);
						unlink(SFIFO);
						fprintf(stderr, "An error occured while trying to write symptoms\n");
						exit(0);
					}
					if (strcmp(patient.symptoms, "#fim\n") != 0)
					{
						if ((bytes = read(p2[0], patient.speciality, sizeof(patient.speciality))) == -1)
						{
							close(fd);
							close(p1[1]);
							close(p2[0]);
							unlink(SFIFO);
							fprintf(stderr, "An error occured while trying to read speciality\n");
							exit(0);
						}
						patient.speciality[bytes] = '\0';
						sprintf(pfifo, "/tmp/p%d", patient.pid);
						if ((fdp = open(pfifo, O_WRONLY)) == -1)
						{
							close(fd);
							close(p1[1]);
							close(p2[0]);
							unlink(SFIFO);
							fprintf(stderr, "An error occured while trying to open patient FIFO\n");
						}
						if (write(fdp, patient.speciality, strlen(patient.speciality)) == -1)
						{
							close(fdp);
							close(fd);
							close(p1[1]);
							close(p2[0]);
							unlink(SFIFO);
							fprintf(stderr, "An error occured while trying to write speciality\n");
							exit(0);
						}
						printf("Registered patient:\n"
							"- name: %s\n"
							"- symptoms: %s"
							"- pid: %d\n"
							"- speciality: %s",
							patient.name, patient.symptoms, patient.pid,patient.speciality);
						close(fdp);
					}
				}
			}
		}
	} while (true);
	close(fd);
	unlink(SFIFO);
	close(p1[1]);
	close(p2[0]);
	return (0);
}

static void	executeClassifier(int p1[2], int p2[2])
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
	fprintf(stderr, "An error occured while attempting to execute the classifier\n");
	unlink(SFIFO);
	exit(0);
}
