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

            for(int i = 0; i < valread; i++)
            {
                cout << rBuffer[i];
            }

            cout << endl;
        #endif

        bool devFound = false;
        uint8_t macAddr[6];

        for(int i = 0; i < valread; )
        {
            #ifdef TESTING
                cout << "Message is from device " << hex << stoi(to_string((uint8_t)rBuffer[i]));
                for(int j = 1; j < 6; j++)
                {
                    cout << "." << stoi(to_string((uint8_t)rBuffer[i + j]));
                }
                cout << dec << ", variable " << rBuffer[i + 7] << " set to state " << rBuffer[i + 9];
                cout << " at " << stoi(to_string((uint8_t)rBuffer[i + 13])) << "/" << stoi(to_string((uint8_t)rBuffer[i + 12])) + 1 << "/" << stoi(to_string((uint8_t)rBuffer[i + 11])) + 1900 << " " << stoi(to_string((uint8_t)rBuffer[i + 14])) << ":" << stoi(to_string((uint8_t)rBuffer[i + 15])) << ":" << stoi(to_string((uint8_t)rBuffer[i + 16])) << endl;
            #endif

            
            for(int j = 0; j < 6; j++)
            {
                macAddr[j] = stoi(to_string((uint8_t)rBuffer[i + j]));
            }

            node_t *listIterator = devices.getHead();
            devRecord *dev;
            devFound = false;

            activityRecord *newEntry = new activityRecord;
            newEntry->variable = rBuffer[i + 9];
            newEntry->state = rBuffer[i + 7];
            newEntry->timestamp.tm_year = stoi(to_string((uint8_t)rBuffer[i + 13]));
            newEntry->timestamp.tm_mon = stoi(to_string((uint8_t)rBuffer[i + 14]));
            newEntry->timestamp.tm_mday = stoi(to_string((uint8_t)rBuffer[i + 15]));
            newEntry->timestamp.tm_hour = stoi(to_string((uint8_t)rBuffer[i + 16]));
            newEntry->timestamp.tm_min = stoi(to_string((uint8_t)rBuffer[i + 17]));
            newEntry->timestamp.tm_sec = stoi(to_string((uint8_t)rBuffer[i + 18]));
            time_t tempTime = mktime(&newEntry->timestamp);
            newEntry->timestamp = *gmtime(&tempTime);

            while (listIterator)
            {
                dev = (devRecord *)listIterator->data;
                if(packMAC(dev->macAddr) == packMAC(macAddr))
                {
                    devFound = true;
                    dev->activity.append(newEntry);
                    listIterator = NULL;
                }
                else
                {
                    listIterator = devices.getNext(listIterator);
                }
            }
            if(devFound = false)
            {
                devRecord *newDev = new devRecord;
                for(int j = 0; j < 6; j++)
                {
                    newDev->macAddr[j] = macAddr[j];
                }
                newDev->devType = rBuffer[i + 9];
                devices.append(newDev);

                newDev->activity.append(newEntry);
            }

            i += 19; //Individual message size
        }
        



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

#ifdef TESTING
    int netInt::printRecords()
    {
        node_t *listIteratorD = devices.getHead();
        node_t *listIteratorA;
        devRecord *dev;
        activityRecord *record;
        while(listIteratorD)
        {
            dev = (devRecord *)listIteratorD->data;
            cout << "Record for device " << hex << dev->macAddr[0];
            for(int i = 1; i < 6; i++)
            {
                cout << "." << dev->macAddr[i];
            }
            cout << dec << ":" << endl;
            
            listIteratorA = dev->activity.getHead();
            while(listIteratorA)
            {
                record = (activityRecord *)listIteratorA->data;
                cout << "Variable " << record->variable << " set to state " << record->state;

                listIteratorA = dev->activity.getNext(listIteratorA);
            }

            listIteratorD = devices.getNext(listIteratorD);
        }
        return 0;
    }
#endif