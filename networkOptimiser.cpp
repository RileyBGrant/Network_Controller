#include "networkOptimiser.h"

using namespace std;

int main()
{
    uint8_t intAddr[4] = {192,168,0,75};
    netInt interface(intAddr);
    netOpt optimiser(&interface);

    interface.connectToHost();

    int r = 0;

    //while(r < 2);
    for(int i = 0; i < 1000000; i++)
    {
        r = interface.readFromHost();
    }
    
    optimiser.sortDevs();
    optimiser.optimise();

    return 0;
}