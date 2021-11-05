#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef struct{
  char NomeCliente[40];
  char Sintomas[255];
}Cliente, *pCliente;

void GuardaNome(char *argv[], pCliente utente){
  strcpy(utente->NomeCliente,"");
  for(int i=1;argv[i]!=NULL;i++){
    strcat(utente->NomeCliente,argv[i]);
    strcat(utente->NomeCliente, " ");
  }
}

void GuardaSintomas(pCliente utente){
  

}

int main(int argc, char* argv[], char* envp[]){
  if(argc < 2){
    printf("Faltam Argumentos de Entrada\nNecessário Nome do Cliente\n");
    return 1;
}

  Cliente utente;
  GuardaNome(argv, &utente);
  printf("Olá %s\n",utente.NomeCliente);

  return 0;
}
