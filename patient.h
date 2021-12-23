#ifndef PATIENT_H
#define PATIENT_H

typedef struct Patient {
	char name[255];
	char symptoms[255];
	int	pid;
} Patient;

#endif
