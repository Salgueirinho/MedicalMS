SOURCES = cliente.c	\
					balcao.c	\
					medico.c	\

OBJECTS	= $(patsubst %.c, %, $(SOURCES))

UTILS = utils.o

FLAGS		= -Wall -Wextra -Werror

CC			= gcc $(FLAGS)

all:			$(OBJECTS) $(UTILS)

cliente: $(UTILS)

balcao: $(UTILS)

medico: $(UTILS)

clean:
	@rm -f $(OBJECTS) $(UTILS)

re:				clean all
