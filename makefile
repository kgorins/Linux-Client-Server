
CC = gcc # variable CC précisant le compilateur
FLAGS = -Wall # variable FLAGS precisant les options de compilation

# compotement par défaut de "make"
all: client server

client: client.o
	gcc -o 	client client.c
server: server.o
	gcc -o server server.c

client.o: client.c
	$(CC) -c client.c -o client.o $(FLAGS)

server.o: server.c
	$(CC) -c server.c -o server.o $(FLAGS)

# Commande "make clean" pour effacer les fichiers de compilation

clean:
	rm -rf client server


