#include <iostream>
using namespace std;

// Data structure to store a binary tree node
struct Node
{
    int key;
    Node *left, *right;

    Node(int key)
    {
        this->key = key;
        this->left = this->right = nullptr;
    }
};

// Recursive function to print the left view of a given binary tree
void leftView(Node* root, int level, int &last_level)
{
    // base case: empty tree
    if (root == nullptr) {
        return;
    }

    // if the current node is the first node of the current level
    if (last_level < level)
    {
        // print the node's data
        cout << root->key << " ";

        // update the last level to the current level
        last_level = level;
    }

    // recur for the left and right subtree by increasing the level by 1
    leftView(root->left, level + 1, last_level);
    leftView(root->right, level + 1, last_level);
}

// Function to print the left view of a given binary tree
void leftViewRecursive(Node* root)
{
    int last_level = 0;
    leftView(root, 1, last_level);
}

// Iterative function to print the left view of a given binary tree
void leftViewIterative(Node* root)
{
    // return if the tree is empty
    if (root == nullptr) {
        return;
    }

    // create an empty queue and enqueue the root node
    list<Node*> queue;
    queue.push_back(root);

    // pointer to store the current node
    Node* curr = nullptr;

    // loop till queue is empty
    while (!queue.empty())
    {
        // calculate the total number of nodes at the current level
        int size = queue.size();
        int i = 0;

        // process every node of the current level and enqueue their
        // non-empty left and right child
        while (i++ < size)
        {
            curr = queue.front();
            queue.pop_front();

            // if this is the first node of the current level, print it
            if (i == 1) {
                cout << curr->key << " ";
            }

            if (curr->left) {
                queue.push_back(curr->left);
            }

            if (curr->right) {
                queue.push_back(curr->right);
            }
        }
    }
}
