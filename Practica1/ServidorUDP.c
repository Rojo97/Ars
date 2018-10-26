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

void error(char *funcion){
    perror(funcion);
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
    short port = 0;
    int descriptor = 0;
    char cadena[] = "";
    struct sockaddr_in localIp;
    struct sockaddr_in addrCliente;
    socklen_t sizeaddr;
    char mensaje[512] = "";
    char charbuff;
    char qotd[512];

    if(argc == 1){
        struct servent *portServent = getservbyname("qotd", "udp");
        if( portServent == NULL){
            error("getservbyname()");
        }
        port = portServent[0].s_port;
    }else if(argc == 3){
        if(strcmp(argv[1], "-p") != 0){
            printf("Uso incorrecto, para seleccionar puerto indiquelo con -p");//colocar bien o usar por defecto
            fflush(stdout);
            return(-1);
        } else {
            sscanf(argv[2],"%hd", &port);
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
    descriptor = socket(AF_INET,SOCK_DGRAM,0);
    if(descriptor<0){
        error("socket()");
    }
    localIp.sin_family = AF_INET;
    localIp.sin_addr.s_addr = INADDR_ANY;
    localIp.sin_port = port;
    int errbind = bind(descriptor, (struct sockaddr *) &localIp,sizeof(localIp));
    if(errbind){
        error("bind()");
    }
    while(1){
        sizeaddr = sizeof(struct sockaddr_in);
        int recverr = recvfrom(descriptor, cadena, (int)(sizeof(char)*512), 0, (struct sockaddr*)&addrCliente, &sizeaddr);
            if(recverr < 0){
                error("recvfrom()");
            }
        system("/usr/games/fortune -s > /tmp/tt.txt");
        FILE *fich = fopen("/tmp/tt.txt","r");
        int nc = 0;
        do {
            charbuff = fgetc(fich);
            if (charbuff != EOF){
               qotd[nc++] = charbuff; 
            }
        } while(nc < 512-1 && !feof(fich));
        fclose(fich);
        qotd[strlen(qotd)]='\0';
        strcat(mensaje,"Quote Of The Day from vm2538:\n");
        strcat(mensaje, qotd);
        strcat(mensaje, "\n");
        printf("%s", mensaje);
        int senderr = sendto(descriptor, mensaje ,(int) (sizeof(char)*strlen(mensaje)),0, (struct sockaddr*)&addrCliente, sizeaddr);
            if(senderr<0){
                error("sendto()");
            }
        memset(mensaje, 0, 512);
        memset(qotd,0,512);     
    }
    return 0;
}