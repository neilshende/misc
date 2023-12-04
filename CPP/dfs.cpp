#include <iostream>
#include <vector>
#include <stack>

using namespace std;

void dfs(int node, vector<vector<int> >& graph, vector<bool>& visited) {
    visited[node] = true;
    cout << node << " ";

    for (int neighbor : graph[node]) {
        if (!visited[neighbor]) {
            dfs(neighbor, graph, visited);
        }
    }
}

int main() {
    int nodes, edges;
    cin >> nodes >> edges;

    vector<vector<int> > graph(nodes);

    for (int i = 0; i < edges; i++) {
        int u, v;
        cin >> u >> v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    vector<bool> visited(nodes, false);

    for (int i = 0; i < nodes; i++) {
        if (!visited[i]) {
            dfs(i, graph, visited);
        }
    }

    return 0;
}
