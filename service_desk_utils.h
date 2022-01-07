#ifndef SERVICE_DESK_UTILS
#define SERVICE_DESK_UTILS

#include "service_desk.h"

// threads

void	*doctorTimerT(void *ptr);
void	*displayPatientQueueT(void *ptr);

// doctor list

DoctorList	*addDoctor(DoctorList *doctorlist, Doctor *doctor);
DoctorList	*removeDoctor(DoctorList *doctorlist, int position);
DoctorList	*removeDoctorPID(DoctorList *doctorlist, int pid);
void				freeDoctorList(DoctorList *doctorlist);
int					getDoctorListSize(DoctorList *doctorlist, char *speciality);
void				displayDoctorList(DoctorList *doctor_list);
void				resetTimer(DoctorList *doctorlist, int pid);

// patient list

PatientQueue	*addPatient(PatientQueue *patientqueue, Patient *patient);
PatientQueue	*removePatient(PatientQueue *patientqueue, int position);
PatientQueue	*removePatientPID(PatientQueue *patientqueue, int pid);
int						getPatientQueueSize(PatientQueue *patientqueue, char *speciality);
void					freePatientQueue(PatientQueue *patientqueue);
void					displayPatientQueue(PatientQueue *patientqueue);

// miscellaneous

void	executeClassifier(int s_to_c[2], int c_to_s[2]);
void	setFreq(int *freq, int value);
void	sendExitSignal(ServerData *serverdata);

#endif
