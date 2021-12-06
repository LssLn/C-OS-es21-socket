/* stream socket, dominio di Internet, client */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#define DATA "ciao"
/* Questo e’ il client. Manda via socket la stringa “ciao“ al server, che la stampa
Per l’esecuzione, occorre aver lanciato prima il server in background e annotare il port number che stampa il server e poi lanciare il client 
con ./client <nome> <port_number>,
dove nome specifica il nome dell'host del server e il port number e' quello restituito dalla printf() del codice del server */

int main (int argc,char *argv[]) {
int sock;
struct sockaddr_in server;
struct hostent *hp;
char buf[1024];
sock=socket(AF_INET, SOCK_STREAM,0);
if (sock<0) {
    perror("opening stream socket");
    exit(1);
}
server.sin_family=AF_INET;
/* ottiene l’indirizzo di rete dell’host il cui nome è stato passato come primo parametro con la funzione
gethostbyname(), che restituisce una struct di tipo hostent che contiene varie informazioni, tra cui l’ind. di rete dell’host . */
hp=gethostbyname(argv[1]);
if (hp==0){
    fprintf(stderr, "%s:unknown host0", argv[1]);
    exit(2);
}
/* copia n (3°par.) byte da source(1°par.) a destination (2°par.) */
bcopy ((char*)hp->h_addr, (char*)&server.sin_addr,hp->h_length);
/* converte lo short da formato host a form. network */
server.sin_port=htons(atoi(argv[2]));
if (connect(sock, (struct sockaddr*)&server, sizeof server)<0) {
    perror ("connecting stream socket");
    exit(1);
}
if (write(sock, DATA, sizeof DATA)<0)
    perror("writing on stream socket");
    close (sock);
    exit(0);
}