CFLAGS = -Wextra -Wall -g

build: tema1.o hashtable.o
	gcc -o so-cpp tema1.o hashtable.o

tema1.o: tema1.c
	gcc $(CFLAGS) -c tema1.c

hashtable.o: hashtable.c
	gcc $(CFLAGS) -c hashtable.c

clean:
	rm -f *.o *~ so-cpp
