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

#include <unistd.h>
#include "cliente.h"
#include "utils.h"

/*

	 void guardaNome(char *nome, pCliente utente)

	 definição: função que guarda a string nome no lugar de nome_cliente,
	 que é membro da struct passada por ponteiro (utente).
	 parâmetros: string nome e ponteiro para struct utente.

*/

void guardaNome(char* nome, pCliente utente){
	utente->nome_cliente = nome;
}

/*

	 void pedeSintomas(pCliente utente)

	 definição: função que pede, através stdin, com recurso ao scanf, os sintomas
	 do cliente e as guarda no struct utente.
	 parâmetro: ponteiro para struct utente.

*/

int pedeSintomas(pCliente utente){
	int	bytes_read = -1;

	if ((bytes_read = read(0, utente->sintomas, 255)) == -1)
		return (1);
	utente->sintomas[bytes_read - 1] = '\0';
	return (0);
}

int main(int argc, char* argv[]){
	if (!balcaoIsRunning(0))
	{
		ourPutString("O Balcão não está em execução, logo o Cliente também não poderá iniciar!\n");
		return (1);
	}
	if(argc < 2)
	{
		ourPutString("Faltam argumentos de entrada: necessário nome do Cliente.\n");
		return (2);
	}
	else
	{
		Cliente utente;
		guardaNome(argv[1], &utente);
		ourPutString("Quais são os seus sintomas?\n");
		pedeSintomas(&utente);
	}
	return (0);
}
