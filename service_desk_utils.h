#ifndef SERVICE_DESK_UTILS_H
#define SERVICE_DESK_UTILS_H

#include "service_desk.h"

void	executeClassifier(int s_to_c[2], int c_to_s[2]);
void	executeCommand(char *command, SharedSData *shared_data);
void	*readFIFO(void *data_ptr);
void	freePatientQueue(PatientQueue *patientqueue);
void	freeDoctorList(DoctorList *doctorlist);
void	*displayQueue(void *data_ptr);
void	*doctorTimer(void *data_ptr);
void	setMaxQueueSize(int *maxqueuesize);
void	setMaxDoctors(int *maxdoctors);
void	*appointmentHandler(void *data_ptr);
void	handleSIGINT(int i);
bool serviceDeskIsRunning(int pid);

#endif
