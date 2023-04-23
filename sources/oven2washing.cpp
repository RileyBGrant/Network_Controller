#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "../include/networkOptimiser.h"
#endif

#include <cstdlib>

using namespace std;

int8_t netOpt::oven2washing(roomMember *oven, roomMember *washing)
{
    devRecord *d1 = (devRecord *)oven->member;
    devRecord *d2 = (devRecord *)washing->member;
    
    #ifdef TESTING
        uint8_t mac[6];
        unpackMAC(d1->macAddr, mac);
        cout << "Compatability test between oven " << hex << (int)mac[0];
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
        cout << dec <<  endl;
    #endif

    #ifdef TESTING
        cout << "Test complete, probability change of " << 0 << endl;
    #endif
    
    return 0;
}