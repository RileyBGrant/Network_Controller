#include "networkOptimiser.h"

using namespace std;

int lightOptimiser::addDevice(devRecord *newDev)
{
    #ifdef TESTING
        cout << "Light optimiser: Adding new device" << endl;
    #endif
    node_t *listIterator = lightDevs.getHead();
    devRecord *dev;
    while(listIterator)
    {
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

    //check current groups are accurate, WITH EXTRA TIME ADD MEMBERSHIP PROBABILITY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    while(listIteratorG1)
    {
        group = (devGroup *)listIteratorG1->data;

        listIteratorD1 = group->mems.getHead();
        masterDev = (devRecord *)listIteratorD1->data;
        len = masterDev->activity.getLen();
        listIteratorD1 = group->mems.getNext(listIteratorD1);

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
                        listIteratorA1 = NULL;
                        listIteratorA2 = NULL;

                        listIteratorG2 = dev->groups.getHead();
                        counterG2 = 0;

                        while(listIteratorG2)
                        {
                            if(listIteratorG2->data == &group)
                            {
                                dev->groups.remove(counterG2);
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

        //Check if device can be added to an existing group
        listIteratorG1 = lightGroups.getHead();
        while(listIteratorG1 != NULL && masterDev->groups.getLen() < 1)
        {
            group = (devGroup *)listIteratorG1->data;
            listIteratorD2 = group->mems.getHead();
            dev = (devRecord *)listIteratorG1->data;

            if(dev->activity.getLen() == masterDev->activity.getLen())
            {
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
                        listIteratorA1 = NULL;
                        listIteratorA2 = NULL;
                    }
                }

                if(devMatch == true)
                {
                    group->mems.append(masterDev);
                    masterDev->groups.append(group);
                }
            }

            listIteratorG1 = lightGroups.getNext(listIteratorG1);
        }

        //create new group
        if(masterDev->groups.getLen() < 1)
        {
            devGroup *newGroup = new devGroup;
            newGroup->mems.append(masterDev);
            newGroup->devtype = masterDev->devType;
            lightGroups.append(newGroup);
        }

        listIteratorD1 = lightDevs.getNext(listIteratorD1);
    }

    #ifdef TESTING
        printGroups();
    #endif

    return 0;
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

            cout << "Group " << i << ":" << endl;
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