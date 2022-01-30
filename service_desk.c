#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "service_desk_utils.h"
#include "service_desk.h"
#include "patient.h"
#include "doctor.h"
#include "utils.h"

int	main(void)
{
	SharedSData	shared_data;
	pthread_t		read_worker;
	pthread_t		queue_worker;
	pthread_t		doctor_worker;
	pthread_t		appointment_worker;
	char				command[50];
	int					pid;

	if (serviceDeskIsRunning(getpid()))
	{
		fprintf(stderr, "There is already a service desk running!\n");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGINT, handleSIGINT) == SIG_ERR)
	{
		fprintf(stderr, "It wasn't possible to configure SIGINT\n");
		exit(EXIT_FAILURE);
	}
	if (mkfifo(SFIFO, 0600) == -1)
	{
		fprintf(stderr, "Couldn't mkfifo\n");
		exit(EXIT_FAILURE);
	}
	if (pipe(shared_data.s_to_c) == -1)
	{
		unlink(SFIFO);
		fprintf(stderr, "An error occured while attempting to make pipe shared_data.s_to_c\n");
		exit(EXIT_FAILURE);
	}
	if (pipe(shared_data.c_to_s) == -1)
	{
		close(shared_data.s_to_c[0]);
		close(shared_data.s_to_c[1]);
		unlink(SFIFO);
		fprintf(stderr, "An error occured while attempting to make pipe shared_data.c_to_s\n");
		exit(EXIT_FAILURE);
	}
	if ((pid = fork()) == -1)
	{
		close(shared_data.c_to_s[0]);
		close(shared_data.c_to_s[1]);
		close(shared_data.s_to_c[0]);
		close(shared_data.s_to_c[1]);
		unlink(SFIFO);
		fprintf(stderr, "An error occured while attempting to fork\n");
		exit(EXIT_FAILURE);
	}
	if (pid == 0)
		executeClassifier(shared_data.s_to_c, shared_data.c_to_s);
	close(shared_data.s_to_c[0]);
	close(shared_data.c_to_s[1]);
	shared_data.exit = false;
	shared_data.patientqueue = NULL;
	shared_data.doctorlist = NULL;
	shared_data.freq = 30;
	setMaxQueueSize(&shared_data.maxqueuesize);
	setMaxDoctors(&shared_data.maxdoctors);
	if ((shared_data.sfd = open(SFIFO, O_RDWR)) == -1)
	{
		fprintf(stderr, "Couldn't open sfifo\n");
		unlink(SFIFO);
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&read_worker, NULL, readFIFO, &shared_data) != 0)
	{
		fprintf(stderr, "Couldn't create read worker\n");
		shared_data.exit = true;
	}
	if (pthread_create(&queue_worker, NULL, displayQueue, &shared_data) != 0)
	{
		fprintf(stderr, "Couldn't create queue worker\n");
		shared_data.exit = true;
	}
	if (pthread_create(&doctor_worker, NULL, doctorTimer, &shared_data) != 0)
	{
		fprintf(stderr, "Couldn't create doctor worker\n");
		shared_data.exit = true;
	}
	if (pthread_create(&appointment_worker, NULL, appointmentHandler, &shared_data) != 0)
	{
		fprintf(stderr, "Couldn't create doctor worker\n");
		shared_data.exit = true;
	}
	while (shared_data.exit == false)
	{
		if (fgets(command, sizeof(command) - 1, stdin) == NULL)
		{
			fprintf(stderr, "Couldn't get command from stdin\n");
		}
		executeCommand(command, &shared_data);
	}
	freePatientQueue(shared_data.patientqueue);
	freeDoctorList(shared_data.doctorlist);
	if (pthread_join(doctor_worker, NULL) != 0)
	{
		fprintf(stderr, "Couldn't join doctor worker\n");
		close(shared_data.sfd);
		unlink(SFIFO);
		exit(EXIT_FAILURE);
	}
	if (pthread_join(queue_worker, NULL) != 0)
	{
		fprintf(stderr, "Couldn't join queue worker\n");
		close(shared_data.sfd);
		unlink(SFIFO);
		exit(EXIT_FAILURE);
	}
	if (pthread_join(read_worker, NULL) != 0)
	{
		fprintf(stderr, "Couldn't join read worker\n");
		close(shared_data.sfd);
		unlink(SFIFO);
		exit(EXIT_FAILURE);
	}
	close(shared_data.sfd);
	unlink(SFIFO);
	return (0);
}
