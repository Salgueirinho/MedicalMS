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
	char	str[40];
	int		canal[2];
	int		process;

	pipe(canal);
	process = fork();
	if (process == 0)
	{
		// child process
		close(0);
		dup(canal[0]);
		close(canal[0]);
		close(canal[1]);
		execl("classificador", "classificador", NULL);
		// if it gets here it's because something went wrong.
		write(2, "Couldn't start up classificador\n", 32);
		exit(1);
	}
	close(canal[0]);
	while (strcmp(str, "#fim\n") != 0)
	{
		// parent process
		fflush(stdout);
		fgets(str, sizeof(str) - 1, stdin);
		write(canal[1], str, strlen(str));
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
