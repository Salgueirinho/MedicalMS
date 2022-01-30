#ifndef SERVICE_DESK_H
#define SERVICE_DESK_H

#define SFIFO "/tmp/sfifo"

#include <stdbool.h>

#define MAXDOCTORS 5
#define MAXQUEUESIZE 5

typedef struct PatientQueue {
	char	name[50];
	char	symptoms[50];
	char	speciality[17];
	int		pid;
	int		pfd;
	int		attending;
	struct PatientQueue *next;
} PatientQueue;

typedef struct DoctorList {
	char	name[50];
	char	speciality[50];
	int		pid;
	int		dfd;
	int		attending;
	int		timer;
	struct DoctorList	*next;
} DoctorList;

typedef struct SharedSData {
	PatientQueue	*patientqueue;
	DoctorList		*doctorlist;
	int		maxqueuesize;
	int		maxdoctors;
	int		sfd;
	int		s_to_c[2];
	int		c_to_s[2];
	int		freq;
	bool	exit;
} SharedSData;

#endif
