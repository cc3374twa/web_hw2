all:
	gcc server.c -o server -pthread
	gcc client.c -o client -pthread
clean:
	rm -rf client server
