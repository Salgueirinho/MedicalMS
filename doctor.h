#ifndef DOCTOR_H
#define DOCTOR_H

#define doctorfifo "/tmp/d%dfifo"

#include <stdbool.h>

typedef struct SharedDData {
	char		name[50];
	int			sfd;
	int			dfd;
	int			pfd;
	int			pid;
	bool		exit;
	bool		appointment;
} SharedDData;

#endif
