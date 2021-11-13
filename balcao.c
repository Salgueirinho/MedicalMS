#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


int	main(void)
{
	char	*max_clientes_str;
	char	*max_medicos_str;
  int 	max_clientes;
	int		max_medicos;
  char	comando[20];

	max_clientes_str = getenv("MAXCLIENTES");
	if (max_clientes_str)
	{
		max_clientes = atoi(max_clientes_str);
	}
	else
	{
		printf("Erro ao ler MAXCLIENTES\n");
		return -1;
	}

	max_medicos_str = getenv("MAXMEDICOS");
	if (max_medicos_str)
	{
		max_medicos = atoi(max_medicos_str);
	}
	else
	{
		printf("Erro ao ler MAXMEDICOS\n");
		return -1;
	}

  while(strcmp(str,"encerra") != 0)
	{
    fflush(stdout);
    scanf("%20s",str);
  }
	return 0;
}
