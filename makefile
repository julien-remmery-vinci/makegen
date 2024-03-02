CFLAGS=-std=c17 -pedantic -Wvla -Werror -Wall -D_DEFAULT_SOURCE -g

makegen: makegen.o makegen_utils.o
	cc $(CFLAGS) -o makegen makegen.o makegen_utils.o

makegen_utils.o: makegen_utils.c makegen_utils.h
	cc $(CFLAGS) -c makegen_utils.c

makegen.o: makegen.c makegen_utils.h
	cc $(CFLAGS) -c makegen.c

clean:
	rm *.o
	rm makegen 
