#include<bits/stdc++.h>

using namespace std;
	

//Define the structs Workshops and Available_Workshops.
//Implement the functions initialize and CalculateMaxWorkshops
typedef struct Available_Workshops {
    int start_time;
    int duration;
    int end_time;
} Available_Workshops;

Available_Workshops * initialize(int *start_time, int *duration, int n) {
    int i;
    Available_Workshops *array=(Available_Workshops *)malloc(sizeof(Available_Workshops)*(n+1));
    for (i=0; i<n; i++) {
        array[i].start_time = start_time[i];
        array[i].duration = duration[i];
        array[i].end_time = start_time[i] + duration[i];
    }
        array[i].start_time = -1;
        array[i].duration = -1;
        array[i].end_time = -2;
    
    //qsort(array)
    return array;
}
bool isAttendable(Available_Workshops *array, int *index, int n) {
    int i;
    //qsort by start_time
    for (i=1; i<n; i++) {
        if (array[index[i-1]].end_time > array[index[i]].start_time || array[index[i-1]].start_time == array[index[i]].start_time) {
            return false;
        }
    }
    return true;
}
char x[] = {0};
char y[sizeof(x)/sizeof(x[0])] = {0};
int used[sizeof(x)] = {0};
void combine(int start)
{
   int i;
   int len=sizeof(y)/sizeof(y[0]);
   int xlen = sizeof(x)/sizeof(x[0]);

   for (int i=start; i<xlen; i++) {
      y[len]= x[i];
      y[++len] = '\0';
      //printf("%s \n",y);
      if (i<xlen) combine(i+1);
      y[--len] = 0;
   }
}

int CalculateMaxWorkshops(Available_Workshops *array) {
    int max = 0;
    int n;
    for(n=0; ; n++){
        if (array[n].end_time == -2) break;
    }
    int index[n] = {0, 1, 3, 5};
    if (isAttendable(array, index, 4)) {
        cout << 444444444;
    }
    
    return n-2;
}

int main(int argc, char *argv[]) {
    int n; // number of workshops
    cin >> n;
    // create arrays of unknown size n
    int* start_time = new int[n];
    int* duration = new int[n];

    for(int i=0; i < n; i++){
        cin >> start_time[i];
    }
    for(int i = 0; i < n; i++){
        cin >> duration[i];
    }

    Available_Workshops * ptr;
    ptr = initialize(start_time,duration, n);
    cout << CalculateMaxWorkshops(ptr) << endl;
    return 0;
}
