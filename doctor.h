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
	int		s_fd;
	bool	*exit;
} LifeSignal;

typedef struct DoctorData {
	Doctor	me;
	int			fd;
	bool		exit;
	bool		attending;
	int			p_fd;
} DoctorData;

#endif
