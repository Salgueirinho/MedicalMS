SOURCES = patient.c	\
					service_desk.c	\
					doctor.c	\

OBJECTS	= $(patsubst %.c, %, $(SOURCES))

FLAGS		= -Wall -Wextra -pthread

CC			= gcc $(FLAGS)

all:			$(OBJECTS) $(UTILS)

%.o: %.c
	$(CC) -c -o $@ $< $(FLAGS)

patient: patient_utils.o utils.o

service_desk: service_desk_utils.o utils.o

doctor: doctor_utils.o utils.o

clean:
	@rm -f $(OBJECTS) *.o

re: clean
	@make all
