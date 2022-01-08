#ifndef PATIENT_H
#define PATIENT_H

#include <stdbool.h>

typedef struct Patient {
	char	speciality[17];
	char	symptoms[50];
	char	name[50];
	int		pid;
} Patient;

typedef struct PatientData {
	Patient	me;
	int			fd;
	bool		exit;
} PatientData;

#endif
