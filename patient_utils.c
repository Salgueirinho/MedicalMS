#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "patient.h"

void	*readFIFO(void *data_ptr)
{
	SharedPData *shared_data = data_ptr;
	char	incoming_message[255];
	int		bytes;
	char	dfifo[50];
	char	doctorname[50];

	if ((bytes = read(shared_data->pfd, incoming_message, sizeof(incoming_message) - 1)) == -1)
	{
		fprintf(stderr, "Couldn't read incoming message\n");
		shared_data->exit = true;
		return (NULL);
	}
	if (incoming_message[0] == 'M')
	{
		shared_data->exit = true;
		fprintf(stderr, "The queue is full, try again later!\n");
		return (NULL);
	}
	incoming_message[bytes] = '\0';
	printf("%s", incoming_message);
	while (shared_data->exit == false)
	{
		if ((bytes = read(shared_data->pfd, incoming_message, sizeof(incoming_message) - 1)) == -1)
		{
			fprintf(stderr, "Couldn't read incoming message\n");
			shared_data->exit = true;
			return (NULL);
		}
		incoming_message[bytes] = '\0';
		switch (incoming_message[0])
		{
			case 'E':
				shared_data->exit = true;
				printf("Press <Enter> to exit");
				return (NULL);
			case 'A':
				printf("Time for appointment\n");
				sprintf(dfifo, "/tmp/d%d", atoi(incoming_message + 1));
				if ((shared_data->dfd = open(dfifo, O_WRONLY)) == -1)
				{
					fprintf(stderr, "Couldn't open dfifo\n");
					shared_data->exit = true;
					return (NULL);
				}
				write(shared_data->dfd, shared_data->name, strlen(shared_data->name));
				bytes = read(shared_data->pfd, doctorname, sizeof(doctorname) - 1);
				doctorname[bytes] = '\0';
				shared_data->appointment = true;
				while (shared_data->exit == false)
				{
					if ((bytes = read(shared_data->pfd, incoming_message, sizeof(incoming_message) - 1)) == -1)
					{
						fprintf(stderr, "Couldn't read incoming message from pfd\n");
					}
					incoming_message[bytes] = '\0';
					if (strcmp(incoming_message, "exit\n") == 0)
					{
						printf("The doctor had an emergency, your appointment is finished\n");
						printf("Press <Enter> to exit");
						close(shared_data->dfd);
						shared_data->dfd = 1;
						shared_data->exit = true;
						return (NULL);
					}
					printf("Dr. %s: %s", doctorname, incoming_message);
					if (strcmp(incoming_message, "bye\n") == 0)
					{
						printf("Press <Enter> to exit");
						shared_data->exit = true;
						return (NULL);
					}
				}
				break;
		}
	}
	return (NULL);
}
