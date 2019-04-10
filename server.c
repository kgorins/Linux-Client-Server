/*Libraries:*/

#include <stdio.h>  // C standards
#include <stdlib.h>
#include <string.h>

#include <unistd.h> // unix

#include <signal.h> //Signaux

#include <arpa/inet.h>  //Reseaux
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

/*Constantes*/

#define BUFFER_SIZE 1024	// Buffer pour les messages
#define SIZE_TABLEAU 2		// Taille du tableau pour stocker les sockets

/*Les fonctions utiliseés:*/

int cree_socket_tcp_ip (char id[]);
int affiche_adresse_socket (int sock);
int connexion (int socket_contact);
int affiche_adresse_distante (int sock);
void numUtilisateurs (int sig);
void clotureConnexion (int sig);
void traitement2 (int socket_c1, int socket_c2);

/*Variables globales*/

int clientsSock[SIZE_TABLEAU]={0,0};	//Variables pour stocker les deux sockets des deux utilisateurs
int socket_contact;	//Numero de la socket de contact
struct sockaddr_in adresse;	//Addresse  associée à la socket 
socklen_t lg;	//Taille en octets de l’adresse
int nb=0;	//Nombre des utilisateurs en ligne

/*Programme principale*/

int main (int arg, char *argv[]) 
{
	/*Signaux*/

	signal (SIGUSR1, clotureConnexion);
	signal (SIGUSR2, numUtilisateurs);

	/*On verifie des parametres d'entrée*/

	if (arg!= 2) {
		printf("Erreur usage: ./server num_IP\n");
		exit(1);
	}

	/*Variables locales*/

	pid_t pid;	//PID qu'on utilise pour creer le fork
	char id_addr [14];	//adresse IP

	printf ("Bonjour!\n");

	sscanf (argv[1], "%s", &id_addr);	//On utilise IP qui a été passé comme argument
	socket_contact = cree_socket_tcp_ip (id_addr);		// Creation socket de contact
	affiche_adresse_socket (socket_contact);		// Affichage de l'adresse
	listen (socket_contact, 6);		//6 connexions max
	printf ("Attente des clients...\n");
	
	while (1) 
	{
		/*Acceptation d’une connexion sur deux sockets*/
		
		clientsSock[0] = accept (socket_contact, (struct sockaddr*)&adresse, &lg);
		nb++;
		clientsSock[1] = accept (socket_contact, (struct sockaddr*)&adresse, &lg);
		nb++;

		/*Fork*/
		pid = fork();

		if (pid==-1) //Erreur
		{
			perror ("Fork");

			/*Fermeture des connexions*/

			if ((close (clientsSock[0])<0) || (close (clientsSock[1])<0))
			{
				perror("Erreur close: ");
				exit(0);
			}//if


			if (close (socket_contact)<0)
			{
				perror("Erreur close: ");
				exit(0);
			}//if

			exit(1);
		}//if

		else if (pid==0) // fils -> on continue avec la socket connectee
		{ 
			/*Debut de traitement*/

			write (clientsSock[0],"[SERVER] Debut de chat\n",29);
			
			affiche_adresse_distante (clientsSock[0]);
			affiche_adresse_distante (clientsSock[1]);
			
			traitement2 (clientsSock[0], clientsSock[1]);

			/*Fermeture des connexions*/

			if ((close (clientsSock[0])<0) || (close (clientsSock[1])<0))
			{
				perror("Erreur close: ");
				exit(0);
			}//if
			

		}//else if			

		else 	// père -> traiter la requète suivante
		{  	
			printf ("Pere: %d\n", getpid());	//PID à utiliser avec les signaux
		
		}//else
	}//while

	/*Fermeture des connexions*/

	if (close (socket_contact)) 
	{
		perror("Erreur close: ");
		exit(0);
	}//if

	printf ("Au revoir!\n");
	return 0;
}//main

/*Création d’une socket serveur*/

int cree_socket_tcp_ip(char id[])
{
	/*Variables locales*/

	int sock=0;
	struct sockaddr_in adresse;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)	//IPv4, flux d'octets de connexion bidirectionnels
	{
		perror("Erreur ");
		exit(2);
	}//if

	memset (&adresse, 0, sizeof(struct sockaddr_in));     //Mettre le champs de la structure à zero

	adresse.sin_family = PF_INET;     // Protocol IP
	adresse.sin_port = htons (33016);       //Port 33016
	inet_aton (id, &adresse.sin_addr);      // IP localhost

	/*Affectation d'un nom à une socket, liason avec un port adresse*/

	int b = bind (sock, (struct sockaddr*) &adresse, sizeof(struct sockaddr_in));

	if (b < 0)
	{
		close(sock);
		perror("Erreur ");
		exit(2);
	}//if

  return sock;
}//fonction

/*Communication entre les clients*/

void traitement2 (int socket_c1, int socket_c2)
{
	/*Buffers pour les messages*/
	
	char bufferWrite[BUFFER_SIZE];
	char bufferRead1[BUFFER_SIZE];
	char bufferRead2[BUFFER_SIZE];

	int n1, n2;	//Numeros des bits reçu

	/*Vider le contenu des buffers avant utilisation*/

	memset (bufferWrite,0,sizeof(bufferWrite));	
	memset (bufferRead1,0,sizeof(bufferRead1));	
	memset (bufferRead2,0,sizeof(bufferRead2));	

	while (1) 
	{	
		/*Premier utilisateur*/

		n1 = read (socket_c1, bufferRead1, BUFFER_SIZE);	//Reception d'une message
		memcpy (bufferWrite, bufferRead1, sizeof(bufferRead1));    //On recopie la contenu du buffer Read dans le Write
		write (socket_c2, bufferWrite, strlen(bufferWrite));	//On envoie le bufferWrite au deuxième utilisateur

		/*Vider le contenu des buffers*/

		memset (&bufferRead1[0],0,sizeof(bufferRead1));
		memset (&bufferWrite[0],0,sizeof(bufferWrite));

		/*Meme principe pour le deuxième utilisateur*/

		n2 = read (socket_c2, bufferRead2, BUFFER_SIZE);
		memcpy (bufferWrite, bufferRead2, sizeof(bufferRead2));    //On recopie la contenu du buffer
		write (socket_c1, bufferWrite, strlen(bufferWrite));

		memset (&bufferRead2[0],0,sizeof(bufferRead2));
		memset (&bufferWrite[0],0,sizeof(bufferWrite));
		
		/*Si il n'y avait pas de lecture*/

		if (n1 == 0) {
			bufferRead1[n1-1] = '\0';
			n--;
			break; 
		}//if

		if (n2 == 0) {
			bufferRead1[n2-1] = '\0';
			n--;
			break;
		}//if
	}//while
}//fonction


/*Affichage de l'adresse IP et port du serveur*/

int affiche_adresse_socket(int sock)
{
	/*Creation d'une structure sockaddr_in*/

	struct sockaddr_in adresse;
	socklen_t longueur;
	longueur = sizeof (struct sockaddr_in);
	
	/*Renvoier une socket*/

	if (getsockname (sock, (struct sockaddr*)&adresse, &longueur) < 0) 
	{
		fprintf (stderr, "Server: Erreur getsockname");
		return -1;
	}//if

	printf ("Server -> ");
	printf ("IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));
	return 0;
}//fonction

/*Affichage de l'adresse IP du client*/

int affiche_adresse_distante(int sock)
{
	/*Creation d'une structure sockaddr_in*/

	struct sockaddr_in adresse;
	socklen_t longueur;
	longueur = sizeof(struct sockaddr_in);

	/*Renvoier une socket*/

	if (getpeername(sock, (struct sockaddr*)&adresse, &longueur) < 0)
	{
		perror ("Erreur getpeername: ");
		exit (1);
	}//if

	printf ("Utilisateur distant -> IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));
	return 0;
}//fonction

/*Signal 1: Cloture de connexion*/

void clotureConnexion (int sig) 
{
	signal (SIGUSR1, clotureConnexion);

	int i;

	printf ("Attention! Cloture de la connexion.\n");

	/*Message de fermeture*/

	for (i=4; i<nb+4;i++) 
	{
		if (write (i,"[SERVER] Connexion perdue!\nVotre prochains messages ne seront pas envoies et vont etre suivi par la fermeture de la messagerie.\n", 127)<0) 
		{
			perror ("Erreur write: ");
			break;
		}//if
	}//for

	/*Interdiction de la reception et envoie des messages*/

	for (i=4; i<nb+4;i++) 
	{
		if (shutdown (i, SHUT_RDWR)<0) {
			perror ("Erreur shutdown: ");
			break;
		}

		if (close (i)<0) {
			perror ("Erreur close: ");
			exit (0);
		}//if
	}//for

	pid_t self = getpid();

	/*Fermeture complete de connexion*/

	if (close (socket_contact)<0) {
		perror ("Erreur close: ");
		exit (0);
	}//if

	if (kill (self, SIGKILL) <0){
		perror ("Erreur kill: ");
		exit (0);
	}//if

}//fonction

/*Signal 2: affichage dèun nombre des utilisateurs actifs*/

void numUtilisateurs (int sig)
{
	signal (SIGUSR2, numUtilisateurs);
	printf ("\nNombre d'utilisateurs en ligne: %d\n", nb);
}//fonction



