#include "networkOptimiser.h"

using namespace std;

int main()
{
    uint8_t intAddr[4] = {192,168,0,75};
    netInt interface(intAddr);

    interface.connectToHost();

    while()
    {
        interface.readFromHost()
    }

    return 0;
}