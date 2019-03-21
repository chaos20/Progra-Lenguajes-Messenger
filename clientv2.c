#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ini.h"


//#define PORT


//seleccionen uno de estos colores en el printf del mensaje
// obtenido de: https://stackoverflow.com/questions/3585846/color-text-in-terminal-applications-in-unix
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
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

// fin del codigo del parser


// funcion para enviar texto de tamaño más grande que una palabra
char* scan_line(char *line)
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

int main(int argc, char* argv[]){

  configuration config;
  if (ini_parse("test.ini", handler, &config) < 0) {
        printf("Can't load 'test.ini'\n");
        return 1;
  }
  printf("Config loaded from 'test.ini': port=%d\n",
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
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(config.Usport);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");

	while(1){
        int position = 0;

        strcpy(buffer+position, user);
        position = strlen(buffer)+1;

        printf("Digite el usuario del destinatario: \n");
        scanf("%s", &buffer[position]);
        position = strlen(buffer)+1;

        printf("Mensaje de %s: \n", user);
        scanf("%s", &buffer[position]);
/*
		printf("%s: \t", user.name);
		scanf("%s", &buffer[0]);*/
		send(clientSocket, buffer, strlen(buffer), 0);

		if(strcmp(buffer, ":exit") == 0){
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
			exit(1);
		}

		if(recv(clientSocket, buffer, 1024, 0) < 0){
			printf("[-]Error in receiving data.\n");
		}else{
			printf(YEL"Server: \t%s"RESET"\n", buffer);
		}
	}

	return 0;
}
