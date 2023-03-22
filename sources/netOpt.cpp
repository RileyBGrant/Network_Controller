#include "networkOptimiser.h"

using namespace std;

netOpt::netOpt(netInt *interface)
{
    devices = interface->getDevices();
}

int netOpt::sortDevs()
{
    #ifdef TESTING
        cout << "Sorting Records" << endl;
    #endif
    node_t *listIterator = devices->getHead();
    devRecord *dev;
    while(listIterator)
    {
        dev = (devRecord*)listIterator->data;

        switch(dev->devType)
        {
        case 0:
            lightOpt.addDevice(dev);
            break;
        
        }

        listIterator = devices->getNext(listIterator);
    }
}