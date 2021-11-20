#ifndef BALCAO_H
#define BALCAO_H

#define MAX_CLIENTES_DEFAULT 5
#define MAX_MEDICOS_DEFAULT 5
#define MAX_FILA 5 // MAX UTENTES POR FILA POR ESPECIALIDADE

typedef struct {
	int	max_clientes;				// N
	int	max_medicos;				// M
	int	max_especialidades;	// X
	int	max_lugares;				// Y
} ValoresMaximos, *pValoresMaximos;

#endif // BALCAO_H
