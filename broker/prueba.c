#include <stdio.h>
#include <stdint.h>

// MQTT Fixed Header
typedef struct {
    uint8_t controlPacketType : 4;
    uint8_t reserved : 4;
    uint8_t remainingLength;
} MqttFixedHeader;

// MQTT Variable Header
typedef struct {
    // Add your variable header fields here
    // For example:
    uint16_t packetIdentifier;
    // ...
} MqttVariableHeader;

// MQTT Payload
typedef struct {
    // Add your payload fields here
    // For example:
    char* message;
    // ...
} MqttPayload;

// MQTT Packet
typedef struct {
    MqttFixedHeader fixedHeader;
    MqttVariableHeader variableHeader;
    MqttPayload payload;
} MqttPacket;

int main() {
    // Create an MQTT packet
    MqttPacket packet;

    // Set the values for the fixed header, variable header, and payload
    packet.fixedHeader.controlPacketType = 1; // Example control packet type
    packet.fixedHeader.reserved = 0;
    packet.fixedHeader.remainingLength = 10; // Example remaining length

    packet.variableHeader.packetIdentifier = 1234; // Example packet identifier

    packet.payload.message = "Hello, MQTT!"; // Example payload message

    // Print the values of the MQTT packet
    printf("Fixed Header:\n");
    printf("Control Packet Type: %d\n", packet.fixedHeader.controlPacketType);
    printf("Reserved: %d\n", packet.fixedHeader.reserved);
    printf("Remaining Length: %d\n", packet.fixedHeader.remainingLength);

    printf("\nVariable Header:\n");
    printf("Packet Identifier: %d\n", packet.variableHeader.packetIdentifier);

    printf("\nPayload:\n");
    printf("Message: %s\n", packet.payload.message);

    return 0;
}


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 8080

// make a threads func to handle the client
void *client_handler(void *arg)
{
    int clientfd = *(int *)arg;

    const int NAMESIZE = 100;
    char hostName[NAMESIZE];
    memset(&hostName, 0, NAMESIZE);
    char serviceName[NAMESIZE];
    memset(&serviceName, 0, NAMESIZE);
    int flags = NI_NUMERICHOST + NI_NUMERICSERV;

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);

    if (getnameinfo((struct sockaddr *)&their_addr, addr_size, hostName, NAMESIZE, serviceName, NAMESIZE, flags) != 0)
    {
        perror("GET NAME INFO");
        return NULL;
    }

    printf("ip conection from %s to %s\n", hostName, serviceName);

    // print the info recieved by the client

    char buf[1000];
    int len = 1000;

    while (buf[0] != 'q')
    {
        recv(clientfd, buf, len, 0);
        printf("info del cliente:\n\n%s\n\n", buf);
    }

    close(clientfd);
    return NULL;
}

int main()
{
    // create a socket

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        perror("SOCKET");
        return 1;
    }

    // define the address of the server

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // bind the address to the socket

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) != 0)
    {
        perror("BINDING");
        return 1;
    }

    return 0;
}
