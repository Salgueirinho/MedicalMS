#ifndef DOCTOR_H
#define DOCTOR_H

#include <stdbool.h>

typedef struct Doctor {
	char	name[50];
	char	speciality[50];
	int		pid;
} Doctor;

typedef struct LifeSignal {
	int		pid;
	int		service_desk_fd;
	bool	*exit;
} LifeSignal;

typedef struct DoctorData {
	Doctor	me;
	int			fd;
	bool		exit;
} DoctorData;

#endif
