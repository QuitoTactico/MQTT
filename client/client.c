#include "mqttClient.h"

int main(int argc, char *argv[])
{
    char broker_ip[16];
    char broker_port[6]; // Asume que un puerto no superará los 5 caracteres
    char *logDir = NULL;

    // Comprueba cuántos argumentos ha proporcionado el usuario. Si sólo proporcionó el log dir, 
    // solicita la dirección IP del broker, y define el puerto como el default.
    if (argc == 2){
        printf("Enter the broker IP: ");
        scanf("%s", broker_ip);
    
        strncpy(broker_port, BROKER_PORT, sizeof(broker_port) - 1);
            
        logDir = argv[1];
    }
    else if(argc == 4){
        strncpy(broker_ip, argv[1], sizeof(broker_ip) - 1);

        strncpy(broker_port, argv[2], sizeof(broker_port) - 1);

        logDir = argv[3];
    }
    else
    {
        printf("Please use one of this options:\n./client </path/log.log>\n./client <ip> <port> </path/log.log>\n");
        return 1;
    }


    char message[500];
    int brokerSockfd;

    int connectAccepted = 0;
    while (connectAccepted != 1)
    {
        char connack[4];

        int connectQos = createConnect(message);

        brokerSockfd = connectSocket(broker_ip, broker_ip);

        send(brokerSockfd, &message, 500, 0);

        // receive connack
        recv(brokerSockfd, connack, 4, 0);

        char brokerIP[INET_ADDRSTRLEN];
        getSocketIP(brokerSockfd, brokerIP);
        DBsaveLog(logDir, brokerIP, "CONNACK", connack);

        printf("Connecting to the broker\n\n");
        connectAccepted = handleConnack(connack);
    }

    ThreadArgs args;
    args.sockfd = brokerSockfd;
    args.logDir = logDir;

    thrd_t recvThread;

    thrd_create(&recvThread, handleRecv, &args);

    char answer[20];

    for (;;)
    {
        printf("select: publish, subscribe, exit\n");
        memset(answer, 0, 20);
        scanf("%s", answer);
        printf("You selected: %s\n", answer);
        memset(message, 0, 500);
        
        if (strcmp(answer, "publish") == 0)
        {
            createPublish(message);
            send(brokerSockfd, &message, 500, 0);
            printf("publish message sent\n");
        }
        else if (strcmp(answer, "subscribe") == 0)
        {
            createSubscribe(message);
            send(brokerSockfd, &message, 500, 0);
            printf("subscribe message sent\n");
        }
        else if (strcmp(answer, "exit") == 0)
        {
            send(brokerSockfd, "q\0", 2, 0);
            printf("exiting the server\n");
            break;
        }
    }

    int res;
    thrd_join(recvThread, &res);

    // close(brokerSockfd);

    return 0;
}