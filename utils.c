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

	- definição: função que coloca uma string passada por argumento em stdout
	- parâmetro: string a mostrar

*/

void	ourPutString(const char *str)
{
	write(1, str, strlen(str));
}

/*

	static bool isNumber(const char *str)

	- definição: função que verifica se uma string é totalmente numérica.
	- return value:
	false) caso a string não seja totalmente numérica;
	true) caso a string seja totalmente numérica.
	- parâmetro: string a verificar.

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

	definição: função que verifica se existe algum ./balcao em execução,
	ao verificar o conteúdo do ficheiro "cmdline" nas pastas de todos os
	PID's existentes em /proc/
	return value:
	true) caso seja encontrado um /proc/PID/cmdline com conteúdo inícial
	"./balcao" onde PID pode tomar vários valores;
	false: caso não seja encontrado um /proc/PID/cmdline com conteúdo
	inicial "./balcao".
	parâmetros: inteiro PID que no caso do balcão vai receber argumento
	(int) getpid() e no caso de todos os outros ficheiros vai receber o
	valor 0, isto acontece porque caso não passassemos o PID do balcão
	na chamada da função e posteriormente a usassemos para verificar que o
	PID da pasta pela qual estamos a iterar é de facto não igual ao PID
	passado, o balcão nunca iria conseguir executar (em palavras simples,
	sem o PID, quando executassemos o balcão, ele iria encontrar-se a si
	próprio e isso iria impedir o balcão de alguma vez executar).

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
