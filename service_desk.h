#ifndef SERVICE_DESK_H
#define SERVICE_DESK_H

#include "patient.h"
#include "doctor.h"

#define SFIFO "/tmp/SFIFO"

typedef struct PatientQueue {
	Patient	patient;
	int			fd;
	struct PatientQueue	*next;
} PatientQueue;

typedef struct DoctorList {
	Doctor	doctor;
	int			timer;
	bool		busy;
	int			fd;
	struct DoctorList	*next;
} DoctorList;

typedef struct ServerData {
	PatientQueue	*patientqueue;
	DoctorList		*doctorlist;
	int						s_to_c[2];
	int						c_to_s[2];
	int						freq;
	int						fd;
	bool					exit;
} ServerData;

#endif
