#ifndef DOCTOR_H
#define DOCTOR_H

#define DFIFO "/tmp/d%d"

typedef struct	Doctor {
	char	name[50];
	char	speciality[50];
	int		pid;
	int		busy;
} Doctor;

typedef struct	DoctorList {
	Doctor	doctor;
	struct DoctorList	*next;
} DoctorList;
#endif
