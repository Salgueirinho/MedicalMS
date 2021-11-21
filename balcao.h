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

#ifndef BALCAO_H
#define BALCAO_H

// Numero maximo de clientes diferentes (valor por omissao)
#define MAX_CLIENTES_DEFAULT 5

// Numero maximo de medicos diferentes (valor por omissao)
#define MAX_MEDICOS_DEFAULT 5

// Numero maximo de especialidades diferentes (valor por omissao)
#define MAX_ESPECIALIDADES_DEFAULT 5

// Numero maximo de clientes na fila de uma especialidade
#define MAX_FILA 5

/*

	 ValoresMaximos:

	 Esta struct tem como objetivo possibilitar a centralizacao de toda a
	 informacao relativa aos valores maximos, definidos no momento de Runtime:
	 - max_clientes: numero maximo de clientes diferentes;
	 - max_medicos: numero maximo de medicos diferentes;
	 - max_especialidades: numero maximo de especialidades diferentes;
	 - max_lugares: numero maximo de lugares na fila de espera de uma qualquer
	 especialidade.

*/

typedef struct {
	int	max_clientes;				// N
	int	max_medicos;				// M
	int	max_especialidades;	// X
	int	max_lugares;				// Y
} ValoresMaximos, *pValoresMaximos;

#endif // BALCAO_H
