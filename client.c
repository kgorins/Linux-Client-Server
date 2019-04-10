/*Libraries:*/

#include <stdio.h>  // C standards
#include <stdlib.h>
#include <string.h>

#include <signal.h> //Signaux

#include <unistd.h> // unix

#include <arpa/inet.h>  //Reseaux
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

/*Constantes*/

#define BUFFER_SIZE 1024	// Buffer pour les messages

/*Les fonctions utiliseés:*/

int cree_socket_tcp_client (char *ip, int port);
void communication (int socket);
int affiche_adresse_socket (int sock);


int main (int arg, char *argv[]) 
{

	/*On verifie des parametres d'entrée*/
	if (arg!= 2) {
		printf ("Erreur usage: ./client num_IP \n");
		exit(1);
	}//if

	int sock = cree_socket_tcp_client (argv[1], 33016);	// Creation socket 

	printf ("Bonjour et bienvenue dans la messagerie!\n");
	affiche_adresse_socket (sock);	//Affichage sur l'ecran
	printf("Veuillez patienter un moment...\n");

	
	communication (sock);	//Communication

	close (sock);
	printf ("Au revoir!\n");
	return 0;
}//main


/*Affichage de l'adresse IP et port du client*/

int affiche_adresse_socket(int sock) 
{
	/*Creation d'une structure sockaddr_in*/

	struct sockaddr_in adresse;
	socklen_t longueur;
	longueur = sizeof (struct sockaddr_in);

	/*Renvoier une socket*/

	if (getsockname (sock, (struct sockaddr*)&adresse, &longueur) < 0) {
		perror ("Erreur getsockname: ");
		exit(0);
	}//if

	printf ("IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));
	return 0;
}//fonction

/*Création d’une socket client*/

int cree_socket_tcp_client(char *ip, int port) 
{
	/*Creation d'une structure sockaddr_in*/

	struct sockaddr_in adresse;
	int sock=0;
	
	/*Création d’une socket*/

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {	//IPv4, flux d'octets de connexion bidirectionnels
		perror("Erreur socket ");
		exit(1);
	}//if

	memset (&adresse, 0, sizeof(struct sockaddr_in));	//Mettre les champs à 0
	adresse.sin_family = AF_INET;     //IPv4
	adresse.sin_port = htons (port);  //Numero de porte quùon donne comme argument

	inet_aton (ip, &adresse.sin_addr);	//Convertir l'adresse ne flix binaire

	/*Demande de connexion*/

	if (connect (sock,(struct sockaddr*) &adresse, sizeof(struct sockaddr_in)) < 0) {
		close (sock);
		perror ("Erreur connect ");
		exit(2);
	}//if
	
	printf ("Demande de connexion...\n");

	return sock;
}//fonction

/*Communication entre les clients*/

void communication( int socket) 
{	
	/*Buffers pour les messages*/
	char bufferExit[BUFFER_SIZE]="Connexion lost\n";
	char bufferWrite[BUFFER_SIZE];
	char bufferRead[BUFFER_SIZE];
	int r;	//Flag receprion d'un message

	/*Vider le contenu des buffers avant utilisation*/

	memset (&bufferRead[0],0,sizeof(bufferRead));
	memset (&bufferWrite[0],0,sizeof(bufferWrite));

	/*Communication*/

	while(1) 
	{
		r = read (socket, bufferRead, BUFFER_SIZE); //Lecture

		if (r>0) 	//Si on reçoit le message, on l'affiche sur l'ècran et puis on envoie notre reponce
		{
			printf ("Message reçu: %s\n", bufferRead);
			memset (&bufferRead[0],0,sizeof(bufferRead));	//Vider le contenu des buffers
			memset (&bufferWrite[0],0,sizeof(bufferWrite));
			printf ("Entrez votre message : ");
			fgets (bufferWrite, BUFFER_SIZE, stdin);	//Le message tapé sur le clavier est mise dans le bufferWrite
			write (socket, bufferWrite, strlen(bufferWrite));	//Envoyer le message
		}//if
		
		else 	//Si on reçoit le message, on l'affiche sur l'ècran et puis on envoie notre reponce
		{
			memset (&bufferRead[0],0,sizeof(bufferRead));	//Vider le contenu des buffers
			memset (&bufferWrite[0],0,sizeof(bufferWrite));
			printf ("\nEntrez votre message : ");
			fgets (bufferWrite, BUFFER_SIZE, stdin);	//Le message tapé sur le clavier est mise dans le bufferWrite
			write (socket, bufferWrite, strlen(bufferWrite));
		}//else
	}//while
}//fonction



