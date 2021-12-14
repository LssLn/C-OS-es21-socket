#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#define DIMMSG 160
#define DIMUTENTE 30

struct dati {			//definiamo una struttura dati condivisa tra i vari thread
	int idmsg;		//id del messaggio viene incrementato di volta in volta
	char messaggio[DIMMSG];	//contiene il testo del messaggio
	char utente[DIMUTENTE];	//contiene il nome utente del mittente
};

struct dati messaggio;	//istanziamo una variabile globale che condivideranno tutti i worker thread
pthread_mutex_t mutex;	//gestiamo l'accesso con un mutex

void *worker_thread(void *args);	//definiamo la funzione del thread che verrà eseguito

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
	servsock = socket(AF_INET, SOCK_STREAM, 0); //facciamo un controllo
	if(servsock == -1) {
        	perror("Errore creazione socket");
        	exit(1);
    	}
	
	baddr.sin_family = AF_INET;
	baddr.sin_port = htons(7777);	//host to network shorts
	baddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//assegnamo un indirizzo con bind
	if(bind(servsock, (struct sockaddr *)&baddr, sizeof(baddr))==-1){ //facciamo il controllo
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
	char buff[200];	//buff è una variabile utilizzata temporaneamente per la lettura/scrittura sul socket
	char utente[DIMUTENTE];//Questa stringa locale mantiene il nome utente passato con la register
	size_t len;	

	sock = (*(int *)args); //prima faccio un casting ad int, ottengo un puntatore ad intero e io voglio il valore dell'intero quindi metto un altro asterisco
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
            /* Se è arrivato register vuol dire che la parte successiva è il nome utente
             * questo lo andiamo a copiare nella variabile utente*/
            strcpy(utente, &buff[9]);
            fflush(stdout);
            /* snprintf scrive l'output di una printf in una stringa ( massimo n caratteri ) */
            snprintf(buff, 200, "\nWelcome %s!\n", utente);
            /* inviamo la risposta */
            write(sock, (void *)buff, strlen(buff));
        } else if(len >= 4 && strncmp(buff, "read", 4) == 0) {
            /* Quando arriva read dobbiamo accedere alla variabile condivisa per
             * creare la stringa di risposta... si utilizzano i mutex per proteggere
             * l'accesso in sezione critica */
            pthread_mutex_lock(&mutex);
            snprintf(buff, 200, "\n%d %s: %s\n", messaggio.idmsg, 
                    messaggio.utente, messaggio.messaggio);
            pthread_mutex_unlock(&mutex);
            write(sock, (void *)buff, strlen(buff));
        } else if(len >= 5 && strncmp(buff, "send ", 5) == 0) {
            /* Quando arriva send dobbiamo scrivere il messaggio nella variabile
             * condivisa il messaggio è la stringa dopo send<spazio> quindi
             * copiamo solo a partire dal carattere di indice 5 */
            pthread_mutex_lock(&mutex);
            strcpy(messaggio.messaggio, &buff[5]);
            strcpy(messaggio.utente, utente);
            messaggio.idmsg++;
            pthread_mutex_unlock(&mutex);
            snprintf(buff, 200, "\nok\n");
            write(sock, (void *)buff, strlen(buff));
        } else if(len >= 5 && strncmp(buff, "close", 5) == 0) {
            /* Nel caso di close interrompiamo il ciclo */
            break;
        } else {
            snprintf(buff, 200, "\nComando non riconoscito!\n");
            write(sock, (void *)buff, strlen(buff));
        }
    }
    
    printf("Client disconnesso\n");
    close(sock);
}




