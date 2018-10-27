// Practica tema 5, Rojo Álvarez Víctor
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFSIZE 512 //Tamaño maximo a enviar

/*  Error llamada tras comprobar que alguna fución ha devuelto un error
    Recibe como parametro una cadena que indica el nombre de la función que ha fallado
    Imprime el mensaje de eror correspondiente y finaliza el programa
*/
void error(char *funcion){
    perror(funcion);
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{   
    /*Declaración de las variables a utilizar*/
    short port = 0; //Guarda el puerto donde se ejecuta el servidor en network byte order
    int descriptor = 0; //Descriptor del socket
    char cadena[] = ""; //Cadena del mensaje recibido
    char mensaje[BUFFSIZE] = ""; //Mensaje a mandar
    char charbuff; //Buffer para el caracter leido desde fichero
    char qotd[BUFFSIZE]; //Cadena con el qotd a mandar
    struct sockaddr_in localIp; //Dirección local
    struct sockaddr_in addrCliente; //Dirección del cliente
    socklen_t sizeAddrCliente; //Tamaño del struct sockaddr_in addrCliente

    /*Comprobacion de argumentos*/
    if(argc == 1){ //No se indica puerto
        struct servent *portServent = getservbyname("qotd", "udp"); //Busqueda del puerto por defecto
        if( portServent == NULL){
            error("getservbyname()");
        }
        port = portServent[0].s_port;
    }else if(argc == 3){ //Se indica puerto
        if(strcmp(argv[1], "-p") != 0){ //Uso incorrecto del programa
            printf("Uso incorrecto, el uso del programa es el siguiente:\nqotd-udp-server-Rojo-Alvarez [-p puerto-servidor]\n");
            return(-1);
        } else {
            sscanf(argv[2],"%hd", &port); 
            port = htons(port); //Transformación del puerto proporcionado a network byte order
        }
    } else { //Numero de argumentos incorrecto
        printf("Numero de argumentos incorrecto, el uso del programa es el siguiente:\nqotd-udp-server-Rojo-Alvarez [-p puerto-servidor]\n");
        return(-1);
    }

    /*Creación del descriptor de socket*/
    descriptor = socket(AF_INET,SOCK_DGRAM,0);
    if(descriptor<0){
        error("socket()");
    }

    /*Rellenamos el struct con los datos locales*/
    localIp.sin_family = AF_INET; //Tipo de ip
    localIp.sin_addr.s_addr = INADDR_ANY; //Asignamos cualquier dirección de la maquina host
    localIp.sin_port = port;    //Indicamos el puerto donde ejecutar el revidor

    /*Bind del socket a la dirección local*/
    int errbind = bind(descriptor, (struct sockaddr *) &localIp,sizeof(localIp));
    if(errbind){
        error("bind()");
    }

    /*Bucle infinito donde se espera un mensaje y se manda una respuesta*/
    while(1){
        sizeAddrCliente = sizeof(struct sockaddr_in); //Calculo del tamaño del struct con la direccion del cliente

        /*Esperamos y recibimos un mensaje*/
        int recverr = recvfrom(descriptor, cadena, (int)(sizeof(char)*BUFFSIZE), 0, (struct sockaddr*)&addrCliente, &sizeAddrCliente);
        if(recverr < 0){
            error("recvfrom()");
        }

        /*Generación de la nota mediante el juego fortune*/
        system("/usr/games/fortune -s > /tmp/tt.txt"); //Ejecuta el comando y redirige la salida
        FILE *fich = fopen("/tmp/tt.txt","r");  //Abrimos el fichero con la nota
        
        /*Paso de la nota del fichero al mensaje*/
        int nc = 0; //Contador con el caracter en el que se esta
        do {
            charbuff = fgetc(fich); //Siguiente caracter
            if (charbuff != EOF){ //Si no es el final de fichero lo guardamos
               qotd[nc++] = charbuff; 
            }
        } while(nc < BUFFSIZE-1 && !feof(fich)); //Se repite hasta fin de fichero o alcanzar BUFFSIZE
        fclose(fich); //Cerramos el fichero

        /*Creacion de la cadena a mandar*/      
        strcat(mensaje,"Quote Of The Day from vm2538:\n"); //Primero cabecera del mensaje
        strcat(mensaje, qotd); //Segundo la nota generada por fortune
        strcat(mensaje, "\n"); //Salto de linea
        strcat(mensaje, "\0"); //Final de cadena
        
        /*Envio de la cadena mensaje a la direccion de origen del mensaje recibido*/
        int senderr = sendto(descriptor, mensaje ,(int) (sizeof(char)*strlen(mensaje)),0, (struct sockaddr*)&addrCliente, sizeAddrCliente);
            if(senderr<0){
                error("sendto()");
            }

        /*Reset de las cadenas empleadas*/
        memset(mensaje, 0, BUFFSIZE);
        memset(qotd, 0, BUFFSIZE);     
    }
    return 0;
}