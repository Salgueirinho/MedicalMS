#include <stdlib.h>
#include "medico.h"
#include "utils.h"

void	guardaNome(char* nome, pMedico medico){
	medico->nomeMedico = nome;
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
		Medico especialista;
		guardaNome(argv[1], &especialista);
		guardaEspecialidade(argv[2], &especialista);
	}
	return 0;
}
