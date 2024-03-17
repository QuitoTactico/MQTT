#include "mqttClient.h"

int main() {

    struct sockaddr sa; // IPv4

    if (inet_pton(AF_INET, "172.19.64.1", &sa) < 0)  // IPv4
    {
        return 1;
    }


    return 0;
}