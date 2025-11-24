#ifndef PACKET_H
#define PACKET_H


#define MAX_BUFFER_LEN 1024
#define MAX_PACKET_PAYLOAD_LEN MAX_BUFFER_LEN - 70

enum TcpMessageType {
    TcpMessage_Forward = 0,
    TcpMessage_Show
};

struct Packet {
    unsigned short signature;
    unsigned short type;
    unsigned short datasize;
    char src[32];
    char dst[32];
    unsigned char data[MAX_PACKET_PAYLOAD_LEN];
};

#endif // COMMON_H
