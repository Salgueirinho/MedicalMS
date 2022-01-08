#include "medical_os.h"

// threads

void	*FIFOHandlerT(void *ptr)
{
	ServerData	*serverdata = (ServerData *) ptr;
	Patient			current_patient;
	Doctor			current_doctor;
	char				control;
	int					bytes;

	while (serverdata->exit == false)
	{
		if (read(serverdata->fd, &control , 1) == -1)
		{
			fprintf(stderr, "An error occured while trying to read control character\n");
			serverdata->exit = true;
		}
		switch (control)
		{
			case 'P':
				if ((bytes = read(serverdata->fd, &current_patient, sizeof(Patient))) == -1)
				{
					fprintf(stderr, "Error occured while reading patient\n");
					serverdata->exit = true;
				}
				printf("Succesfully received patient %s, with symptoms %s", current_patient.name, current_patient.symptoms);
				if (write(serverdata->s_to_c[1], current_patient.symptoms, strlen(current_patient.symptoms)) == -1)
				{
					fprintf(stderr, "Error occured while writing symptoms to classifier\n");
					serverdata->exit = true;
				}
				if (read(serverdata->c_to_s[0], current_patient.speciality, sizeof(current_patient.speciality) - 1) == -1)
				{
					fprintf(stderr, "Error occured while reading speciality from classifier\n");
					serverdata->exit = true;
				}
				printf("Succesfully determined speciality %s", current_patient.speciality);
				serverdata->patientqueue = addPatient(serverdata->patientqueue, &current_patient);
				break;
			case 'D':
				if ((bytes = read(serverdata->fd, &current_doctor, sizeof(Doctor))) == -1)
				{
					fprintf(stderr, "Error occured while reading doctor\n");
					serverdata->exit = true;
				}
				printf("Succesfully received doctor %s, with speciality %s\n", current_doctor.name, current_doctor.speciality);
				serverdata->doctorlist = addDoctor(serverdata->doctorlist, &current_doctor);
				break;
			case 'N':
				if ((bytes = read(serverdata->fd, &current_doctor.pid, sizeof(int))) == -1)
				{
					fprintf(stderr, "Error occured while reading life signal\n");
					serverdata->exit = true;
				}
				resetTimer(serverdata->doctorlist, current_doctor.pid);
				printf("Succesfully received life signal from doctor with PID %d\n", current_doctor.pid);
				break;
			case 'E':
				if ((bytes = read(serverdata->fd, &current_doctor.pid, sizeof(int))) == -1)
				{
					fprintf(stderr, "Error occured while reading exit signal from doctor\n");
					serverdata->exit = true;
				}
				serverdata->doctorlist = removeDoctorPID(serverdata->doctorlist, current_doctor.pid);
				printf("Succesfully received exit signal from doctor with PID %d\n", current_doctor.pid);
				break;
			case 'F':
				if ((bytes = read(serverdata->fd, &current_patient.pid, sizeof(int))) == -1)
				{
					fprintf(stderr, "Error occured while reading exit signal from patient\n");
					serverdata->exit = true;
				}
				serverdata->patientqueue = removePatientPID(serverdata->patientqueue, current_patient.pid);
				printf("Succesfully received exit signal from patient with PID %d\n", current_patient.pid);
				break;
			case 'Z':
				break;
			default:
				fprintf(stderr, "Control digit came out corrupt\n");
				// error occured
				break;
		}
	}
	return (NULL);
}

void	*doctorTimerT(void *ptr)
{
	ServerData		*serverdata = (ServerData *) ptr;
	DoctorList		*aux1;
	DoctorList		*aux2;
	int						counter;

	while (serverdata->exit == false)
	{
		aux1 = (DoctorList *) serverdata->doctorlist;
		counter = 0;
		while (aux1 != NULL)
		{
			counter++;
			aux1->timer--;
			aux2 = aux1->next;
			if (aux1->timer <= 0)
			{
				printf("Didn't receive life signal on time for %d\n", aux1->doctor.pid);
				serverdata->doctorlist = removeDoctor(serverdata->doctorlist, counter);
				counter--;
			}
			aux1 = aux2;
		}
		sleep(1);
	}
	return (NULL);
}

void	*displayPatientQueueT(void *ptr)
{
	ServerData		*serverdata = (ServerData *) ptr;
	int						i;

	while (true)
	{
		if (serverdata->exit == true)
			break;
		i = 0;
		while (i < serverdata->freq)
		{
			sleep(1);
			i++;
		}
		if (serverdata->exit == true)
			break;
		printf("Speciality\tPatients\n"
				"geral\t\t%d\n"
				"ortopedia\t%d\n"
				"estomatologia\t%d\n"
				"neurologia\t%d\n"
				"oftalmologia\t%d\n",
				getPatientQueueSize(serverdata->patientqueue, "geral"),
				getPatientQueueSize(serverdata->patientqueue, "ortopedia"),
				getPatientQueueSize(serverdata->patientqueue, "estomatologia"),
				getPatientQueueSize(serverdata->patientqueue, "neurologia"),
				getPatientQueueSize(serverdata->patientqueue, "oftalmologia"));
	}
	return (NULL);
}

// classifier-specific function

void	executeClassifier(int s_to_c[2], int c_to_s[2])
{
	close(s_to_c[1]);
	close(c_to_s[0]);
	close(0);
	if (dup(s_to_c[0]) == -1)
	{
		// do something
	}
	close(1);
	if (dup(c_to_s[1]) == -1)
	{
		// do something
	}
	close(s_to_c[0]);
	close(c_to_s[1]);
	execl("classifier", "classifier", NULL);
	fprintf(stderr, "An error occured while attempting to execute the classifier\n");
	unlink(SFIFO);
	exit(0);
}

// Doctor list

DoctorList	*addDoctor(DoctorList *doctorlist, Doctor *doctor)
{
	DoctorList	*aux = doctorlist;
	char					fifo[20];

	sprintf(fifo, "/tmp/d%d", doctor->pid);
	if (access(fifo, F_OK) == -1)
	{
		fprintf(stderr, "The doctor's FIFO isn't open\n");
		// error occured
	}
	if (doctorlist == NULL)
	{
		doctorlist = (DoctorList *) malloc(sizeof(DoctorList));
		memcpy(&doctorlist->doctor, doctor, sizeof(Doctor));
		doctorlist->next = NULL;
		if ((doctorlist->fd = open(fifo, O_WRONLY)) == -1)
		{
			fprintf(stderr, "An error occured while trying to open FIFO for new doctor\n");
			// error occured
		}
		doctorlist->timer = 20;
	}
	else
	{
		while (aux->next != NULL)
			aux = aux->next;
		aux->next = (DoctorList *) malloc(sizeof(DoctorList));
		memcpy(&aux->next->doctor, doctor, sizeof(Doctor));
		aux->next->next = NULL;
		if ((aux->next->fd = open(fifo, O_WRONLY)) == -1)
		{
			fprintf(stderr, "An error occured while trying to open FIFO for new doctor\n");
			// error occured
		}
		aux->next->timer = 20;
	}
	return (doctorlist);
}

DoctorList	*removeDoctor(DoctorList *doctorlist, int position)
{
	int list_size = getDoctorListSize(doctorlist, "");
	DoctorList	*aux = doctorlist;
	DoctorList	*temp = NULL;

	if(doctorlist != NULL)
	{
		if(position == 1 && list_size >= 1)
		{
			if (doctorlist->busy == 0)
			{
				temp = doctorlist;
				doctorlist = doctorlist->next;
				free(temp);
			}
		}
		else if (position == list_size && position !=0)
		{
			while(aux->next->next != NULL)
				aux= aux->next; 
			if (aux->next->busy == 0)
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
			if (temp->busy == 0)
			{
				aux->next = aux->next->next;
				free(temp);
			}
		}
	}
	return doctorlist;
}

DoctorList	*removeDoctorPID(DoctorList *doctorlist, int pid)
{
	DoctorList	*aux = doctorlist;
	DoctorList	*temp = NULL;

	if(doctorlist != NULL)
	{
		if(doctorlist->doctor.pid == pid)
		{
			temp = doctorlist;
			doctorlist = doctorlist->next;
			free(temp);
		}
		else
		{
			while (aux->next != NULL)
			{
				if (aux->next->doctor.pid == pid)
				{
					temp = aux->next;
					aux->next = aux->next->next;
					free(temp);
					break;
				}
				aux = aux->next;
			}
		}
	}
	return (doctorlist);
}

void	resetTimer(DoctorList *doctorlist, int pid)
{
	DoctorList	*aux = doctorlist;

	while (aux != NULL)
	{
		if (aux->doctor.pid == pid)
		{
			aux->timer = 20;
			return ;
		}
		aux = aux->next;
	}
	fprintf(stderr, "Couldn't find doctor by PID (in order to reset timer)\n");
}

void	freeDoctorList(DoctorList *doctorlist)
{
	DoctorList	*aux = doctorlist;

	while (doctorlist)
	{
		aux = doctorlist;
		doctorlist = doctorlist->next;
		free(aux);
	}
}

int getDoctorListSize(DoctorList *doctorlist, char *speciality)
{
	DoctorList *aux = doctorlist;
	int len = strlen(speciality)-3;
	len = len > 0 ? len : 0;
	int size = 0;

	while(aux != NULL)
	{
		if (strncmp(aux->doctor.speciality, speciality, len) == 0)
			size++;
		aux = aux->next;
	}
	return (size);
}

void	displayDoctorList(DoctorList *doctor_list)
{
	DoctorList	*aux = doctor_list;

	if (aux == NULL)
		fprintf(stderr, "Currently there are no doctors\n");

	while (aux)
	{
		printf("doctor: %s, %s\n", aux->doctor.name,
				aux->doctor.speciality);
		aux = aux->next;
	}
}

// Patient list

PatientQueue	*addPatient(PatientQueue *patientqueue, Patient *patient)
{
	PatientQueue	*aux = patientqueue;
	char					fifo[20];
	int						size;

	sprintf(fifo, "/tmp/p%d", patient->pid);
	if (access(fifo, F_OK) == -1)
	{
		fprintf(stderr, "The patient's FIFO isn't open\n");
		// error occured
	}
	size = strlen(patient->speciality);
	if (patientqueue == NULL)
	{
		patientqueue = (PatientQueue *) malloc(sizeof(PatientQueue));
		memcpy(&patientqueue->patient, patient, sizeof(Patient));
		if ((patientqueue->fd = open(fifo, O_WRONLY)) == -1)
		{
			fprintf(stderr, "An error occured while trying to open FIFO for new patient\n");
			// error occured
		}
		if (write(patientqueue->fd, &size, sizeof(int)) != sizeof(int))
		{
			fprintf(stderr, "An error occured while trying to write size to patient's FIFO\n");
			// error occured
		}
		if (write(patientqueue->fd, patientqueue->patient.speciality, size) == -1)
		{
			fprintf(stderr, "An error occured while trying to write speciality to patient's FIFO\n");
			// error occured
		}
		size = 0;
		if (write(patientqueue->fd, &size, sizeof(int)) == -1)
		{
			fprintf(stderr, "An error occured while trying to write queue size to patient's FIFO\n");
			// error occured
		}
		patientqueue->next = NULL;
	}
	else
	{
		while (aux->next != NULL)
			aux = aux->next;
		aux->next = (PatientQueue *) malloc(sizeof(PatientQueue));
		memcpy(&aux->next->patient, patient, sizeof(Patient));
		if ((aux->next->fd = open(fifo, O_WRONLY)) == -1)
		{
			fprintf(stderr, "An error occured while trying to open FIFO for new patient\n");
			// error occured
		}
		if (write(aux->next->fd, &size, sizeof(int)) != sizeof(int))
		{
			fprintf(stderr, "An error occured while trying to write size to patient's FIFO\n");
			// error occured
		}
		if (write(aux->next->fd, aux->next->patient.speciality, size) == -1)
		{
			fprintf(stderr, "An error occured while trying to write speciality to patient's FIFO\n");
			// error occured
		}
		size = getQueueInFrontOfPatient(patientqueue, *patient) - 1;
		if (write(aux->next->fd, &size, sizeof(int)) == -1)
		{
			fprintf(stderr, "An error occured while trying to write queue size to patient's FIFO\n");
			// error occured
		}
		aux->next->next = NULL;
	}
	return (patientqueue);
}

PatientQueue *removePatient(PatientQueue  *patientqueue, int position)
{
	int queue_size = getPatientQueueSize(patientqueue, "");
	PatientQueue  *aux = patientqueue;
	PatientQueue  *temp = NULL;

	if(patientqueue != NULL)
	{
		if(position == 1 && queue_size >= 1)
		{
			temp = patientqueue;
			patientqueue = patientqueue->next;
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
	return patientqueue;
}

PatientQueue	*removePatientPID(PatientQueue *patientqueue, int pid)
{
	PatientQueue	*aux = patientqueue;
	PatientQueue	*temp = NULL;

	if(patientqueue != NULL)
	{
		if(patientqueue->patient.pid == pid)
		{
			temp = patientqueue;
			patientqueue = patientqueue->next;
			free(temp);
		}
		else
		{
			while (aux->next != NULL)
			{
				if (aux->next->patient.pid == pid)
				{
					temp = aux->next;
					aux->next = aux->next->next;
					free(temp);
					break;
				}
				aux = aux->next;
			}
		}
	}
	return (patientqueue);
}

void	freePatientQueue(PatientQueue *patientqueue)
{
	PatientQueue	*aux = patientqueue;

	while (patientqueue)
	{
		aux = patientqueue;
		patientqueue = patientqueue->next;
		free(aux);
	}
}

int getPatientQueueSize(PatientQueue *patientqueue, char *speciality)
{
	int size = 0;
	PatientQueue *aux = patientqueue;
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

void	displayPatientQueue(PatientQueue *patientqueue)
{
	PatientQueue	*aux = patientqueue;

	if (aux == NULL)
		fprintf(stderr, "There are currently no patients in queue\n");

	while (aux)
	{
		printf("patient: %s, %s", aux->patient.name,
				aux->patient.speciality);
		aux = aux->next;
	}
}

int getPatientPriority(Patient patient)
{
	for (int i=0; patient.speciality[i]; i++)
		if (patient.speciality[i+1] == '\n')
			return (patient.speciality[i] - '0'); 
	return (3);
}

int	getQueueInFrontOfPatient(PatientQueue *patientqueue, Patient patient)
{
	PatientQueue *aux = patientqueue;
	int len = strlen(patient.speciality)-3;
	len = len > 0 ? len : 0;
	int size = 0;

	while(aux)
	{
		if (strncmp(aux->patient.speciality, patient.speciality, len) == 0
				&& getPatientPriority(patient) <= getPatientPriority(aux->patient))
			size++;
		aux = aux->next;
	}
	return (size);
}

// miscellaneous

void	setFreq(int *freq, int value)
{
	if (0 < value)
	{
		printf("Setting freq to %d\n", value);
		*freq = value;
	}
	else
		fprintf(stderr, "Error: argument has non-positive number\n");
}

void	sendExitSignal(ServerData *serverdata)
{
	PatientQueue	*p_aux = serverdata->patientqueue;
	DoctorList	*d_aux = serverdata->doctorlist;

	while (p_aux)
	{
		if (write(p_aux->fd, "Z", 1) == -1)
			fprintf(stderr, "Couldn't write exit signal to patient with PID %d's FIFO\n", p_aux->patient.pid);
		p_aux = p_aux->next;
	}
	while (d_aux)
	{
		if (write(d_aux->fd, "Z", 1) == -1)
			fprintf(stderr, "Couldn't write exit signal to doctor with PID %d's FIFO\n", d_aux->doctor.pid);
		d_aux = d_aux->next;
	}
	if (write(serverdata->fd, "Z", 1) == -1)
		fprintf(stderr, "Couldn't write exit signal to service desk FIFO\n");
}
