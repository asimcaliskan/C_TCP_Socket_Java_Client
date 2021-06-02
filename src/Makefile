CC      = gcc
CFLAGS  = -g
RM      = rm -f


default: all

all: sserver

sserver:	sserver.c
	$(CC)	sserver.c	-o	sserver	-lrt	-lpthread
	
clean veryclean:
	$(RM) sserver