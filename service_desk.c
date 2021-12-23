#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "service_desk.h"
#include "utils.h"

static int	callClassifier(void)
{
	char	symptoms[40];
	char	speciality[40];
	int		p1[2] = {-1, -1};
	int		p2[2] = {-1, -1};
	int		pid = -1;
	int		bytes = -1;

	if (pipe(p1) == -1)
		exit(0);
	if (pipe(p2) == -1)
		exit(0);
	pid = fork();
	if (pid == -1)
		exit(0);
	else if (pid == 0)
	{
		/*
			Child process - executes the classifier and is partially responsible for
			the I/O of the two unnamed pipes
		*/
		close(p1[1]);
		close(p2[0]);
		close(0);
		dup(p1[0]);
		close(1);
		dup(p2[1]);
		close(p1[0]);
		close(p2[1]);
		execl("classifier", "classifier", NULL);
		putString("An error occured while attempting to execute the classifier\n",
			STDERR_FILENO);
		exit(0);
	}
	else
	{
		/*
			Parent process - sends symptoms and receives the speciality and urgency
			from the classifier
		*/
		close(p1[0]);
		close(p2[1]);
		do
		{
			bytes = 0;
			putString("[admin] symptoms: ", STDOUT_FILENO);
			if ((bytes = read(0, &symptoms, sizeof(symptoms) - 1)) == -1)
				exit(0);
			symptoms[bytes] = '\0';
			bytes = 0;
			if (write(p1[1], symptoms, strlen(symptoms)) == -1)
				exit(0);
			if (strcmp(symptoms, "#fim\n") != 0)
			{
				if ((bytes = read(p2[0], speciality, sizeof(speciality) - 1)) == -1)
					exit(0);
				speciality[bytes] = '\0';
				putString(speciality, STDOUT_FILENO);
			}
		} while (strcmp(symptoms, "#fim\n") != 0);
		close(p1[1]);
		close(p2[0]);
	}
	return (0);
}

/*
static int	getNumberFromEnv(const char *env_name, int default_value)
{
	int		value = -1;
	char	*s = NULL;

	s = getenv(env_name);
	if (s)
		value = atoi(s);
	return (value > 0 ? value : default_value);
}
*/

int	main(void)
{
	// MaxValues	max_values = {0, 0, 0, 0};
	char			command[40] = "";
	int				bytes = -1;

	if (serviceDeskIsRunning((int) getpid()) == true)
	{
		putString("There is already a service desk running!\n", STDERR_FILENO);
		exit(0);
	}
	if (signal(SIGINT, handleSIGINT) == SIG_ERR)
	{
		putString("It wasn't possible to configure SIGINT\n", STDERR_FILENO);
		exit(0);
	}
	/*
	max_values.max_patients = getNumberFromEnv("MAXPATIENTS", MAX_PATIENTS);
	max_values.max_doctors = getNumberFromEnv("MAXDOCTORS", MAX_DOCTORS);
	max_values.max_specialties = MAX_SPECIALITIES;
	max_values.max_line = MAX_LINE;
	*/
	while (strcmp(command, "exit") != 0)
	{
		putString("[admin] command: ", STDOUT_FILENO);
		bytes = read(0, command, sizeof(command) - 1);
		if (bytes == -1)
		{
			putString("An error occured while trying to read command!\n",
				STDERR_FILENO);
			exit(0);
		}
		command[bytes - 1] = '\0';
		if (strcmp(command, "symptoms") == 0)
			callClassifier();
	}
	return (0);
}
