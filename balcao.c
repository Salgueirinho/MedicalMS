#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include "utils.h"
#include "balcao.h"

int	callClassificador(void)
{
	char	sintomas[40];
	char	especialidade[40];
	int		p1[2];
	int		p2[2];
	int		pid;
	int		bytes_read;

	if (pipe(p1) == -1)
		return (1);
	if (pipe(p2) == -1)
		return (2);
	pid = fork();
	if (pid == -1)
		return (3);
	else if (pid == 0)
	{
		// child process - reads from classificador
		close(p1[1]);
		close(p2[0]);
		close(0);
		dup(p1[0]);
		close(1);
		dup(p2[1]);
		close(p1[0]);
		close(p2[1]);
		execl("classificador", "classificador", NULL);
		// if it gets here it's because something went wrong.
		if (write(2, "Couldn't start up classificador\n", 32) == -1)
			return (4);
		exit(1);
	}
	else
	{
		// parent process - writes to classificador
		close(p1[0]);
		close(p2[1]);
		do
		{
			bytes_read = 0;
			fgets(sintomas, sizeof(sintomas) - 1, stdin);
			if (write(p1[1], sintomas, strlen(sintomas)) == -1)
				return (5);
			if (strcmp(sintomas, "#fim\n") != 0)
			{
				bytes_read = read(p2[0], especialidade, sizeof(especialidade) - 1);
				//if (bytes_read = -1)
				//	return (6);
				especialidade[bytes_read - 1] = '\0';
				//fflush(stdout);
				printf("%s\n", especialidade);
			}
		} while (strcmp(sintomas, "#fim\n") != 0);
		close(p1[1]);
		close(p2[0]);
	}
	return (0);
}

int	getNumberFromEnv(char *env_name)
{
	char *value_str;
	int	value;

	value_str = getenv(env_name);
	if (value_str)
	{
		value = atoi(value_str);
		if (value <= 0)
			printf("%s toma um valor nao positivo\n", env_name);
		else
			return value;
	}
	else
		printf("Erro ao ler %s\n", env_name);
	return -1;
}

int	main(void)
{
	ValoresMaximos valoresMaximos;
	//char	comando[20];

	valoresMaximos.max_clientes = getNumberFromEnv("MAXCLIENTES");
	if (valoresMaximos.max_clientes <= 0)
		return -1;

	valoresMaximos.max_medicos = getNumberFromEnv("MAXMEDICOS");
	if (valoresMaximos.max_medicos <= 0)
		return -2;

	callClassificador();
	return 0;
}
