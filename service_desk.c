#include "medical_os.h"

bool	forceexit = false;

int	main(void)
{
	ServerData	serverdata;
	pthread_t		patientqueue_t;
	pthread_t		doctortimer_t;
	pthread_t		fifohandler_t;
	char				command[50];
	int					pid;

	if (serviceDeskIsRunning(getpid()) == true)
	{
		fprintf(stderr, "There is already a service desk running!\n");
		exit(0);
	}
	setSIGINT();
	/*if (serviceDeskIsRunning((int)getpid()) == true)
		{
		fprintf(stderr, "There is already a service desk running!\n");
		exit(0);
		}*/
	if (access(SFIFO, F_OK) == 0)
	{
		fprintf(stderr, "There is already a service desk FIFO open\n");
		exit(0);
	}
	if (mkfifo(SFIFO, 0600) == -1)
	{
		fprintf(stderr, "An error occured while attempting to make FIFO\n");
		exit(0);
	}
	if (pipe(serverdata.s_to_c) == -1)
	{
		unlink(SFIFO);
		fprintf(stderr, "An error occured while attempting to make pipe serverdata.s_to_c\n");
		exit(0);
	}
	if (pipe(serverdata.c_to_s) == -1)
	{
		close(serverdata.s_to_c[0]);
		close(serverdata.s_to_c[1]);
		unlink(SFIFO);
		fprintf(stderr, "An error occured while attempting to make pipe serverdata.c_to_s\n");
		exit(0);
	}
	if ((pid = fork()) == -1)
	{
		close(serverdata.c_to_s[0]);
		close(serverdata.c_to_s[1]);
		close(serverdata.s_to_c[0]);
		close(serverdata.s_to_c[1]);
		unlink(SFIFO);
		fprintf(stderr, "An error occured while attempting to fork\n");
		exit(0);
	}
	if (pid == 0)
		executeClassifier(serverdata.s_to_c, serverdata.c_to_s);
	close(serverdata.s_to_c[0]);
	close(serverdata.c_to_s[1]);
	if ((serverdata.fd = open(SFIFO, O_RDWR)) == -1)
	{
		close(serverdata.s_to_c[1]);
		close(serverdata.c_to_s[0]);
		unlink(SFIFO);
		fprintf(stderr, "An error occured while attempting to open serice desk FIFO\n");
		exit(0);
	}
	serverdata.patientqueue = NULL;
	serverdata.doctorlist = NULL;
	serverdata.freq = 20;
	serverdata.exit = false;
	pthread_create(&doctortimer_t, NULL, doctorTimerT, &serverdata);
	pthread_create(&patientqueue_t, NULL, displayPatientQueueT, &serverdata);
	pthread_create(&fifohandler_t, NULL, FIFOHandlerT, &serverdata);
	while(true)
	{
		if (forceexit == true)
			break;
		if (fgets(command, sizeof(command) - 1, stdin) == NULL)
		{
			close(serverdata.s_to_c[1]);
			close(serverdata.c_to_s[0]);
			unlink(SFIFO);
			fprintf(stderr, "An error occured while trying to read command from stdin\n");
			exit(0);
		}
		if (forceexit == true || strcmp(command, "exit\n") == 0)
		{
			if (write(serverdata.s_to_c[1], "#fim\n", 5) == -1)
				fprintf(stderr, "An error occured while attempting to write \"fim\" to classifier\n");
			// send signal to threads
			serverdata.exit = true;
			// send signal to everyone
			sendExitSignal(&serverdata);
			break;
		}
		else
		{
			if(strncmp(command, "freq ", 5) == 0)
				setFreq(&serverdata.freq, atoi(command + 5));
			else if (strcmp(command, "patients\n") == 0)
				displayPatientQueue(serverdata.patientqueue);
			else if (strcmp(command, "doctors\n") == 0)
				displayDoctorList(serverdata.doctorlist);
			else if (strncmp(command, "delp ", 5) == 0)
				serverdata.patientqueue = removePatient(serverdata.patientqueue, atoi(command + 5));
			else if (strncmp(command, "deld ", 5) == 0)
				serverdata.doctorlist = removeDoctor(serverdata.doctorlist, atoi(command + 5));
		}
	}
	serverdata.freq = 0;
	pthread_join(doctortimer_t, NULL);
	pthread_join(patientqueue_t, NULL);
	pthread_join(fifohandler_t, NULL);
	close(serverdata.s_to_c[1]);
	close(serverdata.c_to_s[0]);
	freePatientQueue(serverdata.patientqueue);
	freeDoctorList(serverdata.doctorlist);
	close(serverdata.fd);
	unlink(SFIFO);
	printf("See you later!\n");
	return (0);
}

void	handleSIGINT(int i)
{
	(void) i;
	printf("\nPress <enter> to exit\n");
	forceexit = true;
}
