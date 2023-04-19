#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif

using namespace std;

linkedList_t *lightOptimiser::getGroups()
{
    return &lightGroups;
}

int lightOptimiser::addDevice(devRecord *newDev)
{
    #ifdef TESTING
        cout << "Light optimiser: Adding new device" << endl;
    #endif
    node_t *listIterator = lightDevs.getHead();
    devRecord *dev;
    while(listIterator)
    {
        dev = (devRecord *)listIterator->data;
        if(dev->macAddr == newDev->macAddr)
        {
            #ifdef TESTING
                cout << "Light optimiser: Device already added" << endl;
            #endif

            return 0;
        }
        listIterator = lightDevs.getNext(listIterator);
    }
    #ifdef TESTING
        cout << "Light optimiser: Device not found adding to list" << endl;
    #endif
    lightDevs.append(newDev);
    //printDevs();

    return 0;
}

int lightOptimiser::groupLights()
{
    #ifdef TESTING
        cout << "Light optimiser: Grouping lights" << endl;
    #endif
    node_t *listIteratorG1 = lightGroups.getHead();
    node_t *listIteratorG2;
    node_t *listIteratorD1;
    node_t *listIteratorD2;
    node_t *listIteratorA1;
    node_t *listIteratorA2;
    node_t *listIteratorR1;
    node_t *listIteratorR2;
    devRoom *r1;
    devRoom *r2;
    int counterR1;
    devGroup *group;
    int counterG1 = 0;
    int counterG2 = 0;
    devRecord *dev;
    int counterD;
    devRecord *masterDev;
    activityRecord *activity1;
    activityRecord *activity2;
    int len;
    bool devMatch;
    bool roomMatch;

    //check current groups are accurate, WITH EXTRA TIME ADD MEMBERSHIP PROBABILITY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    while(listIteratorG1)
    {
        group = (devGroup *)listIteratorG1->data;

        listIteratorD1 = group->mems.getHead();
        masterDev = (devRecord *)listIteratorD1->data;
        len = masterDev->activity.getLen();
        listIteratorD1 = group->mems.getNext(listIteratorD1);
        counterD = 1;

        while(listIteratorD1)
        {
            dev = (devRecord *)listIteratorD1->data;

            if(dev->activity.getLen() == len)
            {
                devMatch = true;
                listIteratorA1 = masterDev->activity.getHead();
                listIteratorA2 = dev->activity.getHead();
                
                while(listIteratorA1 != NULL && listIteratorA2 != NULL)
                {
                    activity1 = (activityRecord *)listIteratorA1->data;
                    activity2 = (activityRecord *)listIteratorA2->data;

                    if(activity1->timestamp != activity2->timestamp)
                    {
                        #ifdef TESTIN
                            cout << "Light Optimiser: Timestep mismatch, masterDev: " << activity1->timestamp << ", dev " << activity2->timestamp << endl;
                        #endif
                        devMatch = false;
                    }

                    if(activity1->state != activity2->state)
                    {
                        #ifdef TESTIN
                            cout << "Light Optimiser: Activity mismatch, masterDev: " << activity1->state << ", dev " << activity2->state << endl;
                        #endif
                        devMatch = false;
                    }

                    if(activity1->variable != activity2->variable)
                    {
                        #ifdef TESTIN
                            cout << "Light Optimiser: Variable mismatch, masterDev: " << activity1->variable << ", dev " << activity2->variable << endl;
                        #endif
                        devMatch = false;
                    }

                    if(devMatch == false)
                    {
                        #ifdef TESTING
                            uint8_t mac[6];
                            unpackMAC(dev->macAddr, mac);
                            cout << "Light Optimiser: Removing device " << hex << (int)mac[0];
                            for(int i = 1; i < 6; i++)
                            {
                                cout << "." << (int)mac[i];
                            }
                            cout << dec << " from group" << endl;
                        #endif
                        listIteratorA1 = NULL;
                        listIteratorA2 = NULL;

                        listIteratorG2 = dev->groups.getHead();
                        counterG2 = 0;

                        while(listIteratorG2)
                        {
                            if(listIteratorG2->data == group)
                            {
                                dev->groups.remove(counterG2);
                                listIteratorR1 = dev->rooms.getHead();
                                counterR1 = 0;

                                while(listIteratorR1)
                                {
                                    r1 = (devRoom *)listIteratorR1->data;
                                    roomMatch = false;
                                    listIteratorR2 = masterDev->rooms.getHead();

                                    while(listIteratorR2)
                                    {
                                        r2 = (devRoom *)listIteratorR2->data;

                                        if(r2 == r1)
                                        {
                                            roomMatch = true;
                                            listIteratorR2 = NULL;
                                        }
                                        else
                                        {
                                            listIteratorR2 = masterDev->rooms.getNext(listIteratorR2);
                                        }
                                    }

                                    listIteratorR1 = dev->rooms.getNext(listIteratorR1);
                                    if(roomMatch == true)
                                    {
                                        dev->rooms.remove(counterR1);
                                    }
                                    else
                                    {
                                        counterR1++;
                                    }
                                }

                                listIteratorG2 = NULL;
                            }
                            else
                            {
                                listIteratorG2 = dev->groups.getNext(listIteratorG2);
                                counterG2++;
                            }
                        }
                    }
                    else
                    {
                        listIteratorA1 = masterDev->activity.getNext(listIteratorA1);
                        listIteratorA2 = dev->activity.getNext(listIteratorA2);
                    }  
                }
            }

            listIteratorD1 = group->mems.getNext(listIteratorD1);
            if(devMatch == false)
            {
                group->mems.remove(counterD);
            }
            else
            {
                counterD++;
            }
        }

        /*if(group->mems.getLen() <= 1)
        {
            listIteratorG1 = lightGroups.getNext(listIteratorG1);
            group->mems.remove(counterG1);
        }
        else
        {
            listIteratorG1 = lightGroups.getNext(listIteratorG1);
            counterG1++;
        }*/

        listIteratorG1 = lightGroups.getNext(listIteratorG1);
        counterG1++;
    }

    //check if any other groups need to be created
    listIteratorD1 = lightDevs.getHead();

    while(listIteratorD1)
    {
        masterDev = (devRecord *)listIteratorD1->data;

        #ifdef TESTING
            int counter = 0;
        #endif

        //Check if device can be added to an existing group
        listIteratorG1 = lightGroups.getHead();
        while(listIteratorG1 != NULL && masterDev->groups.getLen() < 1)
        {
            #ifdef TESTIN
                cout << "Light optimiser: Checking for match with group " << counter << endl;
                counter++;
            #endif
            group = (devGroup *)listIteratorG1->data;
            listIteratorD2 = group->mems.getHead();
            dev = (devRecord *)listIteratorD2->data;

            #ifdef TESTIN
                cout << "Light optimiser: Master Dev record length is  " << masterDev->activity.getLen() << endl;
                cout << "Light optimiser: Group Dev record length is  " << dev->activity.getLen() << endl;
            #endif

            if(dev != masterDev)
            {
                if(dev->activity.getLen() == masterDev->activity.getLen())
                {
                    #ifdef TESTIN
                        cout << "Light optimiser: length match " << endl;
                    #endif
                    
                    listIteratorA1 = masterDev->activity.getHead();
                    listIteratorA2 = dev->activity.getHead();
                    devMatch = true;

                    while(listIteratorA1 != NULL && listIteratorA2 != NULL)
                    {
                        activity1 = (activityRecord *)listIteratorA1->data;
                        activity2 = (activityRecord *)listIteratorA2->data;

                        if(activity1->timestamp != activity2->timestamp)
                        {
                            devMatch = false;
                        }

                        if(activity1->state != activity2->state)
                        {
                            devMatch = false;
                        }

                        if(activity1->variable != activity2->variable)
                        {
                            devMatch = false;
                        }

                        if(devMatch == false)
                        {
                            #ifdef TESTIN
                                cout << "Records do not match" << endl;
                                uint8_t mac[6];
                                unpackMAC(masterDev->macAddr, mac);
                                cout << "Record for device " << hex << stoi(to_string(mac[0]));
                                for(int i = 1; i < 6; i++)
                                {
                                    cout << "." << stoi(to_string(mac[i]));
                                }
                                cout << dec << " of type " << (int)masterDev->devType << ": ";
                                tm tempTime = *gmtime(&activity1->timestamp);
                                cout << "Variable " << (int)activity1->variable << " set to state " << (int)activity1->state << " at " << asctime(&tempTime);
                                unpackMAC(dev->macAddr, mac);
                                cout << "Record for device " << hex << stoi(to_string(mac[0]));
                                for(int i = 1; i < 6; i++)
                                {
                                    cout << "." << stoi(to_string(mac[i]));
                                }
                                cout << dec << " of type " << (int)dev->devType << ": ";
                                tempTime = *gmtime(&activity2->timestamp);
                                cout << "Variable " << (int)activity2->variable << " set to state " << (int)activity2->state << " at " << asctime(&tempTime);
                            #endif
                            listIteratorA1 = NULL;
                            listIteratorA2 = NULL;
                        }
                        else
                        {
                            listIteratorA1 = masterDev->activity.getNext(listIteratorA1);
                            listIteratorA2 = dev->activity.getNext(listIteratorA2);
                        }
                    }

                    if(devMatch == true)
                    {
                        group->mems.append(masterDev);
                        masterDev->groups.append(group);
                    }
                }
            }

            listIteratorG1 = lightGroups.getNext(listIteratorG1);
        }

        //create new group
        if(masterDev->groups.getLen() < 1)
        {
            #ifdef TESTING
                cout << "Light optimiser: No matching group found, creating new group" << endl;
            #endif
            devGroup *newGroup = new devGroup;
            newGroup->mems.append(masterDev);
            newGroup->devtype = masterDev->devType;

            lightGroups.append(newGroup);
            masterDev->groups.append(newGroup);
        }
        else
        

        listIteratorD1 = lightDevs.getNext(listIteratorD1);
    }

    #ifdef TESTING
        printGroups();
    #endif

    return 0;
}

string lightOptimiser::inactivity(devRecord *d0)
{
    string message = "";
    uint8_t macAddr[6];
    unpackMAC(d0->macAddr, macAddr);

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

    return message;
}

#ifdef TESTING
    int lightOptimiser::printDevs()
    {
        cout << "Light Optimiser: Light devices" << endl;
        node_t *listIteratorD = lightDevs.getHead();
        devRecord *dev;
        uint8_t mac[6];
        while(listIteratorD)
        {
            dev = (devRecord *)listIteratorD->data;
            unpackMAC(dev->macAddr, mac);
            cout << "Light Optimiser: " << hex << stoi(to_string(mac[0]));
            for(int i = 1; i < 6; i++)
            {
                cout << "." << stoi(to_string(mac[i]));
            }
            cout << dec << endl;

            listIteratorD = lightDevs.getNext(listIteratorD);
        }
        return 0;
    }

    int lightOptimiser::printGroups()
    {
        cout << "Light Optimiser: Light groups" << endl;
        node_t *listIteratorG = lightGroups.getHead();
        devGroup *group;
        node_t *listIteratorM;
        devRecord *dev;
        int i = 0;
        uint8_t mac[6];

        while(listIteratorG)
        {
            group = (devGroup *)listIteratorG->data;

            cout << "Group " << i << " with " << group->mems.getLen() << " members:" << endl;
            i++;
            listIteratorM = group->mems.getHead();

            while(listIteratorM)
            {
                dev = (devRecord *)listIteratorM->data;

                unpackMAC(dev->macAddr, mac);
                cout << hex << stoi(to_string(mac[0]));
                for(int i = 1; i < 6; i++)
                {
                    cout << "." << stoi(to_string(mac[i]));
                }
                cout << dec << endl;

                listIteratorM = group->mems.getNext(listIteratorM);
            }

            listIteratorG = lightGroups.getNext(listIteratorG);
        }
        return 0;
    }
#endif