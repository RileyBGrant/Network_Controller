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
    groups.append(speakerOpt.getGroups());

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
            {
                lightOpt.addDevice(dev);
                break;
            }

            case 2:
            {
                speakerOpt.addDevice(dev);
            }

            #ifdef TESTING    
                default:
                {
                    
                        cout << "Not a group DevType not found" << endl;
                    
                }
            #endif
        }

        listIterator = devices->getNext(listIterator);
    }

    lightOpt.printDevs();
    speakerOpt.printDevs();

    return 0;
}

int netOpt::optimise()
{
    #ifdef TESTING
        cout << "Optimising Devices" << endl;
    #endif

    lightOpt.groupLights();
    speakerOpt.groupSpeakers();

    groupRooms();
    #ifdef TESTING
        printRooms();
    #endif

    return 0;
}

int netOpt::groupRooms()
{
    node_t *listIteratorR1 = rooms.getHead();
    int counterR1 = 0;
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
        
        printRooms();
    #endif
    while (listIteratorR1)
    {
        r1 = (devRoom *) listIteratorR1->data;
        listIteratorM1 = r1->groups.getHead();
        counterM1 = 0;

        #ifdef TESTING
            cout << "Checking room " << counterR1 << endl;
        #endif

        while(listIteratorM1)
        {
            m1 = (roomMember *)listIteratorM1->data;
            probChange = 0;
            //g1 = (devGroup *) m1->member;
            listIteratorM2 = r1->groups.getNext(listIteratorM1);

            while(listIteratorM2)
            {
                m2 = (roomMember *)listIteratorM2->data;
                //g2 = (devGroup *)m2->member;

                switch(((devGroup *)m1->member)->devtype)
                {
                    case 0: //light
                    {
                        switch (((devGroup *)m2->member)->devtype)
                        {
                            case 0: //light
                            { 
                                probChange += light2light(m1,m2);
                                break;
                            }
                            case 2: //speaker
                            {
                                probChange += light2mainGroup(m1,m2);
                                break;
                            }
                        }
                        break;
                    }
                    case 2: //speaker
                    {
                        switch (((devGroup *)m2->member)->devtype)
                        {
                            case 0: //light
                            { 
                                probChange += light2mainGroup(m2,m1);
                                break;
                            }
                            case 2: //speaker
                            {
                                probChange += speaker2speaker(m1,m2);
                                break;
                            }
                        }
                        break;
                    }
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

            listIteratorM2 = r1->mems.getHead();

            while(listIteratorM2)
            {
                m2 = (roomMember *)listIteratorM2->data;

                switch(((devRecord *)m1->member)->devType)
                {
                    case 0: //light
                    {
                        switch (((devGroup *)m2->member)->devtype)
                        {
                            case 1: //tv
                            {
                                probChange += light2mainDev(m1,m2);                                
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += light2mainDev(m1,m2);                                
                                break;
                            }
                        }
                        break;
                    }
                    case 2: //speaker
                    {
                        switch (((devGroup *)m2->member)->devtype)
                        {
                            case 1: //tv
                            {
                                probChange += tv2speaker(m2,m1);                                
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += speaker2audioDev(m1,m2);                                
                                break;
                            }
                        }
                        break;
                    }
                }          

                if(probChange + 128 < 0)
                {
                    m1->memberProb = 0;
                }
                else if (probChange + 128 > 255)
                {
                    m1->memberProb = 255;
                }
                else
                {
                    m1->memberProb = probChange + 128;
                }

                if(probChange + 128 < 0)
                {
                    m2->memberProb = 0;
                }
                else if (probChange + 128 > 255)
                {
                    m2->memberProb = 255;
                }
                else
                {
                    m2->memberProb = probChange + 128;
                }

                listIteratorM2 = r1->mems.getNext(listIteratorM2);
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

        listIteratorM1 = r1->mems.getHead();
        counterM1 = 0;

        while(listIteratorM1)
        {
            m1 = (roomMember *)listIteratorM1->data;
            probChange = 0;

            listIteratorM2 = r1->mems.getNext(listIteratorM1);

            while(listIteratorM2)
            {
                m2 = (roomMember *)listIteratorM2->data;

                switch(((devRecord *)m1->member)->devType)
                {
                    case 1: //tv
                    {
                        switch (((devRecord *)m2->member)->devType)
                        {
                            case 1: //tv
                            {
                                probChange += tv2tv(m1,m2);
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += tv2mainDev(m1,m2);
                                break;
                            }
                        }
                        break;
                    }
                    case 3: //Record player
                    {
                        switch (((devRecord *)m2->member)->devType)
                        {
                            case 1: //tv
                            {
                                probChange += tv2mainDev(m2,m1);
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += recordPlayer2recordPlayer(m1,m2);
                                break;
                            }
                        }
                        break;
                    }
                    
                }
      

                if(probChange + 128 < 0)
                {
                    m1->memberProb = 0;
                }
                else if (probChange + 128 > 255)
                {
                    m1->memberProb = 255;
                }
                else
                {
                    m1->memberProb = probChange + 128;
                }

                if(probChange + 128 < 0)
                {
                    m2->memberProb = 0;
                }
                else if (probChange + 128 > 255)
                {
                    m2->memberProb = 255;
                }
                else
                {
                    m2->memberProb = probChange + 128;
                }

                listIteratorM2 = r1->mems.getNext(listIteratorM2);
            }

            if(counterM1 + r1->groups.getLen() > 0 && m1->memberProb < 100)
            {
                #ifdef TESTING
                    cout << "Device no longer in room";
                #endif

                d1 = (devRecord *)m1->member;

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

                delete(m1);
                listIteratorM1 = r1->mems.getNext(listIteratorM1);
                r1->mems.remove(counterM1);
            }
            else
            {
                #ifdef TESTIN
                    cout << "Device still in room" << endl;
                #endif
                listIteratorM1 = r1->mems.getNext(listIteratorM1);
                counterM1++;
            }
        }

        if(r1->groups.getLen() + r1->mems.getLen() < 2)
        {
            #ifdef TESTING
                cout << "Room with 1 member deleting room" << endl;
            #endif

            if(r1->groups.getLen() == 1)
            {
                m1 = ((roomMember *)r1->groups.getHead()->data);

                listIteratorD1 = ((devGroup *)m1->member)->mems.getHead();

                while(listIteratorD1)
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
            }

            if(r1->mems.getLen() == 1)
            {
                #ifdef TESTING
                    cout << "Removing solo member";
                #endif

                d1 = (devRecord *)m1->member;

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

                delete(m1);

                #ifdef TESTIN
                    cout << "solo member removed" << endl;
                #endif
            }

            delete(r1);
            listIteratorR1 = rooms.getNext(listIteratorR1);
            rooms.remove(counterR1);

            #ifdef TESTING
                cout << "room removed" << endl;
            #endif
        }
        else
        {
            listIteratorR1 = rooms.getNext(listIteratorR1);
            counterR1++;
        }
        
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
                cout << "First room pointer is " << d1->rooms.getHead() << endl;
            #endif

            if(d1->rooms.getLen() == 0)
            {
                #ifdef TESTING
                    cout << "Group has no room assigned" << endl;
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
                            case 0: //light
                            {
                                switch (((devGroup *)m2->member)->devtype)
                                {
                                    case 0: //light
                                    {
                                        compatability += light2light(m1,m2);   
                                        break;
                                    }
                                    case 2: //speaker
                                    {
                                        compatability += light2mainGroup(m1,m2);   
                                        break;
                                    }
                                }
                                break;
                            }
                            case 2: //speaker
                            {
                                switch (((devGroup *)m2->member)->devtype)
                                {
                                    case 0: //light
                                    {
                                        compatability += light2mainGroup(m2,m1);   
                                        break;
                                    }
                                    case 2: //speaker
                                    {
                                        compatability += speaker2speaker(m1,m2);   
                                        break;
                                    }
                                }
                                break;
                            }     
                        }  

                        listIteratorM2 = r1->mems.getNext(listIteratorM2);
                    }

                    listIteratorM2 = r1->mems.getHead();

                    while(listIteratorM2)
                    {
                        m2 = (roomMember *)listIteratorM2->data;

                        switch(((devGroup *)m1->member)->devtype)
                        {
                            case 0: //light
                            {
                                switch (((devRecord *)m2->member)->devType)
                                {
                                    case 1: //tv
                                    {
                                        compatability += light2mainDev(m1,m2);   
                                        break;
                                    }
                                    case 3: //Record player
                                    {
                                        compatability += light2mainDev(m1,m2);   
                                        break;
                                    }
                                }
                                break;
                            }
                            case 2: //speaker
                            {
                                switch (((devRecord *)m2->member)->devType)
                                {
                                    case 1: //tv
                                    {
                                        compatability += tv2speaker(m2,m1);   
                                        break;
                                    }
                                    case 3: //Record player
                                    {
                                        compatability += speaker2audioDev(m1,m2);   
                                        break;
                                    }
                                }
                                break;
                            }
                        }  

                        listIteratorM2 = r1->mems.getNext(listIteratorM2);
                    }

                    #ifdef TESTING
                        cout << "Room compatability: " << compatability << endl;
                    #endif

                    if(compatability > 0)
                    {
                        #ifdef TESTING
                            cout << "Group compatible with a room" << endl;
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
                    r1->activeProb = 0;
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

    listIteratorD1 = devices->getHead();

    while(listIteratorD1)
    {
        d1 = (devRecord *)listIteratorD1->data;

        #ifdef TESTING
            uint8_t mac[6];
            
            cout << "Device is ";
            unpackMAC(d1->macAddr, mac);
            cout << hex << stoi(to_string(mac[0]));
            for(int i = 1; i < 6; i++)
            {
                cout << "." << stoi(to_string(mac[i]));
            }
            cout << dec << " assigned to " << d1->rooms.getLen() << " rooms" << endl;
            cout << "First room pointer is " << d1->rooms.getHead() << endl;
        #endif

        if(d1->rooms.getLen() == 0)
        {
            #ifdef TESTING
                cout << "Device has no room assigned" << endl;
            #endif
            m1 = new roomMember;
            m1->member = d1;
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

                    switch(((devRecord *)m1->member)->devType)
                    {
                        case 1: //tv
                        {
                            switch (((devGroup *)m2->member)->devtype)
                            {
                                case 0: //light
                                {
                                    compatability += light2mainDev(m2,m1);   
                                    break;
                                }
                                case 2: //speaker
                                {
                                    compatability += tv2speaker(m1,m2);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 3: //Record player
                        {
                            switch (((devGroup *)m2->member)->devtype)
                            {
                                case 0: //light
                                {
                                    compatability += light2mainDev(m2,m1);   
                                    break;
                                }
                                case 2: //speaker
                                {
                                    compatability += speaker2audioDev(m2,m1);   
                                    break;
                                }
                            }
                            break;
                        }
                    }  

                    listIteratorM2 = r1->mems.getNext(listIteratorM2);
                }

                listIteratorM2 = r1->mems.getHead();

                while(listIteratorM2)
                {
                    m2 = (roomMember *)listIteratorM2->data;
                    

                    switch(((devRecord *)m1->member)->devType)
                    {
                        case 1: //tv
                        {
                            switch (((devRecord *)m2->member)->devType)
                            {
                                case 1: //tv
                                {
                                    compatability += tv2tv(m1,m2);   
                                    break;
                                }
                                case 3: //Record player
                                {
                                    compatability += tv2mainDev(m1,m2);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 3: //Record player
                        {
                            switch (((devRecord *)m2->member)->devType)
                            {
                                case 1: //tv
                                {
                                    compatability += tv2mainDev(m2,m1);   
                                    break;
                                }
                                case 3: //Record player
                                {
                                    compatability += recordPlayer2recordPlayer(m1,m2);   
                                    break;
                                }
                            }
                            break;
                        }
                    }  

                    listIteratorM2 = r1->mems.getNext(listIteratorM2);
                }

                #ifdef TESTING
                    cout << "Room compatibility: " << compatability << endl;
                #endif

                if(compatability > 0)
                {
                    #ifdef TESTING
                        cout << "Solo dev compatible with a room" << endl;
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
                    r1->mems.append(m1);
                    roomFound = true;
                    ((devRecord *)m1->member)->rooms.append(r1);

                    listIteratorR1 = NULL;
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
                r1->activeProb = 0;
                r1->mems.append(m1);
                rooms.append(r1);
                
                d1->rooms.append(r1);
            }
        }
        #ifdef TESTING
            else
            {
                cout << "Group already part of a room" << endl;
            }
        #endif

        listIteratorD1 = devices->getNext(listIteratorD1);
    }

    return 0;
}

int8_t netOpt::light2light(roomMember *m1, roomMember *m2)
{
    devGroup *g1 = (devGroup *)m1->member;
    devGroup *g2 = (devGroup *)m2->member;
    
    #ifdef TESTIN
        uint8_t mac[6];
        unpackMAC(((devRecord *)g1->mems.getHead()->data)->macAddr, mac);
        cout << "Compatability test between light group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(((devRecord *)g2->mems.getHead()->data)->macAddr, mac);
        cout << dec << " and light group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif
    
    
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

int8_t netOpt::light2mainDev(roomMember *light, roomMember *mainDev)
{
    devGroup *l1 = (devGroup *)light->member;
    devRecord *d1 = (devRecord *)mainDev->member;

    #ifdef TESTING
    uint8_t mac[6];
        unpackMAC(((devRecord *)l1->mems.getHead()->data)->macAddr, mac);
        cout << "Compatability test between light group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(d1->macAddr, mac);
        cout << dec << " and main dev " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif

    if(((devRecord *)l1->mems.getHead()->data)->activity.getLen() <= 2 || d1->activity.getLen() < 1)
    {
        #ifdef TESTIN
            cout << "activity records are too short" << endl;
        #endif

        return -1;
    }

    node_t *listIteratorA1 = ((devRecord *)l1->mems.getHead()->data)->activity.getHead();
    node_t *listIteratorA2 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA1);
    node_t *listIteratorA3 = d1->activity.getHead();
    activityRecord *a1;
    activityRecord *a2;
    activityRecord *a3;

    int probChange = 0;
    
    while(listIteratorA1 != NULL && listIteratorA2 != NULL && listIteratorA3 != NULL)
    {
        #ifdef TESTIN
            cout << listIteratorA1->data << " " << listIteratorA2->data << " " << listIteratorA3->data << endl;
        #endif
        
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;
        a3 = (activityRecord *)listIteratorA3->data;

        #ifdef  TESTING
            //tm tempTime;
            //tempTime = *gmtime(&a1->timestamp);
            cout << "a1: " << listIteratorA1 << ", variable " << (int)a1->variable << ", state " << (int)a1->state << ", timestamp " << a1->timestamp << endl;
            //tempTime = *gmtime(&a2->timestamp);
            cout << "a2: " << listIteratorA2 << ", variable " << (int)a2->variable << ", state " << (int)a2->state << ", timestamp " << a2->timestamp << endl;
            //tempTime = *gmtime(&a3->timestamp);
            cout << "a3: " << listIteratorA3 << ", variable " << (int)a3->variable << ", state " << (int)a3->state << ", timestamp " << a3->timestamp << endl;
        #endif

        if(a1->variable == 0 && a1->state == 1)
        { 
            if(a2->variable == 0 && (a2->state == 0 || a2->state == 2))
            {
                if(a3->variable == 0 && (a3->state == 3 || a3->state == 4) && a3->timestamp >= a1->timestamp)
                {
                    if(a2->timestamp > a3->timestamp)
                    {
                        if(probChange <= 77)
                        {
                            probChange = probChange + 50;
                        }
                        else
                        {
                            probChange = 127;
                        }

                        listIteratorA3 = d1->activity.getNext(listIteratorA3);
                    }
                    else
                    {
                        if(probChange >= -127)
                        {
                            probChange -= 1;
                        }
                        else
                        {
                            probChange = -128;
                        }

                        listIteratorA1 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA2);
                        if(listIteratorA1 != NULL)
                        {
                            listIteratorA2 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA1);
                        }
                        
                    }
                }
                else
                {
                    #ifdef TESTING
                        cout << "a3 invalid: " << endl;
                    #endif
                    listIteratorA3 = d1->activity.getNext(listIteratorA3);
                }
            }
            else
            {
                #ifdef TESTING
                    cout << "a2 invalid" << endl;
                #endif
                listIteratorA2 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA2);
            }
        }
        else
        {
            #ifdef TESTING
                cout << "a1 invalid" << endl;
            #endif
            
            listIteratorA1 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA1);
            if(listIteratorA1 != NULL)
            {
                listIteratorA2 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA1);
            }
        }

        #ifdef TESTING
            cout << "Compatability: " << probChange << endl;
        #endif
    }

    #ifdef TESTIN
        cout << "Compatability: " << probChange << endl;
    #endif

    return probChange;
}

int8_t netOpt::light2mainGroup(roomMember *light, roomMember *mainGroup)
{
    devGroup *l1 = (devGroup *)light->member;
    devGroup *g1 = (devGroup *)mainGroup->member;

    #ifdef TESTIN
    uint8_t mac[6];
        unpackMAC(((devRecord *)l1->mems.getHead()->data)->macAddr, mac);
        cout << "Compatability test between light group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(((devRecord *)g1->mems.getHead()->data)->macAddr, mac);
        cout << dec << " and main group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif

    if(((devRecord *)l1->mems.getHead()->data)->activity.getLen() <= 2 || ((devRecord *)g1->mems.getHead()->data)->activity.getLen() < 1)
    {
        #ifdef TESTIN
            cout << "activity records are too short" << endl;
        #endif

        return -1;
    }

    node_t *listIteratorA1 = ((devRecord *)l1->mems.getHead()->data)->activity.getHead();
    node_t *listIteratorA2 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA1);
    node_t *listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getHead();
    activityRecord *a1;
    activityRecord *a2;
    activityRecord *a3;

    int probChange = 0;
    
    while(listIteratorA1 != NULL && listIteratorA2 != NULL && listIteratorA3 != NULL)
    {
        #ifdef TESTIN
            cout << listIteratorA1->data << " " << listIteratorA2->data << " " << listIteratorA3->data << endl;
        #endif
        
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;
        a3 = (activityRecord *)listIteratorA3->data;

        #ifdef  TESTIN
            //tm tempTime;
            //tempTime = *gmtime(&a1->timestamp);
            cout << "a1: " << listIteratorA1 << ", variable " << (int)a1->variable << ", state " << (int)a1->state << ", timestamp " << a1->timestamp << endl;
            //tempTime = *gmtime(&a2->timestamp);
            cout << "a2: " << listIteratorA2 << ", variable " << (int)a2->variable << ", state " << (int)a2->state << ", timestamp " << a2->timestamp << endl;
            //tempTime = *gmtime(&a3->timestamp);
            cout << "a3: " << listIteratorA3 << ", variable " << (int)a3->variable << ", state " << (int)a3->state << ", timestamp " << a3->timestamp << endl;
        #endif

        if(a1->variable == 0 && a1->state == 1)
        { 
            if(a2->variable == 0 && (a2->state == 0 || a2->state == 2))
            {
                if(a3->variable == 0 && (a3->state == 3 || a3->state == 4) && a3->timestamp >= a1->timestamp)
                {
                    if(a2->timestamp > a3->timestamp)
                    {
                        if(probChange <= 117)
                        {
                            probChange = probChange + 10;
                        }
                        else
                        {
                            probChange = 127;
                        }

                        listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                    }
                    else
                    {
                        if(probChange >= -123)
                        {
                            probChange -= 5;
                        }
                        else
                        {
                            probChange = -128;
                        }

                        listIteratorA1 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA2);
                        if(listIteratorA1 != NULL)
                        {
                            listIteratorA2 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA1);
                        }
                        
                    }
                }
                else
                {
                    #ifdef TESTIN
                        cout << "a3 invalid: " << endl;
                    #endif
                    listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                }
            }
            else
            {
                #ifdef TESTIN
                    cout << "a2 invalid" << endl;
                #endif
                listIteratorA2 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA2);
            }
        }
        else
        {
            #ifdef TESTIN
                cout << "a1 invalid" << endl;
            #endif
            
            listIteratorA1 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA1);
            if(listIteratorA1 != NULL)
            {
                listIteratorA2 = ((devRecord *)l1->mems.getHead()->data)->activity.getNext(listIteratorA1);
            }
        }

        #ifdef TESTIN
            cout << "Compatability: " << probChange << endl;
        #endif
    }

    #ifdef TESTIN
        cout << "Compatability: " << probChange << endl;
    #endif

    return probChange;
}

int8_t netOpt::tv2tv(roomMember *m1, roomMember *m2)
{
    devRecord *d1 = (devRecord *)m1->member;
    devRecord *d2 = (devRecord *)m2->member;

    #ifdef TESTIN
        uint8_t mac[6];
        unpackMAC(d1->macAddr, mac);
        cout << "Compatability test between tv " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(d2->macAddr, mac);
        cout << dec << " and tv " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif

    if(d1->activity.getLen() < 2 || d2->activity.getLen() < 2)
    {
        #ifdef TESTIN
            cout << "activity records are too short" << endl;
        #endif

        return -1;
    }

    node_t *listIteratorA1 = d1->activity.getHead();
    node_t *listIteratorA2 = d1->activity.getNext(listIteratorA1);
    node_t *listIteratorA3 = d2->activity.getHead();
    node_t *listIteratorA4 = d2->activity.getNext(listIteratorA3);
    activityRecord *a1;
    activityRecord *a2;
    activityRecord *a3;
    activityRecord *a4;

    int probChange = 0;
    int timeDiff = 0;

    while(listIteratorA1 != NULL && listIteratorA2 != NULL && listIteratorA3 != NULL && listIteratorA4 != NULL)
    {
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;
        a3 = (activityRecord *)listIteratorA3->data;
        a4 = (activityRecord *)listIteratorA4->data;
        timeDiff = a1->timestamp - a3->timestamp;

        #ifdef TESTIN
            cout << "a1: " << listIteratorA1 << ", variable " << (int)a1->variable << ", state " << (int)a1->state << ", timestamp " << a1->timestamp << endl;
            cout << "a2: " << listIteratorA2 << ", variable " << (int)a2->variable << ", state " << (int)a2->state << ", timestamp " << a2->timestamp << endl;
            cout << "a3: " << listIteratorA3 << ", variable " << (int)a3->variable << ", state " << (int)a3->state << ", timestamp " << a3->timestamp << endl;
            cout << "a4: " << listIteratorA4 << ", variable " << (int)a4->variable << ", state " << (int)a4->state << ", timestamp " << a4->timestamp << endl;
        #endif

        if(a1->variable == a3->variable && a1->state == a3->state && (timeDiff < 5 && timeDiff > -5))
        {
            if(probChange <= 117)
            {
                probChange = probChange + 10;
            }
            else
            {
                probChange = 127;
            }
            
            listIteratorA1 = d1->activity.getNext(listIteratorA1);
            if(listIteratorA1 != NULL)
            {
                listIteratorA2 = d1->activity.getNext(listIteratorA1);
            }
            listIteratorA3 = d2->activity.getNext(listIteratorA3);
            if(listIteratorA3 != NULL)
            {
               listIteratorA4 = d2->activity.getNext(listIteratorA3); 
            }
        }
        else
        {
            if(a1->variable == 0 && (a1->state == 3 || a1->state == 4))
            {
                if(a2->variable == 0 && (a2->state == 0 || a2->state == 1 || a2->state == 2))
                {
                    if(a3->variable == 0 && (a3->state == 3 || a3->state == 4))
                    {
                        if(a4->variable == 0 && (a4->state == 0 || a4->state == 1 || a4->state == 2))
                        {
                            if(a1->timestamp < a3->timestamp)
                            {
                                if(a2->timestamp > a3->timestamp)
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }

                                    listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                    }
                                }
                                else
                                {
                                    listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                       listIteratorA2 = d1->activity.getNext(listIteratorA1); 
                                    }
                                }
                            }
                            else if(a1->timestamp > a3->timestamp)
                            {
                                if(a4->timestamp > a1->timestamp)
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }

                                    listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                    }
                                }
                                else
                                {
                                    listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                    }
                                }
                            }
                            else
                            {
                                if(a2->timestamp > a4->timestamp)
                                {
                                    listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                    }
                                }
                                else if(a2->timestamp < a4->timestamp)
                                {
                                    listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                    }
                                }
                                else
                                {
                                    listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                    }
                                    listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                    }
                                }
                            }
                        }
                        else
                        {
                            listIteratorA4 = d2->activity.getNext(listIteratorA4);
                        }
                    }
                    else
                    {
                        listIteratorA3 = d2->activity.getNext(listIteratorA3);
                        if(listIteratorA3 != NULL)
                        {
                            listIteratorA4 = d2->activity.getNext(listIteratorA3);
                        }
                    }
                }
                else
                {
                    listIteratorA2 = d1->activity.getNext(listIteratorA2);
                }
            }
            else
            {
                listIteratorA1 = d1->activity.getNext(listIteratorA1);
                if(listIteratorA1 != NULL)
                {
                    listIteratorA2 = d1->activity.getNext(listIteratorA1);
                }
            }
        }

        #ifdef TESTING
            cout << "Compatability: " << probChange << endl;
        #endif
    }

    return probChange;
}

int8_t netOpt::tv2speaker(roomMember *tv, roomMember *speaker)
{
    devRecord *d1 = (devRecord *)tv->member;
    devGroup *g1 = (devGroup *)speaker->member;

    #ifdef TESTIN
        uint8_t mac[6];
        unpackMAC(d1->macAddr, mac);
        cout << "Compatability test between tv " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(((devRecord *)g1->mems.getHead()->data)->macAddr, mac);
        cout << dec << " and speaker group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif

    if(d1->activity.getLen() < 2 || ((devRecord *)g1->mems.getHead()->data)->activity.getLen() < 2)
    {
        #ifdef TESTIN
            cout << "activity records are too short" << endl;
        #endif

        return -1;
    }

    node_t *listIteratorA1 = d1->activity.getHead();
    node_t *listIteratorA2 = d1->activity.getNext(listIteratorA1);
    node_t *listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getHead();
    node_t *listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
    activityRecord *a1;
    activityRecord *a2;
    activityRecord *a3;
    activityRecord *a4;

    int probChange = 0;
    int timeDiff = 0;

    while(listIteratorA1 != NULL && listIteratorA2 != NULL && listIteratorA3 != NULL && listIteratorA4 != NULL)
    {
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;
        a3 = (activityRecord *)listIteratorA3->data;
        a4 = (activityRecord *)listIteratorA4->data;
        timeDiff = a1->timestamp - a3->timestamp;

        #ifdef TESTIN
            cout << "a1: " << listIteratorA1 << ", variable " << (int)a1->variable << ", state " << (int)a1->state << ", timestamp " << a1->timestamp << endl;
            cout << "a2: " << listIteratorA2 << ", variable " << (int)a2->variable << ", state " << (int)a2->state << ", timestamp " << a2->timestamp << endl;
            cout << "a3: " << listIteratorA3 << ", variable " << (int)a3->variable << ", state " << (int)a3->state << ", timestamp " << a3->timestamp << endl;
            cout << "a4: " << listIteratorA4 << ", variable " << (int)a4->variable << ", state " << (int)a4->state << ", timestamp " << a4->timestamp << endl;
        #endif

        if(a1->variable == 0 && (a1->state == 3 || a1->state == 4))
        {
            if(a2->variable == 0 && (a2->state == 0 || a2->state == 1 || a2->state == 2))
            {
                if(a3->variable == 0 && a3->state != 0 && a3->state != 1)
                {
                    if(a4->variable == 0 && a4->state != a3->state)
                    {
                        if(a1->timestamp <= a3->timestamp)
                        {
                            if(a2->timestamp > a3->timestamp)
                            {
                                if(a3->state == 2)
                                {
                                    if(probChange <= 117)
                                    {
                                        probChange = probChange + 10;
                                    }
                                    else
                                    {
                                        probChange = 127;
                                    }
                                }
                                else
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }
                                }

                                listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                if(listIteratorA3 != NULL)
                                {
                                    listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                }
                            }
                            else
                            {
                                if(probChange >= -127)
                                {
                                    probChange -= 1;
                                }
                                else
                                {
                                    probChange = -128;
                                }
                                
                                listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                if(listIteratorA1 != NULL)
                                {
                                    listIteratorA2 = d1->activity.getNext(listIteratorA1); 
                                }
                            }
                        }
                        else //a1->timestamp > a3->timestamp
                        {
                            if(a4->timestamp > a1->timestamp)
                            {
                                if(a3->state == 3)
                                {
                                    if(probChange <= 117)
                                    {
                                        probChange = probChange + 10;
                                    }
                                    else
                                    {
                                        probChange = 127;
                                    }
                                }
                                else
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }
                                }

                                listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                if(listIteratorA1 != NULL)
                                {
                                    listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                }
                            }
                            else
                            {
                                if(a3->state = 3)
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 5;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }
                                }
                                
                                listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                if(listIteratorA3 != NULL)
                                {
                                    listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                }
                            }
                        }
                    }
                    else
                    {
                        listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA4);
                    }
                }
                else
                {
                    listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                    if(listIteratorA3 != NULL)
                    {
                        listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                    }
                }
            }
            else
            {
                listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA2);
            }
        }
        else
        {
            listIteratorA1 = d1->activity.getNext(listIteratorA1);
            if(listIteratorA1 != NULL)
            {
                listIteratorA2 = d1->activity.getNext(listIteratorA1);
            }
        }

        #ifdef TESTING
            cout << "Compatability: " << probChange << endl;
        #endif
    }

    return probChange;
}

int8_t netOpt::tv2mainDev(roomMember *tv, roomMember *mainDev)
{
    devRecord *d1 = (devRecord *)tv->member;
    devRecord *d2 = (devRecord *)mainDev->member;

    #ifdef TESTING
        uint8_t mac[6];
        unpackMAC(d1->macAddr, mac);
        cout << "Compatability test between tv " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(d2->macAddr, mac);
        cout << dec << " and main dev " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif

    if(d1->activity.getLen() < 2 || d2->activity.getLen() < 2)
    {
        #ifdef TESTIN
            cout << "activity records are too short" << endl;
        #endif

        return -1;
    }

    node_t *listIteratorA1 = d1->activity.getHead();
    node_t *listIteratorA2 = d1->activity.getNext(listIteratorA1);
    node_t *listIteratorA3 = d2->activity.getHead();
    node_t *listIteratorA4 = d2->activity.getNext(listIteratorA3);
    activityRecord *a1;
    activityRecord *a2;
    activityRecord *a3;
    activityRecord *a4;

    int probChange = 0;
    int timeDiff = 0;

    while(listIteratorA1 != NULL && listIteratorA2 != NULL && listIteratorA3 != NULL && listIteratorA4 != NULL)
    {
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;
        a3 = (activityRecord *)listIteratorA3->data;
        a4 = (activityRecord *)listIteratorA4->data;
        timeDiff = a1->timestamp - a3->timestamp;

        #ifdef TESTING
            cout << "a1: " << listIteratorA1 << ", variable " << (int)a1->variable << ", state " << (int)a1->state << ", timestamp " << a1->timestamp << endl;
            cout << "a2: " << listIteratorA2 << ", variable " << (int)a2->variable << ", state " << (int)a2->state << ", timestamp " << a2->timestamp << endl;
            cout << "a3: " << listIteratorA3 << ", variable " << (int)a3->variable << ", state " << (int)a3->state << ", timestamp " << a3->timestamp << endl;
            cout << "a4: " << listIteratorA4 << ", variable " << (int)a4->variable << ", state " << (int)a4->state << ", timestamp " << a4->timestamp << endl;
        #endif

        if(a1->variable == 0 && (a1->state == 3 || a1->state == 4))
        {
            if(a2->variable == 0 && (a2->state == 0 || a2->state == 1 || a2->state == 2))
            {
                if(a3->variable == 0 && a3->state == 1)
                {
                    if(a4->variable == 0 && (a4->state == 0 || a4->state == 2))
                    {
                        if(a1->timestamp < a3->timestamp)
                        {
                            if(a2->timestamp > a3->timestamp)
                            {
                                if(probChange >= -125)
                                {
                                    probChange -= 3;
                                }
                                else
                                {
                                    probChange = -128;
                                }

                                listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                if(listIteratorA3 != NULL)
                                {
                                    listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                }
                            }
                            else
                            {
                                listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                if(listIteratorA1 != NULL)
                                {
                                    listIteratorA2 = d1->activity.getNext(listIteratorA1); 
                                }
                            }
                        }
                        else if(a1->timestamp > a3->timestamp)
                        {
                            if(a4->timestamp > a1->timestamp)
                            {
                                if(probChange >= -125)
                                {
                                    probChange -= 3;
                                }
                                else
                                {
                                    probChange = -128;
                                }

                                listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                if(listIteratorA1 != NULL)
                                {
                                    listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                }
                            }
                            else
                            {
                                listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                if(listIteratorA3 != NULL)
                                {
                                    listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                }
                            }
                        }
                        else
                        {
                            if(a2->timestamp > a4->timestamp)
                            {
                                listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                if(listIteratorA3 != NULL)
                                {
                                    listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                }
                            }
                            else if(a2->timestamp < a4->timestamp)
                            {
                                listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                if(listIteratorA1 != NULL)
                                {
                                    listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                }
                            }
                            else
                            {
                                listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                if(listIteratorA1 != NULL)
                                {
                                    listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                }
                                listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                if(listIteratorA3 != NULL)
                                {
                                    listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                }
                            }
                        }
                    }
                    else
                    {
                        listIteratorA4 = d2->activity.getNext(listIteratorA4);
                    }
                }
                else
                {
                    listIteratorA3 = d2->activity.getNext(listIteratorA3);
                    if(listIteratorA3 != NULL)
                    {
                        listIteratorA4 = d2->activity.getNext(listIteratorA3);
                    }
                }
            }
            else
            {
                listIteratorA2 = d1->activity.getNext(listIteratorA2);
            }
        }
        else
        {
            listIteratorA1 = d1->activity.getNext(listIteratorA1);
            if(listIteratorA1 != NULL)
            {
                listIteratorA2 = d1->activity.getNext(listIteratorA1);
            }
        }
        
        #ifdef TESTING
            cout << "Compatability: " << probChange << endl;
        #endif
    }

    return probChange;
}

int8_t netOpt::speaker2speaker(roomMember *m1, roomMember *m2)
{
    devGroup *g1 = (devGroup *)m1->member;
    devGroup *g2 = (devGroup *)m2->member;

    #ifdef TESTIN
        uint8_t mac[6];
        unpackMAC(((devRecord *)g1->mems.getHead()->data)->macAddr, mac);
        cout << "Compatability test between speaker group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(((devRecord *)g2->mems.getHead()->data)->macAddr, mac);
        cout << dec << " and speaker group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif

    if(((devRecord *)g1->mems.getHead()->data)->activity.getLen() < 2 || ((devRecord *)g2->mems.getHead()->data)->activity.getLen() < 2)
    {
        #ifdef TESTIN
            cout << "activity records are too short" << endl;
        #endif

        return -1;
    }

    node_t *listIteratorA1 = ((devRecord *)g1->mems.getHead()->data)->activity.getHead();
    node_t *listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA1);
    node_t *listIteratorA3 = ((devRecord *)g2->mems.getHead()->data)->activity.getHead();
    node_t *listIteratorA4 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
    activityRecord *a1;
    activityRecord *a2;
    activityRecord *a3;
    activityRecord *a4;

    int probChange = 0;
    int timeDiff = 0;

    while(listIteratorA1 != NULL && listIteratorA2 != NULL && listIteratorA3 != NULL && listIteratorA4 != NULL)
    {
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;
        a3 = (activityRecord *)listIteratorA3->data;
        a4 = (activityRecord *)listIteratorA4->data;
        timeDiff = a1->timestamp - a3->timestamp;

        #ifdef TESTIN
            cout << "a1: " << listIteratorA1 << ", variable " << (int)a1->variable << ", state " << (int)a1->state << ", timestamp " << a1->timestamp << endl;
            cout << "a2: " << listIteratorA2 << ", variable " << (int)a2->variable << ", state " << (int)a2->state << ", timestamp " << a2->timestamp << endl;
            cout << "a3: " << listIteratorA3 << ", variable " << (int)a3->variable << ", state " << (int)a3->state << ", timestamp " << a3->timestamp << endl;
            cout << "a4: " << listIteratorA4 << ", variable " << (int)a4->variable << ", state " << (int)a4->state << ", timestamp " << a4->timestamp << endl;
        #endif

        if(a1->variable == a3->variable && a1->state == a3->state && (timeDiff < 5 && timeDiff > -5))
        {
            if(probChange <= 117)
            {
                probChange = probChange + 10;
            }
            else
            {
                probChange = 127;
            }
            
            listIteratorA1 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA1);
            if(listIteratorA1 != NULL)
            {
                listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA1);
            }
            listIteratorA3 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
            if(listIteratorA3 != NULL)
            {
               listIteratorA4 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3); 
            }
        }
        else
        {
            if(a1->variable == 0 && a1->state != 0 && a1->state != 1)
            {
                if(a2->variable == 0 && a2->state != a1->state)
                {
                    if(a3->variable == 0 && a3->state != 0 && a3->state != 1)
                    {
                        if(a4->variable == 0 && a4->state != a3->state)
                        {
                            if(a1->timestamp < a3->timestamp)
                            {
                                if(a2->timestamp > a3->timestamp)
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }

                                    listIteratorA3 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                    }
                                }
                                else
                                {
                                    listIteratorA1 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                       listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA1); 
                                    }
                                }
                            }
                            else if(a1->timestamp > a3->timestamp)
                            {
                                if(a4->timestamp > a1->timestamp)
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }

                                    listIteratorA1 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                        listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA1);
                                    }
                                }
                                else
                                {
                                    listIteratorA3 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                    }
                                }
                            }
                            else
                            {
                                if(a2->timestamp > a4->timestamp)
                                {
                                    listIteratorA3 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                    }
                                }
                                else if(a2->timestamp < a4->timestamp)
                                {
                                    listIteratorA1 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                        listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA1);
                                    }
                                }
                                else
                                {
                                    listIteratorA1 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                        listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA1);
                                    }
                                    listIteratorA3 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                    }
                                }
                            }
                        }
                        else
                        {
                            listIteratorA4 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA4);
                        }
                    }
                    else
                    {
                        listIteratorA3 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                        if(listIteratorA3 != NULL)
                        {
                            listIteratorA4 = ((devRecord *)g2->mems.getHead()->data)->activity.getNext(listIteratorA3);
                        }
                    }
                }
                else
                {
                    listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA2);
                }
            }
            else
            {
                listIteratorA1 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA1);
                if(listIteratorA1 != NULL)
                {
                    listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA1);
                }
            }
        }

        #ifdef TESTIN
            cout << "Compatability: " << probChange << endl;
        #endif
    }

    return probChange;
}

int8_t netOpt::speaker2audioDev(roomMember *speaker, roomMember *audioDev)
{
    devRecord *d1 = (devRecord *)audioDev->member;
    devGroup *g1 = (devGroup *)speaker->member;

    #ifdef TESTING
        uint8_t mac[6];
        unpackMAC(d1->macAddr, mac);
        cout << "Compatability test between tv " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(((devRecord *)g1->mems.getHead()->data)->macAddr, mac);
        cout << dec << " and speaker group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif

    if(d1->activity.getLen() < 2 || ((devRecord *)g1->mems.getHead()->data)->activity.getLen() < 2)
    {
        #ifdef TESTING
            cout << "activity records are too short" << endl;
        #endif

        return -1;
    }

    node_t *listIteratorA1 = d1->activity.getHead();
    node_t *listIteratorA2 = d1->activity.getNext(listIteratorA1);
    node_t *listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getHead();
    node_t *listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
    activityRecord *a1;
    activityRecord *a2;
    activityRecord *a3;
    activityRecord *a4;

    int probChange = 0;
    int timeDiff = 0;

    while(listIteratorA1 != NULL && listIteratorA2 != NULL && listIteratorA3 != NULL && listIteratorA4 != NULL)
    {
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;
        a3 = (activityRecord *)listIteratorA3->data;
        a4 = (activityRecord *)listIteratorA4->data;
        timeDiff = a1->timestamp - a3->timestamp;

        #ifdef TESTIN
            cout << "a1: " << listIteratorA1 << ", variable " << (int)a1->variable << ", state " << (int)a1->state << ", timestamp " << a1->timestamp << endl;
            cout << "a2: " << listIteratorA2 << ", variable " << (int)a2->variable << ", state " << (int)a2->state << ", timestamp " << a2->timestamp << endl;
            cout << "a3: " << listIteratorA3 << ", variable " << (int)a3->variable << ", state " << (int)a3->state << ", timestamp " << a3->timestamp << endl;
            cout << "a4: " << listIteratorA4 << ", variable " << (int)a4->variable << ", state " << (int)a4->state << ", timestamp " << a4->timestamp << endl;
        #endif

        if(a1->variable == 0 && a1->state == 1)
        {
            if(a2->variable == 0 && (a2->state == 0 || a2->state == 2))
            {
                if(a3->variable == 0 && a3->state != 0 && a3->state != 1)
                {
                    if(a4->variable == 0 && a4->state != a3->state)
                    {
                        if(a1->timestamp <= a3->timestamp)
                        {
                            if(a2->timestamp > a3->timestamp)
                            {
                                if(a3->state == 3 || a3->state == 5)
                                {
                                    if(probChange <= 117)
                                    {
                                        probChange = probChange + 10;
                                    }
                                    else
                                    {
                                        probChange = 127;
                                    }
                                }
                                else
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }
                                }

                                listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                if(listIteratorA3 != NULL)
                                {
                                    listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                }
                            }
                            else
                            {
                                if(probChange >= -127)
                                {
                                    probChange -= 1;
                                }
                                else
                                {
                                    probChange = -128;
                                }
                                
                                listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                if(listIteratorA1 != NULL)
                                {
                                    listIteratorA2 = d1->activity.getNext(listIteratorA1); 
                                }
                            }
                        }
                        else //a1->timestamp > a3->timestamp
                        {
                            if(a4->timestamp > a1->timestamp)
                            {
                                if(a3->state == 3)
                                {
                                    if(probChange <= 117)
                                    {
                                        probChange = probChange + 10;
                                    }
                                    else
                                    {
                                        probChange = 127;
                                    }
                                }
                                else
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }
                                }

                                listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                if(listIteratorA1 != NULL)
                                {
                                    listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                }
                            }
                            else
                            {
                                if(a3->state = 3)
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 5;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }
                                }
                                
                                listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                if(listIteratorA3 != NULL)
                                {
                                    listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                                }
                            }
                        }
                    }
                    else
                    {
                        listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA4);
                    }
                }
                else
                {
                    listIteratorA3 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                    if(listIteratorA3 != NULL)
                    {
                        listIteratorA4 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA3);
                    }
                }
            }
            else
            {
                listIteratorA2 = ((devRecord *)g1->mems.getHead()->data)->activity.getNext(listIteratorA2);
            }
        }
        else
        {
            listIteratorA1 = d1->activity.getNext(listIteratorA1);
            if(listIteratorA1 != NULL)
            {
                listIteratorA2 = d1->activity.getNext(listIteratorA1);
            }
        }

        #ifdef TESTING
            cout << "Compatability: " << probChange << endl;
        #endif
    }

    return probChange;
}

int8_t netOpt::recordPlayer2recordPlayer(roomMember *m1, roomMember *m2)
{
    devRecord *d1 = (devRecord *)m1->member;
    devRecord *d2 = (devRecord *)m2->member;

    #ifdef TESTIN
        uint8_t mac[6];
        unpackMAC(d1->macAddr, mac);
        cout << "Compatability test between tv " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(d2->macAddr, mac);
        cout << dec << " and tv " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif

    if(d1->activity.getLen() < 2 || d2->activity.getLen() < 2)
    {
        #ifdef TESTIN
            cout << "activity records are too short" << endl;
        #endif

        return -1;
    }

    node_t *listIteratorA1 = d1->activity.getHead();
    node_t *listIteratorA2 = d1->activity.getNext(listIteratorA1);
    node_t *listIteratorA3 = d2->activity.getHead();
    node_t *listIteratorA4 = d2->activity.getNext(listIteratorA3);
    activityRecord *a1;
    activityRecord *a2;
    activityRecord *a3;
    activityRecord *a4;

    int probChange = 0;
    int timeDiff = 0;

    while(listIteratorA1 != NULL && listIteratorA2 != NULL && listIteratorA3 != NULL && listIteratorA4 != NULL)
    {
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;
        a3 = (activityRecord *)listIteratorA3->data;
        a4 = (activityRecord *)listIteratorA4->data;
        timeDiff = a1->timestamp - a3->timestamp;

        #ifdef TESTIN
            cout << "a1: " << listIteratorA1 << ", variable " << (int)a1->variable << ", state " << (int)a1->state << ", timestamp " << a1->timestamp << endl;
            cout << "a2: " << listIteratorA2 << ", variable " << (int)a2->variable << ", state " << (int)a2->state << ", timestamp " << a2->timestamp << endl;
            cout << "a3: " << listIteratorA3 << ", variable " << (int)a3->variable << ", state " << (int)a3->state << ", timestamp " << a3->timestamp << endl;
            cout << "a4: " << listIteratorA4 << ", variable " << (int)a4->variable << ", state " << (int)a4->state << ", timestamp " << a4->timestamp << endl;
        #endif

        if(a1->variable == a3->variable && a1->state == a3->state && (timeDiff < 5 && timeDiff > -5))
        {
            if(probChange <= 117)
            {
                probChange = probChange + 10;
            }
            else
            {
                probChange = 127;
            }
            
            listIteratorA1 = d1->activity.getNext(listIteratorA1);
            if(listIteratorA1 != NULL)
            {
                listIteratorA2 = d1->activity.getNext(listIteratorA1);
            }
            listIteratorA3 = d2->activity.getNext(listIteratorA3);
            if(listIteratorA3 != NULL)
            {
               listIteratorA4 = d2->activity.getNext(listIteratorA3); 
            }
        }
        else
        {
            if(a1->variable == 0 && a1->state == 1)
            {
                if(a2->variable == 0 && (a2->state == 0 || a2->state == 2))
                {
                    if(a3->variable == 0 && a3->state == 1)
                    {
                        if(a4->variable == 0 && (a4->state == 0 || a4->state == 2))
                        {
                            if(a1->timestamp < a3->timestamp)
                            {
                                if(a2->timestamp > a3->timestamp)
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }

                                    listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                    }
                                }
                                else
                                {
                                    listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                       listIteratorA2 = d1->activity.getNext(listIteratorA1); 
                                    }
                                }
                            }
                            else if(a1->timestamp > a3->timestamp)
                            {
                                if(a4->timestamp > a1->timestamp)
                                {
                                    if(probChange >= -125)
                                    {
                                        probChange -= 3;
                                    }
                                    else
                                    {
                                        probChange = -128;
                                    }

                                    listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                    }
                                }
                                else
                                {
                                    listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                    }
                                }
                            }
                            else
                            {
                                if(a2->timestamp > a4->timestamp)
                                {
                                    listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                    }
                                }
                                else if(a2->timestamp < a4->timestamp)
                                {
                                    listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                    }
                                }
                                else
                                {
                                    listIteratorA1 = d1->activity.getNext(listIteratorA2);
                                    if(listIteratorA1 != NULL)
                                    {
                                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                                    }
                                    listIteratorA3 = d2->activity.getNext(listIteratorA3);
                                    if(listIteratorA3 != NULL)
                                    {
                                        listIteratorA4 = d2->activity.getNext(listIteratorA3);
                                    }
                                }
                            }
                        }
                        else
                        {
                            listIteratorA4 = d2->activity.getNext(listIteratorA4);
                        }
                    }
                    else
                    {
                        listIteratorA3 = d2->activity.getNext(listIteratorA3);
                        if(listIteratorA3 != NULL)
                        {
                            listIteratorA4 = d2->activity.getNext(listIteratorA3);
                        }
                    }
                }
                else
                {
                    listIteratorA2 = d1->activity.getNext(listIteratorA2);
                }
            }
            else
            {
                listIteratorA1 = d1->activity.getNext(listIteratorA1);
                if(listIteratorA1 != NULL)
                {
                    listIteratorA2 = d1->activity.getNext(listIteratorA1);
                }
            }
        }

        #ifdef TESTING
            cout << "Compatability: " << probChange << endl;
        #endif
    }

    return probChange;
}

int netOpt::activeRoomUpdate() 
{
    #ifdef TESTING
        cout << "Updating room activity probabilities" << endl;
    #endif

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
        uint8_t v1 = ((activityRecord *)lastDevUpdated->activity.getTail()->data)->variable;
        uint8_t s1 = ((activityRecord *)lastDevUpdated->activity.getTail()->data)->state;

        //Check if device is part of a group
        if(lastDevUpdated->groups.getLen() > 0)
        {
            uint8_t v2;
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
                devInRoom = false;

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
                        {
                            int numLights = 0;
                            listIteratorG1 = r1->groups.getHead();

                            while(listIteratorG1)
                            {
                                g1 = (devGroup *)((roomMember *)listIteratorG1->data)->member;

                                if(g1->devtype == 0)
                                {
                                    numLights++;
                                }

                                listIteratorG1 = r1->groups.getNext(listIteratorG1);
                            }

                            #ifdef TESTING
                                cout << "Original room probability is " << fixed << setprecision(2) << r1->activeProb << endl;
                            #endif

                            if(v1 == 0 && s1 == 1)
                            {
                                if(r1->activeProb <= 100.0 - (1.0 / numLights))
                                {
                                    r1->activeProb += 1.0 / numLights;
                                }
                                else
                                {
                                    r1->activeProb = 100.0;
                                }

                                #ifdef TESTING
                                    cout << ", new room probability is " << r1->activeProb << endl;
                                #endif
                            }
                            else if (v1 == 0 && s1 == 0)
                            {
                                if(r1->activeProb >= 1.0 / numLights)
                                {
                                    r1->activeProb -= 1.0 / numLights;

                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }

                            break;
                        }

                        case 1:
                        {
                            if(v1 == 0)
                            {
                                if(s1 == 3 || s1 == 4)
                                {
                                    if(r1->activeProb <= 50.0)
                                    {
                                        r1->activeProb += 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 100.0;
                                    }
                                }
                                else if(s1 == 0 || s1 == 2)
                                {
                                    if(r1->activeProb >= 60.0)
                                    {
                                        r1->activeProb -= 60.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                                else
                                {
                                    if(r1->activeProb <= 95.0)
                                    {
                                        r1->activeProb += 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 100.0;
                                    }
                                }
                            }
                            
                            break;
                        }
                    }
                }
                else
                {
                    switch(lastDevUpdated->devType)
                    {
                        case 0:
                        {
                            if(v1 == 0 && s1 == 1)
                            {
                                if(r1->activeProb >= 1.0)
                                {
                                    r1->activeProb -= 1.0;

                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }
                            break;
                        }

                        case 1:
                        {
                            if(v1 == 0 && (s1 == 3 || s1 == 4))
                            {              
                                if(r1->activeProb >= 25.0)
                                {
                                    r1->activeProb -= 25.0;
                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }

                            break;
                        }
                    }
                }

                listIteratorR1 = rooms.getNext(listIteratorR1);
            }

            #ifdef TESTING
                printActivity();
            #endif
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
                        
                        string message = lightOpt.inactivity(d1);                       

                        #ifdef TESTING
                            cout << "Message: " << message << endl;
                        #endif

                        interface->sendtoHost((void *)message.c_str(), REPLY_LENGTH);
                        interface->readFromHost();
                        interface->endBurst();
                    }
                }
                break;

            case 1: // tv
                a1 = (activityRecord *)d1->activity.getTail()->data;
                if(a1->variable == 0 && (a1->state == 1 && a1->state == 3 && a1->state == 4))
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

                        uint8_t macAddr[6];

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

            case 2: // tv
                a1 = (activityRecord *)d1->activity.getTail()->data;
                if(a1->variable == 0 && a1->state != 0)
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

                        uint8_t macAddr[6];

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
            
            case 3: //Record
                a1 = (activityRecord *)d1->activity.getTail()->data;
                if(a1->variable == 0 && a1->state != 0)
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

                        uint8_t macAddr[6];

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

    int netOpt::printActivity()
    {
        node_t *listIteratorR1 = rooms.getHead();
        devRoom *r1;
        devRecord *d1;
        int counterR1 = 0;
        uint8_t mac[6];

        while(listIteratorR1)
        {
            r1 = (devRoom *)listIteratorR1->data;

            cout << "Room " << counterR1 << " ";
            if(r1->groups.getLen() > 0)
            {
                d1 = (devRecord *)((devGroup *)((roomMember *)r1->groups.getHead()->data)->member)->mems.getHead()->data;
            }
            else
            {
                d1 = (devRecord *)((roomMember *)r1->mems.getHead()->data)->member;
            }
            unpackMAC(d1->macAddr, mac);
            cout << "with lead device " << hex << (int)mac[0];
            for(int i = 1; i < 6; i++)
            {
                cout << "." << (int)mac[i];
            }
            
            cout << dec << ", has activety probability " << fixed << setprecision(2) << r1->activeProb << endl;

            listIteratorR1 = rooms.getNext(listIteratorR1);
            counterR1++;
        }

        return 0;
    }
#endif