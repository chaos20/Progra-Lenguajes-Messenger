#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ini.h"

#include "user.c"

//#define PORT


//seleccionen uno de estos colores en el printf del mensaje
// obtenido de: https://stackoverflow.com/questions/3585846/color-text-in-terminal-applications-in-unix
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define RESET "\x1B[0m"


//parsing de el archivo config para el puerto del client
//se utiliza la libreria inih
//recuperado de: https://github.com/benhoyt/inih

// para complilar: gcc clientv2.c ini.c -o a

typedef struct
{
    int Usport;
} configuration;

static int handler(void* port, const char* section, const char* name, const char* value)
  {
    configuration* pconfig = (configuration*)port;
    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("port", "clientport")) {
        pconfig->Usport = atoi(value);
    }
    else{
      return 0;
    }
    return 1;
  }

int main(int argc, char* argv[]){
  pid_t pid;

  configuration config;
  if (ini_parse("test.ini", handler, &config) < 0) {
        printf("No se puede cargar 'test.ini'\n");
        return 1;
  }
  printf("Configuracion cargada de 'test.ini': puerto=%d\n",
        config.Usport);

  //PORT = config.Usport;

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];
    char user[15];

    //Obtencion y guardado de nombre de usuario en struct User
    //struct User user;
    printf("Ingrese el nombre de usuario: ");
    //scanf("%s", &user.name[0]);
    scanf("%s", &user[0]);
    //printf("El nombre de usuario guardado es:%s\n", user.name);
    printf("El nombre de usuario guardado es:%s\n", user);

    //Obtencion y guardado de nombre de direccion ip en struct User
    //Extraido de: https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/

    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    host_entry = gethostbyname(hostbuffer);
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    //strcpy(user.ipAddress, IPbuffer);
    //printf("%s\n", user.ipAddress);

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error en la conexion.\n");
		exit(1);
	}
	printf("[+]Socket del cliente se ha creado.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(config.Usport);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error en la conexion.\n");
		exit(1);
	}
	printf("[+]Conectado al servidor.\n");


  //una vez conectado, se envia el username como primer mensaje
  send(clientSocket, user, 1024, 0);
  //fork
  if((pid = fork()) != 0){//hijo espera que el usuario digite un mensaje y enviarlo al servidor
    while(1){
      //el formato del mensaje es el siguiente:
      //buffer [1024]
      //0-14 > Origen
      //15-29 > destino
      //30-1024 > mensaje
      strcpy(buffer, user);
      scanf("%s", buffer+30);
      //revisa si el  mensaje es para salir dek programa
      if(strcmp(buffer+30, ":exit") == 0){
  			close(clientSocket);
  			printf(RED"[-]Disconnected from server."RESET"\n");
        send(clientSocket, buffer, 1024, 0);
  			exit(1);
  		}
      else{// si no es par salir, pide destinatario
        printf(GRN"Para: \t"RESET"");
        scanf("%s", (buffer+15));
        send(clientSocket, buffer, 1024, 0);
      }
    }
  }

  else{ //padre espera recibir un mensaje
    while(1){
      if(recv(clientSocket, buffer, 1024, 0) < 0){
        printf("[-]Error in receiving data.\n");
      }
      else{
        if(strcmp(buffer,"\0") > 0){
          printf(YEL"%s : %s "RESET"\n", buffer, buffer+30);
        }
      }
      bzero(buffer, sizeof(buffer));
    }

  }
	return 0;
}
