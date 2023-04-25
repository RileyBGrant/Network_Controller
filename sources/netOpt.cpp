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

    ofstream ofile;

    if(!ofile.is_open())
    {
        #ifdef TESTING
            cout << "opening file: " << "logs/activeRoom.csv" << endl;
        #endif

        ofile.open("logs/activeRoom.csv", ios::out);
        
        ofile.close();
    }
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
                break;
            }

            case 9:
            {
                plugOpt.addDevice(dev);
                break;
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
    plugOpt.pairPlugs(devices);

    groupRooms();

    //characteriseUsage();

    #ifdef TESTING
        printRooms();
        plugOpt.printPlugs();
    #endif

    #ifdef LOG
        //saveUsage();
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
                                probChange += light2speaker(m1,m2);
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
                                probChange += light2speaker(m2,m1);
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

                switch(((devGroup *)m1->member)->devtype)
                {
                    case 0: //light
                    {
                        switch (((devRecord *)m2->member)->devType)
                        {
                            case 1: //tv
                            {
                                probChange += light2tv(m1,m2);                                
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += light2recordPlayer(m1,m2);                                
                                break;
                            }
                            case 4: //Oven
                            {
                                probChange += light2oven(m1,m2);                                
                                break;
                            }
                            case 5: //Fridge
                            {
                                probChange += light2fridge(m1,m2);                                
                                break;
                            }
                            case 6: //Assistant
                            {
                                probChange += light2assistant(m1,m2);                                
                                break;
                            }
                            case 7: //Kettle
                            {
                                probChange += light2kettle(m1,m2);                                
                                break;
                            }
                            case 8: //Washing machine
                            {
                                probChange += light2washing(m1,m2);                                
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
                                probChange += speaker2recordPlayer(m1,m2);                                
                                break;
                            }
                            case 4: //Oven
                            {
                                probChange += speaker2oven(m1,m2);                                
                                break;
                            }
                            case 5: //Fridge
                            {
                                probChange += speaker2fridge(m1,m2);                                
                                break;
                            }
                            case 6: //Assistant
                            {
                                probChange += speaker2assistant(m1,m2);                                
                                break;
                            }
                            case 7: //Kettle
                            {
                                probChange += speaker2kettle(m1,m2);                                
                                break;
                            }
                            case 8: //Washing machine
                            {
                                probChange += speaker2washing(m1,m2);                                
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
                    m1->memberProb = probChange + m1->memberProb;
                }

                if(probChange + m1->memberProb < 0)
                {
                    m2->memberProb = 0;
                }
                else if (probChange + m1->memberProb > 255)
                {
                    m2->memberProb = 255;
                }
                else
                {
                    m2->memberProb = probChange + m1->memberProb;
                }

                listIteratorM2 = r1->mems.getNext(listIteratorM2);
            }

            if(r1->groups.getLen() + r1->mems.getLen() > 1 && m1->memberProb < 100)
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
                                probChange += tv2recordPlayer(m1,m2);
                                break;
                            }
                            case 4: //Oven
                            {
                                probChange += tv2oven(m1,m2);                                
                                break;
                            }
                            case 5: //Fridge
                            {
                                probChange += tv2fridge(m1,m2);                                
                                break;
                            }
                            case 6: //Assistant
                            {
                                probChange += tv2assistant(m1,m2);                                
                                break;
                            }
                            case 7: //Kettle
                            {
                                probChange += tv2kettle(m1,m2);                                
                                break;
                            }
                            case 8: //Washing machine
                            {
                                probChange += tv2washing(m1,m2);                                
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
                                probChange += tv2recordPlayer(m2,m1);
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += recordPlayer2recordPlayer(m1,m2);
                                break;
                            }
                            case 4: //Oven
                            {
                                probChange += recordPlayer2oven(m1,m2);                                
                                break;
                            }
                            case 5: //Fridge
                            {
                                probChange += recordPlayer2fridge(m1,m2);                                
                                break;
                            }
                            case 6: //Assistant
                            {
                                probChange += recordPlayer2assistant(m1,m2);                                
                                break;
                            }
                            case 7: //Kettle
                            {
                                probChange += recordPlayer2kettle(m1,m2);                                
                                break;
                            }
                            case 8: //Washing machine
                            {
                                probChange += recordPlayer2washing(m1,m2);                                
                                break;
                            }
                        }
                        break;
                    }
                    case 4: //Oven
                    {
                        switch (((devRecord *)m2->member)->devType)
                        {
                            case 1: //tv
                            {
                                probChange += tv2oven(m2,m1);
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += recordPlayer2oven(m2,m1);
                                break;
                            }
                            case 4: //Oven
                            {
                                probChange += oven2oven(m1,m2);                                
                                break;
                            }
                            case 5: //Fridge
                            {
                                probChange += oven2fridge(m1,m2);                                
                                break;
                            }
                            case 6: //Assistant
                            {
                                probChange += oven2assistant(m1,m2);                                
                                break;
                            }
                            case 7: //Kettle
                            {
                                probChange += oven2kettle(m1,m2);                                
                                break;
                            }
                            case 8: //Washing machine
                            {
                                probChange += oven2washing(m1,m2);                                
                                break;
                            }
                        }
                        break;
                    }
                    case 5: //Fridge
                    {
                        switch (((devRecord *)m2->member)->devType)
                        {
                            case 1: //tv
                            {
                                probChange += tv2fridge(m2,m1);
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += recordPlayer2fridge(m2,m1);
                                break;
                            }
                            case 4: //Oven
                            {
                                probChange += oven2fridge(m2,m1);                                
                                break;
                            }
                            case 5: //Fridge
                            {
                                probChange += fridge2fridge(m1,m2);                                
                                break;
                            }
                            case 6: //Assistant
                            {
                                probChange += fridge2assistant(m1,m2);                                
                                break;
                            }
                            case 7: //Kettle
                            {
                                probChange += fridge2kettle(m1,m2);                                
                                break;
                            }
                            case 8: //Washing machine
                            {
                                probChange += fridge2washing(m1,m2);                                
                                break;
                            }
                        }
                        break;
                    }
                    case 6: //Assistant
                    {
                        switch (((devRecord *)m2->member)->devType)
                        {
                            case 1: //tv
                            {
                                probChange += tv2assistant(m2,m1);
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += recordPlayer2assistant(m2,m1);
                                break;
                            }
                            case 4: //Oven
                            {
                                probChange += oven2assistant(m2,m1);                                
                                break;
                            }
                            case 5: //Fridge
                            {
                                probChange += fridge2assistant(m2,m1);                                
                                break;
                            }
                            case 6: //Assistant
                            {
                                probChange += assistant2assistant(m1,m2);                                
                                break;
                            }
                            case 7: //Kettle
                            {
                                probChange += assistant2kettle(m1,m2);                                
                                break;
                            }
                            case 8: //Washing machine
                            {
                                probChange += assistant2washing(m1,m2);                                
                                break;
                            }
                        }
                        break;
                    }
                    case 7: //Kettle
                    {
                        switch (((devRecord *)m2->member)->devType)
                        {
                            case 1: //tv
                            {
                                probChange += tv2kettle(m2,m1);
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += recordPlayer2kettle(m2,m1);
                                break;
                            }
                            case 4: //Oven
                            {
                                probChange += oven2kettle(m2,m1);                                
                                break;
                            }
                            case 5: //Fridge
                            {
                                probChange += fridge2kettle(m2,m1);                                
                                break;
                            }
                            case 6: //Assistant
                            {
                                probChange += assistant2kettle(m2,m1);                                
                                break;
                            }
                            case 7: //Kettle
                            {
                                probChange += kettle2kettle(m1,m2);                                
                                break;
                            }
                            case 8: //Washing machine
                            {
                                probChange += kettle2washing(m1,m2);                                
                                break;
                            }
                        }
                        break;
                    }
                    case 8: //Washing machine
                    {
                        switch (((devRecord *)m2->member)->devType)
                        {
                            case 1: //tv
                            {
                                probChange += tv2washing(m2,m1);
                                break;
                            }
                            case 3: //Record player
                            {
                                probChange += recordPlayer2washing(m2,m1);
                                break;
                            }
                            case 4: //Oven
                            {
                                probChange += oven2washing(m2,m1);                                
                                break;
                            }
                            case 5: //Fridge
                            {
                                probChange += fridge2washing(m2,m1);                                
                                break;
                            }
                            case 6: //Assistant
                            {
                                probChange += assistant2washing(m2,m1);                                
                                break;
                            }
                            case 7: //Kettle
                            {
                                probChange += kettle2washing(m2,m1);                                
                                break;
                            }
                            case 8: //Washing machine
                            {
                                probChange += washing2washing(m1,m2);                                
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
                    m1->memberProb = probChange + m1->memberProb;
                }

                if(probChange + m1->memberProb < 0)
                {
                    m2->memberProb = 0;
                }
                else if (probChange + m1->memberProb > 255)
                {
                    m2->memberProb = 255;
                }
                else
                {
                    m2->memberProb = probChange + m1->memberProb;
                }

                listIteratorM2 = r1->mems.getNext(listIteratorM2);
            }

            if(r1->groups.getLen() + r1->mems.getLen() > 1 && m1->memberProb < 100)
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
    devRoom *r3;
    int r3Compatability= 0;
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
                r3 = NULL;
                r3Compatability = 0;
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
                                        compatability += light2speaker(m1,m2);   
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
                                        compatability += light2speaker(m2,m1);   
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
                                        compatability += light2tv(m1,m2);   
                                        break;
                                    }
                                    case 3: //Record player
                                    {
                                        compatability += light2recordPlayer(m1,m2);   
                                        break;
                                    }
                                    case 4: //Oven
                                    {
                                        compatability += light2oven(m1,m2);   
                                        break;
                                    }
                                    case 5: //Fridge
                                    {
                                        compatability += light2fridge(m1,m2);   
                                        break;
                                    }
                                    case 6: //Assistant
                                    {
                                        compatability += light2assistant(m1,m2);   
                                        break;
                                    }
                                    case 7: //Kettle
                                    {
                                        compatability += light2kettle(m1,m2);   
                                        break;
                                    }
                                    case 8: //Washing machine
                                    {
                                        compatability += light2washing(m1,m2);   
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
                                        compatability += speaker2recordPlayer(m1,m2);   
                                        break;
                                    }
                                    case 4: //Oven
                                    {
                                        compatability += speaker2oven(m1,m2);   
                                        break;
                                    }
                                    case 5: //Fridge
                                    {
                                        compatability += speaker2fridge(m1,m2);   
                                        break;
                                    }
                                    case 6: //Assistant
                                    {
                                        compatability += speaker2assistant(m1,m2);   
                                        break;
                                    }
                                    case 7: //Kettle
                                    {
                                        compatability += speaker2kettle(m1,m2);   
                                        break;
                                    }
                                    case 8: //Washing machine
                                    {
                                        compatability += speaker2washing(m1,m2);   
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

                    if(compatability > r3Compatability)
                    {
                        #ifdef TESTING
                            cout << "Group compatible with a room" << endl;
                        #endif

                        r3 = r1;
                        r3Compatability = compatability;
                        roomFound = true;
                    }

                    listIteratorR1 = rooms.getNext(listIteratorR1);
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
                else
                {
                    #ifdef TESTING
                        cout << "Adding group to room" << endl;
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
                    r3->groups.append(m1);
                    listIteratorR1 = NULL;
                    listIteratorD1 = ((devGroup *)m1->member)->mems.getHead();
                
                    while(listIteratorD1)
                    {
                        d1 = (devRecord *)listIteratorD1->data;
                        
                        d1->rooms.append(r3);

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
        r3 = NULL;
        r3Compatability = 0;
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
                                    compatability += light2tv(m2,m1);   
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
                                    compatability += light2recordPlayer(m2,m1);   
                                    break;
                                }
                                case 2: //speaker
                                {
                                    compatability += speaker2recordPlayer(m2,m1);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 4: //Oven
                        {
                            switch (((devGroup *)m2->member)->devtype)
                            {
                                case 0: //light
                                {
                                    compatability += light2oven(m2,m1);   
                                    break;
                                }
                                case 2: //speaker
                                {
                                    compatability += speaker2oven(m2,m1);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 5: //Fridge
                        {
                            switch (((devGroup *)m2->member)->devtype)
                            {
                                case 0: //light
                                {
                                    compatability += light2fridge(m2,m1);   
                                    break;
                                }
                                case 2: //speaker
                                {
                                    compatability += speaker2fridge(m2,m1);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 6: //Assistant
                        {
                            switch (((devGroup *)m2->member)->devtype)
                            {
                                case 0: //light
                                {
                                    compatability += light2assistant(m2,m1);   
                                    break;
                                }
                                case 2: //speaker
                                {
                                    compatability += speaker2assistant(m2,m1);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 7: //kettle
                        {
                            switch (((devGroup *)m2->member)->devtype)
                            {
                                case 0: //light
                                {
                                    compatability += light2kettle(m2,m1);   
                                    break;
                                }
                                case 2: //speaker
                                {
                                    compatability += speaker2kettle(m2,m1);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 8: //Washing machine
                        {
                            switch (((devGroup *)m2->member)->devtype)
                            {
                                case 0: //light
                                {
                                    compatability += light2washing(m2,m1);   
                                    break;
                                }
                                case 2: //speaker
                                {
                                    compatability += speaker2washing(m2,m1);   
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
                                    compatability += tv2recordPlayer(m1,m2);   
                                    break;
                                }
                                case 4: //Oven
                                {
                                    compatability += tv2oven(m1,m2);   
                                    break;
                                }
                                case 5: //Fridge
                                {
                                    compatability += tv2fridge(m1,m2);   
                                    break;
                                }
                                case 6: //Assistant
                                {
                                    compatability += tv2assistant(m1,m2);   
                                    break;
                                }
                                case 7: //kettle
                                {
                                    compatability += tv2kettle(m1,m2);   
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
                                    compatability += tv2recordPlayer(m2,m1);   
                                    break;
                                }
                                case 3: //Record player
                                {
                                    compatability += recordPlayer2recordPlayer(m1,m2);   
                                    break;
                                }
                                case 4: //Oven
                                {
                                    compatability += recordPlayer2oven(m1,m2);   
                                    break;
                                }
                                case 5: //Fridge
                                {
                                    compatability += recordPlayer2fridge(m1,m2);   
                                    break;
                                }
                                case 6: //Assistant
                                {
                                    compatability += recordPlayer2assistant(m1,m2);   
                                    break;
                                }
                                case 7: //kettle
                                {
                                    compatability += recordPlayer2kettle(m1,m2);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 4: //Oven
                        {
                            switch (((devRecord *)m2->member)->devType)
                            {
                                case 1: //tv
                                {
                                    compatability += tv2oven(m2,m1);   
                                    break;
                                }
                                case 3: //Record player
                                {
                                    compatability += recordPlayer2oven(m2,m1);   
                                    break;
                                }
                                case 4: //Oven
                                {
                                    compatability += oven2oven(m1,m2);   
                                    break;
                                }
                                case 5: //Fridge
                                {
                                    compatability += oven2fridge(m1,m2);   
                                    break;
                                }
                                case 6: //Assistant
                                {
                                    compatability += oven2assistant(m1,m2);   
                                    break;
                                }
                                case 7: //Kettle
                                {
                                    compatability += oven2kettle(m1,m2);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 5: //Fridge
                        {
                            switch (((devRecord *)m2->member)->devType)
                            {
                                case 1: //tv
                                {
                                    compatability += tv2fridge(m2,m1);   
                                    break;
                                }
                                case 3: //Record player
                                {
                                    compatability += recordPlayer2fridge(m2,m1);   
                                    break;
                                }
                                case 4: //Oven
                                {
                                    compatability += oven2fridge(m2,m1);   
                                    break;
                                }
                                case 5: //Fridge
                                {
                                    compatability += fridge2fridge(m1,m2);   
                                    break;
                                }
                                case 6: //Assistant
                                {
                                    compatability += fridge2assistant(m1,m2);   
                                    break;
                                }
                                case 7: //Kettle
                                {
                                    compatability += fridge2kettle(m1,m2);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 6: //Assistant
                        {
                            switch (((devRecord *)m2->member)->devType)
                            {
                                case 1: //tv
                                {
                                    compatability += tv2assistant(m2,m1);   
                                    break;
                                }
                                case 3: //Record player
                                {
                                    compatability += recordPlayer2assistant(m2,m1);   
                                    break;
                                }
                                case 4: //Oven
                                {
                                    compatability += oven2assistant(m2,m1);   
                                    break;
                                }
                                case 5: //Fridge
                                {
                                    compatability += fridge2assistant(m2,m1);   
                                    break;
                                }
                                case 6: //Assistant
                                {
                                    compatability += assistant2assistant(m1,m2);   
                                    break;
                                }
                                case 7: //Kettle
                                {
                                    compatability += assistant2kettle(m1,m2);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 7: //Kettle
                        {
                            switch (((devRecord *)m2->member)->devType)
                            {
                                case 1: //tv
                                {
                                    compatability += tv2kettle(m2,m1);   
                                    break;
                                }
                                case 3: //Record player
                                {
                                    compatability += recordPlayer2kettle(m2,m1);   
                                    break;
                                }
                                case 4: //Oven
                                {
                                    compatability += oven2kettle(m2,m1);   
                                    break;
                                }
                                case 5: //Fridge
                                {
                                    compatability += fridge2kettle(m2,m1);   
                                    break;
                                }
                                case 6: //Assistant
                                {
                                    compatability += assistant2kettle(m2,m1);   
                                    break;
                                }
                                case 7: //Kettle
                                {
                                    compatability += kettle2kettle(m1,m2);   
                                    break;
                                }
                            }
                            break;
                        }
                        case 8: //Washing machine
                        {
                            switch (((devRecord *)m2->member)->devType)
                            {
                                case 1: //tv
                                {
                                    probChange += tv2washing(m2,m1);
                                    break;
                                }
                                case 3: //Record player
                                {
                                    probChange += recordPlayer2washing(m2,m1);
                                    break;
                                }
                                case 4: //Oven
                                {
                                    probChange += oven2washing(m2,m1);                                
                                    break;
                                }
                                case 5: //Fridge
                                {
                                    probChange += fridge2washing(m2,m1);                                
                                    break;
                                }
                                case 6: //Assistant
                                {
                                    probChange += assistant2washing(m2,m1);                                
                                    break;
                                }
                                case 7: //Kettle
                                {
                                    probChange += kettle2washing(m2,m1);                                
                                    break;
                                }
                                case 8: //Washing machine
                                {
                                    probChange += washing2washing(m1,m2);                                
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

                if(compatability > r3Compatability)
                {
                    #ifdef TESTING
                        cout << "Solo dev compatible with a room" << endl;
                    #endif

                    r3 = r1;
                    r3Compatability = compatability;
                    roomFound = true;
                }
                
                listIteratorR1 = rooms.getNext(listIteratorR1);
            }

            if(roomFound == false)
            {
                if(((devRecord *)m1->member)->devType != 9)
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
            else
            {
                #ifdef TESTING
                    cout << "Adding device to room" << endl;
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
                r3->mems.append(m1);
                ((devRecord *)m1->member)->rooms.append(r3);
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

int netOpt::characteriseUsage()
{
    node_t *listIteratorD1 = devices->getHead();
    devRecord *d1;
    node_t *listIteratorA1;
    activityRecord *a1;
    time_t time1 = 0;
    int counterA1;
    node_t *listIteratorA2;
    activityRecord *a2;
    time_t time2 = 0;
    int counterA2;

    bool winSet = false;
    int day1 = 0;
    int day2 = 0;
    tm tempTime = *gmtime(&time1);
    double timeDiff;
    int dayCounter = 0;
    time_t timeBin1 = 0;
    time_t timeBin2 = 0;
    bool headRemoved = false;

    #ifdef TESTING
        cout << "Chracterising usage" << endl;
    #endif

    while(listIteratorD1)
    {
        d1 = (devRecord *)listIteratorD1->data;
        listIteratorA1 = d1->activity.getHead();
        counterA1 = 0;

        #ifdef TESTING
            uint8_t mac[6];
            unpackMAC(d1->macAddr, mac);
            cout << "Characterising device " << hex << (int)mac[0];
            for(int i = 1; i < 6; i++)
            {
                cout << "." << (int)mac[i];
            }
            cout << dec << endl;
        #endif

        while(listIteratorA1)
        {
            a1 = (activityRecord *)listIteratorA1->data;
            winSet = false;
            headRemoved = false;

            #ifdef TESTING
                cout << "a1: variable: " << (int)a1->variable << ", state: " << (int)a1->state << ", timestamp " << a1->timestamp << endl;
            #endif

            //set timeA1 and timeA2
            switch(d1->devType)
            {
                case 0: //lights
                case 3: //record player
                case 9: //switch
                {
                    if(a1->variable == 0 && a1->state == 1)
                    {
                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                        counterA2 = counterA1 + 1;

                        while(listIteratorA2 /* && (a2->variable != 0 || a2->state == 1)*/)
                        {
                            a2 = (activityRecord *)listIteratorA2->data;

                            #ifdef TESTING
                                cout << "a2: variable: " << (int)a2->variable << ", state: " << (int)a2->state << ", timestamp: " << a2->timestamp << endl;
                            #endif

                            if(a2->variable == 0 && a2->state != 1)
                            {
                                listIteratorA2 = NULL;
                                winSet = true;
                            }
                            else
                            {
                                listIteratorA2 = d1->activity.getNext(listIteratorA2);
                                counterA2++;
                            }
                        }

                        if(winSet == true)
                        {
                            time1 = a1->timestamp;
                            time2 = a2->timestamp;
                            d1->activity.remove(counterA2);
                            listIteratorA1 = d1->activity.getNext(listIteratorA1);
                            d1->activity.remove(counterA1);
                        }
                    }
                    else if(counterA1 = 0)
                    {
                        d1->activity.remove(counterA1);
                        headRemoved = true;
                    }
                    break;
                }
                case 1: //tv
                {
                    if(a1->variable == 0 && (a1->state == 3 || a1->state == 4))
                    {
                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                        counterA2 = counterA1 + 1;

                        while(listIteratorA2 /* && (a2->variable != 0 || a2->state == 1)*/)
                        {
                            a2 = (activityRecord *)listIteratorA2->data;

                            #ifdef TESTING
                                cout << "a2: variable: " << (int)a2->variable << ", state: " << (int)a2->state << ", timestamp: " << a2->timestamp << endl;
                            #endif

                            if(a2->variable == 0 && a2->state != a1->state)
                            {
                                listIteratorA2 = NULL;
                                winSet = true;
                            }
                            else
                            {
                                listIteratorA2 = d1->activity.getNext(listIteratorA2);
                                counterA2++;
                            }
                        }
                        

                        if(winSet == true)
                        {
                            time1 = a1->timestamp;
                            time2 = a2->timestamp;
                            d1->activity.remove(counterA2);
                            listIteratorA1 = d1->activity.getNext(listIteratorA1);
                            d1->activity.remove(counterA1);
                        }
                    }
                    else if(counterA1 = 0)
                    {
                        d1->activity.remove(counterA1);
                        headRemoved = true;
                    }
                    break;
                }
                case 2: //speaker
                {
                    if(a1->variable == 0 && a1->state >= 2)
                    {
                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                        counterA2 = counterA1 + 1;

                        while(listIteratorA2 /* && (a2->variable != 0 || a2->state == 1)*/)
                        {
                            a2 = (activityRecord *)listIteratorA2->data;

                            #ifdef TESTING
                                cout << "a2: variable: " << (int)a2->variable << ", state: " << (int)a2->state << ", timestamp: " << a2->timestamp << endl;
                            #endif

                            if(a2->variable == 0 && a2->state != a1->state)
                            {
                                listIteratorA2 = NULL;
                                winSet = true;
                            }
                            else
                            {
                                listIteratorA2 = d1->activity.getNext(listIteratorA2);
                                counterA2++;
                            }
                        }

                        if(winSet == true)
                        {
                            time1 = a1->timestamp;
                            time2 = a2->timestamp;
                            d1->activity.remove(counterA2);
                            listIteratorA1 = d1->activity.getNext(listIteratorA1);
                            d1->activity.remove(counterA1);
                        }
                    }
                    else if(counterA1 = 0)
                    {
                        d1->activity.remove(counterA1);
                        headRemoved = true;
                    }
                    break;
                }
                case 4: //oven
                {
                    if(a1->variable >= 1 && a1->state == 1)
                    {
                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                        counterA2 = counterA1 + 1;

                        while(listIteratorA2 /* && (a2->variable != 0 || a2->state == 1)*/)
                        {
                            a2 = (activityRecord *)listIteratorA2->data;

                            #ifdef TESTING
                                cout << "a2: variable: " << (int)a2->variable << ", state: " << (int)a2->state << ", timestamp: " << a2->timestamp << endl;
                            #endif

                            if(a2->variable == a1->variable && a2->state == 0)
                            {
                                listIteratorA2 = NULL;
                                winSet = true;
                            }
                            else
                            {
                                listIteratorA2 = d1->activity.getNext(listIteratorA2);
                                counterA2++;
                            }
                        }

                        if(winSet == true)
                        {
                            time1 = a1->timestamp;
                            time2 = a2->timestamp;
                            d1->activity.remove(counterA2);
                            listIteratorA1 = d1->activity.getNext(listIteratorA1);
                            d1->activity.remove(counterA1);
                        }
                    }
                    else if(counterA1 = 0)
                    {
                        d1->activity.remove(counterA1);
                        headRemoved = true;
                    }
                    break;
                }
                case 5: //fridge
                {
                    if(a1->variable == 1 && a1->state == 1)
                    {
                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                        counterA2 = counterA1 + 1;

                        while(listIteratorA2 /* && (a2->variable != 0 || a2->state == 1)*/)
                        {
                            a2 = (activityRecord *)listIteratorA2->data;

                            #ifdef TESTING
                                cout << "a2: variable: " << (int)a2->variable << ", state: " << (int)a2->state << ", timestamp: " << a2->timestamp << endl;
                            #endif

                            if(a2->variable == 1 && a2->state == 0)
                            {
                                listIteratorA2 = NULL;
                                winSet = true;
                            }
                            else
                            {
                                listIteratorA2 = d1->activity.getNext(listIteratorA2);
                                counterA2++;
                            }
                        }

                        if(winSet == true)
                        {
                            time1 = a1->timestamp;
                            time2 = a2->timestamp;
                            d1->activity.remove(counterA2);
                            listIteratorA1 = d1->activity.getNext(listIteratorA1);
                            d1->activity.remove(counterA1);
                        }
                    }
                    else if(counterA1 = 0)
                    {
                        d1->activity.remove(counterA1);
                        headRemoved = true;
                    }
                    break;
                }
                case 6: //assistant
                {
                    if(a1->variable == 0 && a1->state >= 2)
                    {
                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                        counterA2 = counterA1 + 1;

                        while(listIteratorA2 /* && (a2->variable != 0 || a2->state == 1)*/)
                        {
                            a2 = (activityRecord *)listIteratorA2->data;

                            #ifdef TESTING
                                cout << "a2: variable: " << (int)a2->variable << ", state: " << (int)a2->state << ", timestamp: " << a2->timestamp << endl;
                            #endif

                            if(a2->variable == 0 && a2->state != a1->state)
                            {
                                listIteratorA2 = NULL;
                                winSet = true;
                            }
                            else
                            {
                                listIteratorA2 = d1->activity.getNext(listIteratorA2);
                                counterA2++;
                            }
                        }

                        if(winSet == true)
                        {
                            time1 = a1->timestamp;
                            time2 = a2->timestamp;
                            d1->activity.remove(counterA2);
                            listIteratorA1 = d1->activity.getNext(listIteratorA1);
                            d1->activity.remove(counterA1);
                        }
                    }
                    else if(a1->variable == 1 && a1->state == 1)
                    {
                        winSet = true;
                        time1 = a1->timestamp;
                        time2 = a1->timestamp;
                        listIteratorA1 = d1->activity.getNext(listIteratorA1);
                        d1->activity.remove(counterA1);
                    }
                    else if(counterA1 = 0)
                    {
                        d1->activity.remove(counterA1);
                        headRemoved = true;
                    }
                    break;
                }
                case 7: //kettle
                case 8: //washing machine
                {
                    if(a1->variable == 0 && a1->state == 2)
                    {
                        listIteratorA2 = d1->activity.getNext(listIteratorA1);
                        counterA2 = counterA1 + 1;

                        while(listIteratorA2)
                        {
                            a2 = (activityRecord *)listIteratorA2->data;

                            #ifdef TESTING
                                cout << "a2: variable: " << (int)a2->variable << ", state: " << (int)a2->state << ", timestamp: " << a2->timestamp << endl;
                            #endif

                            if(a2->variable == 0 && a2->state != a1->state)
                            {
                                listIteratorA2 = NULL;
                                winSet = true;
                            }
                            else
                            {
                                listIteratorA2 = d1->activity.getNext(listIteratorA2);
                                counterA2++;
                            }
                        }

                        if(winSet == true)
                        {
                            time1 = a1->timestamp;
                            time2 = a2->timestamp;
                            d1->activity.remove(counterA2);
                            listIteratorA1 = d1->activity.getNext(listIteratorA1);
                            d1->activity.remove(counterA1);
                        }
                    }
                    else if(a1->variable == 0 && a1->state != 2 && counterA1 == 0)
                    {
                        d1->activity.remove(counterA1);
                        headRemoved = true;
                    }
                    /*else if(a1->variable == 0 && a1->state == 1)
                    {
                        winSet = true;
                        time1 = a1->timestamp;
                        time2 = a1->timestamp;
                        listIteratorA1 = d1->activity.getNext(listIteratorA1);
                        d1->activity.remove(counterA1);
                    }*/
                    break;
                }
            }
            

            if(winSet == true)
            {
                #ifdef TESTING
                    cout << "Window set " << time1 << " to " << time2 << endl;
                    //cout << gmtime(&time1)->tm_hour << ":" << gmtime(&time1)->tm_min << ":" << gmtime(&time1)->tm_sec << endl;
                    //cout << gmtime(&time2)->tm_hour << ":" << gmtime(&time2)->tm_min << ":" << gmtime(&time2)->tm_sec << endl;
                #endif

                d1->usage.numOfSample++;

                day1 = gmtime(&time1)->tm_wday;
                day2 = gmtime(&time2)->tm_wday;
                timeDiff == difftime(time1,time2);

                tempTime.tm_sec = gmtime(&time1)->tm_sec;
                tempTime.tm_min = gmtime(&time1)->tm_min;
                tempTime.tm_hour = gmtime(&time1)->tm_hour;
                time1 = mktime(&tempTime) + 3600;

                tempTime.tm_sec = gmtime(&time2)->tm_sec;
                tempTime.tm_min = gmtime(&time2)->tm_min;
                tempTime.tm_hour = gmtime(&time2)->tm_hour;
                time2 = mktime(&tempTime) + 3600;

                #ifdef TESTING
                    cout << "day1: " << day1 << ", day2: " << day2 << endl;
                    cout << "tod1: " << time1 << ", tod2: " << time2 << endl << endl;
                    //cout << gmtime(&time1)->tm_hour << ":" << gmtime(&time1)->tm_min << ":" << gmtime(&time1)->tm_sec << endl;
                    //cout << gmtime(&time2)->tm_hour << ":" << gmtime(&time2)->tm_min << ":" << gmtime(&time2)->tm_sec << endl;
                #endif

                if(day1 == day2)
                {
                    if(time1 <= time2)
                    {
                        for(int i = 0; i < 48; i++)
                        {
                            timeBin1 = i * 1800;
                            timeBin2 = (i + 1) * 1800;
                            
                            #ifdef TESTIN
                                cout << timeBin1 << " - " << timeBin2 << endl;
                            #endif

                            if(timeBin1 < time2 && timeBin2 > time1)
                            {
                                #ifdef TESTIN
                                    cout << "Incremented" << endl;
                                #endif
                                d1->usage.time[day1][i]++;

                                if(d1->usage.time[day1][i] > d1->usage.time[d1->usage.modeTime[0]][d1->usage.modeTime[1]])
                                {
                                    d1->usage.modeTime[0] = day1;
                                    d1->usage.modeTime[1] = i;
                                }
                            }
                        }
                    }
                    else
                    {
                        for(int i = 0; i < 48; i++)
                        {
                            timeBin1 = i * 1800;
                            timeBin2 = (i + 1) * 1800;

                            if(timeBin1 < time2 || timeBin2 > time1)
                            {
                                d1->usage.time[day1][i]++;

                                if(d1->usage.time[day1][i] > d1->usage.time[d1->usage.modeTime[0]][d1->usage.modeTime[1]])
                                {
                                    d1->usage.modeTime[0] = day1;
                                    d1->usage.modeTime[1] = i;
                                }
                            }
                        }
                    }
                }
                else if(day1 < day2)
                {
                    for(int i = 0; i < 7; i++)
                    {
                        if(i == day1)
                        {
                            for(int j = 0; j < 48; j++)
                            {
                                timeBin2 = (j + 1) * 1800;

                                #ifdef TESTIN
                                    cout  << "Timebin2: " << timeBin2 << endl;
                                #endif

                                if(timeBin2 > time1)
                                {
                                    d1->usage.time[i][j]++;

                                    if(d1->usage.time[i][j] > d1->usage.time[d1->usage.modeTime[0]][d1->usage.modeTime[1]])
                                    {
                                        d1->usage.modeTime[0] = i;
                                        d1->usage.modeTime[1] = j;
                                    }
                                }
                            }
                        }
                        else if(i > day1 && i < day2)
                        {
                            for(int j = 0; j < 48; j++)
                            {
                                d1->usage.time[i][j]++;

                                if(d1->usage.time[i][j] > d1->usage.time[d1->usage.modeTime[0]][d1->usage.modeTime[1]])
                                {
                                    d1->usage.modeTime[0] = i;
                                    d1->usage.modeTime[1] = j;
                                }
                                
                            }
                        }
                        else if(i == day2)
                        {
                            for(int j = 0; j < 48; j++)
                            {
                                timeBin1 = j * 1800;

                                #ifdef TESTIN
                                    cout << "Timebin1: " << timeBin1 << ", i: " << i << ", j: " << j << endl;
                                #endif

                                if(timeBin1 < time2)
                                {
                                    d1->usage.time[i][j]++;

                                    if(d1->usage.time[i][j] > d1->usage.time[d1->usage.modeTime[0]][d1->usage.modeTime[1]])
                                    {
                                        d1->usage.modeTime[0] = i;
                                        d1->usage.modeTime[1] = j;
                                    }
                                }
                            }
                        }
                    }
                }
                else if(day1 > day2)
                {
                    for(int i = 0; i < 7; i++)
                    {
                        if(i == day1)
                        {
                            for(int j = 0; j < 48; j++)
                            {
                                timeBin2 = (j + 1) * 1800;

                                if(timeBin2 > time1)
                                {
                                    d1->usage.time[i][j]++;

                                    if(d1->usage.time[i][j] > d1->usage.time[d1->usage.modeTime[0]][d1->usage.modeTime[1]])
                                    {
                                        d1->usage.modeTime[0] = i;
                                        d1->usage.modeTime[1] = j;
                                    }
                                }
                            }
                        }
                        else if(i > day1 || i < day2)
                        {
                            for(int j = 0; j < 48; j++)
                            {
                                d1->usage.time[i][j]++;

                                if(d1->usage.time[i][j] > d1->usage.time[d1->usage.modeTime[0]][d1->usage.modeTime[1]])
                                {
                                    d1->usage.modeTime[0] = i;
                                    d1->usage.modeTime[1] = j;
                                }
                                
                            }
                        }
                        else if(i == day2)
                        {
                            for(int j = 0; j < 48; j++)
                            {
                                timeBin1 = j * 1800;

                                if(timeBin1 < time2)
                                {
                                    d1->usage.time[i][j]++;

                                    if(d1->usage.time[i][j] > d1->usage.time[d1->usage.modeTime[0]][d1->usage.modeTime[1]])
                                    {
                                        d1->usage.modeTime[0] = i;
                                        d1->usage.modeTime[1] = j;
                                    }
                                }
                            }
                        }
                    }
                }

                #ifdef TESTING
                    cout << "Window added" << endl;
                #endif
            }
            else if(headRemoved == true)
            {
                listIteratorA1 = d1->activity.getHead();
                counterA1 = 0;
            }
            else
            {
                listIteratorA1 = d1->activity.getNext(listIteratorA1);
                counterA1++;
            }
        }

        listIteratorD1 = devices->getNext(listIteratorD1);
    }

    #ifdef TESTING
        cout << "Characterising room usage" << endl;
    #endif

    node_t *listIteratorR1 = rooms.getHead();
    devRoom *r1;
    node_t *listIteratorM1;
    roomMember *m1;
    int filterFloor = 0;
    int window[2];
    usageWindow *w1;
    #ifdef TESTING
        int counterR1 = 0;
        uint8_t mac[6];
    #endif

    while(listIteratorR1)
    {
        r1 = (devRoom *)listIteratorR1->data;
        #ifdef TESTING
            cout << "Chracterisig room " << counterR1 << endl;
            counterR1++;
        #endif
        
        for(int i = 0; i < 7; i++)
        {
            for(int j = 0; j < 48; j++)
            {
                r1->usage.time[i][j] = 0;
            }
        }
        
        if(r1->groups.getLen() > 0)
        {
            listIteratorM1 = r1->groups.getHead();

            while(listIteratorM1)
            {
                m1 = (roomMember *)listIteratorM1->data;
                d1 = (devRecord *)((devGroup *)m1->member)->mems.getHead()->data;

                #ifdef TESTING
                    unpackMAC(d1->macAddr, mac);
                    cout << dec << "Adding device " << hex << (int)mac[0];
                    for(int i = 1; i < 6; i++)
                    {
                        cout << "." << (int)mac[i];
                    }
                    cout << dec << " usage to room " << endl;
                #endif

                for(int i = 0; i < 7; i++)
                {
                    for(int j = 0; j < 48; j++)
                    {
                        r1->usage.time[i][j] += d1->usage.time[i][j];

                        if(r1->usage.time[i][j] > r1->usage.time[r1->usage.modeTime[0]][r1->usage.modeTime[1]])
                        {
                            r1->usage.modeTime[0] = i;
                            r1->usage.modeTime[1] = j;
                        }
                    }
                }

                listIteratorM1 = r1->groups.getNext(listIteratorM1);
            }
        }
        
        if(r1->mems.getLen() > 0)
        {
            listIteratorM1 = r1->mems.getHead();

            while(listIteratorM1)
            {
                m1 = (roomMember *)listIteratorM1->data;
                d1 = (devRecord *)m1->member;

                #ifdef TESTING
                    unpackMAC(d1->macAddr, mac);
                    cout << dec << "Adding device " << hex << (int)mac[0];
                    for(int i = 1; i < 6; i++)
                    {
                        cout << "." << (int)mac[i];
                    }
                    cout << dec << " usage to room " << endl;
                #endif

                for(int i = 0; i < 7; i++)
                {
                    for(int j = 0; j < 48; j++)
                    {
                        r1->usage.time[i][j] += d1->usage.time[i][j];

                        if(r1->usage.time[i][j] > r1->usage.time[r1->usage.modeTime[0]][r1->usage.modeTime[1]])
                        {
                            r1->usage.modeTime[0] = i;
                            r1->usage.modeTime[1] = j;
                        }
                    }
                }

                listIteratorM1 = r1->mems.getNext(listIteratorM1);
            }
        }
        

        #ifdef TESTING
            cout << "Creating rooming windows" << endl;
        #endif
        
        filterFloor = floor(r1->usage.time[r1->usage.modeTime[0]][r1->usage.modeTime[1]] / 4);

        while(r1->usage.windows.getLen() > 0)
        {
            free(r1->usage.windows.getHead()->data);
            r1->usage.windows.remove(0);
        }
        
        for(int i = 0; i < 7; i++)
        {
            winSet = false;
            for(int j = 0; j < 48; j++)
            {
                if(r1->usage.time[i][j] >= filterFloor)
                {
                    if(winSet != true)
                    {
                        winSet = true;
                        window[0] = j;
                    }
                }
                else
                {
                    if(winSet == true)
                    {
                        if(j < 47)
                        {
                            if(r1->usage.time[i][j + 1] >= filterFloor)
                            {
                                j++;
                            }
                            else
                            {
                                window[1]= j - 1;

                                w1 = new usageWindow;
                                w1->start = window[0];
                                w1->end = window[1];
                                w1->day = i;

                                #ifdef TESTING
                                    cout << "Window found on day " << i + 1;
                                    cout << " at " << (window[0] * 1800) / 3600 << ":" << ((window[0] * 1800) % 3600) / 60;
                                    cout << " - " << ((window[1] + 1) * 1800) / 3600 << ":" << ((window[1] + 1 * 1800) % 3600) / 60 << endl;
                                #endif

                                r1->usage.windows.append(w1);
                                winSet = false;
                            }
                        }
                        else
                        {
                            if(r1->usage.time[(i + 1) % 7][0] < filterFloor)
                            {
                                window[1]= j - 1;

                                w1 = new usageWindow;
                                w1->start = window[0];
                                w1->end = window[1];
                                w1->day = i;

                                #ifdef TESTING
                                    cout << "Window found on day " << i + 1;
                                    cout << " at " << (window[0] * 1800) / 3600 << ":" << ((window[0] * 1800) % 3600) / 60;
                                    cout << " - " << ((window[1] + 1) * 1800) / 3600 << ":" << ((window[1] + 1 * 1800) % 3600) / 60 << endl;
                                #endif

                                r1->usage.windows.append(w1);
                                winSet = false;
                            }
                        }
                    }
                    
                }
            }

            if(winSet == true)
            {
                window[1]= 47;

                w1 = new usageWindow;
                w1->start = window[0];
                w1->end = window[1];
                w1->day = i;

                #ifdef TESTING
                    cout << "Window found on day " << i + 1;
                    cout << " at " << (window[0] * 1800) / 3600 << ":" << ((window[0] * 1800) % 3600) / 60;
                    cout << " - " << ((window[1] + 1) * 1800) / 3600 << ":" << ((window[1] + 1 * 1800) % 3600) / 60 << endl;
                #endif

                r1->usage.windows.append(w1);
                winSet = false;
            }
        }

        listIteratorR1 = rooms.getNext(listIteratorR1);
    }

    

    return 0;
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
                    if(d1->activity.getLen() > 0)
                    {
                        v2 = ((activityRecord *)d1->activity.getTail()->data)->variable;
                        s2 = ((activityRecord *)d1->activity.getTail()->data)->state;

                        #ifdef TESTING
                            uint8_t mac[6];
                            unpackMAC(d1->macAddr, mac);
                            cout << "Group member " << hex << (int)mac[0];
                            for(int i = 1; i < 6; i++)
                            {
                                cout << "." << (int)mac[i];
                            }
                            unpackMAC(lastDevUpdated->macAddr, mac);
                            cout << dec << ", variable: " << (int)v2 << ", state: " << (int)s2 << endl;
                            cout << "lastDevupdated: " << hex << (int)mac[0];
                            for(int i = 1; i < 6; i++)
                            {
                                cout << "." << (int)mac[i];
                            }
                            cout << dec << ", variable: " << (int)v1 << ", state: " << (int)s1 << endl;
                        #endif

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
                    else
                    {
                        groupChanged = false;
                        listIteratorD1 = NULL;
                    }
                }
                
                listIteratorG1 = lastDevUpdated->groups.getNext(listIteratorG1);
            }
        }

        if(groupChanged == true)
        {
            #ifdef TESTING
                cout << "Group changed" << endl;
            #endif
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
                    #ifdef TESTING
                        cout << "In Dev room" << endl;
                    #endif
                    switch(lastDevUpdated->devType)
                    {
                        case 0: //Lights
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

                            #ifdef TESTIN
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

                                #ifdef TESTIN
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

                        case 1: //tv
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
                                else if(s1 == 0 || s1 == 1 || s1 == 2)
                                {
                                    if(r1->activeProb >= 50.0)
                                    {
                                        r1->activeProb -= 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            
                            break;
                        }
                        case 2: //speaker
                        {
                            if(v1 == 0)
                            {
                                if(s1 >= 2 && s1 <= 4)
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
                                else if(s1 == 0 || s1 == 1)
                                {
                                    if(r1->activeProb >= 50.0)
                                    {
                                        r1->activeProb -= 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            
                            break;
                        }
                        case 3: //record player
                        {
                            if(v1 == 0)
                            {
                                if(s1 == 1)
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
                                    if(r1->activeProb >= 50.0)
                                    {
                                        r1->activeProb -= 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            
                            break;
                        }
                        case 4: //ovem
                        {
                            if(v1 == 0)
                            {
                                if(s1 == 1)
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
                                else if(s1 == 0)
                                {
                                    if(r1->activeProb >= 50.0)
                                    {
                                        r1->activeProb -= 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            else if(v1 >= 1 && v1 <= 6)
                            {
                                if(s1 == 1)
                                {
                                    if(r1->activeProb <= 90.0)
                                    {
                                        r1->activeProb += 10.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 100.0;
                                    }
                                }
                                else if(s1 == 0)
                                {
                                    if(r1->activeProb >= 10.0)
                                    {
                                        r1->activeProb -= 10.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            
                            break;
                        }
                        case 5: //fridge
                        {
                            if(v1 == 0)
                            {
                                if(s1 == 1)
                                {
                                    if(r1->activeProb <= 90.0)
                                    {
                                        r1->activeProb += 10.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 100.0;
                                    }
                                }
                                else if(s1 == 0)
                                {
                                    if(r1->activeProb >= 10.0)
                                    {
                                        r1->activeProb -= 10.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            else if(v1 == 1)
                            {
                                if(s1 == 1)
                                {
                                    if(r1->activeProb <= 90.0)
                                    {
                                        r1->activeProb += 10.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 100.0;
                                    }
                                }
                                else if(s1 == 0 || s1 == 1)
                                {
                                    if(r1->activeProb >= 10.0)
                                    {
                                        r1->activeProb -= 10.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            
                            break;
                        }
                        case 6: //assistant
                        {
                            if(v1 == 0)
                            {
                                if(s1 == 2 || s1 == 3)
                                {
                                    if(r1->activeProb <= 99.0)
                                    {
                                        r1->activeProb += 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 100.0;
                                    }
                                }
                                else if(s1 == 0 || s1 == 1)
                                {
                                    if(r1->activeProb >= 50.0)
                                    {
                                        r1->activeProb -= 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            else if(v1 == 1)
                            {
                                if(s1 == 1)
                                {
                                    if(r1->activeProb <= 99.5)
                                    {
                                        r1->activeProb += 0.5;
                                    }
                                    else
                                    {
                                        r1->activeProb = 100.0;
                                    }
                                }
                            }
                            
                            break;
                        }
                        case 7: //kettle
                        {
                            if(v1 == 0)
                            {
                                if(s1 == 1)
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
                                else if(s1 == 2)
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
                                else if(s1 == 0)
                                {
                                    if(r1->activeProb >= 50.0)
                                    {
                                        r1->activeProb -= 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            
                            break;
                        }
                        case 8: //washing machine
                        {
                            if(v1 == 0)
                            {
                                if(s1 == 1)
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
                                else if(s1 == 2)
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
                                else if(s1 == 0)
                                {
                                    if(r1->activeProb >= 50.0)
                                    {
                                        r1->activeProb -= 50.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 0.0;
                                    }
                                }
                            }
                            
                            break;
                        }
                        case 9: //plug
                        {
                            if(v1 == 0)
                            {
                                if(s1 == 1)
                                {
                                    if(r1->activeProb <= 99.0)
                                    {
                                        r1->activeProb += 1.0;
                                    }
                                    else
                                    {
                                        r1->activeProb = 100.0;
                                    }
                                }
                                else if(s1 == 0)
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
                            }
                            
                            break;
                        }
                    }
                }
                else
                {
                    #ifdef TESTING
                        cout << "Not in dev room" << endl;
                    #endif
                    switch(lastDevUpdated->devType)
                    {
                        case 0: //lights
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

                        case 1: //tv
                        {
                            if(v1 == 0 && (s1 == 3 || s1 == 4))
                            {              
                                if(r1->activeProb >= 40.0)
                                {
                                    r1->activeProb -= 40.0;
                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }

                            break;
                        }
                        case 2: //speaker
                        {
                            if(v1 == 0 && s1 >= 2 && s1 <= 4)
                            {
                                if(r1->activeProb >= 40.0)
                                {
                                    r1->activeProb -= 40.0;
                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }
                            
                            break;
                        }
                        case 3: //record player
                        {
                            if(v1 == 0 && s1 == 1)
                            {
                                if(r1->activeProb >= 40.0)
                                {
                                    r1->activeProb -= 40.0;
                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }
                            
                            break;
                        }
                        case 4: //oven
                        {
                            if(v1 == 0 && s1 == 1)
                            {
                                
                                if(r1->activeProb >= 40.0)
                                {
                                    r1->activeProb -= 40.0;
                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }

                            }
                            else if(v1 >= 1 && v1 <= 6 && s1 == 1)
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
                        case 5: //fridge
                        {
                            if(v1 == 0 && s1 == 1)
                            {
                                if(r1->activeProb >= 10.0)
                                {
                                    r1->activeProb -= 10.0;
                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }
                            else if(v1 == 1 && s1 == 1)
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
                        case 6: //assistant
                        {
                            if(v1 == 0 && (s1 == 2 || s1 == 3))
                            {
                                if(r1->activeProb >= 40.0)
                                {
                                    r1->activeProb -= 40.0;
                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }
                            else if(v1 == 1 && s1 == 1)
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
                        case 7: //kettle
                        {
                            if(v1 == 0 && s1 == 1)
                            {
                                if(r1->activeProb >= 40.0)
                                {
                                    r1->activeProb -= 40.0;
                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }
                            
                            break;
                        }
                        case 8: //washing machine
                        {
                            if(v1 == 0 && s1 == 1)
                            {
                                if(r1->activeProb >= 40.0)
                                {
                                    r1->activeProb -= 40.0;
                                }
                                else
                                {
                                    r1->activeProb = 0.0;
                                }
                            }
                            
                            break;
                        }
                        case 9: //plug
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

            #ifdef LOG
                ofstream ofile;
                uint8_t mac[6];

                if(activeRoom->groups.getLen() > 0)
                {
                    d1 = (devRecord *)((devGroup *)((roomMember *)activeRoom->groups.getHead()->data)->member)->mems.getHead()->data;
                }
                else if(activeRoom->mems.getLen() > 0)
                {
                    d1 = (devRecord *)((roomMember *)activeRoom->mems.getHead()->data)->member;
                }
                else
                {
                    #ifdef TESTING
                        cout << "Room has no devices" << endl;
                    #endif

                    return 1;
                }

                unpackMAC(d1->macAddr, mac);

                time_t tempT = interface->getLastTimestamp();
                tm tempTM = *gmtime(&tempT);

                if(!ofile.is_open())
                {
                    #ifdef TESTING
                        cout << "opening file: " << "logs/activeRoom.csv" << endl;
                    #endif

                    ofile.open("logs/activeRoom.csv", ios::app);

                    ofile << tempTM.tm_year + 1900 << "," << tempTM.tm_mon + 1 << "," << tempTM.tm_mday << "," << tempTM.tm_hour << "," << tempTM.tm_min << "," << tempTM.tm_sec << "," << (int)mac[5] << endl;
                    
                    ofile.close();
                }
            #endif

            //interface->requestStim((time_t)(300 + ((activityRecord *)lastDevUpdated->activity.getTail()->data)->timestamp)); //in 5 mins
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
            
            case 3: //Record player
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

            case 4: //Oven
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

            case 5: //Fridge
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

            case 6: //Assistant
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

            case 7: //Kettle
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
            case 8: //Kettle
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

int8_t netOpt::getProbAdjustment(devRecord *d1, devRecord *d2, float adjustmentChange)
{
    node_t *listIteratorP1 = d1->probAdjusment.getHead();
    devProbAdjustment *p1;
    node_t *listIteratorP2;
    devProbAdjustment *p2;
    bool devFound = false;
    int8_t output;

    #ifdef TESTIN
        if(adjustmentChange != 0)
        {
            uint8_t mac[6];
            unpackMAC(d1->macAddr, mac);
            cout << "Compatability probability between device " << hex << (int)mac[0];
            for(int i = 1; i < 6; i++)
            {
                cout << "." << (int)mac[i];
            }
            unpackMAC(d2->macAddr, mac);
            cout << dec << " and device " << hex << (int)mac[0];
            for(int i = 1; i < 6; i++)
            {
                cout << "." << (int)mac[i];
            }
            cout << dec << " being changed by " << adjustmentChange << endl;
        }
    #endif

    while(listIteratorP1)
    {
        p1 = (devProbAdjustment *)listIteratorP1->data;

        if(p1->dev == d2)
        {    
            #ifdef TESTIN
                if(adjustmentChange != 0)
                {
                    cout << "P1 found" << endl;
                }
            #endif
            listIteratorP2 = d2->probAdjusment.getHead();

            while(listIteratorP2)
            {
                p2 = (devProbAdjustment *)listIteratorP2->data;

                if(p2->dev == d1)
                {
                    #ifdef TESTIN
                        if(adjustmentChange != 0)
                        {
                            cout << "P2 found" << endl;
                        }
                    #endif
                    output = lround(p1->adjustment);
                    p1->adjustment += adjustmentChange;
                    if(p1->adjustment > 127)
                    {
                        p1->adjustment = 127;
                    }
                    else if(p1->adjustment < -128)
                    {
                        p1->adjustment = -128;
                    }
                    p2->adjustment = p1->adjustment;
                    listIteratorP2 = NULL;

                    #ifdef TESTIN
                        if(adjustmentChange != 0)
                        {
                            cout << "Probablitiy adjustment is now: " << p1->adjustment << endl;
                        }
                    #endif
                    
                    return output;
                }
                else
                {
                    listIteratorP2 = d2->probAdjusment.getNext(listIteratorP2);
                }
            }

            #ifdef TESTIN
                if(adjustmentChange != 0)
                {
                    cout << "P2 not found" << endl;
                }
            #endif
            
            devFound = true;
            listIteratorP1 = NULL;
        }
        else
        {
            listIteratorP1 = d1->probAdjusment.getNext(listIteratorP1);
        }
    }

    #ifdef TESTIN
        if(adjustmentChange != 0)
        {
            cout << "P1 not found" << endl;
        }
    #endif

    if(devFound == false)
    {
        #ifdef TESTING
            cout << "No adjustment record found creating new one" << endl;
        #endif
        p1 = new devProbAdjustment;
        p1->dev = d2;
        p1->adjustment = adjustmentChange;
        d1->probAdjusment.append(p1);

        p2 = new devProbAdjustment;
        p2->dev = d1;
        p2->adjustment = adjustmentChange;
        d2->probAdjusment.append(p2);
    }

    return 0;
}

#ifdef TESTING
    int netOpt::printRooms()
    {
        cout << endl << "Printing " << rooms.getLen() << " rooms" << endl;

        node_t *listIteratorR1 = rooms.getHead();
        devRoom *r1;
        int counterR1 = 0;
        node_t *listIteratorM1;
        roomMember *m1;
        int counterM1;
        node_t *listIteratorD1;
        devRecord *d1;
        uint8_t mac[6];
        node_t *listIteratorW1;
        usageWindow *w1;

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

                //cout << "Group " << counterM1 << "(" << m1 << ", " << m1->member << ") with " << ((devGroup *)m1->member)->mems.getLen() << " members, has membership probability " << (int)m1->memberProb  << ":" << endl;
                cout << "Group " << counterM1 << " with " << ((devGroup *)m1->member)->mems.getLen() << " members, has membership probability " << (int)m1->memberProb  << ":" << endl;
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
                    //cout << dec << " (" << d1 << ")" << endl;
                    cout << dec << endl;

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

            cout << "Usage windows" << endl;

            listIteratorW1 = r1->usage.windows.getHead();

            while(listIteratorW1)
            {
                w1 = (usageWindow *)listIteratorW1->data;

                cout << "Window found on day " << w1->day + 1;
                cout << " at " << (w1->start * 1800) / 3600 << ":" << ((w1->start * 1800) % 3600) / 60;
                cout << " - " << ((w1->end + 1) * 1800) / 3600 << ":" << ((w1->end + 1 * 1800) % 3600) / 60 << endl;

                listIteratorW1 = r1->usage.windows.getNext(listIteratorW1);
            }

            listIteratorR1 = rooms.getNext(listIteratorR1);
        }
        cout << endl;
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

    int netOpt::printPlugs()
    {
        return(plugOpt.printPlugs());
    }
#endif

#ifdef LOG
    int netOpt::saveUsage()
    {
        #ifdef TESTING
            cout << "Saving usage" << endl;
        #endif

        node_t *listIteratorD1 = devices->getHead();
        devRecord *d1;
        uint8_t mac[6];

        ofstream ofile;
        ostringstream ss;

        while(listIteratorD1)
        {
            d1 = (devRecord *)listIteratorD1->data;

            unpackMAC(d1->macAddr, mac);

            ss.str("");
            ss.clear();

            ss << hex;
            for(int i = 0; i < 6; i++)
            {
                ss << (int)mac[i];
            }
            ss << dec;

            string csv = ss.str();

            if(!ofile.is_open())
            {
                #ifdef TESTING
                    cout << "opening file: " << "logs/usage/" << csv << ".csv" << endl;
                #endif

                ofile.open("logs/usage/" + csv + ".csv", ios::out);

                for(int i = 0; i < 7; i++)
                {
                    for(int j = 0; j < 47; j++)
                    {
                        ofile << d1->usage.time[i][j] << ",";
                    }
                    ofile << d1->usage.time[i][47] << endl;
                }
                
                ofile.close();
            }

            listIteratorD1 = devices->getNext(listIteratorD1);
        }

        node_t *listIteratorR1 = rooms.getHead();
        devRoom *r1;
        int counterR1 = 0;

        while(listIteratorR1)
        {
            r1 = (devRoom *)listIteratorR1->data;
            
            if(r1->groups.getLen() > 0)
            {
                d1 = (devRecord *)((devGroup *)((roomMember *)r1->groups.getHead()->data)->member)->mems.getHead()->data;
            }
            else if(r1->mems.getLen() > 0)
            {
                d1 = (devRecord *)((roomMember *)r1->mems.getHead()->data)->member;
            }
            else
            {
                #ifdef TESTING
                    cout << "Room has no devices" << endl;
                #endif

                return 1;
            }

            unpackMAC(d1->macAddr, mac);

            ss.str("");
            ss.clear();

            ss << "r" << counterR1 << "_" << hex;
            for(int i = 0; i < 6; i++)
            {
                ss << (int)mac[i];
            }
            ss << dec;

            string csv = ss.str();

            if(!ofile.is_open())
            {
                #ifdef TESTING
                    cout << "opening file: " << "logs/usage/" << csv << ".csv" << endl;
                #endif

                ofile.open("logs/usage/" + csv + ".csv", ios::out);

                for(int i = 0; i < 7; i++)
                {
                    for(int j = 0; j < 47; j++)
                    {
                        ofile << r1->usage.time[i][j] << ",";
                    }
                    ofile << r1->usage.time[i][47] << endl;
                }
                
                ofile.close();
            }

            listIteratorR1 = rooms.getNext(listIteratorR1);
            counterR1++;
        }

        #ifdef TESTING
            cout << "Usage saved" << endl;
        #endif

        return 0;
    }
#endif
