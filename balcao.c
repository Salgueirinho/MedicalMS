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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "balcao.h"

/*

	 static int	callClassificador(void)

	 - definicao: funcao que executa o classificador e permite a comunicacao
	 com ela atraves do stdin/stdout (para tel exige o uso de dois pipes anonimos).
	 - return value: inteiro que pode tomar um de varios valores:
	 0)
	 1) o primeiro pipe falhou (aquele que e responsavel pela leitura do
	 output do classificador);
	 2) o segundo pipe falhou (aquele que e responsavel pela escrita de
	 sintomas no classificador);
	 3) o fork falhou;
	 4) o classificador falhou a executar e o write para o stderr tambem
	 falhou;
	 5) o classificador falhou;
	 6) o read dos sintomas para o stdin falhou;
	 7) o write dos sintomas para o pipe de escrita para o classificador
	 falhou;
	 8) o read da especialidade e urgencia do pipe de leitura do
	 classificador falhou.

*/

static int	callClassificador(void)
{
	char	sintomas[40];
	char	especialidade[40];
	int		p1[2];
	int		p2[2];
	int		pid;
	int		bytes_read;

	if (pipe(p1) == -1)
		return (1);
	if (pipe(p2) == -1)
		return (2);
	pid = fork();
	if (pid == -1)
		return (3);
	else if (pid == 0)
	{
		/*

			 Processo Crianca - executa o classificador, e
			 tambem responsavel por abrir a entrada do pipe
			 de leitura e a saida do pipe de escrita, para
			 alem de fechar as entradas e saidas nao necessarias
			 do mesmo 

		*/
		close(p1[1]);
		close(p2[0]);
		close(0);
		dup(p1[0]);
		close(1);
		dup(p2[1]);
		close(p1[0]);
		close(p2[1]);
		execl("classificador", "classificador", NULL);
		/*

			 Se durante a execucao do codigo, o programa chega aqui,
			 e porque houve algum problema a tentar executar o './classificador'

		*/
		if (write(2, "Erro ao executar Classificador\n", 31) == -1)
			return (4);
		return (5);
	}
	else
	{
		/*

			 Processo Pai - envia input ao classificador
			 e recebe output do mesmo

		*/
		close(p1[0]);
		close(p2[1]);
		do
		{
			bytes_read = 0;
			ourPutString("[admin] sintomas: ");
			if ((bytes_read = read(0, &sintomas, sizeof(sintomas) - 1)) == -1)
				return (6);
			sintomas[bytes_read] = '\0';
			bytes_read = 0;
			if (write(p1[1], sintomas, strlen(sintomas)) == -1)
				return (7);
			if (strcmp(sintomas, "#fim\n") != 0)
			{
				if ((bytes_read = read(p2[0], especialidade, sizeof(especialidade) - 1)) == -1)
					return (8);
				especialidade[bytes_read - 1] = '\0';
				ourPutString(especialidade);
				ourPutString("\n");
			}
		} while (strcmp(sintomas, "#fim\n") != 0);
		close(p1[1]);
		close(p2[0]);
	}
	return (0);
}

/*

	 static int	getNumberFromEnv(const char *env_name)

	 - definicao: funcao responsavel pela obtencao de valores
	 numericos positivos de entre as variaveis de ambiente.
	 - return value: inteiro que toma valores entre [-1, 0]
	 onde o 0 significa que nao houveram erros durante a
	 execucao da funcao e o -1 significa que:
	 1) o nome da variavel de ambiente passada por
	 argumento nao existe;
	 2) o valor encontrado nao e numerico;
	 3) o valor encontrado e numerico nao positivo.
	 - parametro: nome da variavel de ambiente da qual
	 queremos obter um valor numerico positivo.

*/

static int	getNumberFromEnv(const char *env_name)
{
	char *value_str;
	int	value;

	value_str = getenv(env_name);
	if (value_str)
	{
		value = atoi(value_str);
		if (value <= 0)
		{
			ourPutString(env_name);
			ourPutString("toma um valor nao positivo\n");
		}
		else
			return value;
	}
	else
	{
		ourPutString("Erro ao lera a variavel de ambiente $(");
		ourPutString(env_name);
		ourPutString(")\n");
	}
	return -1;
}

/*

	 static int	getMax(const char *name, const int default_value)

	 - definicao: funcao responsavel por verificar se getNumberFromEnv(name)
	 e inteiro positivo, caso seja, devolve esse valor, senao, devolve
	 o default_value.
	 - return value: inteiro igual a getNumberFromEnv(name) caso seja positivo ou
	 o valor do seu proprio argumento default_value, caso a condicao anterior nao
	 se verifique.
	 - parametros: nome da variavel de ambiente e valor por omissao.

*/

static int	getMax(const char *name, const int default_value)
{
	int	n = getNumberFromEnv(name);
	return n > 0 ? n : default_value;
}

/*

	 static void	setMaxValues(pValoresMaximos valoresMaximos)

	 - definicao: funcao que atribui os valores dos membros do struct
	 valoresMaximos, p.e. max_clientes ou max_medicos.
	 - parametro: ponteiro para o struct valoresMaximos.

*/

static void	setMaxValues(pValoresMaximos valoresMaximos)
{
	valoresMaximos->max_clientes = getMax("MAXCLIENTES", MAX_CLIENTES_DEFAULT);
	valoresMaximos->max_medicos = getMax("MAXMEDICOS", MAX_MEDICOS_DEFAULT);
	valoresMaximos->max_lugares = MAX_FILA;
	valoresMaximos->max_especialidades = MAX_ESPECIALIDADES_DEFAULT;
}

/*

	 static void	interpretCommand(const char *comando)

	 - definicao: funcao que consoante o comando recebido por argumento
	 mete uma frase apropriada em stdout.
	 - parametro: string comando.

*/

static void interpretCommand(const char *comando)
{
	if (strcmp(comando, "utentes") == 0)
		ourPutString("O sistema de momento tem 0 utentes.\n");
	else if (strcmp(comando, "especialistas") == 0)
		ourPutString("O sistema de momento tem 0 especialistas.\n");
	else if (strncmp(comando, "delut ", 6) == 0)
		ourPutString("O funcionamento deste comando ainda nao se encontra implementado!\n");
	else if (strncmp(comando, "delesp ", 7) == 0)
		ourPutString("O funcionamento deste comando ainda nao se encontra implementado!\n");
	else if (strncmp(comando, "freq ", 5) == 0)
		ourPutString("O funcionamento deste comando ainda nao se encontra implementado!\n");
	else if (strcmp(comando, "encerra") == 0)
		ourPutString("O sistema vai encerrar dentro de momentos.\n");
	else
		ourPutString("Comando invalido!\n");
}

int	main(void)
{
	ValoresMaximos valoresMaximos;
	char	comando[40];
	int	bytes_read;

	if (balcaoIsRunning((int) getpid()))
	{
		ourPutString("Ja existe um balcao em execucao!\n");
		return (-1);
	}

	setMaxValues(&valoresMaximos);
	while (strcmp(comando, "encerra") != 0)
	{
		ourPutString("[admin] comando: ");
		bytes_read = read(0, comando, 40);
		if (bytes_read == -1)
		{
			ourPutString("Ocorreu um erro ao ler o comando!\n");
			return (-1);
		}
		comando[bytes_read - 1] = '\0';
		if (strcmp(comando, "sintomas") == 0)
			callClassificador();
		else
			interpretCommand(comando);
	}
	return 0;
}
