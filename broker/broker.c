#include "mqttBroker.h"

int main(int argc, char *argv[])
{
    char *logDir = NULL;
    // Comprueba si el usuario ha proporcionado el argumento logDir
    if (argc == 2){
        *logDir = argv[1];
    }
    else if(argc == 4){
        *logDir = argv[3];
    }
    else
    {
        printf("Please use one of this options:\n./broker </path/log.log>\n./broker <ip> <port> </path/log.log>\n");
        return 1;
    }


    // create the socket file descriptor with the ip, port and queue size
    int sockfd = createSocket(NULL, MY_PORT, QUEUESIZE);

    if (sockfd == -1)
    {
        return 1;
    }

    thrd_t server;
    int res;

    thrd_create(&server, handleServer, 0);

    ThreadArgs args;
    args.sockfd = sockfd;
    args.logDir = logDir;

    thrd_t t[QUEUESIZE];

    while (close_server())
    {
        for (int i = 0; i < QUEUESIZE; i++) {
            thrd_create(t + i, handleRecv, &args);
        }

        for (int i = 0; i < QUEUESIZE; i++)
        {
            int res;
            thrd_join(t[i], &res);
            if (res == -1) printf("error");
        }
    }

    thrd_join(server, &res);

    close(sockfd);

    return 0;
}