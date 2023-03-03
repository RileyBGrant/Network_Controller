#include "networkOpimiser.h"

using namespace std;

int main()
{
    uint8_t intAddr[4] = {169,254,49,240}
    netInt interface(intAddr);

    interface.connectToNetwork();

    return 0;
}