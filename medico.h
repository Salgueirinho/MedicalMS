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

	 Medico:

	 Esta struct tem como objetivo possibilitar centralizar a informação
	 sobre cada médico diferente, isto inclui:
	 - o nome do médico, que permitirá a diferenciação de médico para médico,
	 porém, será possível ter dois médicos com o mesmo nome;
	 - a especialidade do médico, que irá servir de referencia para o início
	 de uma comunicação cliente (que à partida, já saberá a especialidade e
	 urgência dos seus sintomas) - médico:
	 - NOTA: existe a possibilidade de um médico ter uma especialidade não
	 existente no programa classificador e neste caso, ele nunca irá comunicar
	 com um cliente.

*/

typedef struct {
	char *nome_medico;
	char *especialidade;
} Medico, *pMedico;

#endif // MEDICO_H
