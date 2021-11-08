SOURCES = cliente.c	\
					balcao.c	\
					medico.c	\

OBJECTS	= $(patsubst %.c, %, $(SOURCES))

FLAGS		= -Wall -Wextra -Werror

CC			= gcc $(FLAGS)

all:			$(OBJECTS)

cliente:

balcao:

medico:

clean:
	@echo "Cleaning all"
	@rm -f $(OBJECTS)

re:				clean all
