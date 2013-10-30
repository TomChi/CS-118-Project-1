CC=gcc

webserver: webserver.c
	$(CC) webserver.c -o webserver

.PHONY: clean

clean:
	rm webserver
