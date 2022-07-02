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
void leftView(Node* root)
{
    int last_level = 0;
    leftView(root, 1, last_level);
}
