.PHONY: all

all:
	gcc -w -pthread multi-client.c -o multi-client
	g++ -w -fpermissive -pthread server-mt.cpp -o server-mt 

clean:
	rm -f multi-client
	rm -f server-mt