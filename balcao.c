#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "balcao.h"

int	getNumberFromEnv(char *env_name)
{
	char *value_str;
	int	value;

	value_str = getenv(env_name);
	if (value_str)
	{
		value = atoi(value_str);
		if (value <= 0)
		{
			printf("%s toma um valor nao positivo\n", env_name);
			return -1;
		}
		else
		{
			return value;
		}
	}
	else
	{
		printf("Erro ao ler %s\n", env_name);
		return -1;
	}
}

int	main(void)
{
	char	comando[20];
	ValoresMaximos valoresMaximos;

	valoresMaximos.max_clientes = getNumberFromEnv("MAXCLIENTES");
	if (valoresMaximos.max_clientes <= 0)
		return -1;

	valoresMaximos.max_medicos = getNumberFromEnv("MAXMEDICOS");
	if (valoresMaximos.max_medicos <= 0)
		return -2;

  while(strcmp(comando, "encerra"))
	{
    fflush(stdout);
    scanf("%20s", comando);
  }
	return 0;
}
