#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include "service_desk.h"

void	removePatientPos(PatientQueue **patientqueue, int pos)
{
	PatientQueue	*temp = *patientqueue;
	PatientQueue	*prev = NULL;
	int					counter = 1;

	if (pos <= 0)
	{
		fprintf(stderr, "Please insert a positive position\n");
		return ;
	}
	if (temp != NULL && pos == 1)
	{
		*patientqueue = temp->next;
		if (temp->attending == -1)
		{
			if (write(temp->pfd, "E", 1) == -1)
			{
				fprintf(stderr, "Couldn't write E to pfd\n");
			}
		}
		close(temp->pfd);
		free(temp);
		return ;
	}
	else
	{
		while (temp != NULL && counter != pos)
		{
			prev = temp;
			temp = temp->next;
			counter++;
		}
		if (temp == NULL)
		{
			fprintf(stderr, "Position %d out of bounds\n", pos);
			return ;
		}
		if (temp->attending == -1)
		{
			if (write(temp->pfd, "E", 1) == -1)
			{
				fprintf(stderr, "Couldn't write E to pfd\n");
			}
		}
		close(temp->pfd);
		prev->next = temp->next;
		free(temp);
	}
}

void	setDoctorAvailable(DoctorList *doctorlist, int pid)
{
	DoctorList	*aux = doctorlist;

	if (pid <= 0)
	{
		fprintf(stderr, "Non-positive PID's are invalid\n");
		return ;
	}
	while (aux)
	{
		if (aux->pid == pid)
		{
			aux->attending = -1;
			return ;
		}
		aux = aux->next;
	}
	if (!aux)
	{
		fprintf(stderr, "PID wasn't in doctorlist\n");
	}
}

int		getQueueInFront(PatientQueue *patientqueue, PatientQueue *patient)
{
	PatientQueue	*patient_aux = patientqueue;
	int	counter = -1;

	while (patient_aux)
	{
		if (strncmp(patient_aux->speciality, patient->speciality, strlen(patient_aux->speciality) - 2) == 0)
		{
			if (patient->speciality[strlen(patient->speciality) - 2]
					<= patient_aux->speciality[strlen(patient_aux->speciality) - 2])
				counter++;
		}
		patient_aux = patient_aux->next;
	}
	return (counter);
}

void	handleSIGINT(int i)
{
	(void) i;
	unlink(SFIFO);
	exit(EXIT_SUCCESS);
}

void	*appointmentHandler(void *data_ptr)
{
	SharedSData	*shared_data = data_ptr;
	DoctorList	*doctor_aux;
	PatientQueue	*patient_aux;
	char					outbound_message[50];
	int	i;
	int	j;

	while (shared_data->exit == false)
	{
		doctor_aux = shared_data->doctorlist;
		patient_aux = shared_data->patientqueue;
		i = 0;
		j = 0;
		while (doctor_aux)
		{
			while (patient_aux)
			{
				if (strncmp(patient_aux->speciality, doctor_aux->speciality, strlen(doctor_aux->speciality) - 1) == 0
						&& patient_aux->attending == -1
						&& doctor_aux->attending == -1)
				{
					patient_aux->attending = i;
					doctor_aux->attending = j;
					sprintf(outbound_message, "A%d", doctor_aux->pid);
					if (write(patient_aux->pfd, outbound_message, strlen(outbound_message)) == -1)
					{
						fprintf(stderr, "Couldn't write A to pfd\n");
					}
					sprintf(outbound_message, "A%d", patient_aux->pid);
					if (write(doctor_aux->dfd, outbound_message, strlen(outbound_message)) == -1)
					{
						fprintf(stderr, "Couldn't write A to dfd");
					}
					printf("dr. %s and %s are a match\n", doctor_aux->name, patient_aux->name);
					removePatientPos(&shared_data->patientqueue, j + 1);
				}
				j++;
				patient_aux = patient_aux->next;
			}
			i++;
			doctor_aux = doctor_aux->next;
		}
		sleep(1);
	}
	return (NULL);
}

void	setMaxDoctors(int *maxdoctors)
{
	char	*env = getenv("MAXDOCTORS");

	if (env == NULL)
	{
		fprintf(stderr, "Couldn't find environment variable MAXDOCTORS\n");
		*maxdoctors = MAXDOCTORS;
	}
	else
	{
		*maxdoctors = atoi(env);
		if (0 >= *maxdoctors)
		{
			fprintf(stderr, "Value set through environment variable MAXDOCTORS "
					"is non-positive, resorting to default\n");
			*maxdoctors = MAXDOCTORS;
		}
	}
}

void	setMaxQueueSize(int *maxqueuesize)
{
	char	*env = getenv("MAXQUEUESIZE");

	if (env == NULL)
	{
		fprintf(stderr, "Couldn't find environment variable MAXQUEUESIZE\n");
		*maxqueuesize = MAXQUEUESIZE;
	}
	else
	{
		*maxqueuesize = atoi(env);
		if (0 >= *maxqueuesize)
		{
			fprintf(stderr, "Value set through environment variable MAXQUEUESIZE "
					"is non-positive, resorting to default\n");
			*maxqueuesize = MAXQUEUESIZE;
		}
	}
}

void	resetTimer(SharedSData *shared_data, int pid)
{
	DoctorList	*aux = shared_data->doctorlist;

	if (pid <= 0)
	{
		fprintf(stderr, "PID should be a positive number!\n");
		return ;
	}
	if (shared_data->doctorlist->pid == pid)
	{
		shared_data->doctorlist->timer = 20;
		printf("Succesfully received life signal for %s\n", aux->name);
	}
	else
	{
		while (aux != NULL)
		{
			if (aux->pid == pid)
			{
				printf("Succesfully received life signal for %s\n", aux->name);
				aux->timer = 20;
				return ;
			}
			aux = aux->next;
		}
		fprintf(stderr, "Couldn't find doctor with PID %d\n", pid);
	}
}

int		getQueueSize(PatientQueue *patientqueue, char *speciality)
{
	PatientQueue	*aux = patientqueue;
	int	counter = 0;

	while (aux)
	{
		if (strncmp(aux->speciality, speciality, strlen(speciality)) == 0)
			counter++;
		aux = aux->next;
	}
	return (counter);
}

void	*displayQueue(void *data_ptr)
{
	SharedSData	*shared_data = data_ptr;
	int	counter = 0;

	while (shared_data->exit == false)
	{
		sleep(1);
		counter++;
		if (counter >= shared_data->freq)
		{
			printf("Oftalmologia: %d\n", getQueueSize(shared_data->patientqueue, "oftalmologia"));
			printf("Neurologia: %d\n", getQueueSize(shared_data->patientqueue, "neurologia"));
			printf("Estomatologia: %d\n", getQueueSize(shared_data->patientqueue, "estomatologia"));
			printf("Ortopedia: %d\n", getQueueSize(shared_data->patientqueue, "ortopedia"));
			printf("Geral: %d\n", getQueueSize(shared_data->patientqueue, "geral"));
			counter = 0;
		}
	}
	return (NULL);
}

void	removePatientPID(PatientQueue **patientqueue, int pid)
{
	PatientQueue	*temp = *patientqueue;
	PatientQueue	*prev = NULL;

	if (temp != NULL && temp->pid == pid)
	{
		*patientqueue = temp->next;
		close(temp->pfd);
		free(temp);
		return ;
	}
	else
	{
		while (temp != NULL && temp->pid != pid)
		{
			prev = temp;
			temp = temp->next;
		}
		if (temp == NULL)
		{
			return ;
		}
		close(temp->pfd);
		prev->next = temp->next;
		free(temp);
	}
}

void	removeDoctorPID(DoctorList **doctorlist, int pid)
{
	DoctorList	*temp = *doctorlist;
	DoctorList	*prev = NULL;

	if (temp != NULL && temp->pid == pid)
	{
		*doctorlist = temp->next;
		close(temp->dfd);
		free(temp);
		return ;
	}
	else
	{
		while (temp != NULL && temp->pid != pid)
		{
			prev = temp;
			temp = temp->next;
		}
		if (temp == NULL)
		{
			fprintf(stderr, "%d isn't in doctorlist\n", pid);
			return ;
		}
		close(temp->dfd);
		prev->next = temp->next;
		free(temp);
	}
}

void	removeDoctorPos(DoctorList **doctorlist, int pos)
{
	DoctorList	*temp = *doctorlist;
	DoctorList	*prev = NULL;
	int					counter = 1;

	if (pos <= 0)
	{
		fprintf(stderr, "Please insert a positive position\n");
		return ;
	}
	if (temp != NULL && pos == 1)
	{
		*doctorlist = temp->next;
		if (write(temp->dfd, "E", 1) == -1)
		{
			fprintf(stderr, "Couldn't write E to dfd\n");
		}
		close(temp->dfd);
		free(temp);
		return ;
	}
	else
	{
		while (temp != NULL && counter != pos)
		{
			prev = temp;
			temp = temp->next;
			counter++;
		}
		if (temp == NULL)
		{
			fprintf(stderr, "Position %d out of bounds\n", pos);
			return ;
		}
		if (write(temp->dfd, "E", 1) == -1)
		{
			fprintf(stderr, "Couldn't write E to dfd\n");
		}
		close(temp->dfd);
		prev->next = temp->next;
		free(temp);
	}
}

void	*doctorTimer(void *data_ptr)
{
	SharedSData	*shared_data = data_ptr;
	DoctorList	*aux;

	while (shared_data->exit == false)
	{
		aux = shared_data->doctorlist;
		sleep(1);
		while (aux)
		{
			aux->timer--;
			if (aux->timer <= 0)
			{
				fprintf(stderr, "Didn't get life signal from %s on time.\n", aux->name);
				removeDoctorPID(&shared_data->doctorlist, aux->pid);
			}
			aux = aux->next;
		}
	}
	return (NULL);
}

static void	parseDoctorInfo(DoctorList *doctornode, char *doctorinfo)
{
	int	i = 0;
	int	j = 0;
	int	k = 0;

	doctornode->pid = atoi(doctorinfo);
	while (doctorinfo[i])
	{
		if (doctorinfo[i] == '\n')
			break;
		i++;
	}
	j = i + 1;
	while (doctorinfo[j])
	{
		if (doctorinfo[j] == '\n')
		{
			strncpy(doctornode->name, doctorinfo + i + 1, j - i - 1);
			break;
		}
		j++;
	}
	k = j + 1;
	while (doctorinfo[k])
	{
		if (doctorinfo[k] == '\n')
		{
			strncpy(doctornode->speciality, doctorinfo + j + 1, k - j);
			break;
		}
		k++;
	}
}

static void	parsePatientInfo(PatientQueue *patientnode, char *patientinfo)
{
	int	i = 0;
	int	j = 0;
	int	k = 0;

	patientnode->pid = atoi(patientinfo);
	while (patientinfo[i])
	{
		if (patientinfo[i] == '\n')
			break;
		i++;
	}
	j = i + 1;
	while (patientinfo[j])
	{
		if (patientinfo[j] == '\n')
		{
			strncpy(patientnode->name, patientinfo + i + 1, j - i - 1);
			break;
		}
		j++;
	}
	k = j + 1;
	while (patientinfo[k])
	{
		if (patientinfo[k] == '\n')
		{
			strncpy(patientnode->symptoms, patientinfo + j + 1, k - j);
			break;
		}
		k++;
	}
}

PatientQueue	*addPatient(SharedSData *shared_data, char *patientinfo)
{
	PatientQueue	*aux = shared_data->patientqueue;
	char	outbound_message[255];
	char	pfifo[50];
	int		bytes;

	if (shared_data->patientqueue == NULL)
	{
		shared_data->patientqueue = calloc(1, sizeof(PatientQueue));
		if (!shared_data->patientqueue)
		{
			fprintf(stderr, "Couldn't allocate patientqueue\n");
			shared_data->exit = true;
			return (NULL);
		}
		parsePatientInfo(shared_data->patientqueue, patientinfo);
		shared_data->patientqueue->next = NULL;
		shared_data->patientqueue->attending = -1;
		printf("Registering new patient:\n"
				"Name: %s\n"
				"Symptoms: %s",
				shared_data->patientqueue->name,
				shared_data->patientqueue->symptoms);
		if (write(shared_data->s_to_c[1], shared_data->patientqueue->symptoms, strlen(shared_data->patientqueue->symptoms)) == -1)
		{
			fprintf(stderr, "Error occured while writing symptoms to classifier\n");
			shared_data->exit = true;
			return (shared_data->patientqueue);
		}
		if ((bytes = read(shared_data->c_to_s[0], shared_data->patientqueue->speciality, sizeof(shared_data->patientqueue->speciality) - 1)) == -1)
		{
			fprintf(stderr, "Error occured while reading speciality from classifier\n");
			shared_data->exit = true;
			return (shared_data->patientqueue);
		}
		shared_data->patientqueue->speciality[bytes] = '\0';
		sprintf(pfifo, "/tmp/p%d", shared_data->patientqueue->pid);
		if ((shared_data->patientqueue->pfd = open(pfifo, O_WRONLY)) == -1)
		{
			fprintf(stderr, "Error occured while opening patient file descriptor\n");
			shared_data->exit = true;
			return (shared_data->patientqueue);
		}
		sprintf(outbound_message,
				"Your speciality: %s"
				"Queue size: %d\n",
				shared_data->patientqueue->speciality,
				0);
		if (write(shared_data->patientqueue->pfd, outbound_message, strlen(outbound_message)) == -1)
		{
			fprintf(stderr, "Error occured while writing speciality to patient file descriptor\n");
			shared_data->exit = true;
			return (shared_data->patientqueue);
		}
	}
	else
	{
		while (aux->next != NULL)
			aux = aux->next;
		aux->next = calloc(1, sizeof(PatientQueue));
		if (!aux->next)
		{
			fprintf(stderr, "Couldn't allocate patientqueue\n");
			shared_data->exit = true;
			return (shared_data->patientqueue);
		}
		parsePatientInfo(aux->next, patientinfo);
		aux->next->next = NULL;
		aux->next->attending = -1;
		printf("Registering new patient:\n"
				"Name: %s\n"
				"Symptoms: %s",
				aux->next->name,
				aux->next->symptoms);
		if (write(shared_data->s_to_c[1], aux->next->symptoms, strlen(aux->next->symptoms)) == -1)
		{
			fprintf(stderr, "Error occured while writing symptoms to classifier\n");
			shared_data->exit = true;
			return (shared_data->patientqueue);
		}
		if ((bytes = read(shared_data->c_to_s[0], aux->next->speciality, sizeof(aux->next->speciality) - 1)) == -1)
		{
			fprintf(stderr, "Error occured while reading speciality from classifier\n");
			shared_data->exit = true;
			return (shared_data->patientqueue);
		}
		aux->next->speciality[bytes] = '\0';
		sprintf(pfifo, "/tmp/p%d", aux->next->pid);
		if ((aux->next->pfd = open(pfifo, O_WRONLY)) == -1)
		{
			fprintf(stderr, "Error occured while opening patient file descriptor\n");
			shared_data->exit = true;
			return (shared_data->patientqueue);
		}
		if (getQueueSize(shared_data->patientqueue, aux->next->speciality) > shared_data->maxqueuesize)
		{
			fprintf(stderr, "Maximum queue size exceeded\n");
			sprintf(outbound_message, "M");
			if (write(aux->next->pfd, outbound_message, strlen(outbound_message)) == -1)
			{
				fprintf(stderr, "Error occured while writing speciality to patient file descriptor\n");
				shared_data->exit = true;
				return (shared_data->patientqueue);
			}
			close(aux->next->pfd);
			removePatientPID(&shared_data->patientqueue, aux->next->pid);
		}
		else
		{
			sprintf(outbound_message,
					"Your speciality: %s"
					"Queue size: %d\n",
					aux->next->speciality,
					getQueueInFront(shared_data->patientqueue, aux->next));
			if (write(aux->next->pfd, outbound_message, strlen(outbound_message)) == -1)
			{
				fprintf(stderr, "Error occured while writing speciality to patient file descriptor\n");
				shared_data->exit = true;
				return (shared_data->patientqueue);
			}
		}
	}
	return (shared_data->patientqueue);
}

DoctorList	*addDoctor(SharedSData *shared_data, char *doctorinfo)
{
	DoctorList	*aux = shared_data->doctorlist;
	char	dfifo[50];

	if (shared_data->doctorlist == NULL)
	{
		shared_data->doctorlist = calloc(1, sizeof(DoctorList));
		if (!shared_data->doctorlist)
		{
			fprintf(stderr, "Couldn't allocate doctorlist\n");
			shared_data->exit = true;
			return (NULL);
		}
		parseDoctorInfo(shared_data->doctorlist, doctorinfo);
		shared_data->doctorlist->next = NULL;
		shared_data->doctorlist->attending = -1;
		shared_data->doctorlist->timer = 20;
		printf("Registering new doctor:\n"
				"Name: %s\n"
				"Speciality: %s",
				shared_data->doctorlist->name,
				shared_data->doctorlist->speciality);
		sprintf(dfifo, "/tmp/d%d", shared_data->doctorlist->pid);
		if ((shared_data->doctorlist->dfd = open(dfifo, O_WRONLY)) == -1)
		{
			fprintf(stderr, "Error occured while opening doctor file descriptor\n");
			shared_data->exit = true;
			return (shared_data->doctorlist);
		}
	}
	else
	{
		while (aux->next != NULL)
			aux = aux->next;
		aux->next = calloc(1, sizeof(DoctorList));
		if (!aux->next)
		{
			fprintf(stderr, "Couldn't allocate doctorlist\n");
			shared_data->exit = true;
			return (shared_data->doctorlist);
		}
		parseDoctorInfo(aux->next, doctorinfo);
		aux->next->next = NULL;
		aux->next->attending = -1;
		aux->next->timer = 20;
		printf("Registering new doctor:\n"
				"Name: %s\n"
				"Speciality: %s",
				aux->next->name,
				aux->next->speciality);
		sprintf(dfifo, "/tmp/d%d", aux->next->pid);
		if ((aux->next->dfd = open(dfifo, O_WRONLY)) == -1)
		{
			fprintf(stderr, "Error occured while opening doctor file descriptor\n");
			shared_data->exit = true;
			return (shared_data->doctorlist);
		}
	}
	return (shared_data->doctorlist);
}

void	freeDoctorList(DoctorList *doctorlist)
{
	DoctorList	*aux = doctorlist;

	while (doctorlist)
	{
		aux = doctorlist->next;
		free(doctorlist);
		doctorlist = aux;
	}
}

void	freePatientQueue(PatientQueue *patientqueue)
{
	PatientQueue	*aux = patientqueue;

	while (patientqueue)
	{
		aux = patientqueue->next;
		free(patientqueue);
		patientqueue = aux;
	}
}

void	*readFIFO(void *data_ptr)
{
	SharedSData *shared_data = data_ptr;
	char	incoming_message[255];
	int		bytes;

	while (shared_data->exit == false)
	{
		if ((bytes = read(shared_data->sfd, incoming_message,
						sizeof(incoming_message) - 1)) == -1)
		{
			fprintf(stderr, "Couldn't read from sfd\n");
		}
		incoming_message[bytes] = '\0';
		switch (incoming_message[0])
		{
			case 'P':
				shared_data->patientqueue =
					addPatient(shared_data, incoming_message + 1);
				break;
			case 'D':
				shared_data->doctorlist =
					addDoctor(shared_data, incoming_message + 1);
				break;
			case 'S':
				resetTimer(shared_data, atoi(incoming_message + 1));
				break;
			case 'F':
				removeDoctorPID(&shared_data->doctorlist, atoi(incoming_message + 1));
				break;
			case 'G':
				removePatientPID(&shared_data->patientqueue, atoi(incoming_message + 1));
				break;
			case 'A':
				setDoctorAvailable(shared_data->doctorlist, atoi(incoming_message + 1));
				break;
			case 'E':
				shared_data->exit = true;
				return (NULL);
		}
	}
	return (NULL);
}

void	executeClassifier(int s_to_c[2], int c_to_s[2])
{
	close(s_to_c[1]);
	close(c_to_s[0]);
	close(0);
	if (dup(s_to_c[0]) == -1)
	{
		fprintf(stderr, "Couldn't dup s_to_c[0]\n");
	}
	close(1);
	if (dup(c_to_s[1]) == -1)
	{
		fprintf(stderr, "Couldn't dup c_to_s[1]\n");
	}
	close(s_to_c[0]);
	close(c_to_s[1]);
	execl("classifier", "classifier", NULL);
	fprintf(stderr, "An error occured while attempting to execute the classifier\n");
	unlink(SFIFO);
	exit(0);
}

void	setFreq(char *s, SharedSData *shared_data)
{
	int	val = atoi(s);

	if (0 < val)
		shared_data->freq = val;
	else
		fprintf(stderr, "Please insert a positive value\n");
}

void	executeCommand(char *command, SharedSData *shared_data)
{
	PatientQueue	*patient_aux = shared_data->patientqueue;
	DoctorList		*doctor_aux = shared_data->doctorlist;

	if (strcmp(command, "exit\n") == 0)
	{
		shared_data->exit = true;
		if (write(shared_data->s_to_c[1], "#fim\n", 5) == -1)
		{
			fprintf(stderr, "Couldn't write #fim\\n to classifier\n");
		}
		while (patient_aux)
		{
			if (write(patient_aux->pfd, "E", 1) == -1)
			{
				fprintf(stderr, "Couldn't write E to pfd\n");
			}
			close(patient_aux->pfd);
			patient_aux = patient_aux->next;
		}
		while (doctor_aux)
		{
			if (write(doctor_aux->dfd, "E", 1) == -1)
			{
				fprintf(stderr, "Couldn't write E to pfd\n");
			}
			close(doctor_aux->dfd);
			doctor_aux = doctor_aux->next;
		}
		if (write(shared_data->sfd, "E", 1) == -1)
		{
			fprintf(stderr, "Couldn't write E to sfd\n");
			close(shared_data->sfd);
			unlink(SFIFO);
			exit(EXIT_FAILURE);
		}
	}
	else if (strcmp(command, "patients\n") == 0)
	{
		while (patient_aux != NULL)
		{
			if (patient_aux->attending == -1)
				printf("\nName: %s\n"
						"Speciality and priority: %s"
						"Status: in line\n\n",
						patient_aux->name,
						patient_aux->speciality);
			else
				printf("Name: %s\n"
						"Speciality and priority: %s"
						"Being attended by doctor %d\n",
						patient_aux->name,
						patient_aux->speciality,
						patient_aux->attending);
			patient_aux = patient_aux->next;
		}
	}
	else if (strcmp(command, "doctors\n") == 0)
	{
		while (doctor_aux != NULL)
		{
			if (doctor_aux->attending == -1)
				printf("\nName: %s\n"
						"Speciality: %s"
						"Status: waiting\n\n",
						doctor_aux->name,
						doctor_aux->speciality);
			else
				printf("Name: %s\n"
						"Speciality: %s"
						"Attending patient: %d\n",
						doctor_aux->name,
						doctor_aux->speciality,
						doctor_aux->attending);
			doctor_aux = doctor_aux->next;
		}
	}
	else if (strncmp(command, "freq ", 5) == 0)
	{
		setFreq(command + 4, shared_data);
	}
	else if (strncmp(command, "delp ", 5) == 0)
	{
		removePatientPos(&shared_data->patientqueue, atoi(command + 4));
	}
	else if (strncmp(command, "deld ", 5) == 0)
	{
		removeDoctorPos(&shared_data->doctorlist, atoi(command + 4));
	}
}
