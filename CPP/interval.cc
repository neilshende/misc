#include <iostream>
#include <vector>
#include <algorithm>
using Interval = std::pair<int, int>;
std::vector<Interval> mergeIntervals(std::vector<Interval> &intervals) {
    std::vector<Interval> mergedIntervals;

    // Sort intervals by their start values
    std::sort(intervals.begin(), intervals.end(), [](Interval a, Interval b){return a.first < b.first;});

    // Merge overlapping intervals
    mergedIntervals.push_back(intervals[0]);
    for (int i = 1; i < intervals.size(); ++i) {
        Interval& last = mergedIntervals.back();
        Interval& curr = intervals[i];

        if (curr.first <= last.second) { // Overlapping intervals
            last.second = std::max(last.second, curr.second);
        } else {
            mergedIntervals.push_back(curr);
        }
    }

    return mergedIntervals;
}
int main() {
    std::vector<Interval> intervals = {{1, 7}, {2, 6}, {8, 10}, {9, 18}};

    std::vector<Interval> merged = mergeIntervals(intervals);

    for (const Interval& interval : merged) {
        std::cout << "[" << interval.first << ", " << interval.second << "]" << std::endl;
    }

    return 0;
}
