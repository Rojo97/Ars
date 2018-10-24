#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <unistd.h>

void error(char *funcion){
    perror(funcion);
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
    short port = 0;
    struct in_addr ip;
    int descriptor = 0;
    char cadena[] = "Hola";
    struct sockaddr_in localIp;
    struct sockaddr_in addr;
    char *mensaje = "";
    if(argc == 2){
        struct servent *portServent = getservbyname("qotd", "udp");
        if( portServent == NULL){
            error("getservbyname()");
        }
        port = portServent[0].s_port;
    }else if(argc == 4){
        if(strcmp(argv[2], "-p") != 0){
            printf("Uso incorrecto, para seleccionar puerto indiquelo con -p");//colocar bien o usar por defecto
            fflush(stdout);
            return(-1);
        } else {
            sscanf(argv[3],"%hd", &port);
            port = htons(port);
            if(port <0){
                error("htons()");
            }
        }
    } else {
        printf("Numero de argumentos incorrecto");//colocar bien
        fflush(stdout);
        return(-1);
    }
    int inetVar = inet_aton(argv[1], &ip);
    if(inetVar <0){
        error("inet_atom()");
    }
    descriptor = socket(AF_INET,SOCK_DGRAM,0);
    if(descriptor<0){
        error("socket()");
    }
    localIp.sin_family = AF_INET;
    localIp.sin_addr.s_addr = INADDR_ANY;
    int errbind = bind(descriptor, (struct sockaddr *) &localIp,sizeof(localIp));
    if(errbind){
        error("bind()");
    }
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = ip.s_addr;
    socklen_t sizeaddr = sizeof(addr);
    int senderr = sendto(descriptor, cadena ,sizeof(char)*strlen(cadena),0, (struct sockaddr*)&addr, sizeaddr);
    if(senderr<0){
        error("sendto()");
    }
    sizeaddr == -1;
    int recverr = recvfrom(descriptor, mensaje, sizeof(char)*512,0, (struct sockaddr*)&addr, &sizeaddr);
    if(recverr < 0){
        error("recvfrom()");
    }
    printf("%s",mensaje);
    int closeerr = close(descriptor);
    if(closeerr < 0){
        error("close()");
    }
    return 0;
}