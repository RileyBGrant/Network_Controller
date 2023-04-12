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
#include <iomanip>

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
    linkedList_t groups; //list of devGroups that this dev is a part of
    linkedList_t rooms; //list of devRooms that this dev is a part of
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
    string inactivity(devRecord *d0);

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
    linkedList groups; //roomMember where roomMember->member is devGroup
    linkedList mems; //roomMember where roomMember->member is devRecord
    float activeProb;
};

//class for network optimiser
class netOpt
{
private:
    netInt *interface;
    linkedList_t *devices; //devRecord
    lightOptimiser lightOpt;
    linkedList_t groups; //devGroup
    linkedList_t rooms; //devRoom
    devRoom *activeRoom;

public:
    netOpt(netInt *netInterface);
    int sortDevs();
    int optimise();
    int groupRooms();
    int8_t light2Light(roomMember *m1, roomMember *m2);
    int8_t light2mainDev(roomMember *light, roomMember *mainDev);
    int8_t tv2tv(roomMember *m1, roomMember *m2);
    int activeRoomUpdate();
    int sendDevStims();

    #ifdef TESTING
        int printRooms();
        int printActivity();
    #endif
};

struct devGroup
{
    uint8_t devtype;
    linkedList_t mems; //List of devRecord
};


