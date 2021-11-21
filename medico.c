/*

	 Trabalho pratico realizado por:
	 - Goncalo Salgueirinho - a2020142627@isec.pt
	 - Kylix Afonso - a2020146228@isec.pt
	 Docente responsavel pela unidade curricular:
	 - prof. Joao Duraes
	 Unidade curricular:
	 - Sistemas Operativos
	 Insituto Superior de Engenharia de Coimbra

*/

#include <stdlib.h>
#include "medico.h"
#include "utils.h"

/*

	 void guardaNome(char *nome, pMedico medico)

	 definicao: funcao que guarda a string nome no lugar de nome_medico,
	 que e membro da struct passada por ponteiro (medico).
	 parametros: string nome e ponteiro para struct medico.

*/

void	guardaNome(char* nome, pMedico medico){
	medico->nome_medico = nome;
}

/*

	 void guardaEspecialidade(char *especialidade, pMedico medico)

	 definicao: funcao que guarda a string especialidade no lugar de especialidade,
	 que e membro da struct passada por ponteiro (medico).
	 parametros: string especialidade e ponteiro para struct medico.

*/

void	guardaEspecialidade(char *especialidade, pMedico medico)
{
	medico->especialidade = especialidade;
}

int	main(int argc, char *argv[])
{
	if (!balcaoIsRunning(0))
	{
		ourPutString("O Balcao nao esta em execucao, logo o Medico tambem nao podera iniciar!\n");
		return (1);
	}
	if (argc < 3)
	{
		ourPutString("Faltam argumentos de entrada: necessario nome e especialidade do Medico.\n");
		return (2);
	}
	else
	{
		Medico especialista;
		guardaNome(argv[1], &especialista);
		guardaEspecialidade(argv[2], &especialista);
	}
	return (0);
}
