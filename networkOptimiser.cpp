#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "include/networkOptimiser.h"
#endif

using namespace std;

int main()
{
    uint8_t intAddr[4] = {192,168,0,75};
    netInt interface(intAddr);
    netOpt optimiser(&interface);
    time_t lastOpt = 0;

    interface.connectToHost();

    int iR = 0; //interface return value
    int oR = 0; //optimiser return value

    while(iR < 2)
    //for(int i = 0; i < 1000000; i++)
    {
        iR = interface.readFromHost();
        oR = optimiser.activeRoomUpdate(interface.getLastDevUpdated());
        if(oR >= 0)
        {
            cout << "Stim request of time " << (time_t)oR << endl;
            interface.requestStim((time_t)oR);
        }
        else
        {
            uint8_t returnMessage[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
            interface.sendtoHost(&returnMessage, 16);
        }

        if(interface.getLastTimestamp() - lastOpt > 86400)
        {
            lastOpt = interface.getLastTimestamp();
            optimiser.sortDevs();
            optimiser.optimise();
        }
    }

    //cout << interface.getLastTimestamp() << endl;
    optimiser.sortDevs();
    optimiser.optimise();
    optimiser.printRooms();    

    return 0;
}