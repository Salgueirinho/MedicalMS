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

#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

void	ourPutChar(const char c)
{
	write(1, &c, 1);
}

/*

	void	ourPutString(const char *str)

	- definicao: funcao que coloca uma string passada por argumento em stdout
	- parametro: string a mostrr

*/

void	ourPutString(const char *str)
{
	write(1, str, strlen(str));
}

/*

	static bool isNumber(const char *str)

	- definicao: funcao que verifica se uma string e totalmente numerica.
	- return value:
	false) caso a string nao seja totalmente numerica;
	true) caso a string seja totalmente numerica.
	- parametro: string a verificar.

*/

static bool	isNumber(const char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (!('0' <= str[i] && str[i] <= '9'))
			return (false);
		i++;
	}
	return (true);
}

/*

	bool balcaoIsRunning(const int pid)

	definicao: funcao que verifica se existe algum ./balcao em execucao,
	ao verificar o conteudo do ficheiro "cmdline" nas pastas de todos os
	PID's existentes em /proc/
	return value:
	true) caso seja encontrado um /proc/PID/cmdline com conteudo inicial
	"./balcao" onde PID pode tomar varios valores;
	false: caso nao seja encontrado um /proc/PID/cmdline com conteudo
	inicial "./balcao".
	parametros: inteiro PID que no caso do balcao vai receber argumento
	(int) getpid() e no caso de todos os outros ficheiros vai receber o
	valor 0, isto acontece porque caso nao passassemos o PID do balcao
	na chamada da funcao e posteriormente a usassemos para verificar que o
	PID da pasta pela qual estamos a iterar e de facto nao igual ao PID
	passado, o balcao nunca iria conseguir executar (em palavras simples,
	sem o PID, quando executassemos o balcao, ele iria encontrar-se a si
	proprio e isso iria impedir o balcao de alguma vez executar).

*/

bool balcaoIsRunning(const int pid)
{
	DIR* dir;
	struct dirent* ent;
	char buf[50] = "\0";
	int	fd;

	if (!(dir = opendir("/proc")))
	{
		exit(1);
	}

	while((ent = readdir(dir)) != NULL)
	{
		if (isNumber(ent->d_name) && atoi(ent->d_name) != pid)
		{
			strcat(buf, "/proc/");
			strcat(buf, ent->d_name);
			strcat(buf, "/cmdline");
			fd = open(buf, O_RDONLY);
			if (fd)
			{
				strcpy(buf, "\0");
				if (read(fd, buf, sizeof(buf)) != -1)
				{
					if (strncmp(buf, "./balcao", 8) == 0)
					{
						close(fd);
						closedir(dir);
						return (true);
					}
				}
				close(fd);
			}
		}
		strcpy(buf, "");
	}
	return (false);
}
