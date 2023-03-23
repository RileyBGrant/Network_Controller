#include "LList.h"
#include "netData.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

#define TESTING
#define PORT 8080
#define BUFFER_LENGTH 1024

using namespace std;

//Class for the network interface
class netInt
{
private:
    int sock;
    int client_fd;
    struct sockaddr_in serv_addr;
    char rBuffer[BUFFER_LENGTH];
    bool connectedToNetwork;
    uint8_t netAddr[4];
    linkedList_t devices; //list of devRecords
    
public:
    netInt();
    netInt(uint8_t ipAddr[4]);
    ~netInt();
    linkedList_t *getDevices();
    int connectToHost();
    int sendtoHost(void *data, int dataLen);
    int readFromHost();
    int disconnectFromHost();

    #ifdef TESTING
        int printRecords();
    #endif
};

struct devRecord
{
    uint64_t macAddr;
    uint8_t devType;
    linkedList_t activity; //list of activity record types
    linkedList_t groups;
};

struct activityRecord
{ 
    uint8_t variable;
    uint8_t state;
    time_t timestamp;
};

class lightOptimiser
{
private:
    linkedList_t lightDevs;
    linkedList_t lightGroups;

public:
    int addDevice(devRecord *newDev);
    int groupLights();

    #ifdef TESTING
        int printDevs();
    #endif
};

//class for network optimiser
class netOpt
{
private:
    linkedList_t *devices;
    lightOptimiser lightOpt;

public:
    netOpt(netInt *interface);
    int sortDevs();
};

struct devGroup
{
    uint8_t devtype;
    linkedList_t mems; //List of groupMember structs
};

/*struct groupMember
{
    devRecord *dev;
    uint8_t memberProb; //membership probability
};*/