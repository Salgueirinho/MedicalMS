#include <unistd.h>
#include <stdlib.h>
#include "patient.h"
#include "utils.h"

void registerPatient(char *name, Patient *patient)
{
	int	bytes = -1;

	patient->name = name;
	putString("Your symptoms: ", STDOUT_FILENO);
	if ((bytes = read(0, patient->symptoms, 255)) == -1)
	{
		putString("Couldn't read patient's symptoms\n", STDERR_FILENO);
		exit(0);
	}
	patient->symptoms[bytes - 1] = '\0';
}

int main(int argc, char* argv[])
{
	if (serviceDeskIsRunning(0) == false)
	{
		putString("The service desk isn't running\n", STDERR_FILENO);
		exit(0);
	}
	else if (argc < 2)
	{
		putString("Too few arguments provided\n", STDERR_FILENO);
		exit(0);
	}
	else
	{
		Patient	patient;
		registerPatient(argv[1], &patient);
	}
	return (0);
}
