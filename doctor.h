#ifndef DOCTOR_H
#define DOCTOR_H

#define DFIFO "/tmp/d%d"

typedef struct Signal
{
  int pid;
  int fd;
} Signal;

typedef struct	Doctor 
{
	char	name[50];
	char	speciality[50];
	char	signal[40];
	int		pid;
	int		busy;
  int   timer;
} Doctor;

typedef struct	DoctorList 
{
	Doctor	doctor;
	struct DoctorList	*next;
} DoctorList;
#endif
