#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;


int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */   
//    return 0;
    int T;
    cin >> T;
    for (int i=0;i<T;i++) {
        int b;
        cin >> b;
        char x[33];
        memset(x, 0, 33);
        int j = 1;
        while (b) {
            if (b&1) x[j] = '1'; else x[j] ='0';
            j++;
            b=b>>1;
        }
        while(j--) {
        cout << x[j];
        }
        cout << endl;
    }
return 0;
}
