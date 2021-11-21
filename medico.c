#include <stdlib.h>
#include "medico.h"
#include "utils.h"

void	guardaNome(char* nome, pMedico medico){
	medico->nome_medico = nome;
}

void	guardaEspecialidade(char *especialidade, pMedico medico)
{
	medico->especialidade = especialidade;
}

int	main(int argc, char *argv[])
{
	if (argc < 3)
	{
		ourPutString("Faltam argumentos de entrada: necessario nome e especialidade do Medico.\n");
		exit(-1);
	}
	else
	{
		if (!balcaoIsRunning(0))
		{
			ourPutString("O Balcao nao esta em execucao, logo o Medico tambem nao podera iniciar!\n");
			return (-1);
		}
		Medico especialista;
		guardaNome(argv[1], &especialista);
		guardaEspecialidade(argv[2], &especialista);
	}
	return 0;
}
