#include <stdio.h>
#include <stdlib.h>
#include "cliente.h"
#include "utils.h"

void guardaNome(char* nome, pCliente utente){
	utente->nomeCliente = nome;
}

void pedeSintomas(pCliente utente){
	scanf("%255[^\n]", utente->sintomas);
}

int main(int argc, char* argv[]){
  if(argc < 2)
	{
		ourPutString("Faltam argumentos de entrada: necessario nome do Cliente.\n");
	exit(-1);
	}
	else
	{
  	Cliente utente;
	  guardaNome(argv[1], &utente);
		ourPutString("Quais sao os seus sintomas?\n");
		pedeSintomas(&utente);
	}
  return 0;
}
