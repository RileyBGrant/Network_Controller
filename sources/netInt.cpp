#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif
#include <cstdlib>

using namespace std;

netInt::netInt()
{
    sock = 0;
    lastTimestamp = 0;
    lastDevUpdated = NULL;

    hubAddr[0] = 100;
    hubAddr[1] = 100;
    hubAddr[2] = 100;
    hubAddr[3] = 100;
    hubAddr[4] = 100;
    hubAddr[5] = 17;

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
    lastTimestamp = 0;
    lastDevUpdated = NULL;
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

time_t netInt::getLastTimestamp()
{
    return lastTimestamp;
}

devRecord * netInt::getLastDevUpdated()
{
    return lastDevUpdated;
}

linkedList_t *netInt::getDevices()
{
    return &devices;
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
            if(rBuffer[i] == hubAddr[0] && rBuffer[i + 1] == hubAddr[1] && rBuffer[i + 2] == hubAddr[2] && rBuffer[i + 3] == hubAddr[3] && rBuffer[i + 4] == hubAddr[4] && rBuffer[i + 5] == hubAddr[5])
            {
                if(rBuffer[i + 7] == 255 && rBuffer[i + 9] == 0 && rBuffer[i + 11] == 170)
                {
                    #ifdef TESTING
                        cout << "Server closed" << endl;
                    #endif

                    return 2;
                }
            }

            #ifdef TESTING
                cout << "Message is from device " << hex << stoi(to_string((uint8_t)rBuffer[i]));
                for(int j = 1; j < 6; j++)
                {
                    cout << "." << stoi(to_string((uint8_t)rBuffer[i + j]));
                }
                cout << dec << " devType " <<  rBuffer[i + 9] << ", variable " << rBuffer[i + 9] << " set to state " << rBuffer[i + 11];
                cout << " at " << stoi(to_string((uint8_t)rBuffer[i + 15])) << "/" << stoi(to_string((uint8_t)rBuffer[i + 14])) + 1 << "/" << stoi(to_string((uint8_t)rBuffer[i + 13])) + 1900 << " " << stoi(to_string((uint8_t)rBuffer[i + 16])) << ":" << stoi(to_string((uint8_t)rBuffer[i + 17])) << ":" << stoi(to_string((uint8_t)rBuffer[i + 18])) << endl;
            #endif
            
            for(int j = 0; j < 6; j++)
            {
                macAddr[j] = (uint8_t)rBuffer[i + j];
            }
            
            activityRecord *newEntry = new activityRecord;
            newEntry->variable = rBuffer[i + 9] - '0';
            newEntry->state = rBuffer[i + 11] - '0';
            tm tempTime;
            tempTime.tm_year = (uint8_t)rBuffer[i + 13];
            tempTime.tm_mon = (uint8_t)rBuffer[i + 14];
            tempTime.tm_mday = (uint8_t)rBuffer[i + 15];
            tempTime.tm_hour = (uint8_t)rBuffer[i + 16] + 1;
            tempTime.tm_min = (uint8_t)rBuffer[i + 17];
            tempTime.tm_sec = (uint8_t)rBuffer[i + 18];
            newEntry->timestamp = mktime(&tempTime);
            lastTimestamp = newEntry->timestamp;

            node_t *listIterator = devices.getHead();
            devRecord *dev;
            devFound = false;

            while (listIterator)
            {
                dev = (devRecord *)listIterator->data;
                if(dev->macAddr == packMAC(macAddr))
                {
                    devFound = true;
                    lastDevUpdated = dev;
                    #ifdef TESTING
                        cout << "Device record found" << endl;
                    #endif

                    dev->activity.append(newEntry);

                    #ifdef TESTING
                        cout << "Device activity record length: " << dev->activity.getLen() << endl;
                    #endif
                    listIterator = NULL;
                }
                else
                {
                    listIterator = devices.getNext(listIterator);
                }
            }
            if(devFound == false)
            {
                #ifdef TESTING
                    cout << "Device record not found, creating new record" << endl;
                #endif
                devRecord *newDev = new devRecord;
                
                newDev->macAddr = packMAC(macAddr);

                newDev->devType = rBuffer[i + 9]  - '0';
                devices.append(newDev);
                #ifdef TESTING
                    cout << "Devices recorded: " << devices.getLen() << endl;
                #endif

                newDev->activity.append(newEntry);
                lastDevUpdated = newDev;

                #ifdef TESTING
                    cout << "Device activity record length: " << newDev->activity.getLen() << endl;
                #endif
            }

            i += 19; //Individual message size
            #ifdef TESTIN
                printRecords();
            #endif
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

int netInt::requestStim(time_t stimTime)
{
    #ifdef TESTING
        cout << "Creating stim message" << endl;
    #endif
    
    string message = "";
    char_time byteTime;
    byteTime.t = stimTime;

    for(int i = 0; i < 6; i++) //MAC
    {
        message += (char)hubAddr[i];
    }
    cout << "Stim message: " << message << endl;
    message += ","; 
    message += (char)1; //varID
    message += ",";
    cout << "Stim message: " << message << endl;
    for(int i = 0; i < sizeof(time_t); i++)
    {
        message += byteTime.c[i]; //value
    }
    cout << "Stim message: " << message << endl;
    for(int i = 0; 7 - sizeof(time_t); i++)
    {
        message += (char)0; //padding
    }
    cout << "Stim message: " << message << " with length " << message.length() << endl;

    #ifdef TESTING
        cout << "Stim message: " << message << endl;
    #endif

    sendtoHost((void *)message.c_str(), REPLY_LENGTH);

    return 0;
}

#ifdef TESTING
    int netInt::printRecords()
    {
        cout << "Printing Records" << endl;
        node_t *listIteratorD = devices.getHead();
        node_t *listIteratorA;
        devRecord *dev;
        activityRecord *record;
        uint8_t mac[6];
        while(listIteratorD)
        {
            dev = (devRecord *)listIteratorD->data;
            unpackMAC(dev->macAddr, mac);
            cout << "Record for device " << hex << stoi(to_string(mac[0]));
            for(int i = 1; i < 6; i++)
            {
                cout << "." << stoi(to_string(mac[i]));
            }
            cout << dec << " of type " << (int)dev->devType << ":" << endl;
            
            listIteratorA = dev->activity.getHead();
            while(listIteratorA)
            {
                record = (activityRecord *)listIteratorA->data;
                tm tempTime = *gmtime(&record->timestamp);
                cout << "Variable " << (int)record->variable << " set to state " << (int)record->state << " at " << asctime(&tempTime);

                listIteratorA = dev->activity.getNext(listIteratorA);
            }

            listIteratorD = devices.getNext(listIteratorD);
        }
        return 0;
    }
#endif