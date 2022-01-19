#include "fonctions.h"

int main(int argc, char *argv[]){ //argc=nombre d'arguments	
	char* port;
	char* hostIP; //adresse IP du serveur
	char* fileName;
	char *buffer = calloc(516*sizeof(char),0);
	char acq[ACQ_TAILLE];
	
	if (argc !=3){ //on vérifie ici que l'on a bien 3 arguments en entrée: nom_du_programme host:port file
		printf(ERROR);
		exit(EXIT_FAILURE);
	}
  	
  	//on récupère les chaînes de caractère host et port
	hostIP=strtok(argv[1],":");
	port=strtok(NULL,":");
	
	//si on ne définit pas le port, on initialise sa valeur à "69" 
	if (port==NULL){
		port = malloc(3);
		port= "69";
	}
	
	//on récupère la chaîne de caractère fichier	
	fileName=malloc(strlen(argv[2]));
	strcpy(fileName,argv[2]);
	
	printf(HELLO, hostIP, port,fileName);
	//Le programme compile et nous renvoie les valeurs souhaitées, tout va bien
	
	
	//Configuration du client UDP  
    struct addrinfo hints; 
    struct addrinfo *result;
    struct sockaddr adresse;
    socklen_t adresse_len = sizeof(adresse);
    
    memset(&hints, 0, sizeof(struct addrinfo));  //remplit la zone mémoire
	
	hints.ai_family = PF_INET;   //Ipv4
    hints.ai_socktype = SOCK_DGRAM;   //mode datagramme
    hints.ai_flags = AI_PASSIVE;    
    hints.ai_protocol = IPPROTO_UDP;
    
    int s = getaddrinfo(hostIP, port, &hints, &result);
    
        if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    
    int fd = open(fileName, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU|S_IRUSR|S_IRGRP|S_IROTH);
    
    //initialisation du socket
    int Socketfd=socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    //sendto(socket,"Hello !",strlen("Hello !"),0, result->ai_addr, result->ai_addrlen);
    
    //4a_Construction d'une requète RRQ
    
    char* rrq; 
    rrq=malloc(128*sizeof(char));  //contient la demande de lecture
    rrq[0]=0;
        rrq[1]=1;
    strcpy(rrq+2,fileName);
    strcpy(rrq+strlen(fileName)+3,"octet");
    
    int nsend =sendto(Socketfd,rrq,4+strlen(fileName)+strlen("octet"),0,result->ai_addr,result->ai_addrlen);
    if (nsend == -1){
		perror("Erreur dans le premier envoi de données");
		exit(EXIT_FAILURE);
	}
	
	
	//4b_Réception d’un fichier constitué d’un seul paquet de données (DAT) et son acquittement (ACK) ;
	
    int count = recvfrom(Socketfd, buffer, BUFFER_TAILLE,0, &adresse, &adresse_len);
	if (count == -1){
		perror("Erreur dans la réception");
		close(Socketfd);
		close(fd);
		freeaddrinfo(result);
		exit(EXIT_FAILURE);
	}
	
	printf("Reception de %d octets venant du serveur\n", count);
    //Envoi d'un acquittement (00 04 buffer[2] buffer[3]).
	//Analyse du packet reçu :
	//Si c'est un packet d'erreur (00 05) affiche message d'erreur d'erreur puis fin de programme
	//Si c'est un packet de donnée et que le numéro de block est le bon, on écrit le fichier
	
	
	
	if(buffer[0] == 0 && buffer[1] == 3){
		acq[0] = 0;
		acq[1] = 4;
		acq[2] = buffer[2];
		acq[3] = buffer[3];
    	int sendacq = sendto(Socketfd, acq, ACQ_TAILLE,0, &adresse, adresse_len);
		if (sendacq == -1){
			perror("Erreur dans le deuxième envoi de données (acquittement)");
			close(Socketfd);
			freeaddrinfo(result);
			exit(EXIT_FAILURE);
		}
	}
}		
    
    
    
    
