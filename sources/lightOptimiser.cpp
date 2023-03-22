#include "networkOptimiser.h"

using namespace std;

int lightOptimiser::addDevice(devRecord *newDev)
{
    #ifdef TESTING
        cout << "Light optimiser: Adding new device" << endl;
    #endif
    node_t *listIterator = lightDevs.getHead();
    devRecord *dev;
    while(listIterator)
    {
        if(dev->macAddr == newDev->macAddr)
        {
            #ifdef TESTING
                cout << "Light optimiser: Device already added" << endl;
            #endif

            return 0;
        }
        listIterator = lightDevs.getNext(listIterator);
    }
    #ifdef TESTING
        cout << "Light optimiser: Device not found adding to list" << endl;
    #endif
    lightDevs.append(newDev);
    printDevs();

    return 0;
}

#ifdef TESTING
    int lightOptimiser::printDevs()
    {
        cout << "Light Optimiser: Light devices" << endl;
        node_t *listIteratorD = lightDevs.getHead();
        devRecord *dev;
        uint8_t mac[6];
        while(listIteratorD)
        {
            dev = (devRecord *)listIteratorD->data;
            unpackMAC(dev->macAddr, mac);
            cout << "Light Optimiser: " << hex << stoi(to_string(mac[0]));
            for(int i = 1; i < 6; i++)
            {
                cout << "." << stoi(to_string(mac[i]));
            }
            cout << dec << endl;

            listIteratorD = lightDevs.getNext(listIteratorD);
        }
        return 0;
    }
#endif