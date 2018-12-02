// Practica tema 7, Rojo Álvarez Víctor
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

/** TODO
 * - Comentarios
 * - Borrar el archivo en caso de error ya que es corrupto
 * - 100 caractes nombre
 * - Opcion -v
*/

#define BUFFSIZE 516 //Tamaño maximo a recibir o enviar
#define ACKSIZE 4 //Tamaño del paquete ack

/*  Error llamada tras comprobar que alguna fución ha devuelto un error
    Recibe como parametro una cadena que indica el nombre de la función que ha fallado
    Imprime el mensaje de eror correspondiente y finaliza el programa
*/
void error(char *funcion){
    perror(funcion);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    /*Declaración de las variables a utilizar*/

    int tamBlock = BUFFSIZE;    //Tamaño que se está enviado o recibiendo
    int descriptor = 0; //Descriptor del socket
    unsigned char rw; //Lectura o escritura
    char mode[] = "octet"; //Modo de transmisión
    char sendBuff[BUFFSIZE]; //Cadena del mensaje recibido
    char recvBuff[BUFFSIZE]; //Mensaje enviado
    char ack[ACKSIZE]; //Ack para confirmar la recepción de mensajes
    struct sockaddr_in localIp; //Dirección local
    struct sockaddr_in remoteIp;    //Dirección destino
    struct in_addr ip;  //Ip de destino en network byte order
    short port = 0; //Guarda el puerto de destino en network byte order
    short expected = 1; //Numero de paquete experado
    short recived; //Numero de paquete recibido
    short type; //tipo de paquete recibido
    short vFlag = 0;    //Controla si la opcion -v está activada
    socklen_t sizeRemoteIp; //Tamaño del struct sockaddr_in remoteIp
    FILE *file;  //Archivo de lectura o escritura

    /*Comprobación de argumentos*/
    if(argc > 3|| argc < 6){
        if(strcmp(argv[2], "-r") == 0){  //Lectura
            rw = 1;
        } else if(strcmp(argv[2], "-w") == 0){ //Escritura
            rw = 2;
        } else {
            printf("Uso incorrecto, el uso del programa es el siguiente:\ntftp-client ip-servidor {-r|-w} archivo [-v]\n");
            return(-1);
        } 
        if(argc == 5 && strcmp(argv[4], "-v")==0){ //Modo -v
            vFlag = 1;
        } else {
            printf("Uso incorrecto, el uso del programa es el siguiente:\ntftp-client ip-servidor {-r|-w} archivo [-v]\n");
            return(-1);
        }
    } else { //Argumentos incorrectos
        printf("Numero de argumentos incorrecto, el uso del programa es el siguiente:\ntftp-client ip-servidor {-r|-w} archivo [-v]\n");
        return(-1);
    }

    struct servent *portServent = getservbyname("tftp", "udp"); //Busqueda del puerto por defecto
    if( portServent == NULL){
        error("getservbyname()");
    }

    /*Transformacion de la ip destino a network byte order*/
    int inetVar = inet_aton(argv[1], &ip); 
    if(inetVar <0){
        error("inet_atom()");
    }

    /*Creación del descriptor de socket*/
    descriptor = socket(AF_INET,SOCK_DGRAM,0);
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
    port = portServent[0].s_port;
    remoteIp.sin_family = AF_INET;  //Tipo de ip
    remoteIp.sin_port = port;   //Puerto de destino
    remoteIp.sin_addr.s_addr = ip.s_addr;   //Ip de destino
    sizeRemoteIp = sizeof(struct sockaddr_in);
    
    memset(sendBuff, 0, BUFFSIZE);
    memset(recvBuff,0, BUFFSIZE);
    memcpy(sendBuff+1, &rw, 1);
    strcpy(sendBuff+2, argv[3]);
    strcpy(sendBuff+3+strlen(argv[3]), mode);
    ack[0] = '\0';
    ack[1] = '\4';
    /*Enviamos un mensaje con el texto contenido en cadena al destino indicado*/
    if(vFlag==1){
        if(strcmp(argv[2], "-r")==0){
            printf("Enviada solicitud de lectura de %s a servidor tftp en %s.\n", argv[1], argv[3]);
        }else if(strcmp(argv[2], "-w")==0){
            printf("Enviada solicitud de escritura de %s a servidor tftp en %s.\n", argv[1], argv[3]);
        }
    }
    int senderr = sendto(descriptor, &sendBuff ,(strlen(argv[3])+ 4 +strlen(mode)),0, (struct sockaddr*)&remoteIp, sizeRemoteIp);
    if(senderr<0){
        error("sendto()");
    }

    if(strcmp(argv[2], "-r")==0){
        file = fopen(argv[3], "w");
        while(!(tamBlock<BUFFSIZE)){
            int recverr = recvfrom(descriptor, &recvBuff, tamBlock, 0, (struct sockaddr*)&remoteIp, &sizeRemoteIp);
            if(recverr < 0){
                error("recvfrom()");
            }
            /*Recibimos la respuesta y la guardamos en mensaje*/
            if(vFlag==1) printf("Recibido bloque del servidor tftp.\n"); 
            tamBlock = recverr;
            type = (unsigned char) recvBuff[0]*256 + (unsigned char) recvBuff[1];
            recived =(unsigned char) recvBuff[2]*256 + (unsigned char) recvBuff[3];
            if(vFlag==1 && type == 3){
                if(recived == 1){
                    printf("Es el primer bloque (numero de bloque 1).\n");
                } else if(recived!= 1){
                    printf("Es el bloque con codigo %d.\n", recived);
                } 
            }
            if(expected == recived && type == 3){
                ack[2] = recvBuff[2];
                ack[3] = recvBuff[3];
                if(vFlag==1) printf("Enviamos el ACK del bloque %d.\n", recived);
                int senderr = sendto(descriptor, &ack ,ACKSIZE,0, (struct sockaddr*)&remoteIp, sizeRemoteIp);
                if(senderr<0){
                    error("sendto()");
                }
                fwrite(recvBuff+4,1, tamBlock-4, file);
                expected++;
            }else if(type == 5){
                printf("Error: %s\n", recvBuff+4);
                exit(EXIT_FAILURE);
            } else{
                printf("Error desconocido\n");
                exit(EXIT_FAILURE);
            }
        }
    } else if(strcmp(argv[2], "-w")==0){
        file = fopen(argv[3], "r");
        expected = 0;
        sendBuff[0] = 0;
        sendBuff[1] = 3;
        while(!(tamBlock<BUFFSIZE-4)){
            int recverr = recvfrom(descriptor, &recvBuff, BUFFSIZE, 0, (struct sockaddr*)&remoteIp, &sizeRemoteIp);
            if(recverr < 0){
                error("recvfrom()");
            }
            /*Recibimos la respuesta y la guardamos en mensaje*/
            type = (unsigned char) recvBuff[0]*256 + (unsigned char) recvBuff[1];
            if(vFlag==1 && type == 4) printf("Recibido ACK del servidor tftp.\n"); 
            recived =(unsigned char) recvBuff[2]*256 + (unsigned char) recvBuff[3];
            if(vFlag==1 && type == 4){
                if(recived == 0){
                    printf("Es el primer ACK (numero de ACK %x).\n", recived);
                } else if(recived!= 0){
                    printf("Es el ACK con codigo %d.\n", recived);
                } 
            }
            if(expected == recived && type == 4){
                expected++;
                sendBuff[2] = expected/256;
                sendBuff[3] = expected%256;
                tamBlock = fread(sendBuff+4,1, BUFFSIZE-4, file);
                if(vFlag==1) printf("Enviamos el bloque  numero %d con tamaño %d.\n", expected, tamBlock);
                int senderr = sendto(descriptor, &sendBuff ,tamBlock+4,0, (struct sockaddr*)&remoteIp, sizeRemoteIp);
                if(senderr<0){
                    error("sendto()");
                }
            }else if(type == 5) {
                printf("%s\n", recvBuff+4);
                exit(EXIT_FAILURE);
            } else {
                printf("Error desconocido\n");
                exit(EXIT_FAILURE);
            }
        }
            int recverr = recvfrom(descriptor, &recvBuff, BUFFSIZE, 0, (struct sockaddr*)&remoteIp, &sizeRemoteIp);
            if(recverr < 0){
                error("recvfrom()");
            }
            /*Recibimos la respuesta y la guardamos en mensaje*/
            if(vFlag==1) printf("Recibido ACK del servidor tftp.\n"); 
            //tamBlock = recverr;
            recived =(unsigned char) recvBuff[2]*256 + (unsigned char) recvBuff[3];
            if(vFlag==1) printf("Es el ACK con codigo %d.\n", recived);
    }

    if(vFlag == 1) printf("El bloque %d era el ultimo: cerramos el fichero.\n", recived);
    fclose(file);
    /*Cerramos el socket*/
    int closeerr = close(descriptor);
    if(closeerr < 0){
        error("close()");
    }
    return 0;
}