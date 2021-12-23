#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "doctor.h"
#include "utils.h"

int	main(int argc, char *argv[])
{
	if (serviceDeskIsRunning(0) == false)
	{
		putString("The service desk isn't running\n", STDERR_FILENO);
		exit(0);
	}
	else if (argc < 3)
	{
		putString(argv[0], STDERR_FILENO);
		putString(" <name> <speciality>\n", STDERR_FILENO);
		exit(0);
	}
	else
	{
		Doctor	doctor;
		strncpy(doctor.name, argv[1], sizeof(doctor.name));
		strncpy(doctor.speciality, argv[2], sizeof(doctor.speciality));
	}
	return (0);
}
