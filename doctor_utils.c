#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include "doctor.h"

void	*sendSignal(void *data_ptr)
{
	SharedDData	*shared_data = data_ptr;
	int					timer = 19;
	char				outbound_message[20];

	sprintf(outbound_message, "S%d", shared_data->pid);
	while (shared_data->exit == false)
	{
		sleep(1);
		timer--;
		if (timer <= 0)
		{
			timer = 19;
			if (write(shared_data->sfd, outbound_message, strlen(outbound_message)) == -1)
			{
				fprintf(stderr, "Couldn't write outbound message to sfd\n");
			}
		}
	}
	return (NULL);
}

void	*readFIFO(void *data_ptr)
{
	SharedDData	*shared_data = data_ptr;
	char	incoming_message[255];
	char	pfifo[50];
	int		bytes;
	char	patientname[50];

	while (shared_data->exit == false)
	{
		if ((bytes = read(shared_data->dfd, incoming_message, sizeof(incoming_message) - 1)) == -1)
		{
			fprintf(stderr, "Couldn't read incoming message from dfd\n");
		}
		incoming_message[bytes] = '\0';
		switch (incoming_message[0])
		{
			case 'E':
				shared_data->exit = true;
				printf("Press <Enter> to exit");
				return (NULL);
			case 'A':
				signal(SIGPIPE, SIG_IGN);
				printf("Time for appointment\n");
				sprintf(pfifo, "/tmp/p%d", atoi(incoming_message + 1));
				if ((shared_data->pfd = open(pfifo, O_WRONLY)) == -1)
				{
					fprintf(stderr, "Couldn't open pfifo\n");
					shared_data->exit = true;
					return (NULL);
				}
				write(shared_data->pfd, shared_data->name, strlen(shared_data->name));
				bytes = read(shared_data->dfd, patientname, sizeof(patientname) - 1);
				patientname[bytes] = '\0';
				shared_data->appointment = true;
				while (shared_data->exit == false && shared_data->appointment == true)
				{
					if ((bytes = read(shared_data->dfd, incoming_message, sizeof(incoming_message) - 1)) == -1)
					{
						fprintf(stderr, "Couldn't read incoming message from dfd\n");
					}
					incoming_message[bytes] = '\0';
					if (strcmp(incoming_message, "exit\n") == 0)
					{
						printf("The patient had an emergency, your appointment is finished\n");
						shared_data->appointment = false;
						close(shared_data->pfd);
						shared_data->pfd = 1;
						break;
					}
					printf("%s: %s", patientname, incoming_message);
					if (strcmp(incoming_message, "bye\n") == 0)
					{
						printf("The appointment is over\n");
						shared_data->appointment = false;
						close(shared_data->pfd);
						shared_data->pfd = 1;
						break;
					}
				}
						sprintf(incoming_message, "A%d", shared_data->pid);
				if (write(shared_data->sfd, incoming_message, strlen(incoming_message)) == -1)
				{
					fprintf(stderr, "Couldn't write A to sfd\n");
				}
				break;
		}
	}
	return (NULL);
}
