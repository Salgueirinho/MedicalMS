#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "service_desk.h"
#include "patient.h"
#include "utils.h"

void registerPatient(Patient *patient)
{
	int	bytes = -1;

	putString("Your symptoms: ", STDOUT_FILENO);
	if ((bytes = read(0, patient->symptoms, 255)) == -1)
	{
		putString("Couldn't read patient's symptoms\n", STDERR_FILENO);
		exit(0);
	}
	patient->symptoms[bytes - 1] = '\n';
	patient->symptoms[bytes] = '\0';
}

int main(int argc, char* argv[])
{
	Patient	patient;
	int	fd;

	if (serviceDeskIsRunning(0) == false)
	{
		putString("The service desk isn't running\n", STDERR_FILENO);
		exit(0);
	}
	else if (argc < 2)
	{
		putString(argv[0], STDERR_FILENO);
		putString(" <name>\n", STDERR_FILENO);
		exit(0);
	}
  if (access(SERV_FIFO, F_OK) != 0)
	{
		putString("Error, service desk FIFO doesn't exist\n", STDERR_FILENO);
		exit(0);
  }
	strncpy(patient.name, argv[1], sizeof(patient.name));
	registerPatient(&patient);

  if (!(fd = open(SERV_FIFO, O_WRONLY)))
	{
		putString("Couldn't open named pipe file\n", STDERR_FILENO);
		exit(0);
	}
	if (write(fd, &patient, sizeof(Patient)) == -1)
	{
		putString("Couldn't write to named pipe\n", STDERR_FILENO);
		close(fd);
		exit(0);
	}
	close(fd);
	return (0);
}
