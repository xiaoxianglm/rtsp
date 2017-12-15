TARGET=myrtsp
#CC=arm-hisiv100nptl-linux-gcc
CC=gcc -g

CFLAGS=
LDFLAGS=
C_SOURCES=$(wildcard *.c)
C_OBJS=$(patsubst %.c,%.o,$(C_SOURCES))
.c.o:
	$(CC) -c -o $*.o $(CFLAGS) $*.c
compile:$(C_OBJS)
	$(CC) -o $(TARGET) $^ $(LDFLAGS)
        
.PHONY:
	clean
clean:
	rm -f *.o $(TARGET)
