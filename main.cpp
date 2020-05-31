#include <iostream>

#include "WMaskArray.h"

using namespace std;

int main()
{
    WMaskArray<2> a;

    a.resize(64);
    a.fill(0, 0, -1);
    // a.fill(3, 5, 13);
    a.set(5, 3);

    //    for (int i = 0; i < 10000000; i++) {
    //        a.append(i % 3);
    //    }

    //    uint8_t v = a.get(3001);
    //    printf("%d[%d]\n", int(v), int(3001 % 3));
    //    printf("------\n");

    //    //    for (int i = 0; i < 10000000; i++) {
    //    //        printf("%d\n", a.at(i % 3));
    //    //    }

    //    printf("size=%d\n", a.size());

    //    for (int i = 0; i < 10000000; i++) {
    //        a.pop();
    //    }


    cout << "Hello World!" << endl;
    return 0;
}
