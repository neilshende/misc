#include<stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

//Define the structs Workshops and Available_Workshops.
//Implement the functions initialize and CalculateMaxWorkshops
typedef struct Available_Workshops {
    int start_time;
    int duration;
    int end_time;
} Available_Workshops;

static int maximum = 0;
Available_Workshops *array;

bool isAttendable(int *index, int n) {
    int i;
    //qsort by start_time
    for (i=1; i<n; i++) {
        if (array[index[i-1]].end_time > array[index[i]].start_time || array[index[i-1]].start_time == array[index[i]].start_time) {
            return false;
        }
    }
    return true;
}

void combinationUtil(int arr[], int data[], int start, int end, int index, int r)
{
if (index == r)
{
if (isAttendable(data, r)) {
maximum = r;
}
return;
}
for (int i=start; i<=end && end-i+1 >= r-index; i++)
{
data[index] = arr[i];
combinationUtil(arr, data, i+1, end, index+1, r);
}
}
void printCombination(int arr[], int n, int r)
{
int data[r];
combinationUtil(arr, data, 0, n-1, 0, r);
}
int compare_function(const void *ll, const void *rr)
{
Available_Workshops *l=(Available_Workshops *)ll;
Available_Workshops *r=(Available_Workshops *)rr;
return l->start_time - r->start_time;
}


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
    
    qsort(array, n, sizeof(array[0]), compare_function);
    return array;
}


int CalculateMaxWorkshops(Available_Workshops *arrayy) {
    int n,i;
    array= arrayy;
    for(n=0; ; n++){
        if (array[n].end_time == -2) break;
    } 
    int data[n];
    for (i = 0; i < n; i++) data[i]=i;
for (i = 1; i <=n; i++) {
printCombination(data, n, i);
if (maximum != i) break;
}   
return maximum;
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
