#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif

#include <cstdlib>

using namespace std;

int8_t netOpt::light2light(roomMember *m1, roomMember *m2)
{
    devGroup *g1 = (devGroup *)m1->member;
    devGroup *g2 = (devGroup *)m2->member;
    
    #ifdef TESTING
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
        cout << dec << " with probability adjustment of " << getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), 0.0) << endl;
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
        if(timeDiff > 30 || timeDiff < -30)
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
            if(-2 + getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), 0.0) < 0)
            {
                if(probChange >= -126 - getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), 0.0))
                {
                    probChange = probChange -2 + getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), -0.1);
                }
                else
                {
                    probChange = -128;
                    getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), -0.1);
                }
            }
            else
            {
                getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), -0.1);
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
            if(4 + getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), 0.0) > 0)
            {
                if(probChange <= 123 - getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), 0.0))
                {
                    probChange = probChange + 4 + getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), 0.6);
                }
                else
                {
                    probChange = 127;
                    getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), 0.6);
                }
            }
            else
            {
                getProbAdjustment(((devRecord *)g1->mems.getHead()->data), ((devRecord *)g2->mems.getHead()->data), 0.6);
            }

            listIteratorA1 = g1->mems.getNext(listIteratorA1);
            listIteratorA2 = g2->mems.getNext(listIteratorA2);
        }
    }

    #ifdef TESTING
        cout << "Test complete, probability change of " << probChange << endl;
    #endif

    return probChange;
}