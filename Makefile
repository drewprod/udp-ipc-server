CC=gcc
CFLAG=-W -Wall

all:
	$(CC) $(CFLAG) main.c -o udp-ipc-server
	
debug:
	$(CC) $(CFLAG) -g main.c -o udp-ipc-server
