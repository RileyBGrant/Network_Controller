#include "networkOptimiser.h"
#include <cstdlib>

using namespace std;

netOpt::netOpt(netInt *interface)
{
    devices = interface->getDevices();

    groups.append(lightOpt.getGroups());
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
    //devGroup *g1;
    //devGroup *g2;
    


    //check exisiting rooms
    while (listIteratorR1)
    {
        r1 = (devRoom *) listIteratorR1->data;
        listIteratorM1 = r1->groups.getHead();
        counterM1 = 0;

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
                r1->mems.remove(counterM1);
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

    while(listIteratorL1)
    {
        l1 = (linkedList_t *)listIteratorL1->data;
        listIteratorG1 = l1->getHead();

        while(listIteratorG1)
        {
            g1 = (devGroup *)listIteratorG1->data;

            if(((devRecord *)g1->mems.getHead())->rooms.getHead() == NULL)
            {
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
                    }
                    else
                    {
                        listIteratorR1 = rooms.getNext(listIteratorR1);
                    }
                }

                if(roomFound == false)
                {
                    r1 = new devRoom;
                    m1->memberProb = 255;
                    r1->groups.append(m1);
                    rooms.append(r1);
                }
            }
            listIteratorG1 = l1->getNext(listIteratorG1);
        }     
        
        listIteratorL1 = groups.getNext(listIteratorL1);
    }

    return 0;
}

int8_t netOpt::light2Light(roomMember *m1, roomMember *m2)
{
    devGroup *g1 = (devGroup *)m1->member;
    devGroup *g2 = (devGroup *)m2->member;
    node_t *listIteratorA1 = ((devRecord *)g1->mems.getHead()->data)->activity.getHead();
    node_t *listIteratorA2 = ((devRecord *)g2->mems.getHead()->data)->activity.getHead();
    activityRecord *a1;
    activityRecord *a2;
    bool devMatch = true;
    int8_t probChange = 0;
    int timeDiff = 0;

    while(listIteratorA1 != NULL && listIteratorA2 != NULL)
    {
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
        }
        else
        {
            if(probChange < 127)
            {
                probChange++;
            } 
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

    return probChange;
}