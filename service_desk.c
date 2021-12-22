#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "service_desk.h"

static int	callClassifier(void)
{
	char	symptoms[40];
	char	speciality[40];
	int		p1[2];
	int		p2[2];
	int		pid;
	int		bytes;

	if (pipe(p1) == -1)
		return (1);
	if (pipe(p2) == -1)
		return (2);
	pid = fork();
	if (pid == -1)
		return (3);
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
		/*
			If during the execution of the code, the program reaches this point,
			it is because an error occured while attempting to execute the
			classifier
		*/
		putString("An error occured while attempting to execute the classifier\n",
			STDERR_FILENO);
		return (5);
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
				return (6);
			symptoms[bytes] = '\0';
			bytes = 0;
			if (write(p1[1], symptoms, strlen(symptoms)) == -1)
				return (7);
			if (strcmp(symptoms, "#fim\n") != 0)
			{
				if ((bytes = read(p2[0], speciality, sizeof(speciality) - 1)) == -1)
					return (8);
				speciality[bytes] = '\0';
				putString(speciality, STDOUT_FILENO);
			}
		} while (strcmp(symptoms, "#fim\n") != 0);
		close(p1[1]);
		close(p2[0]);
	}
	return (0);
}

static int	getNumberFromEnv(const char *env_name)
{
	char *value_str;
	int	value;

	value_str = getenv(env_name);
	if (value_str)
	{
		value = atoi(value_str);
		if (value <= 0)
		{
			putString(env_name, STDERR_FILENO);
			putString("toma um valor não positivo\n", STDERR_FILENO);
		}
		else
			return (value);
	}
	else
	{
		putString("An error occured while trying to get $(", STDERR_FILENO);
		putString(env_name, STDERR_FILENO);
		putString(") from the local variables\n", STDERR_FILENO);
	}
	return (-1);
}

static int	getMax(const char *name, const int default_value)
{
	int	n = getNumberFromEnv(name);
	return (n > 0 ? n : default_value);
}

static void	setMaxValues(MaxValues *max_values)
{
	max_values->max_patients = getMax("MAXPATIENTS", MAX_PATIENTS);
	max_values->max_doctors = getMax("MAXDOCTORS", MAX_DOCTORS);
	max_values->max_line = MAX_LINE;
	max_values->max_specialties = MAX_SPECIALITIES;
}

int	main(void)
{
	MaxValues	max_values;
	char	command[40];
	int		bytes;

	if (serviceDeskIsRunning((int) getpid()) == true)
	{
		putString("There is already a service desk running!\n", STDERR_FILENO);
		exit(0);
	}
	setMaxValues(&max_values);
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
