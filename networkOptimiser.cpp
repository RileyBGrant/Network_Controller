#include "networkOptimiser.h"

using namespace std;

int main()
{
    uint8_t intAddr[4] = {192,168,0,75};
    netInt interface(intAddr);
    netOpt optimiser(&interface);

    interface.connectToHost();

    //while(true)
    for(int i = 0; i < 1000000; i++)
    {
        interface.readFromHost();
    }
    
    optimiser.sortDevs();

    return 0;
}