SOURCES = patient.c	\
					service_desk.c	\
					doctor.c	\

OBJECTS	= $(patsubst %.c, %, $(SOURCES))

UTILS = utils.o

SERVICE_DESK_UTILS = service_desk_utils.o

FLAGS		= -Wall -Wextra -pthread #-Werror

CC			= gcc $(FLAGS)

all:			$(OBJECTS) $(UTILS)

patient: $(UTILS)

service_desk: $(SERVICE_DESK_UTILS) $(UTILS)

doctor: $(UTILS)

clean:
	@rm -f $(OBJECTS) $(UTILS) $(SERVICE_DESK_UTILS)

re:				clean all
