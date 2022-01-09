#include "medical_os.h"

int	main(int argc, char *argv[])
{
	DoctorData	doctordata;
	LifeSignal	lifesignal;
	pthread_t		lifesignal_t, fifohandler_t;
	char				dfifo[20];
	char				message[255];
	int					s_fd;

	if (argc < 3)
	{
		printf("%s <name> <speciality>\n", argv[0]);
		exit(0);
	}
	setSIGINT();
	if (serviceDeskIsRunning(0) == false)
		{
		fprintf(stderr, "The service desk isn't running!\n");
		exit(0);
	}
	strncpy(doctordata.me.name, argv[1], sizeof(doctordata.me.name) - 1);
	strncpy(doctordata.me.speciality, argv[2], sizeof(doctordata.me.speciality) - 1);
	doctordata.me.pid = getpid();
	doctordata.exit = false;
	doctordata.attending = false;
	doctordata.p_fd = -1;
	sprintf(dfifo, "/tmp/d%d", doctordata.me.pid);
	if ((s_fd = open(SFIFO, O_RDWR)) == -1)
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
	if (write(s_fd, "D", 1) == -1)
	{
		fprintf(stderr, "Couldn't write to named pipe\n");
		close(s_fd);
		unlink(dfifo);
		exit(0);
	}
	if (write(s_fd, &doctordata.me, sizeof(Doctor)) == -1)
	{
		fprintf(stderr, "Couldn't write to named pipe\n");
		close(s_fd);
		unlink(dfifo);
		exit(0);
	}
	lifesignal.pid = doctordata.me.pid;
	lifesignal.s_fd = s_fd;
	lifesignal.exit = &doctordata.exit;
	pthread_create(&lifesignal_t, NULL, sendLifeSignal, &lifesignal);
	pthread_create(&fifohandler_t, NULL, FIFOHandlerT, &doctordata);
	while (true)
	{
		if (fgets(message, sizeof(message) - 1, stdin) == NULL)
		{
			fprintf(stderr, "An error occured while trying to read message from stdin\n");
			close(s_fd);
			unlink(dfifo);
			exit(0);
		}
		if (strcmp(message, "exit\n") == 0)
			doctordata.exit = true;
		if (doctordata.exit == true)
			break;
		if (doctordata.attending == true)
		{
			if (doctordata.p_fd != -1)
				if (write(doctordata.p_fd, message, strlen(message)) == -1)
				{
					fprintf(stderr, "Couldn't write to doctor FIFO\n");
					close(s_fd);
					unlink(dfifo);
					exit(0);
				}
		}
	}
	if ((write(doctordata.fd, "E", 1)) == -1)
	{
		fprintf(stderr, "Couldn't write control character \"E\" to doctor's FIFO\n");
		unlink(dfifo);
		close(s_fd);
		exit(0);
	}
	pthread_join(fifohandler_t, NULL);
	pthread_join(lifesignal_t, NULL);
	unlink(dfifo);
	if(write(s_fd, "E", 1) == -1)
	{
		fprintf(stderr, "Couldn't write control character \"E\" to FIFO\n");
		close(s_fd);
		exit(0);
	}
	if ((write(s_fd, &doctordata.me.pid, sizeof(int)) == -1))
	{
		fprintf(stderr, "Couldn't write exit signal's PID to FIFO\n");
		close(s_fd);
		exit(0);
	}
	close(s_fd);
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
	char	patientfifo[20];
	int		pid;
	char	control;
	int		bytes;
	char	message[255];

	while (doctordata->exit == false)
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
			case 'S':
				doctordata->attending = true;
				if (read(doctordata->fd, &pid, sizeof(int)) == -1)
				{
					fprintf(stderr, "Couldn't read patient PID for appointment");
					doctordata->exit = true;
					return (NULL);
				}
				sprintf(patientfifo, "/tmp/p%d", pid);
				if ((doctordata->p_fd = open(patientfifo, O_WRONLY)) == -1)
				{
					fprintf(stderr, "Couldn't open patient fifo for appointment");
					doctordata->exit = true;
					return (NULL);
				}
				printf("Starting appointment\n");
				while (doctordata->exit == false)
				{
					if ((bytes = read(doctordata->fd, message, sizeof(message) - 1)) == -1)
					{
						fprintf(stderr, "Couldn't read incoming message\n");
						doctordata->exit = true;
						return (NULL);
					}
					message[bytes] = '\0';
					printf("Patient: %s", message);
				}
				return (NULL);
			default:
				// error occured
				break;
		}
	}
	return (NULL);
}
