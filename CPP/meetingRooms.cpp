#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

int minMeetingRooms(std::vector<std::pair<int, int>> &meetings) {
    if (meetings.empty()) return 0;

    // Sort meetings by start time
    std::sort(meetings.begin(), meetings.end());

    // Min-heap to track end times
    std::priority_queue<int, std::vector<int>, std::greater<>> minHeap;

    // Push the end time of the first meeting
    minHeap.push(meetings[0].second);

    for (size_t i = 1; i < meetings.size(); ++i) {
        // If the current meeting starts after or at the earliest ended one, reuse the room
        if (meetings[i].first >= minHeap.top()) {
            minHeap.pop();
        }

        // Push current meeting's end time
        minHeap.push(meetings[i].second);
    }

    // The size of the heap is the number of rooms needed
    return minHeap.size();
}

int main() {
    std::vector<std::pair<int, int>> meetings = {
        {0, 30}, {5, 10}, {15, 20}, {25, 35}
    };

    std::cout << "Minimum rooms needed: " << minMeetingRooms(meetings) << std::endl;
    return 0;
}
