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

#ifndef CLIENTE_H
#define CLIENTE_H

/*

	 Esta struct tem como objetivo possibilitar centralizar a informacao
	 sobre cada cliente diferente (ate ao momento essa informacao so inclui
	 o nome do cliente - que tem como objetivo possiblitar a diferenciacao de
	 cada cliente e os sintomas - que a posteriori serao enviados para o balcao
	 atraves de um named pipe ainda nao implementado):
	 - NOTA: Achamos que mais tarde iremos alterar esta estrutura e como tal,
	 esta so servira de prototipo.

*/

typedef struct {
	char *nome_cliente;
	char sintomas[255];
} Cliente, *pCliente;

#endif // CLIENTE_H
