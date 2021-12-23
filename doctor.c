#include <stdlib.h>
#include <unistd.h>
#include "doctor.h"
#include "utils.h"

void	registerDoctor(char* name, char* speciality, Doctor *doctor)
{
	doctor->speciality = speciality;
	doctor->name = name;
}

int	main(int argc, char *argv[])
{
	if (serviceDeskIsRunning(0) == false)
	{
		putString("The service desk isn't running\n", STDERR_FILENO);
		exit(0);
	}
	else if (argc < 3)
	{
		putString("Too few arguments provided\n", STDERR_FILENO);
		exit(0);
	}
	else
	{
		Doctor	doctor;
		registerDoctor(argv[1], argv[2], &doctor);
	}
	return (0);
}
