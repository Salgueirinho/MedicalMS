/*

	 Trabalho prático realizado por:
	 - Gonçalo Salgueirinho - a2020142627@isec.pt
	 - Kylix Afonso - a2020146228@isec.pt
	 Docente responsável pela unidade curricular:
	 - prof. João Durães
	 Unidade curricular:
	 - Sistemas Operativos
	 Insituto Superior de Engenharia de Coimbra

*/

#ifndef BALCAO_H
#define BALCAO_H

// Número máximo de clientes diferentes (valor por omissão)
#define MAX_CLIENTES_DEFAULT 5

// Número máximo de medicos diferentes (valor por omissão)
#define MAX_MEDICOS_DEFAULT 5

// Número máximo de especialidades diferentes (valor por omissão)
#define MAX_ESPECIALIDADES_DEFAULT 5

// Número máximo de clientes na fila de uma especialidade
#define MAX_FILA 5

/*

	 ValoresMaximos:

	 Esta struct tem como objetivo possibilitar a centralização de toda a
	 informação relativa aos valores máximos, definidos no momento de Runtime:
	 - max_clientes: número máximo de clientes diferentes;
	 - max_medicos: número máximo de medicos diferentes;
	 - max_especialidades: número máximo de especialidades diferentes;
	 - max_lugares: número máximo de lugares na fila de espera de uma qualquer
	 especialidade.

*/

typedef struct {
	int	max_clientes;	
	int	max_medicos;
	int	max_especialidades;
	int	max_lugares;
} ValoresMaximos, *pValoresMaximos;

#endif // BALCAO_H
