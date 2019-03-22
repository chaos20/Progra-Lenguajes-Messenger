#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4444

int main(){
	//variables del socket
	int sockfd, ret;
	struct sockaddr_in serverAddr;
	int newSocket;
	struct sockaddr_in newAddr;
	socklen_t addr_size;
	//buffers para enviar y recibir strings
	char buffer[1024];
	char inbuf[1024];
	//inicializador del fork()
	pid_t childpid;
	pid_t childpid2;

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
	//Pipes (usuarios y mensajes)
	int usersPipe[2];
	int p[2];
	if(pipe(usersPipe)<0)
		exit(1);
	//se le agrega el contador de usuarios al pipe
	write(usersPipe[1],"0",4);

	while(1){
		//esperar nueva conexion
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		//NUEVO USUARIO
    //se guarda un username para el cliente
		char username[15];
		recv(newSocket, buffer, 1024, 0);//primer mensaje es el nombres
		strcpy(username, buffer);
		printf("Nueva conexion de %s desde %s:%d\n",username, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		char newUser[20];
		strcpy(newUser, username);
		sprintf(newUser+15, "%d", newSocket);
		//Revisar cuantos usuarios hay con la primera salida del pipe, que seria la cantidad de usuarios
		char cantUs[5];
		read(usersPipe[0],cantUs,4);
		int cant = atoi(cantUs);
		sprintf(cantUs,"%d", cant+1);
		//envia al pipe la cantidad anterior +1
		write(usersPipe[1],cantUs,4);
		//se recorre el pipe la cantidad de usuarios que habian
		//queda el valor de la nueva cantidad de usuarios como primera salida
		//solo se leen y se vuelven a meter para acomodar el pipe
		for(int x = 0; x < cant; x++){
			read(usersPipe[0],buffer,20);
			write(usersPipe[1],buffer,20);
		}
		write(usersPipe[1], newUser, 20);
		printf("%s : %s : %s\n", newUser, newUser+15, cantUs);
		//==============

		if(pipe(p)<0)
			exit(1);
			//primer fork
			if((childpid2 = fork())!= 0){
				//el padre del primer fork, ejecuta otro forks
				if((childpid = fork()) != 0){
					close(sockfd);
					while(1){
						recv(newSocket, buffer, 1024, 0);

						if(strcmp(buffer+30, ":exit") == 0){
							printf("%s has disconnected from %s:%d\n",username, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
							exit(1);
						}else{
							//printf("recibiendo: %s : %s : %s \n", buffer, buffer+15, buffer+30);
		          write(p[1],buffer,1024);
						}
						bzero(buffer, sizeof(buffer));
					}
					break;
				}
				//hijo de segundo fork
				//revisa el pipe por mensajes nuevos
				else{
					while(1){
						read(p[0],buffer, 1024);//lee mensaje
						char sendUser[15];
						strcpy(sendUser, buffer+15);
						read(usersPipe[0],cantUs,4);
						int canti = atoi(cantUs);
						write(usersPipe[1],cantUs,4);
						//recorre todo el pipe de usuarios y revisa el destinatario
						for(int x = 0; x < canti; x++){
							read(usersPipe[0],inbuf,20);
							if((strcmp(inbuf,sendUser)) == 0){// si lo encuentra envia el mensaje
								int sendSocket = atoi(inbuf+15);
								printf("enviando: %s : %s : %s : %d \n", buffer, buffer+15, buffer+30, sendSocket);
								send(sendSocket, buffer,1024,0);
								printf("se envio");
							}
							write(usersPipe[1],inbuf,20);
						}
						bzero(buffer, sizeof(buffer));
						bzero(inbuf, sizeof(inbuf));
					}//--------------------------------------------
					break;
				}
			}
		}
		close(newSocket);
		return 0;
	}
