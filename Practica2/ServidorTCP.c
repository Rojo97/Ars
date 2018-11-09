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
#include <signal.h>

#define BUFFSIZE 512 //Tamaño maximo a enviar

int descriptor = 0; //Descriptor del socket declarado globalmente para la cerrarlo al pulsar ctrl+c

/*  Error llamada tras comprobar que alguna fución ha devuelto un error
    Recibe como parametro una cadena que indica el nombre de la función que ha fallado
    Imprime el mensaje de eror correspondiente y finaliza el programa
*/
void error(char *funcion){
    perror(funcion);
    exit(EXIT_FAILURE);
}

/*  Función que se desencadena al pulsar ctrl+c y que cierra la conexion para liberar el socket
    y asi poder volver a utilizarlo inmediatamente
*/
void signal_handler(int sig){
    int shutdownerr = shutdown(descriptor, SHUT_RDWR); //Iniciamos el cierre de la conexion
    if(shutdownerr<0){
        error("shutdown()");
    }

    int closerr = close(descriptor); //Cerramos el socket
    if(closerr<0){
        error("close()");
    }
}

int main(int argc, char const *argv[])
{   
    /*Declaración de las variables a utilizar*/
    short port = 0; //Guarda el puerto donde se ejecuta el servidor en network byte order
    struct sockaddr_in localIp; //Dirección local
    struct sockaddr_in addrCliente; //Dirección del cliente
    socklen_t sizeAddrCliente; //Tamaño del struct sockaddr_in addrCliente
    int descriptorHilo = 0; //Descriptor del socket de los hijos

    if(signal(SIGINT, signal_handler) == SIG_ERR){ //Asignar la señal ctrl+c a la funcion signal_handler
        error("signal()");
    }
    /*Comprobacion de argumentos*/
    if(argc == 1){ //No se indica puerto
        struct servent *portServent = getservbyname("qotd", "tcp"); //Busqueda del puerto por defecto
        if( portServent == NULL){
            error("getservbyname()");
        }
        port = portServent[0].s_port;
    }else if(argc == 3){ //Se indica puerto
        if(strcmp(argv[1], "-p") != 0){ //Uso incorrecto del programa
            printf("Uso incorrecto, el uso del programa es el siguiente:\nqotd-tcp-server-Rojo-Alvarez [-p puerto-servidor]\n");
            return(-1);
        } else {
            sscanf(argv[2],"%hd", &port); 
            port = htons(port); //Transformación del puerto proporcionado a network byte order
        }
    } else { //Numero de argumentos incorrecto
        printf("Numero de argumentos incorrecto, el uso del programa es el siguiente:\nqotd-tcp-server-Rojo-Alvarez [-p puerto-servidor]\n");
        return(-1);
    }

    /*Creación del descriptor de socket*/
    descriptor = socket(AF_INET,SOCK_STREAM,0);
    if(descriptor<0){
        error("socket()");
    }

    /*Rellenamos el struct con los datos locales*/
    localIp.sin_family = AF_INET; //Tipo de ip
    localIp.sin_addr.s_addr = INADDR_ANY; //Asignamos cualquier dirección de la maquina host
    localIp.sin_port = port;    //Indicamos el puerto donde ejecutar el revidor

    /*Bind del socket a la dirección local*/
    int errbind = bind(descriptor, (struct sockaddr *) &localIp,sizeof(localIp));
    if(errbind<0){
        error("bind()");
    }

    /*Indicamos que el puerto es de escucha y esperamos peticiones*/
    int listenErr = listen(descriptor, 5);
    if(listenErr<0){
        error("listen()");
    }

    /*Bucle infinito donde se espera una conexion y se manda una mensaje tambien se declara el tamaño de la cola, 5.*/
    while(1){
        sizeAddrCliente = sizeof(struct sockaddr_in); //Calculo del tamaño del struct con la direccion del cliente

        /*Aceptamos una peticion*/
        descriptorHilo = accept(descriptor,(struct sockaddr *) &addrCliente, &sizeAddrCliente);
        if(descriptorHilo<0){
            error("accept()");
        }

        /*Creamos un hijo para atender la petición*/
        if(fork() == 0){
            /*Cada hijo declara sus variables*/
            char mensaje[BUFFSIZE] = ""; //Mensaje a mandar
            char charbuff; //Buffer para el caracter leido desde fichero
            char qotd[BUFFSIZE]; //Cadena con el qotd a mandar

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
            qotd[nc++] = 0; //Añadimos un cero al final de la nota

            fclose(fich); //Cerramos el fichero

            /*Creacion de la cadena a mandar*/      
            strcpy(mensaje,"Quote Of The Day from vm2538:\n"); //Primero cabecera del mensaje
            strcat(mensaje, qotd); //Segundo la nota generada por fortune

            /*Envio de la cadena mensaje a la direccion de origen del mensaje recibido*/
            int senderr = send(descriptorHilo, mensaje ,(int) (sizeof(char)*strlen(mensaje)),0);
            if(senderr<0){
                error("sendto()");
            }

            /*Cerramos el socket del hijo*/
            int closerr = close(descriptorHilo);
            if(closerr<0){
                error("close()");
            }

            /*El hijo finaliza*/
            return 0;
        } 
    }
    return 0;
}