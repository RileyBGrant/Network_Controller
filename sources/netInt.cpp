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

    hubAddr[0] = (uint8_t)100;
    hubAddr[1] = (uint8_t)100;
    hubAddr[2] = (uint8_t)100;
    hubAddr[3] = (uint8_t)100;
    hubAddr[4] = (uint8_t)100;
    hubAddr[5] = (uint8_t)17;

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

    hubAddr[0] = (uint8_t)100;
    hubAddr[1] = (uint8_t)100;
    hubAddr[2] = (uint8_t)100;
    hubAddr[3] = (uint8_t)100;
    hubAddr[4] = (uint8_t)100;
    hubAddr[5] = (uint8_t)17;

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
        #ifdef TESTING
            cout << "Message to send is: ";
            for(int i = 0; i < dataLen; i++)
            {
                cout << ((char *)data)[i];
            }
            cout << endl;
        #endif
        
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
                cout << rBuffer[i] << " ";
            }

            cout << endl;
        #endif

        bool devFound = false;
        uint8_t macAddr[6];

        for(int i = 0; i < valread; )
        {
            if(rBuffer[i] == (uint8_t)hubAddr[0] && rBuffer[i + 1] == (uint8_t)hubAddr[1] && rBuffer[i + 2] == (uint8_t)hubAddr[2] && rBuffer[i + 3] == (uint8_t)hubAddr[3] && rBuffer[i + 4] == (uint8_t)hubAddr[4] && rBuffer[i + 5] == (uint8_t)hubAddr[5])
            {
                #ifdef TESTING
                    cout << "Command message";
                    cout << " at " << stoi(to_string((uint8_t)rBuffer[i + 15])) << "/" << stoi(to_string((uint8_t)rBuffer[i + 14])) + 1 << "/" << stoi(to_string((uint8_t)rBuffer[i + 13])) + 1900 << " " << stoi(to_string((uint8_t)rBuffer[i + 16])) << ":" << stoi(to_string((uint8_t)rBuffer[i + 17])) << ":" << stoi(to_string((uint8_t)rBuffer[i + 18])) << endl;
                    cout << "DevType: " << (rBuffer[i + 7] - '0')<< endl;
                    cout << "Variable: " << (int)(rBuffer[i + 9] - '0') << endl;
                    cout << "Value: " << (int)(rBuffer[i + 11] - '0') << endl;
                #endif
                
                if(rBuffer[i + 7] == (uint8_t)255 && rBuffer[i + 9] == '0' && rBuffer[i + 11] == '1')
                {
                    #ifdef TESTING
                        cout << "Server closed" << endl;
                    #endif

                    return SIM_FINISHED;
                }
                else if(rBuffer[i + 7] == (uint8_t)255 && rBuffer[i + 9] == '1' && rBuffer[i + 11] == '1')
                {
                    #ifdef TESTING
                        cout << "Stim request from hub" << endl;
                    #endif

                    tm tempTime;
                    tempTime.tm_year = (uint8_t)rBuffer[i + 13];
                    tempTime.tm_mon = (uint8_t)rBuffer[i + 14];
                    tempTime.tm_mday = (uint8_t)rBuffer[i + 15];
                    tempTime.tm_hour = (uint8_t)rBuffer[i + 16] + 1;
                    tempTime.tm_min = (uint8_t)rBuffer[i + 17];
                    tempTime.tm_sec = (uint8_t)rBuffer[i + 18];
                    lastTimestamp = mktime(&tempTime);

                    return 2;
                }
            }

            #ifdef TESTING
                cout << "Message is from device " << hex << stoi(to_string((uint8_t)rBuffer[i]));
                for(int j = 1; j < 6; j++)
                {
                    cout << "." << stoi(to_string((uint8_t)rBuffer[i + j]));
                }
                cout << dec << " devType " <<  (int)(rBuffer[i + 7] - '0') << ", variable " << (int)(rBuffer[i + 9] - '0') << " set to state " << (int)(rBuffer[i + 11] - '0') ;
                cout << " at " << stoi(to_string((uint8_t)rBuffer[i + 15])) << "/" << stoi(to_string((uint8_t)rBuffer[i + 14])) + 1 << "/" << stoi(to_string((uint8_t)rBuffer[i + 13])) + 1900 << " " << stoi(to_string((uint8_t)rBuffer[i + 16])) << ":" << stoi(to_string((uint8_t)rBuffer[i + 17])) << ":" << stoi(to_string((uint8_t)rBuffer[i + 18])) << endl;
                /*cout << "hub address " << hex << stoi(to_string((uint8_t)hubAddr[i]));
                for(int j = 1; j < 6; j++)
                {
                    cout << "." << stoi(to_string((uint8_t)hubAddr[i + j]));
                }
                cout << dec << endl;*/
            #endif

            #ifdef LOG
                ofstream ofile;

                ostringstream ss;
                ss << hex;
                for(int i = 0; i < 6; i++)
                {
                    ss << stoi(to_string((uint8_t)rBuffer[i]));
                }
                ss << dec;
                
                string csv = ss.str();

                if(!ofile.is_open())
                {
                    ofile.open("logs/CSVs/" + csv + ".csv", ios::app);
                    //cout << csv << " opened" << endl;

                    ofile << stoi(to_string((uint8_t)rBuffer[i + 13])) + 1900 << "," << stoi(to_string((uint8_t)rBuffer[i + 14])) + 1 << "," << stoi(to_string((uint8_t)rBuffer[i + 15])) << "," << stoi(to_string((uint8_t)rBuffer[i + 16])) << "," << stoi(to_string((uint8_t)rBuffer[i + 17])) << "," << stoi(to_string((uint8_t)rBuffer[i + 18])) << "," << rBuffer[i + 11] << endl;
                    //cout << t->tm_year + 1900 << "," << t->tm_mon + 1 << "," << t->tm_mday << "," << t->tm_hour << "," << t->tm_min << "," << t->tm_sec << "," << temp << endl;
                    ofile.close();
                }
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
                    dev->lastActivity.state = newEntry->state;
                    dev->lastActivity.variable = newEntry->variable;
                    dev->lastActivity.timestamp = newEntry->timestamp;

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

                for(int dayCounter = 0; dayCounter < 7; dayCounter++)
                {
                    for(int binCounter = 0; binCounter < 48; binCounter++)
                    newDev->usage.time[dayCounter][binCounter] = 0;
                }
                newDev->usage.modeTime[0] = 0;
                newDev->usage.modeTime[1] = 0;
                newDev->usage.numOfSample = 0;

                newDev->devType = rBuffer[i + 7]  - '0';
                devices.append(newDev);
                #ifdef TESTING
                    cout << "Devices recorded: " << devices.getLen() << endl;
                #endif

                newDev->activity.append(newEntry);
                lastDevUpdated = newDev;
                newDev->lastActivity.state = newEntry->state;
                newDev->lastActivity.variable = newEntry->variable;
                newDev->lastActivity.timestamp = newEntry->timestamp;

                #ifdef TESTING
                    cout << "Device activity record length: " << newDev->activity.getLen() << endl;
                #endif

                #ifdef LOG
                ofstream ofile;

                ostringstream ss;
                ss << hex;
                for(int i = 0; i < 6; i++)
                {
                    ss << stoi(to_string((uint8_t)rBuffer[i]));
                }
                ss << dec;
                
                string csv = ss.str();

                if(!ofile.is_open())
                {
                    ofile.open("logs/CSVs/" + csv + ".csv", ios::out);
                    //cout << csv << " opened" << endl;

                    ofile << stoi(to_string((uint8_t)rBuffer[i + 13])) + 1900 << "," << stoi(to_string((uint8_t)rBuffer[i + 14])) + 1 << "," << stoi(to_string((uint8_t)rBuffer[i + 15])) << "," << stoi(to_string((uint8_t)rBuffer[i + 16])) << "," << stoi(to_string((uint8_t)rBuffer[i + 17])) << "," << stoi(to_string((uint8_t)rBuffer[i + 18])) << "," << rBuffer[i + 11] << endl;
                    //cout << t->tm_year + 1900 << "," << t->tm_mon + 1 << "," << t->tm_mday << "," << t->tm_hour << "," << t->tm_min << "," << t->tm_sec << "," << temp << endl;
                    ofile.close();
                }
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

    for(int i = 0; i < 6; i++) //MAC
    {
        message += (char)hubAddr[i];
    }

    message += ","; 
    message += (char)1; //varID
    message += ",";

    char_time byteTime;
    byteTime.t = stimTime;
    for(int i = 0; i < sizeof(time_t); i++)
    {
        message += byteTime.c[i]; //value
    }

    for(int i = 0; i < 7 - sizeof(time_t); i++)
    {
        message += (char)0; //padding
    }

    #ifdef TESTING
        cout << "Stim message: " << message << endl;
    #endif

    sendtoHost((void *)message.c_str(), REPLY_LENGTH);
    //endBurst();

    return 0;
}

int netInt::endBurst()
{
    #ifdef TESTING
        cout << "Creating EoB message" << endl;
    #endif
    
    string message = "";

    for(int i = 0; i < 6; i++) //MAC
    {
        message += (char)hubAddr[i];
    }

    message += ","; 
    message += (char)0; //varID
    message += ",";
    message += '1'; //signal

    for(int i = 0; i < 6; i++)
    {
        message += (char)0; //padding
    }

    #ifdef TESTING
        cout << "EoB message: " << message << endl;
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