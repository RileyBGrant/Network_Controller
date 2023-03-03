#include "LList.h"
#include "netData.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

#define TESTING
#define PORT 8080

using namespace std;

//Class for the network interface
class netInt
{
private:
    int sock;
    int client_fd;
    struct sockaddr_in serv_addr;
    char rBuffer[1024];
    bool connectedToNetwork;
    uint8_t netAddr[4];
    
public:
    netInt();
    netInt(uint8_t ipAddr[4]);
    ~netInt();
    int connectToHost();
    int sendtoHost();
    int readFromHost();
    int disconnectFromHost();
};


