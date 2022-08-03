#include <iostream>
#include <vector>
template <bool... digits>
int reversed_binary_value() {
    const int size = sizeof...(digits);
    int arr[size] = {digits...};
//Following four lines conver an array to vector and then remove duplicates.
    //std::vector<int> v(arr, arr+size);
    //sort (v.begin(), v.end(), [](int i, int j) { return i<j;});
    //auto it = std::unique (v.begin(), v.end());
    //v.resize( std::distance(v.begin(),it) );
    int result = 0;
    int mult = 1;
    for (int i=0; i<size; i++) {
        result += arr[i] * mult;
        mult *= 2;
    }
    return result;
}


