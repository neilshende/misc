long arrayManipulation(int n, vector<vector<int>> queries) {
    vector<long> A(n);
    for (int k=0 ; k < n; k++) A[k] = 0;
    int b, e, v;
    vector<vector<int>>::iterator it;
    for (it = queries.begin(); it != queries.end(); it++) {
        b= (*it)[0]; e= (*it)[1]; v = (*it)[2];
        if (e >= n) {
            A.resize(e+1);
            for (int j=n; j<e+1; j++) A[j] = 0;
            n = e+1;
        }
        if (b==e) {A[b]=A[b]+v;} else {
            for (int i = b; i <= e; i++) A[i]=A[i]+v;
        }
    }
    long max=A[0];
    for (int i=1; i<n; i++) {
        if (A[i]> max) max = A[i];
    }
    return max;


}
