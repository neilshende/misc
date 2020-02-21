#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string.h>
using namespace std;


int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */ 
    int i,j,k;
    char s[10001];
    char t[10001];
    int n;
    cin >> n;
    for (i=0;i<n;i++) {
    memset(s, 0, 10001);
    memset(t, 0, 10001);
    cin >> s;
        n= strlen(s);
    for(j = 0; j<n; j++ ) t[j] = s[n-j-1];
    
    for(k=1; k<n; k++) {
        if (abs(s[i]-s[i-1])==abs(t[i]-t[i-1])) continue;
    }
    if (k==n) {
        cout << "Not Funny\n";
    } else {
        cout << "Funny\n";
    }
    }
    return 0;
}
