#ifndef PATIENT_H
#define PATIENT_H

#define patientfifo "/tmp/p%dfifo"

#include <stdbool.h>

typedef struct SharedPData {
	char		name[50];
	int			sfd;
	int			pfd;
	int			dfd;
	bool		exit;
	bool		appointment;
} SharedPData;

#endif
