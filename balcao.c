#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include "utils.h"
#include "balcao.h"

static int	callClassificador(void)
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
		if (write(2, "Erro ao executar Classificador\n", 31) == -1)
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
			ourPutString("[admin] sintomas: ");
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

static int	getNumberFromEnv(const char *env_name)
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
		printf("Erro ao ler variavel de ambiente $(%s)\n", env_name);
	return -1;
}

static int	getMax(const char *name, const int val)
{
	int	n = getNumberFromEnv(name);
	return n > 0 ? n : val;
}

static void	setMaxValues(pValoresMaximos valoresMaximos)
{
	valoresMaximos->max_clientes = getMax("MAXCLIENTES", MAX_CLIENTES_DEFAULT);
	valoresMaximos->max_medicos = getMax("MAXMEDICOS", MAX_MEDICOS_DEFAULT);
	valoresMaximos->max_lugares = MAX_FILA;
	valoresMaximos->max_especialidades = MAX_ESPECIALIDADES_DEFAULT;
}

static void interpretCommand(const char *comando)
{
	if (strcmp(comando, "utentes") == 0)
		printf("O sistema de momento tem %d utentes.\n", 0);
	else if (strcmp(comando, "especialistas") == 0)
		printf("O sistema de momento tem %d especialistas.\n", 0);
	else if (strncmp(comando, "delut ", 6) == 0)
		printf("Este comando ainda nao faz nada!\n");
	else if (strncmp(comando, "delesp ", 7) == 0)
		printf("Este comando ainda nao faz nada!\n");
	else if (strncmp(comando, "freq ", 5) == 0)
		printf("Este comando ainda nao faz nada!\n");
	else if (strcmp(comando, "encerra") == 0)
		printf("O sistema vai encerrar dentro de momentos.\n");
	else
		printf("Comando invalido!\n");
}

int	main(void)
{
	ValoresMaximos valoresMaximos;
	char	comando[40];
	int	bytes_read;

	if (balcaoIsRunning((int) getpid()))
	{
		printf("Ja existe um balcao em execucao!\n");
		return (-1);
	}

	setMaxValues(&valoresMaximos);
	while (strcmp(comando, "encerra") != 0)
	{
		ourPutString("[admin] comando: ");
		bytes_read = read(0, comando, 40);
		if (bytes_read == -1)
		{
			printf("Ocorreu um erro ao ler o comando!\n");
			return (-1);
		}
		comando[bytes_read - 1] = '\0';
		if (strcmp(comando, "sintomas") == 0)
			callClassificador();
		else
			interpretCommand(comando);
	}
	return 0;
}
