SOURCES = cliente.c	\
					balcao.c	\
					medico.c	\
					*.o				\

OBJECTS	= $(patsubst %.c, %, $(SOURCES))

FLAGS		= -Wall -Wextra -Werror

CC			= gcc $(FLAGS)

all:			$(OBJECTS)

cliente: utils.o

balcao: utils.o

medico: utils.o

clean:
	@rm -f $(OBJECTS)

re:				clean all
