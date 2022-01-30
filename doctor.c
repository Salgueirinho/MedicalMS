#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include "service_desk.h"
#include "doctor.h"
#include "doctor_utils.h"
#include "utils.h"

int	main(int argc, char *argv[])
{
	SharedDData	shared_data;
	char	outbound_message[255] = "";
	char	dfifo[50];
	pthread_t	read_worker;
	pthread_t	signal_worker;

	if (!serviceDeskIsRunning(0))
	{
		fprintf(stderr, "Service desk isn't running!\n");
		exit(EXIT_FAILURE);
	}
	if (argc < 3)
	{
		fprintf(stderr, "%s <name> <speciality>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	shared_data.pid = getpid();
	shared_data.exit = false;
	shared_data.appointment = false;
	strncpy(shared_data.name, argv[1], sizeof(shared_data.name));
	if ((shared_data.sfd = open(SFIFO, O_RDWR)) == -1)
	{
		fprintf(stderr, "Couldn't open sfd");
	}
	sprintf(dfifo, "/tmp/d%d", shared_data.pid);
	if (mkfifo(dfifo, 0600) == -1)
	{
		fprintf(stderr, "Couldn't make dfifo");
	}
	if ((shared_data.dfd = open(dfifo, O_RDWR)) == -1)
	{
		fprintf(stderr, "Couldn't open dfd\n");
	}
	sprintf(outbound_message, "D%d\n%s\n%s\n", shared_data.pid, shared_data.name, argv[2]);
	if (write(shared_data.sfd, outbound_message, strlen(outbound_message)) == -1)
	{
		fprintf(stderr, "Couldn't write outbound message to sfd\n");
	}
	pthread_create(&read_worker, NULL, readFIFO, &shared_data);
	pthread_create(&signal_worker, NULL, sendSignal, &shared_data);
	while (shared_data.exit == false)
	{
		if (fgets(outbound_message, sizeof(outbound_message) - 1, stdin) == NULL)
		{
			fprintf(stderr, "Couldn't get outbound message from stdin\n");
		}
		if (shared_data.appointment == true)
		{
			write(shared_data.pfd, outbound_message, strlen(outbound_message));
			if (strcmp(outbound_message, "bye\n") == 0)
				shared_data.appointment = false;
		}
		else
			signal(SIGPIPE, SIG_DFL);
		if (strcmp(outbound_message, "exit\n") == 0)
		{
			shared_data.exit = true;
			if (write(shared_data.dfd, "E", 1) == -1)
			{
				fprintf(stderr, "Couldn't write E to dfd");
			}
			sprintf(outbound_message, "F%d", shared_data.pid);
			if (write(shared_data.sfd, outbound_message, strlen(outbound_message)) == -1)
			{
				fprintf(stderr, "Couldn't write Exit control to sfd\n");
				unlink(dfifo);
				close(shared_data.sfd);
				close(shared_data.pfd);
				exit(EXIT_FAILURE);
			}
		}
	}
	pthread_join(read_worker, NULL);
	pthread_join(signal_worker, NULL);
	unlink(dfifo);
	close(shared_data.dfd);
	close(shared_data.sfd);
	return (0);
}
