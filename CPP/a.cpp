#include <vector>
#include <iostream>
#include <stdio.h>
double findMedianSortedArrays(std::vector<int>& nums1, std::vector<int>& nums2) {
    int m = nums1.size();
    int n = nums2.size();
    int i = 0;
    int j = 0;
    std::vector<int> merged;
    while(i<m && j<n) {
        if (nums1[i] < nums2[j]) {
            if (i<m) merged.push_back(nums1[i]);
        } else if (nums1[i] == nums2[j]) {
            if (i<m) merged.push_back(nums1[i++]);
            if (j<n) merged.push_back(nums2[j++]);
        } else {
            if (j<n) merged.push_back(nums2[j++]);
        }
    }
    for (; i<m; i++) merged.push_back(nums1[i]);
    for (; j<n; j++) merged.push_back(nums2[j]);
    return merged[(m+n)/2];
    }
int main() {
std::vector<int> x{1, 2, 5 , 7, 9};
std::vector<int> y{3,4,5,6,7,8,9,10,11};
std::cout << findMedianSortedArrays(x,y) <<std::endl;
return 0;
}
