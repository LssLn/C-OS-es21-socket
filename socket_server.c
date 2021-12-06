/* stream socket, dominio di Internet, server */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#define TRUE 1

/*Il programma crea una socket e da il via ad un loop infinito.
Ad ogni iterazione accetta una connessione e stampa dei messaggi.
Quando la connessione viene interrotta, o arriva un messaggio di 
terminazione, il programma accetta una nuova connessione.
*/
/* Attenzione: il server va lanciato in background con ./server &
Fare attenzione al port number che stampa e poi lanciare il client con 
./client <nome> <port number>
specificando il nome dell'host del server e il port number restituito dalla printf() del codice del server */
int main (){
int sock,length;
struct sockaddr_in server;
int msgsock;
char buf[1024];
int rval;
int i;
/* create a socket */
sock=socket(AF_INET, SOCK_STREAM,0);
if (sock<0) {
perror("opening stream socket");
exit(1);
}
/* assegna il nome alla socket via wildcards*/
/* trova e stampa la porta assegnata */
server.sin_family=AF_INET;
server.sin_addr.s_addr=INADDR_ANY; 
/* accetta richieste di connessione inviate a qualunque dei suoi
indirizzi di rete, ossia a qualunque delle sue interfacce
di rete */
server.sin_port=0;
/*così il port number viene scelto dal kernel, invece di
essere fissato dal server - molti port number sono
riservati per servizi specifici */
if (bind(sock, (struct sockaddr*)&server, sizeof server) <0){
    perror("binding stream socket");
    exit(1);
}
length=sizeof server; /* equivale a sizeof(struct
sockaddr_in)
/* Ora il sistema ha assegnato un nome al socket del
server. Il server invoca quindi getsockname() per
conoscere il port number assegnato dal kernel con la
bind() */
if (getsockname (sock, (struct sockaddr *)&server,&length)<0) {
    perror("getting socket name");
    exit(1);
}
/* poi stampa il valore del port number sullo schermo
nel formato network to host short, ossia per evitare
una scorretta interpretazione del valore, converte lo
short int dalla rappresentazione di rete a quella
dell’host*/
printf("Socket port #%d\n",ntohs(server.sin_port));
/* inizia ad accettare connessioni e inizializza la coda per richieste di connessione in attesa da più clients */
listen(sock,5); /* 5 è un valore tipico */
do /* infinite loop */ {
    msgsock=accept(sock,(struct sockaddr*)0,(int*)0); 
    /*accept() accetterà una richiesta di connessione dalla coda se presente, 
    o si bloccherà in attesa di riceverne una per ogni richiesta di connessione, 
    viene creato un nuovo socket associato al descrittore msgsock e la richiesta viene rimossa dalla coda */
    if (msgsock==-1)
        perror("accept");
    else do {
        bzero(buf, sizeof buf); /* inizializza a zero la stringa buf */
        /* legge fino a 1024 byte dal socket e li stampa su schermo come stringa*/
        if ((rval=read(msgsock, buf,1024))<0)
            perror("reading stream message");
        i=0;
        if (rval==0)
            printf("Ending connection\n");
        else
            printf("-->%s\n", buf);
    } while(rval!=0);
    close (msgsock); /* chiusura connessione */
} while (TRUE);
exit(0);
}
