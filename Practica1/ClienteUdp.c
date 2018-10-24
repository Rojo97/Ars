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

int main(int argc, char const *argv[])
{
    short port = 0;
    struct in_addr ip;
    int descriptor = 0;
    char cadena[] = "Hola";
    struct ifaddrs *localdir;
    struct ifaddrs *tmpdir;
    struct sockaddr_in localIp;
    struct sockaddr_in addr;
    char *mensaje;
    if(argc < 2|| argc > 4 || argc == 3){
        printf("Numero de argumentos incorrecto");//colocar bien
        fflush(stdout);
        return(-1);
    }else if(argc == 4){
        if(strcmp(argv[2], "-p") != 0){
            printf("Uso incorrecto, para seleccionar puerto indiquelo con -p");//colocar bien o usar por defecto
            fflush(stdout);
            return(-1);
        } else {
            sscanf(argv[3],"%d", &port);
            port = htons(port);
            if(port <0){
                perror("htons()");
                exit(EXIT_FAILURE);
            }
        }
    } else {
        struct servent *portServent = getservbyname("qotd", "udp");
        if( portServent == NULL){
            perror("getservbyname()");
            exit(EXIT_FAILURE);
        }
        port = portServent[0].s_port;
    }
    int inetVar = inet_aton(argv[1], &ip);
    if(inetVar <0){
        perror("inet_atom()");
        exit(EXIT_FAILURE);
    }

    descriptor = socket(AF_INET,SOCK_DGRAM,0); //Error
    localIp.sin_family = AF_INET;
    localIp.sin_addr.s_addr = INADDR_ANY;
    bind(descriptor, (struct sockaddr *) &localIp,sizeof(localIp));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = &ip;
    sendto(descriptor,&cadena,sizeof(char)*strlen(cadena),0, (struct sockaddr*)&addr, sizeof(addr));
    recvfrom(descriptor, mensaje, sizeof(char)*512,0, (struct sockaddr*)&addr, sizeof(addr) );
    printf("%s",mensaje);
    close(descriptor);
    return 0;
}