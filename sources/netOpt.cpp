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
    int type;
    while(listIterator)
    {
        dev = (devRecord*)listIterator->data;

        type = dev->devType;

        #ifdef TESTING
            cout << "DevType: " << type << endl;
        #endif

        switch(type)
        {
        case 0:
            lightOpt.addDevice(dev);
            break;
        default:
            #ifdef TESTING
                cout << "DevType not found" << endl;
            #endif
        }

        listIterator = devices->getNext(listIterator);
    }
    return 0;
}