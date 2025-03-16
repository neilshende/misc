struct Graph {
    int value;
    std::list<Graph*> neighbors;
};

void bfs(Graph* startNode) {
    if (startNode == nullptr) {
        return;
    }

    std::queue<Graph*> q;
    std::unordered_set<Graph*> visited;

    q.push(startNode);
    visited.insert(startNode);

    while (!q.empty()) {
        Graph* current = q.front();
        q.pop();

        std::cout << current->value << " ";

        for (Graph* neighbor : current->neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                q.push(neighbor);
                visited.insert(neighbor);
            }
        }
    }
}

void DFS(Graph* startNode) {
    if (startNode == nullptr) {
        return;
    }

    std::stack<Graph*> s;
    std::unordered_set<Graph*> visited;

    s.push(startNode);
    visited.insert(startNode);

    while (!s.empty()) {
        Graph* current = s.top();
        s.pop();

        std::cout << current->value << " ";

        for (Graph* neighbor : current->neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                s.push(neighbor);
                visited.insert(neighbor);
            }
        }
    }
}
