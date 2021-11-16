#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "balcao.h"
#include <unistd.h>
#include <sys/wait.h>
#include "utils.h"

int	callClassificador(char *sintomas)
{
	int	fd[2];
	int	pid1;
	int	pid2;

	if (pipe(fd) == -1)
	{
		return -1;
	}

	pid1 = fork();
	if (pid1 < 0)
	{
		return -2;
	}
	else if (pid1 == 0)
	{
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		write(1, sintomas, ourStrLen(sintomas));
		write(1, "\n", 1);
		write(1, "#fim", 4);
		write(1, "\n", 1);
	}

	pid2 = fork();
	if (pid2 < 0)
	{
		return -3;
	}
	else if (pid2 == 0)
	{
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
		execl("./classificador", "./classificador", NULL);
	}
	
	close(fd[0]);
	close(fd[1]);
	waitpid(pid2, NULL, 0);
	return 0;
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

int	main(int argc, char *argv[])
{
	ValoresMaximos valoresMaximos;
	//char	comando[20];
	int		i;

	i = 1;

	valoresMaximos.max_clientes = getNumberFromEnv("MAXCLIENTES");
	if (valoresMaximos.max_clientes <= 0)
		return -1;

	valoresMaximos.max_medicos = getNumberFromEnv("MAXMEDICOS");
	if (valoresMaximos.max_medicos <= 0)
		return -2;

	// Primeira meta
	if (argc < 2)
		ourPutString("Nao foram indicados sintomas\n");
	else
	{
		while (i < argc)
		{
			ourPutString("Para os sintomas introduzidos:\n - ");
			ourPutString(argv[i]);
			ourPutString("\nDeduzimos a especialidade e urgencia:\n - ");
			callClassificador(argv[i]);
			i++;
		}
	}
	/*
	
	-- So na segunda meta! --
	
  while(strcmp(comando, "encerra"))
	{
    fflush(stdout);
    scanf("%20s", comando);
  }

	*/
	return 0;
}
