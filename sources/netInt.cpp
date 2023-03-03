#include "networkOptimiser.h"
#include <cstdlib>

using namespace std;

netInt::netInt()
{
    sock = 0;
    for(int i = 0; i < BUFFER_LENGTH; i++)
    {
        rBuffer[i] = 0;
    }
    connectedToNetwork = false;
    for(int i = 0; i < 4; i++)
    {
        netAddr[i] = 0;
    }
    
}

netInt::netInt(uint8_t ipAddr[4])
{
    sock = 0;
    for(int i = 0; i < BUFFER_LENGTH; i++)
    {
        rBuffer[i] = 0;
    }
    connectedToNetwork = false;
    for(int i = 0; i < 4; i++)
    {
        netAddr[i] = ipAddr[i];
    }
}

netInt::~netInt()
{
    disconnectFromHost();
}

int netInt::connectToHost()
{
    if(connectedToNetwork == false)
    {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        {
            #ifdef TESTING
                cout << "Socket creation error" << endl;
            #endif

            return 1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        string addrStr = "";
        char *temp;
        for(int i = 0; i < 3; i++)
        {
            //snprintf(temp)
            //temp = itoa(netAddr[i]);
            addrStr += to_string(netAddr[i]);
            addrStr += ".";
        }
        temp = itoa(netAddr[4]);
        addrStr += temp;

        if (inet_pton(AF_INET, addrStr.c_str, &serv_addr.sin_addr) <= 0) 
        {
            #ifdef TESTING
                cout << "Invalid address/ Address not supported" << endl;
            #endif

            return 1;
        }

        if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
        {
            #ifdef TESTING
                cout << "Connection Failed" << endl;
            #endif

            return 1;
        }

        connectedToNetwork = true;

        #ifdef TESTING
            cout << "Connected to network" << endl;
        #endif

        return 0;
    }

    #ifdef TESTING
        cout << "Already connected to a network" << endl;
    #endif

    return 1;
}

int netInt::sendtoHost(void *data, int dataLen)
{
    if(connectedToNetwork == true)
    {
        send(sock, data, dataLen, 0);

        #ifdef TESTING
            cout << "Message sent with " << dataLen << " bytes" << endl;
        #endif

        return 0;
    }
    
    #ifdef TESTING
        cout << "Not connected to a network, cannot send data" << endl;
    #endif

    return 1;
}

int netInt::readFromHost()
{
    if(connectedToNetwork == true)
    {
        int valread;
        valread = read(sock, rBuffer, 1024);

        #ifdef TESTING
            cout << "Message recieved with " << valread << " bytes" << endl;
        #endif

        return 0;
    }
    
    #ifdef TESTING
        cout << "Not connected to a network, cannot read data" << endl;
    #endif

    return 1;
}

int netInt::disconnectFromHost()
{
    if(connectedToNetwork == true)
    {
        close(client_fd);
        connectedToNetwork = false;

        #ifdef TESTING
            cout << "Disconnected from network" << endl;
        #endif

        return 0;
    }

    #ifdef TESTING
        cout << "Not connected to a network" << endl;
    #endif

    return 1;
}