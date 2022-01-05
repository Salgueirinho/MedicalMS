SOURCES = patient.c	\
					service_desk.c	\
					doctor.c	\

OBJECTS	= $(patsubst %.c, %, $(SOURCES))

UTILS = utils.o

FLAGS		= -Wall -Wextra -Werror -pthread

CC			= gcc $(FLAGS)

all:			$(OBJECTS) $(UTILS)

patient: $(UTILS)

service_desk: $(UTILS)

doctor: $(UTILS)

clean:
	@rm -f $(OBJECTS) $(UTILS)

re:				clean all
