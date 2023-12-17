#include <iostream>
#include <vector>
#include <queue>

using namespace std;

// Function to represent a node in the graph
struct Node {
  int id;
  vector<int> neighbors;
};

// Function to perform BFS on a graph
void bfs(const vector<Node>& graph, int start_node) {
  // Create a queue to store the nodes to be visited
  queue<int> q;

  // Mark all nodes as unvisited initially
  vector<bool> visited(graph.size(), false);

  // Push the starting node to the queue and mark it as visited
  q.push(start_node);
  visited[start_node] = true;

  // Loop until the queue is empty
  while (!q.empty()) {
    // Get the current node from the queue
    int current_node = q.front();
    q.pop();

    // Print the current node
    cout << "Visited node: " << current_node << endl;

    // Loop through all the neighbors of the current node
    for (int neighbor : graph[current_node].neighbors) {
      // If the neighbor is not visited, add it to the queue and mark it as visited
      if (!visited[neighbor]) {
        q.push(neighbor);
        visited[neighbor] = true;
      }
    }
  }
}

int main() {
  // Example graph with adjacency list representation
  vector<Node> graph = {
    {0, {1, 2}},
    {1, {3, 4}},
    {2, {5}},
    {3, {}},
    {4, {}},
    {5, {}}
  };

  // Starting node for the BFS
  int start_node = 0;

  // Perform BFS on the graph
  bfs(graph, start_node);

  return 0;
}
