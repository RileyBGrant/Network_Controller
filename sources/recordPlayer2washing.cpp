#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif

#include <cstdlib>

using namespace std;

int8_t netOpt::recordPlayer2washing(roomMember *recordPlayer, roomMember *washing)
{
    devRecord *d1 = (devRecord *)recordPlayer->member;
    devRecord *d2 = (devRecord *)washing->member;

    #ifdef TESTING
        uint8_t mac[6];
        unpackMAC(d1->macAddr, mac);
        cout << "Compatability test between record player " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        unpackMAC(d2->macAddr, mac);
        cout << dec << " and washing machine " << hex << (int)mac[0];
        for(int i = 1; i < 6; i++)
        {
            cout << "." << (int)mac[i];
        }
        cout << dec << " with probability adjustment of " << (int)getProbAdjustment(d1, d2, 0.0) << endl;
    #endif

    if(d1->activity.getLen() < 2 || d2->activity.getLen() < 2)
    {
        #ifdef TESTIN
            cout << "activity records are too short" << endl;
        #endif

        if(getProbAdjustment(d1, d2, 0.0) -1 < -2)
        {
            return -2;
        }
        else if(getProbAdjustment(d1, d2, 0.0) -1 > -1)
        {
            return 1;
        }
        else
        {
            return(getProbAdjustment(d1, d2, 0.0) -1);
        };
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

        if(a1->variable == 0 && (a1->state == 3 || a1->state == 4))
        {
            if(a2->variable == 0 && (a2->state == 0 || a2->state == 1 || a2->state == 2))
            {
                if(a3->variable == 0 && a3->state == 1)
                {
                    if(a4->variable == 0 && a4->state == 0)
                    {
                        if(a1->timestamp < a3->timestamp)
                        {
                            if(a2->timestamp > a3->timestamp)
                            {
                                if(-3 + getProbAdjustment(d1, d2, 0.0) < 0)
                                {
                                    if(probChange >= -125 - getProbAdjustment(d1, d2, 0.0))
                                    {
                                        probChange = probChange -3 + getProbAdjustment(d1, d2, -0.1);
                                    }
                                    else
                                    {
                                        probChange = -128;
                                        getProbAdjustment(d1, d2, -0.1);
                                    }
                                }
                                else
                                {
                                    getProbAdjustment(d1, d2, -0.1);
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
                                if(-3 + getProbAdjustment(d1, d2, 0.0) < 0)
                                {
                                    if(probChange >= -125 - getProbAdjustment(d1, d2, 0.0))
                                    {
                                        probChange = probChange -3 + getProbAdjustment(d1, d2, -0.1);
                                    }
                                    else
                                    {
                                        probChange = -128;
                                        getProbAdjustment(d1, d2, -0.1);
                                    }
                                }
                                else
                                {
                                    getProbAdjustment(d1, d2, -0.1);
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

        #ifdef TESTIN
            cout << "Compatability: " << probChange << endl;
        #endif
    }

    #ifdef TESTING
        cout << "Test complete, probability change of " << probChange << endl;
    #endif

    return probChange;
}