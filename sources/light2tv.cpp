#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif

#include <cstdlib>

using namespace std;

int8_t netOpt::light2tv(roomMember *light, roomMember *tv)
{
    devGroup *l1 = (devGroup *)light->member;
    devRecord *d1 = (devRecord *)tv->member;

    #ifdef TESTING
    uint8_t mac[6];
        unpackMAC(((devRecord *)l1->mems.getHead()->data)->macAddr, mac);
        cout << "Compatability test between light group with lead device " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(d1->macAddr, mac);
        cout << dec << " and tv " << hex << (int)mac[0];
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

                        listIteratorA3 = d1->activity.getNext(listIteratorA3);
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
                    listIteratorA3 = d1->activity.getNext(listIteratorA3);
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

    #ifdef TESTING
        cout << "Test complete, probability change of " << probChange << endl;
    #endif

    return probChange;
}