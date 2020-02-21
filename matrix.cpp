#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

typedef struct Matrix {
vector<vector<int> >  a;   
} Matrix;

Matrix operator+(Matrix x, Matrix y) {
    Matrix z;
    for (int i=0; i<x.a.size() ; i++) {
        auto p =x.a[i].begin();
        auto q =y.a[i].begin();
        vector<int> zz;
        for (; p!=x.a[i].end(); p++,q++) {
            zz.push_back(*p+*q);
        }
        z.a.push_back(zz);
    }
    return z;
}
int main () {
   int cases,k;
   cin >> cases;
   for(k=0;k<cases;k++) {
      Matrix x;
      Matrix y;
      Matrix result;
      int n,m,i,j;
      cin >> n >> m;
      for(i=0;i<n;i++) {
         vector<int> b;
         int num;
         for(j=0;j<m;j++) {
            cin >> num;
            b.push_back(num);
         }
         x.a.push_back(b);
      }
      for(i=0;i<n;i++) {
         vector<int> b;
         int num;
         for(j=0;j<m;j++) {
            cin >> num;
            b.push_back(num);
         }
         y.a.push_back(b);
      }
      result = x+y;
      for(i=0;i<n;i++) {
         for(j=0;j<m;j++) {
            cout << result.a[i][j] << " ";
         }
         cout << endl;
      }
   }  
   return 0;
}


