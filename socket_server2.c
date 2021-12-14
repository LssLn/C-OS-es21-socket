#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#define DIMMSG 160
#define DIMUTENTE 30

struct dati {			
	int idmsg;		
	char messaggio[DIMMSG];	
	char utente[DIMUTENTE];	
};

struct dati messaggio;	
pthread_mutex_t mutex;	

void *worker_thread(void *args);	

int main(){	
	int servsock, clisock;
	pthread_t wt;
	struct sockaddr_in baddr;
	
	/*Inizializzazione mutex*/
	pthread_mutex_init(&mutex, NULL);
	
	/* Inizializzazione valori variabile condivisa */
	messaggio.idmsg = 0;
    	strcpy(messaggio.messaggio, "Ancora nessun messaggio");
   	strcpy(messaggio.utente, "Server");

	/*Creazione socket */
	servsock = socket(AF_INET, SOCK_STREAM, 0); 
	if(servsock == -1) {
        	perror("Errore creazione socket");
        	exit(1);
    	}
	
	baddr.sin_family = AF_INET;
	baddr.sin_port = htons(7777);	
	baddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//assegnamo un indirizzo con bind
	if(bind(servsock, (struct sockaddr *)&baddr, sizeof(baddr))==-1){ 
		perror("Errore bind");
        	close(servsock);
        	exit(1);
    	}

	/* bind socket */
	if(listen(servsock, 5) == -1){
		perror("Errore listen");
        	close(servsock);
        	exit(1);
	}
	
	while(1){
		/* Il server si mette in attesa di una connessione */
		printf("Attendo connessione di un client...\n");
		clisock = accept(servsock, NULL, NULL);
		if(clisock == -1) {
           		perror("Errore accept");
           		continue;
        	}
        
        	/* Una volta avviata una nuova connessione la si fa gestire
         	* ad un worker_thread */
		printf("Nuova connessione\n");
		if(pthread_create(&wt, NULL, worker_thread, (void *)&clisock) !=0) {
			fprintf(stderr, "Errore creazione thread\n");
       		 }
        
       		 /** Questa sleep serve ad aspettare che il worker_thread 
        	 * legga e memorizzi il valore di clisock */
		 sleep(1);	
	}						//fine main thread server	
}

void *worker_thread(void *args){			//inizio del worker thread
	int sock;
	char buff[200];	
	char utente[DIMUTENTE];
	size_t len;	

	sock = (*(int *)args); 
	while(1){
		len = read(sock, (void *)buff, 199); 
		if(len > 0){
			buff[len-1]='\0';		
		}
		/* Telnet inserisce a fine stringa i caratteri \r e \n (Carriage return e new line)
         	* Se ci sono quei caratteri dobbiamo rimuoverli sovrascrivendoli con '\0'*/
		if(buff[len - 1] == '\n')
            		buff[len-1]='\0';
        	if(buff[len - 2] == '\r')
           		 buff[len-2]='\0';

		/* Controlliamo quale comando è arrivato */
		 if(len >= 9 && strncmp(buff, "register ", 9) == 0) {
            strcpy(utente, &buff[9]);
            fflush(stdout);
            snprintf(buff, 200, "\nWelcome %s!\n", utente);
            write(sock, (void *)buff, strlen(buff));
        } else if(len >= 4 && strncmp(buff, "read", 4) == 0) {
            
            pthread_mutex_lock(&mutex);
            snprintf(buff, 200, "\n%d %s: %s\n", messaggio.idmsg, 
                    messaggio.utente, messaggio.messaggio);
            pthread_mutex_unlock(&mutex);
            write(sock, (void *)buff, strlen(buff));
        } else if(len >= 5 && strncmp(buff, "send ", 5) == 0) {
            
            pthread_mutex_lock(&mutex);
            strcpy(messaggio.messaggio, &buff[5]);
            strcpy(messaggio.utente, utente);
            messaggio.idmsg++;
            pthread_mutex_unlock(&mutex);
            snprintf(buff, 200, "\nok\n");
            write(sock, (void *)buff, strlen(buff));
        } else if(len >= 5 && strncmp(buff, "close", 5) == 0) {
            break;
        } else {
            snprintf(buff, 200, "\nComando non riconoscito!\n");
            write(sock, (void *)buff, strlen(buff));
        }
    }
    
    printf("Client disconnesso\n");
    close(sock);
}




