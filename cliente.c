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

#include <stdio.h>
#include "cliente.h"
#include "utils.h"

/*

	 void guardaNome(char *nome, pCliente utente)

	 definicao: funcao que guarda a string nome no lugar de nome_cliente,
	 que e membro da struct passada por ponteiro (utente).
	 parametros: string nome e ponteiro para struct utente.

*/

void guardaNome(char* nome, pCliente utente){
	utente->nome_cliente = nome;
}

/*

	 void pedeSintomas(pCliente utente)

	 definicao: funcao que pede, atraves stdin, com recurso ao scanf, os sintomas
	 do cliente e as guarda no struct utente.
	 parametro: ponteiro para struct utente.

*/

void pedeSintomas(pCliente utente){
	scanf("%255[^\n]", utente->sintomas);
}

int main(int argc, char* argv[]){
	if (!balcaoIsRunning(0))
	{
		ourPutString("O Balcao nao esta em execucao, logo o Cliente tambem nao podera iniciar!\n");
		return (1);
	}
	if(argc < 2)
	{
		ourPutString("Faltam argumentos de entrada: necessario nome do Cliente.\n");
		return (2);
	}
	else
	{
		Cliente utente;
		guardaNome(argv[1], &utente);
		ourPutString("Quais sao os seus sintomas?\n");
		pedeSintomas(&utente);
	}
	return (0);
}
