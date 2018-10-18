#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    short port = 0;
    struct in_addr *ip;
    if(argc < 2|| argc > 4 || argc == 3){
        printf("Numero de argumentos incorrecto");//colocar bien
        fflush(stdout);
        return(-1);
    }else if(argc == 4){
        if(strcmp(argv[3], "-p") != 0){
            printf("Uso incorrecto, para seleccionar puerto indiquelo con -p");//colocar bien o usar por defecto
            fflush(stdout);
            return(-1);
        } else {
            sscanf(argv[4],"%d", &port);
            short htonsVar = htons(port);
            if(htonsVar <0){
                perror("htons()");
                exit(EXIT_FAILURE);
            }
        }
    } else {
        struct servent *portServent = getservbyname("QOTD", "UDP");
        if( portServent == null){
            perror("getservbyname()");
            exit(EXIT_FAILURE);
        }
        port = portServent[0].s_port;
    }
    int inetVar = inet_aton(argv[1], ip);

    return 0;
}