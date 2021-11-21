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

#ifndef MEDICO_H
#define MEDICO_H

/*

	 Esta struct tem como objetivo possibilitar centralizar a informacao
	 sobre cada medico diferente, isto inclui:
	 - o nome do medico, que permitira a diferenciacao de medico para medico,
	 porem, sera possivel ter dois medicos com o mesmo nome;
	 - a especialidade do medico, que ira servir de referencia para o inicio
	 de uma comunicacao cliente (que a partida, ja sabera a especialidade e
	 urgencia dos seus sintomas) - medico:
	 - NOTA: existe a possibilidade de um medico ter uma especialidade nao
	 existente no programa classificador e neste caso, ele nunca ira comunicar
	 com um cliente.

*/

typedef struct {
	char *nome_medico;
	char *especialidade;
} Medico, *pMedico;

#endif // MEDICO_H
