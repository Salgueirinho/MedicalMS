#include "medical_os.h"

int	main(int argc, char *argv[])
{
	PatientData	patientdata;
	char				pfifo[20];
	char				command[50];
	int					service_desk_fd;
	pthread_t		fifohandler_t;
	int					size;
	int					bytes;

	if (argc < 2)
	{
		printf("%s <name>\n", argv[0]);
		exit(0);
	}
	setSIGINT();
	if (serviceDeskIsRunning(0) == false)
		{
		fprintf(stderr, "The service desk isn't running!\n");
		exit(0);
	}
	strncpy(patientdata.me.name, argv[1], sizeof(patientdata.me.name) - 1);
	bzero(patientdata.me.speciality, sizeof(patientdata.me.speciality));
	patientdata.me.pid = getpid();
	patientdata.exit = false;
	sprintf(pfifo, "/tmp/p%d", patientdata.me.pid);
	printf("Describe your symptoms: ");
	if (fgets(patientdata.me.symptoms, sizeof(patientdata.me.symptoms) - 1, stdin) == NULL)
	{
		fprintf(stderr, "An error occured while trying to read symptoms from stdin\n");
		exit(0);
	}	
	if (mkfifo(pfifo, 0600) == -1)
	{
		fprintf(stderr, "Something went wrong while trying to make FIFO\n");
		exit(0);
	}
	if ((patientdata.fd = open(pfifo, O_RDWR)) == -1)
	{
		fprintf(stderr, "Couldn't open patient's FIFO\n");
		unlink(pfifo);
		exit(0);
	}
	if ((service_desk_fd = open(SFIFO, O_WRONLY)) == -1)
	{
		fprintf(stderr, "Couldn't open service desk FIFO\n");
		unlink(pfifo);
		exit(0);
	}
	if (write(service_desk_fd, "P", 1) == -1)
	{
		fprintf(stderr, "Couldn't write to service desk FIFO\n");
		close(service_desk_fd);
		unlink(pfifo);
		exit(0);
	}
	if (write(service_desk_fd, &patientdata.me, sizeof(Patient)) == -1)
	{
		fprintf(stderr, "Couldn't write patient to named pipe\n");
		close(service_desk_fd);
		unlink(pfifo);
		exit(0);
	}
	if (read(patientdata.fd, &size, sizeof(int)) != sizeof(int))
	{
		fprintf(stderr, "Couldn't read speciality size from named pipe\n");
		close(service_desk_fd);
		unlink(pfifo);
		exit(0);
	}
	if ((bytes = read(patientdata.fd, patientdata.me.speciality, size)) == -1)
	{
		fprintf(stderr, "Couldn't read speciality from named pipe\n");
		close(service_desk_fd);
		unlink(pfifo);
		exit(0);
	}
	printf("Speciality: %s", patientdata.me.speciality);
	if (read(patientdata.fd, &size, sizeof(int)) == -1)
	{
		fprintf(stderr, "Couldn't read queue size from named pipe\n");
		close(service_desk_fd);
		unlink(pfifo);
		exit(0);
	}
	printf("Queue size: %d\n", size);
	pthread_create(&fifohandler_t, NULL, FIFOHandlerT, &patientdata);
	while (true)
	{
		if (fgets(command, sizeof(command) - 1, stdin) == NULL)
		{
			fprintf(stderr, "An error occured while trying to read command from stdin\n");
			close(service_desk_fd);
			unlink(pfifo);
			exit(0);
		}
		if (strcmp(command, "exit\n") == 0)
			patientdata.exit = true;
		if (patientdata.exit == true)
			break;
	}
	if ((write(patientdata.fd, "E", 1)) == -1)
	{
		fprintf(stderr, "Couldn't write control character \"Z\" to patient's FIFO\n");
		unlink(pfifo);
		close(service_desk_fd);
		exit(0);
	}
	pthread_join(fifohandler_t, NULL);
	unlink(pfifo);
	if(write(service_desk_fd, "F", 1) == -1)
	{
		fprintf(stderr, "Couldn't write control character \"F\" to FIFO\n");
		unlink(pfifo);
		close(service_desk_fd);
		exit(0);
	}
	if ((write(service_desk_fd, &patientdata.me.pid, sizeof(int)) == -1))
	{
		fprintf(stderr, "Couldn't write exit signal's PID to FIFO\n");
		unlink(pfifo);
		close(service_desk_fd);
		exit(0);
	}
	close(service_desk_fd);
	printf("Hope to see you soon, %s\n", patientdata.me.name);
	return (0);
}

void	handleSIGINT(int i)
{
	(void) i;
	printf("\nCtrl + c is disabled, please use \"exit\" instead!\n");
}

void	*FIFOHandlerT(void *ptr)
{
	PatientData	*patientdata = (PatientData *) ptr;
	char	control;

	while (true)
	{
		if (read(patientdata->fd, &control, 1) == -1)
		{
			fprintf(stderr, "An error occured while trying to read control character\n");
			// error occured
		}
		switch (control)
		{
			case 'E':
				patientdata->exit = true;
				return (NULL);
			case 'Z':
				printf("Received exit signal from service desk, press <Enter> to exit\n");
				patientdata->exit = true;
				return (NULL);
			default:
				fprintf(stderr, "An error occured! Read non-fiable control digit\n");
				patientdata->exit = true;
				// error occured
				break;
		}
	}
	return (NULL);
}
