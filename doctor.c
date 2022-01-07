#include "medical_os.h"

int	main(int argc, char *argv[])
{
	DoctorData	doctordata;
	LifeSignal	lifesignal;
	pthread_t		lifesignal_t;
	pthread_t		fifohandler_t;
	char				dfifo[20];
	char				command[50];
	int					service_desk_fd;

	if (argc < 3)
	{
		printf("%s <name> <speciality>\n", argv[0]);
		exit(0);
	}
	setSIGINT();
	/*if (serviceDeskIsRunning(0) == true)
		{
		fprintf(stderr, "There is already a service desk running!\n");
		exit(0);
		}*/
	strncpy(doctordata.me.name, argv[1], sizeof(doctordata.me.name) - 1);
	strncpy(doctordata.me.speciality, argv[2], sizeof(doctordata.me.speciality) - 1);
	doctordata.me.pid = getpid();
	doctordata.exit = false;
	sprintf(dfifo, "/tmp/d%d", doctordata.me.pid);
	if ((service_desk_fd = open(SFIFO, O_RDWR)) == -1)
	{
		fprintf(stderr, "Couldn't open named pipe file\n");
		unlink(dfifo);
		exit(0);
	}
	if (mkfifo(dfifo, 0600) == -1)
	{
		fprintf(stderr, "Something went wrong while trying to make FIFO\n");
		exit(0);
	}
	if ((doctordata.fd = open(dfifo, O_RDWR)) == -1)
	{
		fprintf(stderr, "Couldn't open doctor's FIFO\n");
		unlink(dfifo);
		exit(0);
	}
	if (write(service_desk_fd, "D", 1) == -1)
	{
		fprintf(stderr, "Couldn't write to named pipe\n");
		close(service_desk_fd);
		unlink(dfifo);
		exit(0);
	}
	if (write(service_desk_fd, &doctordata.me, sizeof(Doctor)) == -1)
	{
		fprintf(stderr, "Couldn't write to named pipe\n");
		close(service_desk_fd);
		unlink(dfifo);
		exit(0);
	}
	lifesignal.pid = doctordata.me.pid;
	lifesignal.service_desk_fd = service_desk_fd;
	lifesignal.exit = &doctordata.exit;
	pthread_create(&lifesignal_t, NULL, sendLifeSignal, &lifesignal);
	pthread_create(&fifohandler_t, NULL, FIFOHandlerT, &doctordata);
	while (true)
	{
		if (fgets(command, sizeof(command) - 1, stdin) == NULL)
		{
			fprintf(stderr, "An error occured while trying to read command from stdin\n");
			close(service_desk_fd);
			unlink(dfifo);
			exit(0);
		}
		if (strcmp(command, "exit\n") == 0 || doctordata.exit == true)
			break;
	}
	if ((write(doctordata.fd, "E", 1)) == -1)
	{
		fprintf(stderr, "Couldn't write control character \"E\" to doctor's FIFO\n");
		unlink(dfifo);
		close(service_desk_fd);
		exit(0);
	}
	pthread_join(fifohandler_t, NULL);
	pthread_join(lifesignal_t, NULL);
	unlink(dfifo);
	if(write(service_desk_fd, "E", 1) == -1)
	{
		fprintf(stderr, "Couldn't write control character \"E\" to FIFO\n");
		close(service_desk_fd);
		exit(0);
	}
	if ((write(service_desk_fd, &doctordata.me.pid, sizeof(int)) == -1))
	{
		fprintf(stderr, "Couldn't write exit signal's PID to FIFO\n");
		close(service_desk_fd);
		exit(0);
	}
	close(service_desk_fd);
	printf("Hope to see you soon, %s\n", doctordata.me.name);
	return (0);
}

void	handleSIGINT(int i)
{
	(void) i;
	printf("\nCtrl + c is disabled, please use \"exit\" instead!\n");
}

void	*FIFOHandlerT(void *ptr)
{
	DoctorData	*doctordata = (DoctorData *) ptr;
	char	control;

	while (true)
	{
		if (read(doctordata->fd, &control, 1) == -1)
		{
			fprintf(stderr, "An error occured while trying to read control character\n");
			// error occured
		}
		switch (control)
		{
			case 'Z':
				printf("Received exit signal from service desk, press <Enter> to exit\n");
				doctordata->exit = true;
				return (NULL);
			case 'E':
				doctordata->exit = true;
				return (NULL);
			default:
				// error occured
				break;
		}
	}
	return (NULL);
}
