#include <assert.h>
#include <iostream>
#include "dmemalloc.h"

using namespace std;


int main()
{
    int* p1 = new int(10);
    std::cout << "P1: " << *p1 <<std::endl;
    assert (*p1==10);

    delete p1;


    int* p2 = new int[10]; // guaranteed to call the replacement in C++11
    delete[] p2;
    cout << "Hello World!" << endl;
#if 0
    cout << "nb  new:" << getNbOfNew() << endl;
    cout << "nb free:" << getNbOfFree() << endl;
    cout << "Delta  :" << getNbOfNew() - getNbOfFree() << endl;
#endif
#if NDEBUG
    assert(dMemAllocEnabled() ==0);

#else
  //  assert(dMemAllocEnabled());
plop();
#endif
    return 0;
}



