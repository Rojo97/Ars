// Practica tema 8, Rojo Álvarez Víctor
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
#include "ip-icmp-ping.h"

#define BUFFSIZE 512 //Tamaño maximo a recibir

/*  Error llamada tras comprobar que alguna fución ha devuelto un error
    Recibe como parametro una cadena que indica el nombre de la función que ha fallado
    Imprime el mensaje de eror correspondiente y finaliza el programa
*/
void error(char *funcion){
    perror(funcion);
    exit(EXIT_FAILURE);
}

unsigned short int checksumICMP(ECHORequest *echoRequest){
    unsigned short int *puntero =(unsigned short int *) echoRequest;
    int numShorts = sizeof(ECHORequest)/2;
    unsigned int acumulador = 0;
    for(int i = 0; i < numShorts; i++){
        acumulador = acumulador + (unsigned int) *puntero;
        puntero ++;
    }
    acumulador = (acumulador >> 16) + (acumulador & 0x0000ffff);
    acumulador = (acumulador >> 16) + (acumulador & 0x0000ffff);
    acumulador = ~acumulador;
    return acumulador;
}

int main(int argc, char const *argv[])
{
    /*Declaración de las variables a utilizar*/
    int descriptor = 0; //Descriptor del socket
    ECHORequest echoRequest;
    ECHOResponse echoResponse;
    //unsigned char *sedBuff = &echoRequest; //Cadena del mensaje recibido
    //unsigned char *recvBuff = &echoResponse; //Mensaje enviado
    struct sockaddr_in localIp; //Dirección local
    struct sockaddr_in remoteIp;    //Dirección destino
    struct in_addr ip;  //Ip de destino en network byte order
    socklen_t sizeRemoteIp; //Tamaño del struct sockaddr_in remoteIp
    unsigned char vFlag=0;
    
    /*Comprobación de argumentos*/
    if(argc<2 || argc>3) { //Argumentos incorrectos
        printf("Numero de argumentos incorrecto, el uso del programa es el siguiente:\nqotd-udp-client-Rojo-Alvarez direccion.IP.servidor [-p puerto-servidor]\n");
        return(-1);
    }else if(argc == 3){ //Se indica puerto
        if(strcmp(argv[2], "-v") != 0){ //Uso incorrecto del programa
            vFlag=1;
        } else {
            printf("Uso incorrecto\n");
            return(-1);
        }
    } 
    /*Transformacion de la ip a network byte order*/
    int inetVar = inet_aton(argv[1], &ip); 
    if(inetVar <0){
        error("inet_atom()");
    }

    /*Creación del descriptor de socket*/
    descriptor = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    if(descriptor<0){
        error("socket()");
    }

    /*Rellenamos el struct con los datos locales*/
    localIp.sin_family = AF_INET; //Tipo de ip
    localIp.sin_addr.s_addr = INADDR_ANY; //Asignamos cualquier dirección de la maquina host

    /*Bind del socket a la dirección local*/
    int errbind = bind(descriptor, (struct sockaddr *) &localIp,sizeof(localIp));
    if(errbind){
        error("bind()");
    }
    
    /*Rellenamos el struct con los datos de destino*/
    remoteIp.sin_family = AF_INET;  //Tipo de ip
    remoteIp.sin_addr.s_addr = ip.s_addr;   //Ip de destino
    sizeRemoteIp = sizeof(struct sockaddr_in);

    /*Rellenamos el paquete a enviar*/
    memset(&echoRequest, 0, sizeof(echoRequest));
    memset(&echoResponse, 0, sizeof(ECHOResponse));
    echoRequest.icmpHeader.Type = 8;
    echoRequest.ID = getpid();
    strcpy(echoRequest.payload, "Hola, soy el payload");
    printf("Check antes: %d\n", checksumICMP(&echoRequest));
    echoRequest.icmpHeader.Checksum = checksumICMP(&echoRequest);
    printf("Check despues: %d\n", checksumICMP(&echoRequest));
    if(checksumICMP(&echoRequest)==0){
        printf("FUNCIONA\n");
        return(0);
    }else{
        printf("No funciona\n");
        return(0);
    }


    /*Enviamos un mensaje con el texto contenido en cadena al destino indicado*/
     int senderr = sendto(descriptor, &echoRequest ,(int) sizeof(ECHORequest),0, (struct sockaddr*)&remoteIp, sizeRemoteIp);
     if(senderr<0){
         error("sendto()");
    }

    /*Recibimos la respuesta y la guardamos en mensaje*/
    int recverr = recvfrom(descriptor, &echoResponse,(int) sizeof(ECHOResponse), 0, (struct sockaddr*)&remoteIp, &sizeRemoteIp);
     if(recverr < 0){
         error("recvfrom()");
     }

    /*Cerramos el socket*/
    int closeerr = close(descriptor);
    if(closeerr < 0){
        error("close()");
    }
    return 0;
 }