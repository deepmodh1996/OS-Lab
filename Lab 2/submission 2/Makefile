.PHONY: all

all:
	gcc -pthread multi-client.c -o multi-client
	gcc server-mp.c -o server-mp

clean:
	rm -f multi-client
	rm -f server-mp