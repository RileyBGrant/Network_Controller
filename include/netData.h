#include <cstdint>
#include <ctime>

using namespace std;

struct segmentHeader //TCP
{
    uint16_t sPort;     //source port
    uint16_t dPort;     //destination port
    uint32_t seqNum;    //sequence number
    uint32_t ackNum;    //acknowledgment number
    uint8_t hlen;       //header length first 4-BITS
    bool urg;           //urgent flag
    bool ack;           //acknowledgment flag
    bool psh;           //push flag
    bool rst;           //reset flag
    bool syn;           //sync flag
    bool fin;           //finish flag
    uint16_t winSize;   //window size
    uint16_t checksum;  //checksum
    uint16_t urgPoint;  //Urgent pointer
};

//defines the structure of a TCP segment
struct segment
{
    segmentHeader header;
    void *data;
};

struct packetHeader //IPv4
{
    uint8_t version;    //verison, always 4, 4-BITS
    uint8_t hLen;       //header length, 4-BITS
    uint8_t tos;        //type of service
    uint16_t tLen;      //total length
    uint16_t id;        //identification
    bool ipFlag;        //always set to 0
    bool dfFlag;        //dont fragment flag
    bool mfFlag;        //more fragmetns flag
    uint16_t fragOff;   //fragment offset, 13-bits
    uint8_t tol;        //time to live
    uint8_t protocol;   //encapsulated protocol
    uint16_t hChecksum; //header checksum
    uint8_t sAddr[4];     //source IP address
    uint8_t dAddr[4];     //destination IP address
};

//defines the structure of a network packet
struct packet
{
    packetHeader header;
    segment dataSegment;
};

struct ethernetFrameHeader //IEEE 802.3
{
    uint8_t pream[7];   //preamble
    uint8_t sfd;        //Start of frame delimiter
    uint8_t dAddr[6];   //destination MAC address
    uint8_t sAddr[6];   //source MAC address
    uint8_t len[2];     //length
};

//defines the structure of an ethernet frame
struct ethernetFrame 
{
    ethernetFrameHeader header;
    packet dataPacket; //46-1500 bytes
    uint32_t CRC;
};

//defines functions for packing and unpacking network addresses
uint32_t packIP(uint8_t ipAddr[4]);
void unpackIP(uint32_t ipAddr, uint8_t unpackedIP[4]);

uint64_t packMAC(uint8_t macAddr[6]);
void unpackMAC(uint64_t macAddr, uint8_t unpackedIP[6]);

union char_time
{
    time_t t;
    unsigned char c[sizeof(time_t)];
};