PROG=driver_process

OBJS=main.o interprocess.o

CFLAGS=-Wall -std=c11 -D_XOPEN_SOURCE=500 -O0 -g -m32
LFLAGS=-lrt -lpthread -m32

.PHONY: clean

all:$(PROG)
	@echo $(OBJS)

$(PROG):$(OBJS)
	g++ $^ $(LFLAGS) -o $@

%.o:%.c
	g++ -c $< $(CFLAGS) -o $@

%.o:%.cpp
	g++ -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o $(PROG)
