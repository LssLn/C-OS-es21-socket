#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>

int sock;

void *reader_thread(void *arg);

int main(){
	//stabilisco connessione con server
	struct sockaddr_in daddr;
	struct hostent *hent;
	char buff[160];
	pthread_t thid;
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	//non dobbiamo fare una bind perché non dobbiamo metterci in attesa di una porta, ma facciamo una connect
	
	daddr.sin_family = AF_INET;
	daddr.sin_port = htons(7777);
	hent = gethostbyname("localhost");	//richiediamo l'indirizzo IP dando un nome (URL) //Nel terminale: cat /etc/hosts
	daddr.sin_addr = *(struct in_addr *)*hent->h_addr_list;


	if(connect(sock, (struct sockaddr *)&daddr, sizeof(daddr)) == -1){
		perror("errore connect");
		exit(1);
	}
	
	pthread_create(&thid, NULL, reader_thread, NULL);
	
	while(1){
		printf("\nInserisci comando: ");
		fgets(buff, 160, stdin); 			//leggo un comando
		write(sock, (void *)buff, strlen(buff));	//lo trasmetto al server
		if(strncmp("close", buff, 5) == 0)		//se uguale a close interrompo il ciclo
			break;
	}

	pthread_cancel(thid);
	pthread_join(thid, NULL);
	close(sock);
	exit(0);
}

void *reader_thread(void *arg){
	char buff[300];
	char oldbuff[160];
	int len;
	while(1){
		sprintf(buff, "read");
		write(sock, (void *)buff, 4);
		len = read(sock, (void *)buff, 300);
		if(len > 0) {
			buff[len]='\0';
			if(strncmp(buff, oldbuff, 300) != 0){
				printf("%s\nInserisci comando: ", buff);
				strncpy(oldbuff, buff, 300);
			}
			
		}
		sleep(1);

	}

}









