all:
	gcc Server.c -o Server
	sudo ./Server
clean:
	rm serfork
