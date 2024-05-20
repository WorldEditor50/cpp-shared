#ifndef BUFFER_HPP
#define BUFFER_HPP
#include <cstring>

class Buffer
{
public:
    unsigned char* wData;
    unsigned long wLen;
    unsigned char *rData;
    unsigned long rLen;
    unsigned long capacity;
public:
    Buffer():wData(nullptr),wLen(0),rData(nullptr),rLen(0),capacity(0){}

    int write(unsigned char* data, unsigned long len)
    {
        if (wLen >= capacity) {
            unsigned char* p = rData;
            rData = wData;
            rLen = wLen;
            wData = p;
            wLen = 0;
            memset(wData, 0, capacity);
            return 1;
        }
        memcpy(wData + wLen, data, len);
        wLen += len;
        return 0;
    }

};

#endif // BUFFER_HPP
