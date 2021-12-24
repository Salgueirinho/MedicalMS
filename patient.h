#ifndef PATIENT_H
#define PATIENT_H

#define PAT_FIFO "/tmp/p%d"

typedef struct Patient {
  char	name[255];
  char	symptoms[255];
  int		pid;
} Patient;

#endif
