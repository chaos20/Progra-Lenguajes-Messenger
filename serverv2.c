#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4444

char* scan_line(char *line)  //puede ser util para crear strings largos
{
    int ch; //as getchar() returns `int`

    if( (line = malloc(sizeof(char))) == NULL) //allocating memory
    {
        //checking if allocation was successful or not
        printf("unsuccessful allocation");
        exit(1);
    }

    line[0]='\0';

    for(int index = 0; ( (ch = getchar())!='\n' ) && (ch != EOF) ; index++)
    {
        if( (line = realloc(line, (index + 2)*sizeof(char))) == NULL )
        {
            //checking if reallocation was successful or not
            printf("unsuccessful reallocation");
            exit(1);
        }

        line[index] = (char) ch; //type casting `int` to `char`
        line[index + 1] = '\0'; //inserting null character at the end
    }

    return line;
}
// get line code


int main(){

	int sockfd, ret;
	 struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 4444);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}

  char inbuf[1024];
  int p[2], pid, nbytes;

	if (pipe(p) < 0)
		exit(1);

  while(1){
    printf("esperando coneccion\n");
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
    //aqui se debe recibir nombre del que se conecta
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
    recv(newSocket, buffer, 1024, 0);
    char username[15];
    strcpy(buffer,username);
    childpid = fork();//se abre el proceso de cliente
		if(childpid == 0){
			close(sockfd);
      childpid = fork();
      if(childpid == 0){//se recibe mensaje y se pone en el pipe
        while(1){
          printf("%s esperando mensaje :\n", username);
  				recv(newSocket, buffer, 1024, 0);
  				if(strcmp(buffer, ":exit") == 0){
  					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
  					break;
  				}
          else{
            write(p[1], buffer, 1024);
  					bzero(buffer, sizeof(buffer));
  				}
        }
      }
      else{//revisa pipe por mensajes para este cliente
        printf("revisar el pipe\n");
        nbytes = read(p[0], inbuf, 1024);
        //  revisa si este username esta en el inbuf como destino
        //  si esta le manda el mensaje
        send(newSocket, inbuf, strlen(inbuf), 0);//debe ser send al destino
        //  si no esta lo devuelve al _pipe
        printf("%s\n",inbuf);
        write(p[1], inbuf, 1024);
        bzero(buffer, sizeof(buffer));

      }
    }
	}

	close(newSocket);


	return 0;
}
