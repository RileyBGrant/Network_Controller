#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif

#include <cstdlib>

using namespace std;

int8_t netOpt::oven2oven(roomMember *m1, roomMember *m2)
{
    devRecord *d1 = (devRecord *)m1->member;
    devRecord *d2 = (devRecord *)m2->member;
    
    #ifdef TESTING
        uint8_t mac[6];
        unpackMAC(d1->macAddr, mac);
        cout << "Compatability test between oven " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(d2->macAddr, mac);
        cout << dec << " and oven " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << endl;
    #endif
    
    
    node_t *listIteratorA1 = d1->activity.getHead();
    node_t *listIteratorA2 = d2->activity.getHead();
    activityRecord *a1;
    activityRecord *a2;
    bool devMatch = true;
    int probChange = 9; //devices are likely to be in the same room
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
            if(a1->timestamp < a2->timestamp)
            {
                listIteratorA1 = d1->activity.getNext(listIteratorA1);
            }
            else if(a1->timestamp > a2->timestamp)
            {
                listIteratorA2 = d2->activity.getNext(listIteratorA2);
            }
            else
            {
                listIteratorA1 = d1->activity.getNext(listIteratorA1);
                listIteratorA2 = d2->activity.getNext(listIteratorA2);
            }
        }
        else
        {
            if(probChange < 123)
            {
                probChange = probChange + 4;
            }
            else
            {
                probChange = 127;
            } 

            listIteratorA1 = d1->activity.getNext(listIteratorA1);
            listIteratorA2 = d2->activity.getNext(listIteratorA2);
        }
    }

    #ifdef TESTING
        cout << "Test complete, probability change of " << probChange << endl;
    #endif

    return probChange;
}