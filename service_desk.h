#ifndef SERVICE_DESK_H
#define SERVICE_DESK_H

#define	SFIFO "/tmp/SFIFO"

#define MAX_SPECIALITIES 5
#define MAX_PATIENTS 5
#define MAX_DOCTORS 5
#define MAX_LINE 5

typedef struct	MaxValues {
	int	max_patients;	
	int	max_doctors;
	int	max_specialties;
	int	max_line;
} MaxValues;

#endif
