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

PatientList *removePatientFromQueue(PatientList * patient_queue, int position);
DoctorList* removeDoctor(DoctorList * doctor_list, int position);
int getPatientQueueSize(PatientList *patient_queue, char *speciality);
int getPatientPriority(Patient patient);
void	freePatientList(PatientList *patient_queue);
void	displayPatientList(PatientList *patient_queue);
PatientList	*addPatient(PatientList *patient_queue, Patient *patient);
void	freeDoctorList(DoctorList *doctor_list);
void	displayDoctorList(DoctorList *doctor_list);
DoctorList	*addDoctor(DoctorList *doctor_list, Doctor *doctor);
static void	executeClassifier(int p1[2], int p2[2]);

int main(void)
{
	Patient	patient = {"default", "", 0, "geral"};
	Doctor	doctor = {"default", "", 0, false};
	PatientList	*patient_queue = NULL;
	DoctorList	*doctor_list = NULL;
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
			fgets(command, sizeof(command), stdin);
			if (strcmp(command, "exit\n") == 0)
				break;
			else if (strcmp(command, "patients\n") == 0)
				displayPatientList(patient_queue);
			else if (strcmp(command, "doctors\n") == 0)
				displayDoctorList(doctor_list);
			else if (strncmp(command, "delp ", 5) == 0)
				patient_queue = removePatientFromQueue(patient_queue, atoi(command + 5));
			else if (strncmp(command, "deld ", 5) == 0)
				doctor_list = removeDoctor(doctor_list, atoi(command + 5));
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
					doctor_list = addDoctor(doctor_list, &doctor);
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
						if(getPatientQueueSize(patient_queue, patient.speciality) >= 5)
						{
							printf("Queue is full\n");
						}
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
						patient_queue = addPatient(patient_queue, &patient);
						printf("Registered patient:\n"
								"- name: %s\n"
								"- symptoms: %s"
								"- pid: %d\n"
								"- speciality: %s",
								patient.name, patient.symptoms, patient.pid,patient.speciality);
						printf("Patient queue size for this speciality: %d\n",
								getPatientQueueSize(patient_queue, patient.speciality));
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
	freePatientList(patient_queue);
	freeDoctorList(doctor_list);
	return (0);
}

int getPatientQueueSize(PatientList *patient_queue, char *speciality)
{
	int size = 0;
	PatientList *aux = patient_queue;
	int len = strlen(speciality)-3;
	len = len > 0 ? len : 0;

	while(aux != NULL)
	{
		if (strncmp(aux->patient.speciality, speciality, len) == 0)
			size++;
		aux = aux->next;
	}
	return (size);
}

int getPatientPriority(Patient patient)
{
	for (int i=0; patient.speciality[i]; i++)\
		if (patient.speciality[i+1] == '\n')
			return patient.speciality[i] - '0'; 
	return (3);
}

DoctorList	*addDoctor(DoctorList *doctor_list, Doctor *doctor)
{
	DoctorList	*aux = doctor_list;

	if (doctor_list == NULL)
	{
		doctor_list = (DoctorList *) malloc(sizeof(DoctorList));
		memcpy(&doctor_list->doctor, doctor, sizeof(Doctor));
		doctor_list->next = NULL;
	}
	else
	{
		while (aux->next != NULL)
			aux = aux->next;
		aux->next = (DoctorList *) malloc(sizeof(DoctorList));
		memcpy(&aux->next->doctor, doctor, sizeof(Doctor));
		aux->next->next = NULL;
	}
	return (doctor_list);
}

void	freeDoctorList(DoctorList *doctor_list)
{
	DoctorList	*aux = doctor_list;

	while (doctor_list)
	{
		aux = doctor_list;
		doctor_list = doctor_list->next;
		free(aux);
	}
}

void	displayDoctorList(DoctorList *doctor_list)
{
	DoctorList	*aux = doctor_list;

	while (aux)
	{
		printf("doctor: %s, %s\n", aux->doctor.name,
				aux->doctor.speciality);
		aux = aux->next;
	}
}

PatientList	*addPatient(PatientList *patient_queue, Patient *patient)
{
	PatientList	*aux = patient_queue;

	if (patient_queue == NULL)
	{
		patient_queue = (PatientList *) malloc(sizeof(PatientList));
		memcpy(&patient_queue->patient, patient, sizeof(Patient));
		patient_queue->next = NULL;
	}
	else
	{
		while (aux->next != NULL)
			aux = aux->next;
		aux->next = (PatientList *) malloc(sizeof(PatientList));
		memcpy(&aux->next->patient, patient, sizeof(Patient));
		aux->next->next = NULL;
	}
	return (patient_queue);
}

void	freePatientList(PatientList *patient_queue)
{
	PatientList	*aux = patient_queue;

	while (patient_queue)
	{
		aux = patient_queue;
		patient_queue = patient_queue->next;
		free(aux);
	}
}

void	displayPatientList(PatientList *patient_queue)
{
	PatientList	*aux = patient_queue;

	while (aux)
	{
		printf("patient: %s, %s", aux->patient.name,
				aux->patient.speciality);
		aux = aux->next;
	}
}

PatientList* removePatientFromQueue(PatientList * patient_queue, int position)
{
	int queue_size = getPatientQueueSize(patient_queue, "");
	PatientList  *aux = patient_queue;
	PatientList * temp = NULL;

	if(patient_queue != NULL)
	{
		if(position == 1 && queue_size >= 1)
		{
			temp = patient_queue;
			patient_queue = patient_queue->next;
			free(temp);
		}
		else if (position == queue_size && position !=0)
		{
			while(aux->next->next != NULL)
				aux= aux->next; 
			free(aux->next);
			aux->next = NULL; 
		}
		else if(1 < position && position < queue_size)
		{
			for(int i=0;i<position-2;i++)
			{
				aux = aux->next;
			}
			temp = aux->next;
			aux->next = aux->next->next;
			free(temp);
		}
	}
	return patient_queue;
}

int getDoctorListSize(DoctorList *doctor_list, char *speciality)
{
	int size = 0;
	DoctorList *aux = doctor_list;
	int len = strlen(speciality)-3;
	len = len > 0 ? len : 0;

	while(aux != NULL)
	{
		if (strncmp(aux->doctor.speciality, speciality, len) == 0)
			size++;
		aux = aux->next;
	}
	return (size);
}

DoctorList* removeDoctor(DoctorList * doctor_list, int position)
{
	int list_size = getDoctorListSize(doctor_list, "");
	DoctorList	*aux = doctor_list;
	DoctorList	*temp = NULL;

	if(doctor_list != NULL)
	{
		if(position == 1 && list_size >= 1)
		{
			if (doctor_list->doctor.busy == 0)
			{
				temp = doctor_list;
				doctor_list = doctor_list->next;
				free(temp);
			}
		}
		else if (position == list_size && position !=0)
		{
			while(aux->next->next != NULL)
				aux= aux->next; 
			if (aux->next->doctor.busy == 0)
			{
				free(aux->next);
				aux->next = NULL; 
			}
		}
		else if(1 < position && position < list_size)
		{
			for(int i=0; i<position-2; i++)
				aux = aux->next;
			temp = aux->next;
			if (temp->doctor.busy == 0)
			{
				aux->next = aux->next->next;
				free(temp);
			}
		}
	}
	return doctor_list;
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
