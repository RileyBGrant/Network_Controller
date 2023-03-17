#include <cstdint>
#include "netData.h"

using namespace std;

//function for packing a readable IP address into 1 32-bit interger
uint32_t packIP(uint8_t ipAddr[4])
{
    return (uint32_t)(ipAddr[0] << 24 | ipAddr[1] << 16 | ipAddr[2] << 8 | ipAddr[3]);
}

//function for unpacking a 32-bit interger into a readable IP address
void unpackIP(uint32_t ipAddr, uint8_t unpackedIP[4])
{
    unpackedIP[0] = (ipAddr >> 24) & 0xFF;
    unpackedIP[1] = (ipAddr >> 16) & 0xFF;
    unpackedIP[2] = (ipAddr >> 8) & 0xFF;
    unpackedIP[3] = ipAddr & 0xFF;
}

//function for packing a readable MAC address into 1 64-bit interger
uint64_t packMAC(uint8_t macAddr[6]) //left-shift buffer is limited to shifts of less than 32 bits
{
    uint32_t packedMAC[2];
    packedMAC[0] = (uint32_t)(macAddr[0] << 8 | macAddr[1]);
    packedMAC[1] = (uint32_t)(macAddr[2] << 24 | macAddr[3] << 16 | macAddr[4] << 8 | macAddr[5]);
    return ((packedMAC[0] * (uint64_t)0x100000000) + (uint64_t)packedMAC[1]);
}

//function for unpacking a 64-bit interger into a readable MAC address
void unpackMAC(uint64_t macAddr, uint8_t unpackedIP[6])
{
    unpackedIP[0] = (macAddr >> (uint64_t)40) & 0xFF;
    unpackedIP[1] = (macAddr >> (uint64_t)32) & 0xFF;
    unpackedIP[2] = (macAddr >> 24) & 0xFF;
    unpackedIP[3] = (macAddr >> 16) & 0xFF;
    unpackedIP[4] = (macAddr >> 8) & 0xFF;
    unpackedIP[5] = macAddr & 0xFF;
}