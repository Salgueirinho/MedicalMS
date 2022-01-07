#ifndef PATIENT_H
#define PATIENT_H

#include <stdbool.h>

typedef struct Patient {
	char	name[50];
	char	symptoms[50];
	char	speciality[17];
	int		pid;
} Patient;

typedef struct PatientData {
	Patient	me;
	int			fd;
	bool		exit;
} PatientData;

#endif
