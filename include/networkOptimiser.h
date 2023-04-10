#include "LList.h"
#include "netData.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

#define TESTING
#define LOG
#define MAIN_HEADER_MISSING
#define PORT 8080
#define BUFFER_LENGTH 1024
#define REPLY_LENGTH 16
#define SIM_FINISHED 255

using namespace std;

struct devRecord
{
    uint64_t macAddr;
    uint8_t devType;
    linkedList_t activity; //list of activity record types
    linkedList_t groups;
    linkedList_t rooms;
};

//Class for the network interface
class netInt
{
private:
    int sock;
    int client_fd;
    struct sockaddr_in serv_addr;
    char rBuffer[BUFFER_LENGTH];
    bool connectedToNetwork;
    time_t lastTimestamp;
    devRecord *lastDevUpdated;
    uint8_t netAddr[4];
    linkedList_t devices; //list of devRecords
    uint8_t hubAddr[6];
    
public:
    netInt();
    netInt(uint8_t ipAddr[4]);
    ~netInt();
    time_t getLastTimestamp();
    devRecord * getLastDevUpdated();
    linkedList_t *getDevices();
    int connectToHost();
    int sendtoHost(void *data, int dataLen);
    int readFromHost();
    int disconnectFromHost();
    int requestStim(time_t stimtime);
    int endBurst();

    #ifdef TESTING
        int printRecords();
    #endif
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
    linkedList_t *getGroups();
    int addDevice(devRecord *newDev);
    int groupLights();
    int inactivity(string *message);

    #ifdef TESTING
        int printDevs();
        int printGroups();
    #endif
};

struct roomMember
{
    void *member;
    uint8_t memberProb; //membership probability
};

struct devRoom
{
    linkedList groups;
    linkedList mems;
    unsigned float activeProb;
};

//class for network optimiser
class netOpt
{
private:
    netInt *interface;
    linkedList_t *devices;
    lightOptimiser lightOpt;
    linkedList_t groups;
    linkedList_t rooms;
    devRoom *activeRoom;

public:
    netOpt(netInt *netInterface);
    int sortDevs();
    int optimise();
    int groupRooms();
    int8_t light2Light(roomMember *m1, roomMember *m2);
    int activeRoomUpdate();
    int sendDevStims();

    #ifdef TESTING
        int printRooms();
    #endif
};

struct devGroup
{
    uint8_t devtype;
    linkedList_t mems; //List of groupMember structs
};


