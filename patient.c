#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include "service_desk.h"
#include "patient.h"
#include "patient_utils.h"
#include "service_desk_utils.h"
#include "utils.h"

int	main(int argc, char *argv[])
{
	SharedPData	shared_data;
	char	outbound_message[255] = "";
	char	pfifo[50];
	int		pid;
	pthread_t	read_worker;
	char	symptoms[50];

	if (!serviceDeskIsRunning(0))
	{
		fprintf(stderr, "Service desk isn't running!\n");
		exit(EXIT_FAILURE);
	}
	if (argc < 2)
	{
		fprintf(stderr, "%s <name>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	shared_data.exit = false;
	shared_data.appointment = false;
	pid = getpid();
	strncpy(shared_data.name, argv[1], sizeof(shared_data.name));
	if ((shared_data.sfd = open(SFIFO, O_RDWR)) == -1)
	{
		fprintf(stderr, "Couldn't open sfifo\n");
		exit(EXIT_FAILURE);
	}
	sprintf(pfifo, "/tmp/p%d", pid);
	if (mkfifo(pfifo, 0600) == -1)
	{
		fprintf(stderr, "Couldn't make fifo\n");
		close(shared_data.sfd);
		exit(EXIT_FAILURE);
	}
	if ((shared_data.pfd = open(pfifo, O_RDWR)) == -1)
	{
		fprintf(stderr, "Couldn't open pfifo\n");
		unlink(pfifo);
		close(shared_data.sfd);
		exit(EXIT_FAILURE);
	}
	printf("Your symptoms: ");
	if (fgets(symptoms, sizeof(symptoms) - 1, stdin) == NULL)
	{
		fprintf(stderr, "Couldn't get symptoms from stdin\n");
		unlink(pfifo);
		close(shared_data.sfd);
		close(shared_data.pfd);
		exit(EXIT_FAILURE);
	}
	sprintf(outbound_message, "P%d\n%s\n%s", pid, shared_data.name, symptoms);
	if (write(shared_data.sfd, outbound_message, strlen(outbound_message)) == -1)
	{
		fprintf(stderr, "Couldn't write outbound message to sfd\n");
		unlink(pfifo);
		close(shared_data.sfd);
		close(shared_data.pfd);
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&read_worker, NULL, readFIFO, &shared_data) != 0)
	{
		fprintf(stderr, "Couldn't create read worker\n");
		unlink(pfifo);
		close(shared_data.sfd);
		close(shared_data.pfd);
		exit(EXIT_FAILURE);
	}
	while (shared_data.exit == false)
	{
		if (fgets(outbound_message, sizeof(outbound_message) - 1, stdin) == NULL)
		{
			fprintf(stderr, "Couldn't get outbound message from stdin\n");
			unlink(pfifo);
			close(shared_data.sfd);
			close(shared_data.pfd);
			exit(EXIT_FAILURE);
		}
		if (shared_data.appointment == true)
		{
			if (write(shared_data.dfd, outbound_message, strlen(outbound_message)) == -1)
			{
				fprintf(stderr, "Couldn't send outbound message to patient\n");
				shared_data.exit = true;
			}
		}
		if (strcmp(outbound_message, "exit\n") == 0
				|| strcmp(outbound_message, "bye\n") == 0)
		{
			shared_data.exit = true;
			if (write(shared_data.pfd, "E", 1) == -1)
			{
				fprintf(stderr, "Couldn't write Exit control to pfd\n");
				unlink(pfifo);
				close(shared_data.sfd);
				close(shared_data.pfd);
				exit(EXIT_FAILURE);
			}
			sprintf(outbound_message, "G%d", pid);
			if (write(shared_data.sfd, outbound_message, strlen(outbound_message)) == -1)
			{
				fprintf(stderr, "Couldn't write Exit control to sfd\n");
				unlink(pfifo);
				close(shared_data.sfd);
				close(shared_data.pfd);
				exit(EXIT_FAILURE);
			}
		}
	}
	if (pthread_join(read_worker, NULL) != 0)
	{
		fprintf(stderr, "Couldn't join read worker\n");
	}
	unlink(pfifo);
	close(shared_data.pfd);
	close(shared_data.sfd);
	return (0);
}
