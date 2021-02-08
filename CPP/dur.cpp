#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;


//Define the structs Workshops and Available_Workshops.
//Implement the functions initialize and CalculateMaxWorkshops
#include <stdlib.h>
typedef struct Workshop 
    {
    int start_time;
    int duration;
    int end_time;
} Workshop;
typedef struct Available_Workshops 
    {
    int N;
    Workshop W[0];
} Available_Workshops;
int compare_func(const void *ll, const void *rr) 
    {
    Workshop *l=(Workshop *)ll;
    Workshop *r=(Workshop *)rr;
    return l->start_time - r->start_time;
}
Available_Workshops *initialize(int start_time[], int duration[], int N) 
    {
Available_Workshops *WW=(Available_Workshops *)malloc(sizeof(Workshop)*N+sizeof(int)) ;
    WW->N=N;
    for (int i=0; i<N; i++) {
        WW->W[i].start_time= start_time[i];
        WW->W[i].duration = duration[i];
        WW->W[i].end_time=start_time[i]+duration[i];
    }
    qsort(WW->W, WW->N, sizeof(WW->W[0]), compare_func);
    #if 0
        for (int i=0; i<N; i++) {
        cout << WW->W[i].start_time << ' ' << WW->W[i].end_time << endl;

        }
    #endif
    return WW;  
}
#if 0
int calcMax(int n, Workshop *w)
    {
    if (n==1) return 1;
    int maxi=0;
    for (int i=1;i<n;i++){
        if (w[0].end_time<=w[i].start_time) {
            maxi=max(maxi,1+calcMax(n-i,w+i));
        } else { 
            maxi=max(maxi,calcMax(n-i,w+i));
        }
    }
    cout << "Hello " << n << ' ' << maxi << endl;
    return maxi;
}
#endif
int calcMax(int n, Workshop *w)
{
    if (n==1) return 1;
    if (w[0].end_time<=w[1].start_time) {
            return(1+calcMax(n-1,w+1));
    } else {
            if (w[1].end_time > w[0].end_time) w[1].end_time=w[0].end_time;
            return(calcMax(n-1,w+1));
    }
}


int CalculateMaxWorkshops(Available_Workshops *ptr) 
    {
    return calcMax(ptr->N, ptr->W);
}

int main()
{
    int n;
    cin>>n;
    int start_time[n],duration[n];
    for(int i=0;i<n;i++)
    {
        cin>>start_time[i];
    }
    for(int i=0;i<n;i++)
    {
        cin>>duration[i];
    }
    
    Available_Workshops * ptr;
    ptr=initialize(start_time,duration,n);
    cout<<CalculateMaxWorkshops(ptr)<<endl;
    return 0;
}


