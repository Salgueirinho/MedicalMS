#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


int	main(void)
{
  char str[20];
  int max_clientes, max_medicos;

  if(max_clientes == NULL){
    printf("Erro a ler MAXCLIENTES\n");
    return -1;
  }
  if(max_medicos == NULL){
    printf("Erro a ler MAXMEDICOS\n");
    return -1;
  }


  while(strcmp(str,"encerra") != 0){
    fflush(stdout);
    scanf("%20s",str);
    printf("MAXCLIENTES = %s\nMAXMEDICOS = %s\n", getenv("MAXCLIENTES"), getenv("MAXMEDICOS"));
  }

	return 0;
}
