#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif

#include <cstdlib>

using namespace std;

int8_t netOpt::fridge2kettle(roomMember *fridge, roomMember *kettle)
{
    devRecord *d1 = (devRecord *)kettle->member;
    devRecord *d2 = (devRecord *)fridge->member;
    
    #ifdef TESTING
        uint8_t mac[6];
        unpackMAC(d2->macAddr, mac);
        cout << "Compatability test between fridge " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(d1->macAddr, mac);
        cout << dec << " and kettle " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << " with probability adjustment of " << getProbAdjustment(d1, d2, 0.0) << endl;;
    #endif
    
    
    node_t *listIteratorA1 = d1->activity.getHead();
    node_t *listIteratorA2 = d2->activity.getHead();
    activityRecord *a1;
    activityRecord *a2;
    bool devMatch = true;
    int probChange = 0; //devices are likely to be in the same room
    int timeDiff = 0;

    while(listIteratorA1 != NULL && listIteratorA2 != NULL)
    {
        bool devMatch = true;
        a1 = (activityRecord *)listIteratorA1->data;
        a2 = (activityRecord *)listIteratorA2->data;

        timeDiff = a1->timestamp - a2->timestamp;
        if(timeDiff > 120 || timeDiff < -120)
        {
            devMatch = false;
        }

        if(a2->variable == 0)
        {
            listIteratorA2 = d2->activity.getNext(listIteratorA2);
        }
        else if(devMatch == false)
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
            if(4 + getProbAdjustment(d1, d2, 0.0) > 0)
            {
                if(probChange <= 123 - getProbAdjustment(d1, d2, 0.0))
                {
                    probChange = probChange + 4 + getProbAdjustment(d1, d2, 0.5);
                }
                else
                {
                    probChange = 127;
                    getProbAdjustment(d1, d2, 0.5);
                }
            }
            else
            {
                getProbAdjustment(d1, d2, 0.5);
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