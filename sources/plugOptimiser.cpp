#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif

using namespace std;

int plugOptimiser::addDevice(devRecord *newDev)
{
    #ifdef TESTING
        cout << "Plug optimiser: Adding new device" << endl;
    #endif

    node_t *listIterator = plugs.getHead();
    devRecord *dev;

    while(listIterator)
    {
        dev = (devRecord *)((pluggedDev *)listIterator->data)->plug;

        if(dev->macAddr == newDev->macAddr)
        {
            #ifdef TESTING
                cout << "Plug optimiser: Device already added" << endl;
            #endif

            return 0;
        }

        listIterator = plugs.getNext(listIterator);
    }

    #ifdef TESTING
        cout << "Plug optimiser: Device not found adding to list" << endl;
    #endif

    pluggedDev *p1 = new pluggedDev;
    p1->plug = newDev;
    p1->dev = NULL;

    plugs.append(p1);
    //printDevs();

    return 0;
}

int plugOptimiser::pairPlugs(linkedList_t *devices)
{
    #ifdef TESTING
        cout << "Light optimiser: Adding new device" << endl;
    #endif

    node_t *listIteratorD1 = devices->getHead();
    devRecord *d1;
    node_t *listIteratorP2 = plugs.getHead();
    devRecord *d2;
    devRecord *p2;
    node_t *listIteratorA1;
    node_t *listIteratorA2;
    activityRecord *a1;
    activityRecord *a2;
    bool devPaired;

    while(listIteratorD1)
    {
        d1 = (devRecord *)listIteratorD1->data;
        devPaired = false;
        listIteratorP2 = plugs.getHead();

        while(listIteratorP2)
        {
            d2 = ((pluggedDev *)listIteratorP2->data)->dev;

            if(d1 == d2)
            {
                devPaired = true;
                listIteratorP2 = NULL;
            }
            else
            {
                listIteratorP2 = plugs.getNext(listIteratorP2);
            }
        }

        if(devPaired == false)
        {
            listIteratorP2 = plugs.getHead();

            while(listIteratorP2)
            {
                if(((pluggedDev *)listIteratorP2->data)->dev != NULL)
                {
                    devPaired = true;
                    p2 = ((pluggedDev *)listIteratorP2->data)->plug;

                    listIteratorA1 = d1->activity.getHead();
                    listIteratorA2 = p2->activity.getHead();

                    while(listIteratorA1 != NULL && listIteratorA2 != NULL)
                    {
                        a1 = (activityRecord *)listIteratorA1->data;
                        a2 = (activityRecord *)listIteratorA2->data;

                        if(a1->timestamp > a2->timestamp)
                        {
                            listIteratorA1 = NULL;
                            listIteratorA2 = NULL;
                            devPaired = false;
                        }
                        else if(a1->timestamp < a2->timestamp)
                        {
                            listIteratorA1 = d1->activity.getNext(listIteratorA1);
                        }
                        else
                        {
                            if(a1->variable == 0)
                            {
                                if(a2->variable == 0)
                                {
                                    if(a1->state == a2->state)
                                    {
                                        listIteratorA1 = d1->activity.getNext(listIteratorA1);
                                        listIteratorA2 = p2->activity.getNext(listIteratorA2);
                                    }
                                    else
                                    {
                                        listIteratorA1 = NULL;
                                        listIteratorA2 = NULL;
                                        devPaired = false;
                                    }
                                }
                                else
                                {
                                    listIteratorA2 = p2->activity.getNext(listIteratorA2);
                                }
                            }
                            else
                            {
                                listIteratorA1 = d1->activity.getNext(listIteratorA1);
                            }
                        }
                    }
                }

                if(devPaired == true)
                {
                    ((pluggedDev *)listIteratorP2->data)->dev = d1;
                    listIteratorP2 = NULL;
                }
                else
                {
                    listIteratorP2 = plugs.getNext(listIteratorP2);
                }
                
            }
        }

        listIteratorD1 = devices->getNext(listIteratorD1);
    }

    printPlugs();

    return 0;
}

string plugOptimiser::inactivity(devRecord *d0)
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
    int plugOptimiser::printPlugs()
    {
        cout << "Plug Optimiser: Plugged devices" << endl;
        node_t *listIteratorP1 = plugs.getHead();
        pluggedDev *p1;
        uint8_t mac[6];

        while(listIteratorP1)
        {
            cout << "here" << endl;
            p1 = (pluggedDev *)listIteratorP1->data;
            unpackMAC(p1->plug->macAddr, mac);
            cout << "Plug Optimiser: Plug " << hex << stoi(to_string(mac[0]));
            for(int i = 1; i < 6; i++)
            {
                cout << "." << stoi(to_string(mac[i]));
            }

            if(p1->dev != NULL)
            {
                unpackMAC(p1->dev->macAddr, mac);
                cout << dec << " connected to " << hex << stoi(to_string(mac[0]));
                for(int i = 1; i < 6; i++)
                {
                    cout << "." << stoi(to_string(mac[i]));
                }
                cout << dec << endl;

                listIteratorP1 = plugs.getNext(listIteratorP1);
            }
            else
            {
                cout << dec << " bot connected to a device" << endl;
            }
        }

        return 0;
    }
#endif