#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;


int binary_search(int *array, int size, int number) {
  int L = 0;
  int R = size-1;
  int T = number;
  while (L <= R) {
    int M = floor((L+R)/2);
    if (array[M] < T) {
      L = M+1;
    } else if (array[M] > T) {
      R = M-1;
    } else {
      return M;
    }
  }
  return -1;
}


int main() {
  int x[] = {1, 3, 5 ,7 , 8, 9, 10};
  cout << binary_search(x, sizeof(x)/sizeof(x[0]), 3) << endl;
  cout << binary_search(x, sizeof(x)/sizeof(x[0]), 9) << endl;
  return 0;
}
