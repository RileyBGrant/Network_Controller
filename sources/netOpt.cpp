#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif

#include <cstdlib>

using namespace std;

netOpt::netOpt(netInt *netInterface)
{
    interface = netInterface;    
    devices = netInterface->getDevices();

    groups.append(lightOpt.getGroups());

    activeRoom = NULL;
}

int netOpt::sortDevs()
{
    #ifdef TESTING
        cout << "Sorting Records" << endl;
    #endif
    node_t *listIterator = devices->getHead();
    devRecord *dev;
    int type;
    while(listIterator)
    {
        dev = (devRecord*)listIterator->data;

        type = dev->devType;

        #ifdef TESTING
            cout << "DevType: " << type << endl;
        #endif

        switch(type)
        {
        case 0:
            lightOpt.addDevice(dev);
            break;
        default:
            #ifdef TESTING
                cout << "DevType not found" << endl;
            #endif
        }

        listIterator = devices->getNext(listIterator);
    }

    lightOpt.printDevs();

    return 0;
}

int netOpt::optimise()
{
    #ifdef TESTING
        cout << "Optimising Devices" << endl;
    #endif

    lightOpt.groupLights();

    groupRooms();
    printRooms();

    return 0;
}

int netOpt::groupRooms()
{
    node_t *listIteratorR1 = rooms.getHead();
    node_t *listIteratorR2;
    devRoom *r1;
    devRoom *r2;
    int counterR2;
    node_t *listIteratorM1;
    int counterM1;
    node_t *listIteratorM2;
    roomMember *m1;
    roomMember *m2;
    node_t *listIteratorD1;
    devRecord *d1;
    int16_t probChange = 0;  

    //check exisiting rooms
    #ifdef TESTING
        cout << rooms.getLen() << " exisiting rooms" << endl;
        int counterR1 = 0;
    #endif
    while (listIteratorR1)
    {
        r1 = (devRoom *) listIteratorR1->data;
        listIteratorM1 = r1->groups.getHead();
        counterM1 = 0;

        #ifdef TESTING
            cout << "Checking room " << counterR1 << endl;
            counterR1 ++;
        #endif

        while(listIteratorM1)
        {
            m1 = (roomMember *)listIteratorM1->data;
            //g1 = (devGroup *) m1->member;
            listIteratorM2 = r1->groups.getNext(listIteratorM1);

            while(listIteratorM2)
            {
                m2 = (roomMember *)listIteratorM2->data;
                //g2 = (devGroup *)m2->member;

                switch(((devGroup *)m1->member)->devtype)
                {
                case 0:
                    switch (((devGroup *)m2->member)->devtype)
                    {
                    case 0:
                        probChange = light2Light(m1,m2);
                        
                        break;
                    }
                    break;
                }          

                if(probChange + m1->memberProb < 0)
                {
                    m1->memberProb = 0;
                }
                else if (probChange + m1->memberProb > 255)
                {
                    m1->memberProb = 255;
                }
                else
                {
                    m1->memberProb += probChange;
                }

                if(probChange + m2->memberProb < 0)
                {
                    m2->memberProb = 0;
                }
                else if (probChange + m2->memberProb > 255)
                {
                    m2->memberProb = 255;
                }
                else
                {
                    m2->memberProb += probChange;
                }

                listIteratorM2 = r1->groups.getNext(listIteratorM2);
            }

            if(counterM1 > 0 && m1->memberProb < 100)
            {
                listIteratorD1 = ((devGroup *)m1->member)->mems.getHead();

                while (listIteratorD1)
                {
                    d1 = (devRecord *)listIteratorD1->data;
                    listIteratorR2 = d1->rooms.getHead();
                    counterR2 = 0;

                    while(listIteratorR2)
                    {
                        r2 = (devRoom *)listIteratorR2->data;

                        if(r2 == r1)
                        {
                            listIteratorR2 = d1->rooms.getNext(listIteratorR2);
                            d1->rooms.remove(counterR2);
                        }
                        else
                        {
                            listIteratorR2 = d1->rooms.getNext(listIteratorR2);
                            counterR2++;
                        }
                    }

                    listIteratorD1 = ((devGroup *)m1->member)->mems.getNext(listIteratorD1);
                }

                delete(m1);
                listIteratorM1 = r1->groups.getNext(listIteratorM1);
                r1->groups.remove(counterM1);
            }
            else
            {
                listIteratorM1 = r1->groups.getNext(listIteratorM1);
                counterM1++;
            }
        }

        listIteratorR1 = rooms.getNext(listIteratorR1);
    }

    //Fit unroomed devices into rooms
    node_t *listIteratorL1 = groups.getHead();
    linkedList_t *l1;
    node_t *listIteratorG1;
    devGroup *g1;
    int compatability = 0;
    bool roomFound;

    #ifdef TESTING
        cout << "Rooming unsorted groups" << endl;
    #endif

    while(listIteratorL1)
    {
        l1 = (linkedList_t *)listIteratorL1->data;
        listIteratorG1 = l1->getHead();

        while(listIteratorG1)
        {
            g1 = (devGroup *)listIteratorG1->data;
            d1 = ((devRecord *)((node_t *)g1->mems.getHead())->data);
            #ifdef TESTING
                uint8_t mac[6];
                
                cout << "Lead device member is ";
                unpackMAC(d1->macAddr, mac);
                cout << hex << stoi(to_string(mac[0]));
                for(int i = 1; i < 6; i++)
                {
                    cout << "." << stoi(to_string(mac[i]));
                }
                cout << dec << " assigned to " << d1->rooms.getLen() << " rooms" << endl;
                cout << "First room pointer is " << rooms.getHead() << endl;
            #endif

            if(d1->rooms.getLen() == 0)
            {
                #ifdef TESTING
                    cout << "Goup has no room assigned" << endl;
                #endif
                m1 = new roomMember;
                m1->member = g1;
                roomFound = false;

                //check if this group fits into an existing room
                listIteratorR1 = rooms.getHead();

                while(listIteratorR1)
                {
                    compatability = 0;
                    r1 = (devRoom *)listIteratorR1->data;
                    listIteratorM2 = r1->groups.getHead();

                    while(listIteratorM2)
                    {
                        m2 = (roomMember *)listIteratorM2->data;

                        switch(((devGroup *)m1->member)->devtype)
                        {
                        case 0:
                            switch (((devGroup *)m2->member)->devtype)
                            {
                            case 0:
                                compatability += light2Light(m1,m2);   
                                break;
                            }
                            break;
                        }  

                        listIteratorM2 = r1->mems.getNext(listIteratorM2);
                    }

                    if(compatability > 0)
                    {
                        #ifdef TESTING
                            cout << "group compatible with a room" << endl;
                        #endif
                        
                        if(compatability + 128 < 0)
                        {
                            m1->memberProb = 0;
                        }
                        else if (compatability + 128 > 255)
                        {
                            m1->memberProb = 255;
                        }
                        else
                        {
                            m1->memberProb = compatability + 128;
                        }
                        r1->groups.append(m1);
                        roomFound = true;
                        listIteratorR1 = NULL;
                        listIteratorD1 = ((devGroup *)m1->member)->mems.getHead();
                    
                        while(listIteratorD1)
                        {
                            d1 = (devRecord *)listIteratorD1->data;
                            
                            d1->rooms.append(r1);

                            listIteratorD1 = ((devGroup *)m1->member)->mems.getNext(listIteratorD1);
                        }
                    }
                    else
                    {
                        listIteratorR1 = rooms.getNext(listIteratorR1);
                    }
                }

                if(roomFound == false)
                {
                    #ifdef TESTING
                        cout << "No compatible rooms, creating new room" << endl;
                    #endif

                    r1 = new devRoom;
                    m1->memberProb = 255;
                    r1->groups.append(m1);
                    rooms.append(r1);
                    listIteratorD1 = ((devGroup *)m1->member)->mems.getHead();
                    
                    while(listIteratorD1)
                    {
                        d1 = (devRecord *)listIteratorD1->data;
                        
                        d1->rooms.append(r1);

                        listIteratorD1 = ((devGroup *)m1->member)->mems.getNext(listIteratorD1);
                    }
                }
            }
            #ifdef TESTING
                else
                {
                    cout << "Group already part of a room" << endl;
                }
            #endif

            listIteratorG1 = l1->getNext(listIteratorG1);
        }     
        
        listIteratorL1 = groups.getNext(listIteratorL1);
    }

    return 0;
}

int8_t netOpt::light2Light(roomMember *m1, roomMember *m2)
{
    #ifdef TESTIN
        cout << "Light to light compatability test start" << endl;
    #endif
    
    devGroup *g1 = (devGroup *)m1->member;
    devGroup *g2 = (devGroup *)m2->member;
    node_t *listIteratorA1 = ((devRecord *)g1->mems.getHead()->data)->activity.getHead();
    node_t *listIteratorA2 = ((devRecord *)g2->mems.getHead()->data)->activity.getHead();
    activityRecord *a1;
    activityRecord *a2;
    bool devMatch = true;
    int probChange = 0;
    int timeDiff = 0;

    while(listIteratorA1 != NULL && listIteratorA2 != NULL)
    {
        bool devMatch = true;
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;

        timeDiff = a1->timestamp - a2->timestamp;
        if(timeDiff > 5 || timeDiff < -5)
        {
            devMatch = false;
        }

        if(a1->state != a2->state)
        {
            devMatch = false;
        }

        if(a1->variable != a2->variable)
        {
            devMatch = false;
        }

        if(devMatch == false)
        {
            if(probChange > -128)
            {
                probChange--;
            }
            else
            {
                probChange = -128;
            }

            if(a1->timestamp < a2->timestamp)
            {
                listIteratorA1 = g1->mems.getNext(listIteratorA1);
            }
            else if(a1->timestamp > a2->timestamp)
            {
                listIteratorA2 = g2->mems.getNext(listIteratorA2);
            }
            else
            {
                listIteratorA1 = g1->mems.getNext(listIteratorA1);
                listIteratorA2 = g2->mems.getNext(listIteratorA2);
            }
        }
        else
        {
            if(probChange < 123)
            {
                probChange = probChange + 5;
            }
            else
            {
                probChange = 127;
            } 

            listIteratorA1 = g1->mems.getNext(listIteratorA1);
            listIteratorA2 = g2->mems.getNext(listIteratorA2);
        }
    }

    #ifdef TESTIN
        cout << "Test complete, probability change of " << probChange << endl;
    #endif

    return probChange;
}

int netOpt::activeRoomUpdate() //returns time for next device stim, -1 if no preference
{
    devRecord *lastDevUpdated = interface->getLastDevUpdated();
    //check if device is assigned to a room;
    if(lastDevUpdated->rooms.getLen() > 0)
    {
        node_t *listIteratorR1;
        devRoom *r1;
        devRoom *r2;
        node_t *listIteratorG1 = lastDevUpdated->groups.getHead();
        devGroup *g1;
        node_t *listIteratorD1;
        devRecord *d1;
        bool groupChanged = true;
        uint8_t highestProb = 0;
        

        //Check if device is part of a group
        if(lastDevUpdated->groups.getLen() > 0)
        {
            uint8_t v1 = ((activityRecord *)lastDevUpdated->activity.getTail()->data)->variable;
            uint8_t v2;
            uint8_t s1 = ((activityRecord *)lastDevUpdated->activity.getTail()->data)->state;
            uint8_t s2;

            while(listIteratorG1)
            {
                g1 = (devGroup *)listIteratorG1->data;
                listIteratorD1 = g1->mems.getHead();

                while(listIteratorD1)
                {
                    d1 = (devRecord *)listIteratorD1->data;
                    v2 = ((activityRecord *)d1->activity.getTail()->data)->variable;
                    s2 = ((activityRecord *)d1->activity.getTail()->data)->state;

                    if(d1 != lastDevUpdated && (v1 != v2 || s1 != s2))
                    {
                        groupChanged = false;
                        listIteratorD1 = NULL;
                    }
                    else
                    {
                        listIteratorD1 = g1->mems.getNext(listIteratorD1);
                    }
                }
                
                listIteratorG1 = lastDevUpdated->groups.getNext(listIteratorG1);
            }
        }

        if(groupChanged == true)
        {
            node_t *listIteratorR2;
            bool devInRoom = false;

            listIteratorR1 = rooms.getHead();

            while(listIteratorR1)
            {
                r1 = (devRoom *)listIteratorR1->data;
                listIteratorR2 = lastDevUpdated->rooms.getHead();

                while(listIteratorR2)
                {
                    r2 = (devRoom *)listIteratorR2->data;

                    if(r1 == r2)
                    {
                        devInRoom = true;
                        listIteratorR2 = NULL;
                    }
                    else
                    {
                        listIteratorR2 = lastDevUpdated->rooms.getNext(listIteratorR2);
                    }
                }

                if(devInRoom == true)
                {
                    switch(lastDevUpdated->devType)
                    {
                    case 0:
                        int numLights = 0;
                        listIteratorG1 = r1->groups.getHead();

                        while(listIteratorG1)
                        {
                            g1 = (devGroup *)listIteratorG1->data;

                            if(g1->devtype == 0)
                            {
                                numLights++;
                            }

                            listIteratorG1 = r1->groups.getNext(listIteratorG1);
                        }
                        
                        if(r1->activeProb < 100)
                        {
                            r1->activeProb += 1 / numLights;
                        }
                        else
                        {
                            r1->activeProb = 100;
                        }
                        break;
                    }
                }
                else
                {
                    switch(lastDevUpdated->devType)
                    {
                    case 0:
                        if(r1->activeProb > 0)
                        {
                            r1->activeProb -= 1;

                        }
                        else
                        {
                            r1->activeProb = 0;
                        }
                        break;
                    }
                    
                }

                listIteratorR1 = rooms.getNext(listIteratorR1);
            }
        }

        listIteratorR1 = rooms.getHead();
        highestProb = 0;
        r2 = activeRoom;

        while(listIteratorR1)
        {
            r1 = (devRoom *)listIteratorR1->data;

            if(r1->activeProb > highestProb)
            {
                highestProb = r1->activeProb;
                r2 = r1;
            }

            listIteratorR1 = rooms.getNext(listIteratorR1);
        }

        if(r2 != activeRoom)
        {
            #ifdef TESTING
            time_t tempTime1 = ((activityRecord *)lastDevUpdated->activity.getTail()->data)->timestamp + 900;
            tm tempTime2 = *gmtime(&tempTime1);
                cout << "Active room changed, requesting next stim at " << asctime(&tempTime2);
            #endif
            activeRoom = r2;
            interface->requestStim((time_t)(300 + ((activityRecord *)lastDevUpdated->activity.getTail()->data)->timestamp)); //in 5 mins
            return 0;
        }
    }

    //uint8_t returnMessage[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
    //interface->sendtoHost(&returnMessage, 16);
    //interface->endBurst();

    return 0;
}

int netOpt::sendDevStims()
{
    time_t t1 = interface->getLastTimestamp();
    time_t t2;
    node_t *listIteratorD1 = devices->getHead();
    devRecord *d1;
    activityRecord *a1;
    node_t *listIteratorR1;
    devRoom * r1;
    bool inActiveRoom = false;
    uint8_t macAddr[6];
    //activityRecord *a1;

    while(listIteratorD1)
    {
        d1 = (devRecord *)listIteratorD1->data;
        
        if(d1->rooms.getLen() > 0)
        {
            switch ((int)d1->devType)
            {
            case 0: //light

                a1 = (activityRecord *)d1->activity.getTail()->data;
                if(a1->variable == 0 && a1->state == 1)
                {
                    listIteratorR1 = d1->rooms.getHead();
                    inActiveRoom = false;

                    while(listIteratorR1)
                    {
                        r1 = (devRoom *)listIteratorR1->data;

                        if(r1 == activeRoom)
                        {
                            inActiveRoom = true;
                            listIteratorR1 = NULL;
                        }
                        else
                        {
                            listIteratorR1 = d1->rooms.getNext(listIteratorR1);
                        }
                    }
                    
                    if(inActiveRoom == false)
                    {
                        #ifdef TESTING
                            cout << "Sending message to turn off device ";
                        #endif
                        
                        string message = "";

                        unpackMAC(d1->macAddr, macAddr);

                        #ifdef TESTING
                            cout << hex << stoi(to_string(macAddr[0]));
                            for(int i = 1; i < 6; i++)
                            {
                                cout << "." << stoi(to_string(macAddr[i]));
                            }
                            cout << dec << endl;
                        #endif

                        for(int i = 0; i < 6; i++) //MAC
                        {
                            message += (char)macAddr[i];
                        }

                        message += ","; 
                        message += (char)0; //varID
                        message += ",";
                        message += '0'; //signal

                        for(int i = 0; i < 6; i++)
                        {
                            message += (char)0; //padding
                        }

                        #ifdef TESTING
                            cout << "Message: " << message << endl;
                        #endif

                        interface->sendtoHost((void *)message.c_str(), REPLY_LENGTH);
                        interface->readFromHost();
                        interface->endBurst();
                    }
                }
                break;
            }
        }

        listIteratorD1 = devices->getNext(listIteratorD1);
    }

    return 0;
}

#ifdef TESTING
    int netOpt::printRooms()
    {
        cout << "Printing " << rooms.getLen() << " rooms" << endl;

        node_t *listIteratorR1 = rooms.getHead();
        devRoom *r1;
        int counterR1 = 0;
        node_t *listIteratorM1;
        roomMember *m1;
        int counterM1;
        node_t *listIteratorD1;
        devRecord *d1;
        uint8_t mac[6];

        while(listIteratorR1)
        {
            r1 = (devRoom *)listIteratorR1->data;

            cout << "Room " << counterR1 << ":" << endl;
            counterR1++;
            listIteratorM1 = r1->groups.getHead();
            counterM1 = 0;
            cout << r1->groups.getLen() << " groups" << endl;

            while(listIteratorM1)
            {
                m1 = (roomMember *)listIteratorM1->data;

                cout << "Group " << counterM1 << "(" << m1 << ", " << m1->member << ") with " << ((devGroup *)m1->member)->mems.getLen() << " members, has membership probability " << (int)m1->memberProb  << ":" << endl;
                counterM1++;
                listIteratorD1 = ((devGroup *)m1->member)->mems.getHead();

                while(listIteratorD1)
                {
                    d1 = (devRecord *)listIteratorD1->data;

                    unpackMAC(d1->macAddr, mac);
                    cout << hex << stoi(to_string(mac[0]));
                    for(int i = 1; i < 6; i++)
                    {
                        cout << "." << stoi(to_string(mac[i]));
                    }
                    cout << dec << " (" << d1 << ")" << endl;

                    listIteratorD1 = ((devGroup *)m1->member)->mems.getNext(listIteratorD1);
                }

                listIteratorM1 = r1->groups.getNext(listIteratorM1);
            }

            listIteratorM1 = r1->mems.getHead();
            cout << r1->mems.getLen() << " solo members" << endl;

            while(listIteratorM1)
            {
                m1 = (roomMember *)listIteratorM1->data;
                d1 = (devRecord *)m1->member;

                unpackMAC(d1->macAddr, mac);
                cout << hex << stoi(to_string(mac[0]));
                for(int i = 1; i < 6; i++)
                {
                    cout << "." << stoi(to_string(mac[i]));
                }
                cout << dec << " with membership probability " << (int)m1->memberProb << endl;

                listIteratorM1 = r1->mems.getNext(listIteratorM1);
            }

            listIteratorR1 = rooms.getNext(listIteratorR1);
        }
        
        return 0;
    }
#endif