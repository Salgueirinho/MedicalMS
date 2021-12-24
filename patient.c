#include "patient.h"
#include "service_desk.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void registerPatient(Patient *patient) {
  int bytes = -1;

  putString("Your symptoms: ", STDOUT_FILENO);
  if ((bytes = read(0, patient->symptoms, 255)) == -1) {
    putString("Couldn't read patient's symptoms\n", STDERR_FILENO);
    exit(0);
  }
  patient->symptoms[bytes - 1] = '\n';
  patient->symptoms[bytes] = '\0';
}

int main(int argc, char *argv[]) {
  Patient patient;
  int fd, fdp;
  char pat_fifo[15], speciality[40] = "";

  if (serviceDeskIsRunning(0) == false) {
    putString("The service desk isn't running\n", STDERR_FILENO);
    exit(0);
  } else if (argc < 2) {
    putString(argv[0], STDERR_FILENO);
    putString(" <name>\n", STDERR_FILENO);
    exit(0);
  }

  patient.pid = getpid();
  sprintf(pat_fifo, PAT_FIFO, patient.pid);

  if (mkfifo(pat_fifo, 0600) == -1) {
    putString("Error occured while trying to make FIFO\n", STDERR_FILENO);
    exit(0);
  }

  if (access(SERV_FIFO, F_OK) != 0) {
    putString("Error, service desk FIFO doesn't exist\n", STDERR_FILENO);
    exit(0);
  }

  strncpy(patient.name, argv[1], sizeof(patient.name));
  registerPatient(&patient);

  if (!(fd = open(SERV_FIFO, O_WRONLY))) {
    putString("Couldn't open named pipe file\n", STDERR_FILENO);
    exit(0);
  }
  if (write(fd, &patient, sizeof(Patient)) == -1) {
    putString("Couldn't write to named pipe\n", STDERR_FILENO);
    close(fd);
    exit(0);
  }
  fdp = open(pat_fifo, O_RDONLY);
  if (read(fdp, speciality, sizeof(speciality) - 1) == -1) {
    exit(0);
  }

  putString(speciality, STDIN_FILENO);
  close(fdp);
  close(fd);
  unlink(pat_fifo);
  return (0);
}
