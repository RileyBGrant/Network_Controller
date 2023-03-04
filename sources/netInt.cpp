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
    #ifdef TESTING
        cout << "Connecting to the network" << endl;
    #endif
    
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
            addrStr += to_string(netAddr[i]);
            addrStr += ".";
        };
        addrStr += to_string(netAddr[3]);

        #ifdef TESTING
            cout << "Stored address is " << to_string(netAddr[0]) << "." << to_string(netAddr[1]) << "." << to_string(netAddr[2]) << "." << to_string(netAddr[3]) << endl;
            cout << "Network IP address is " << addrStr << endl;
        #endif

        if (inet_pton(AF_INET, addrStr.c_str(), &serv_addr.sin_addr) <= 0) 
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

        if(valread == 0)
        {
            return 1;
        }

        #ifdef TESTING
            cout << "Message recieved with " << valread << " bytes" << endl;
        #endif

        for(int i = 0; i < valread; i++)
        {
            cout << rBuffer[i];
        }

        cout << endl;

        cout << "Message length is " << (int)rBuffer[0] << " from device " << to_string((uint8_t)rBuffer[2]);
        for(int i = 3; i < 8; i++)
        {
            cout << "." << to_string((uint8_t)rBuffer[i]);
        }
        cout << ", variable " << rbuffer[9] << " set to state " << rBuffer[11]
         
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