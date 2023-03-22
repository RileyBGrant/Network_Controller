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
    linkedList_t devices;
    
public:
    netInt();
    netInt(uint8_t ipAddr[4]);
    ~netInt();
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
    uint8_t macAddr[6];
    uint8_t devType;
    linkedList_t activity;
};

struct activityRecord
{ 
    uint8_t variable;
    uint8_t state;
    time_t timestamp;
};

