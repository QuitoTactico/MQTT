#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 1883

/********************************/
/*                              */
/*         FIXED HEADER         */ 
/*                              */ 
/********************************/

//====================CONTROL PACKET====================

#define CONNECT   0b00010000   // 1 || CONNECT     || CLIENT TO SERVER
#define CONNACK   0b00100000   // 2 || CONNECT ACK || SERVER TO CLIENT
#define PUBLISH   0b00110000   // 3 || PUBLISH MESSAGE || BOTH WAYS
#define PUBACK    0b01000000   // 4 || PUBLISH ACK     || BOTH WAYS

// WILL NOT IMPLETEMT FOR NOW
#define PUBREC    0b01010000   // 5 || PUBLISH RECIEVE  || BOTH WAYS
#define PUBREL    0b01100010   // 6 || PUBLISH RELEASE  || BOTH WAYS
#define PUBCOMP   0b01110000   // 7 || PUBLISH COMPLETE || BOTH WAYS
//************

#define SUBSCRIBE 0b10000010   // 8 || SUBSCRIBE REQUEST || CLIENT TO SERVER
#define SUBACK    0b10010000   // 9 || SUBSCRIBE ACK     || SERVER TO CLIENT

// WILL NOT IMPLETEMT FOR NOW
#define UNSUBSCRIBE  0b10100010   // 10 || UNSUBSCRIBE REQUEST || CLIENT TO SERVER
#define UNSUBACK     0b10110000   // 11 || UNSUBSCRIBE ACK     || SERVER TO CLIENT
#define PINGREQ      0b11000000   // 12 || PING REQUEST  || CLIENT TO SERVER
#define PINGRESP     0b11010000   // 13 || PING RESPONSE || SERVER TO CLIENT
//************

#define DISCONNECT   0b11100000   // 14 || CLIENT IS DISCONNECTING || BOTH WAYS
#define AUTH         0b11110000   // 15 || AUTENTICATION EXCHANGE  || BOTH WAYS

//====================FLAGS====================

#define DUP    0b00000001 // DUPLICATE DELIVERY PUBLISH
#define QOS    0b00000110 // PUBLISH CUALITY OF SERVICE
#define RETAIN 0b00001000 // PUBLISH RETEINED MESSAGE FLAG


//==========================================

/*
typedef int32_t utf_8;

typedef struct twoBytestring {
    int16_t size;
    utf_8 *data;
};

typedef struct fourBytestring {
    int32_t size;
    utf_8 *data;
};
*/

/*
utf_8 number;
if (0xd800 <= number && number <= 0xdfff) return false;

if (number == 0) return false;
*/

struct variableHeader {
    uint16_t nameLenght;
    char name[4];
    uint8_t version;
    uint8_t connectFlags;
    uint16_t keepAlive;
    uint8_t propertiesLenght;
    uint8_t expiringIndentifier;
    uint32_t expiringInterval;
};

struct mqttControlPacket {
    uint8_t FIXEDHEADER;
    struct variableHeader VARIBALEHEADER;
    uint8_t PAYLOAD;
};

/*
FIXE HEADER

it is always in there
*/

/*
    VARIABLE HEADER

    if (QOS > 0);

    PUBLISH
    PUBACK
    PUBREC
    PUBREL
    PUBCOMP
    SUBSCRIBE
    SUBACK
    UNSUBSCRIBE
    UNSUBACK
*/

/*
    PAYLOAD

    CONNECT     required
    PUBLISH     optional
    SUBSCRIBE   required
    SUBACK      required
    UNSUBSCRIBE required
    UNSUBACK    required
*/
