#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "user.c"

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

	//array de usuarios
	struct UserSocket users[10];
	int cantUsers = 0;

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


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);

		if(newSocket < 0){
			exit(1);
		}

//josh
		char username[15];
		recv(newSocket, buffer, 1024, 0);
		strcpy(username, buffer);
		printf("User: %s\n", username);

		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);

			while(1){
				recv(newSocket, buffer, 1024, 0);
				if(strcmp(buffer, ":exit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}else{
					printf("%dClient: %s\n", newSocket, buffer);
					printf("%dClient: %s\n", newSocket, buffer+15);
					printf("%dClient: %s\n", newSocket, buffer+30);
					send(newSocket, buffer, strlen(buffer), 0); //dksflsdfjsdjfls
					bzero(buffer, sizeof(buffer));
				}
			}
		}

	}

	close(newSocket);


	return 0;
}
