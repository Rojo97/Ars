// Practica tema 6, Rojo Álvarez Víctor
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

#define BUFFSIZE 512 //Tamaño maximo a recibir

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
    short port = 0; //Guarda el puerto de destino en network byte order
    int descriptor = 0; //Descriptor del socket
    char mensaje[BUFFSIZE] = ""; //Cadena del mensaje recibido
    struct sockaddr_in remoteIp;    //Dirección destino
    struct in_addr ip;  //Ip de destino en network byte order
    int sizeRemoteIp; //Tamaño del struct sockaddr_in remoteIp
    
    /*Comprobación de argumentos*/
    if(argc == 2){ //No se indica puesto
        struct servent *portServent = getservbyname("qotd", "tcp"); //Busqueda del puerto por defecto
        if( portServent == NULL){
            error("getservbyname()");
        }
        port = portServent[0].s_port;
    }else if(argc == 4){ //Se indica puerto
        if(strcmp(argv[2], "-p") != 0){ //Uso incorrecto del programa
            printf("Uso incorrecto, el uso del programa es el siguiente:\nqotd-tcp-client-Rojo-Alvarez direccion.IP.servidor [-p puerto-servidor]\n");
            return(-1);
        } else {
            sscanf(argv[3],"%hd", &port); 
            port = htons(port); //Transformación del puerto proporcionado a network byte order
        }
    } else { //Argumentos incorrectos
        printf("Numero de argumentos incorrecto, el uso del programa es el siguiente:\nqotd-tcp-client-Rojo-Alvarez direccion.IP.servidor [-p puerto-servidor]\n");
        return(-1);
    }

    /*Transformacion de la ip a network byte order*/
    int inetVar = inet_aton(argv[1], &ip); 
    if(inetVar <0){
        error("inet_atom()");
    }

    /*Creación del descriptor de socket*/
    descriptor = socket(AF_INET,SOCK_STREAM,0);
    if(descriptor<0){
        error("socket()");
    }
    
    /*Rellenamos el struct con los datos de destino*/
    remoteIp.sin_family = AF_INET;  //Tipo de ip
    remoteIp.sin_port = port;   //Puerto de destino
    remoteIp.sin_addr.s_addr = ip.s_addr;   //Ip de destino
    sizeRemoteIp = sizeof(remoteIp);

    /*Conectamos con el destino indicado*/
    int connecterr = connect(descriptor, (struct sockaddr*)&remoteIp, sizeRemoteIp);
    if(connecterr<0){
        error("connect()");
    }

    /*Esperamos que nos manden algo y lo guardamos en mensaje*/
    int recverr = recv(descriptor, &mensaje,(int) (sizeof(char)*BUFFSIZE), 0);
    if(recverr < 0){
        error("recvfrom()");
    }

    /*Imprimimos el mensaje*/
    printf("%s",mensaje);

    /*Indicamos que queremos cerrar la conexión*/
    int shuterr = shutdown(descriptor, SHUT_RDWR);
    if(shuterr < 0){
        error("shutdown()");
    }

    /*Cerramos el socket*/
    int closeerr = close(descriptor);
    if(closeerr < 0){
        error("close()");
    }
    return 0;
}