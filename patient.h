#ifndef PATIENT_H
#define PATIENT_H

#define PFIFO "/tmp/p%d"

typedef struct	Patient {
  char	name[50];
  char	symptoms[50];
  int		pid;
	char	speciality[17];
} Patient;

#endif
