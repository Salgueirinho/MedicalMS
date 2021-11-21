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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "balcao.h"

/*

	 static int	callClassificador(void)

	 - definição: função que executa o classificador e permite a comunicação
	 com ela através do stdin/stdout (para tal exige o uso de dois pipes anónimos).
	 - return value: inteiro que pode tomar um de vários valores:
	 0)
	 1) o primeiro pipe falhou (aquele que é responsável pela leitura do
	 output do classificador);
	 2) o segundo pipe falhou (aquele que é responsável pela escrita de
	 sintomas no classificador);
	 3) o fork falhou;
	 4) o classificador falhou a executar e o write para o stderr também
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

			 Processo Crianáa - executa o classificador, e
			 tambem responsável por abrir a entrada do pipe
			 de leitura e a saída do pipe de escrita, para
			 além de fechar as entradas e saídas não necessárias
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

			 Se durante a execução do código, o programa chega aqui,
			 é porque houve algum problema a tentar executar o './classificador'

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
				especialidade[bytes_read] = '\0';
				ourPutString(especialidade);
			}
		} while (strcmp(sintomas, "#fim\n") != 0);
		close(p1[1]);
		close(p2[0]);
	}
	return (0);
}

/*

	 static int	getNumberFromEnv(const char *env_name)

	 - definicao: função responsável pela obtenção de valores
	 numéricos positivos de entre as variáveis de ambiente.
	 - return value: inteiro que toma valores entre [-1, 0]
	 onde o 0 significa que não houveram erros durante a
	 execução da função e o -1 significa que:
	 1) o nome da variável de ambiente passada por
	 argumento não existe;
	 2) o valor encontrado não é numerico;
	 3) o valor encontrado é numérico não positivo.
	 - parâmetro: nome da variável de ambiente da qual
	 queremos obter um valor numérico positivo.

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
			ourPutString("toma um valor não positivo\n");
		}
		else
			return (value);
	}
	else
	{
		ourPutString("Erro ao ler a variável de ambiente $(");
		ourPutString(env_name);
		ourPutString(")\n");
	}
	return (-1);
}

/*

	 static int	getMax(const char *name, const int default_value)

	 - definição: função responsável por verificar se getNumberFromEnv(name)
	 é inteiro positivo, caso seja, devolve esse valor, senão, devolve
	 o default_value.
	 - return value: inteiro igual a getNumberFromEnv(name) caso seja positivo ou
	 o valor do seu próprio argumento default_value, caso a condição anterior não
	 se verifique.
	 - parâmetros: nome da variável de ambiente e valor por omissão.

*/

static int	getMax(const char *name, const int default_value)
{
	int	n = getNumberFromEnv(name);
	return (n > 0 ? n : default_value);
}

/*

	 static void	setMaxValues(pValoresMaximos valoresMaximos)

	 - definição: função que atribui os valores dos membros do struct
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

	 - definição: função que consoante o comando recebido por argumento
	 mete uma frase apropriada em stdout.
	 - parâmetro: string comando.

*/

static void interpretCommand(const char *comando)
{
	if (strcmp(comando, "utentes") == 0)
		ourPutString("O sistema de momento tem 0 utentes.\n");
	else if (strcmp(comando, "especialistas") == 0)
		ourPutString("O sistema de momento tem 0 especialistas.\n");
	else if (strncmp(comando, "delut ", 6) == 0)
		ourPutString("O funcionamento deste comando ainda não se encontra implementado!\n");
	else if (strncmp(comando, "delesp ", 7) == 0)
		ourPutString("O funcionamento deste comando ainda não se encontra implementado!\n");
	else if (strncmp(comando, "freq ", 5) == 0)
		ourPutString("O funcionamento deste comando ainda não se encontra implementado!\n");
	else if (strcmp(comando, "encerra") == 0)
		ourPutString("O sistema vai encerrar dentro de momentos.\n");
	else
		ourPutString("Comando inválido!\n");
}

int	main(void)
{
	ValoresMaximos valoresMaximos;
	char	comando[40];
	int	bytes_read;

	if (balcaoIsRunning((int) getpid()))
	{
		ourPutString("Ja existe um balcao em execucao!\n");
		return (1);
	}

	setMaxValues(&valoresMaximos);
	while (strcmp(comando, "encerra") != 0)
	{
		ourPutString("[admin] comando: ");
		bytes_read = read(0, comando, sizeof(comando) - 1);
		if (bytes_read == -1)
		{
			ourPutString("Ocorreu um erro ao ler o comando!\n");
			return (2);
		}
		comando[bytes_read - 1] = '\0';
		if (strcmp(comando, "sintomas") == 0)
			callClassificador();
		else
			interpretCommand(comando);
	}
	return (0);
}
