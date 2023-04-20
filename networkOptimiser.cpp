#include "networkOptimiser.h"
#ifndef MAIN_HEADER_MISSING
    #include "include/networkOptimiser.h"
#endif

using namespace std;

int main()
{
    uint8_t intAddr[4] = {192,168,0,154};
    netInt interface(intAddr);
    netOpt optimiser(&interface);
    time_t lastOpt = 0;

    interface.connectToHost();

    int iR = 0; //interface return value
    int oR = 0; //optimiser return value

    while(iR < SIM_FINISHED)
    //for(int i = 0; i < 1000000; i++)
    {
        iR = interface.readFromHost();

        if(iR == 2)
        {
            optimiser.sendDevStims();
        }
        else
        {
            optimiser.activeRoomUpdate();
        }

        interface.endBurst();

        if(interface.getLastTimestamp() - lastOpt > 86400)
        {
            lastOpt = interface.getLastTimestamp();
            optimiser.sortDevs();
            optimiser.optimise();
        }
    }

    
    optimiser.sortDevs();
    optimiser.optimise();
    optimiser.characteriseUsage();

    cout << interface.getLastTimestamp() << endl;
    
    #ifdef LOG
        optimiser.saveUsage();
    #endif

    //optimiser.printRooms();    

    return 0;
}