#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <map>
#include <tuple>
using namespace std;

map<tuple<int,int>, int> m;
int N;
const int MAXINT = 1000000000;
vector<int> v;
int ss;

int distance(int s, int k) {
    if (s==k) return 0;
    int min = MAXINT;

    tuple<int,int> r(s,k);
    if (m[r] > 0) min = m[r];

    for (int i=1; i<N+1; i++) {
        bool skip=false;
        for (std::vector<int>::iterator it = v.begin() ; it != v.end(); ++it) {
            if(i==*it) {
                skip=true;
                break;
            }
        }
        if (skip) continue;
        if (ss==i) continue;
        if (k==i) continue;
        tuple<int,int> tt(s, i);
        if (m[tt]>0){
            v.push_back(i);
            int d = distance(i,k);
            //if (d==6) {min=d;v.pop_back();break;}
            min = (min<d+m[tt]) ? min : d+m[tt];
            v.pop_back();
        }
    }
    return (min==MAXINT)? MAXINT: min;
}
int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    int n,e1,e2,e3,E;
    cin>>n;
    for(int i=0;i<n;i++){
        m.clear();
        cin>>N>>E;
        for(int j=0;j<E;j++) {
            cin>>e1>>e2>>e3;
            tuple<int,int> t(e1,e2), q(e2,e1);
            m[t]=e3;
            m[q]=e3;
        }
        cin>>ss;
        for(int k=1;k<N+1;k++) {
            v.clear();
            if (k==ss) continue;
            v.push_back(ss);
            v.push_back(k);
            int d= distance(ss,k);
            cout<< ((d==MAXINT)?-1:d);
            cout<<" ";
        }
        cout<<endl;
    }
    return 0;
}


