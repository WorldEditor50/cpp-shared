#ifndef PACKET_H
#define PACKET_H


#define MAX_BUFFER_LEN 1024
#define MAX_PACKET_PAYLOAD_LEN MAX_BUFFER_LEN - 6

struct Packet {
    unsigned short signature;
    unsigned short type;
    unsigned short datasize;
    unsigned char data[MAX_PACKET_PAYLOAD_LEN];
};

#endif // COMMON_H
