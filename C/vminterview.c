// This is the text editor interface. 
// Anything you type or change here will be seen by the other person in real time.
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    struct node *left;
    struct node *right;
    int data;
} node;

node *new_node(int d) {
  node *ret = (node *)malloc(sizeof(node));
  ret->data = d;
  ret->left = ret->right = NULL;
  return ret;
}
int array[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

node *BSTinsert(node *head, node *x) 
{
    if (head == NULL) {
        return x;
    }
    if (head->data == x->data) { return head;}
    if (head->data < x->data) {
        if (head->left == NULL) {
            head->left = x;
            return head;
        }
        head->left = BSTinsert(head->left, x);
    }
    if (head->data > x->data) {
        if (head->right == NULL) {
            head->right = x;
            return head;
        }

        head->right = BSTinsert(head->right, x);
    }
    return head;
}

void bal_insert(int *a, node *head, int start, int end) 
{
    if (start==end) {
         head->data = a[start];
         return;
    }
    int mid= (start+end)/2;
    node *n = (node *)malloc(sizeof(node));
    n->left = n->right = NULL;
    head->left =n;
    n = (node *)malloc(sizeof(node));
    n->left = n->right = NULL; 
    head->right =n;
    head->data = a[mid];

    head->data = a[mid];
    if (start < mid ) bal_insert(a, head->left, start, mid-1);
    if (mid < end) bal_insert(a, head->right, mid+1, end);
    return;
    
}

int main() 
{

    
    node head;
    node *h;
    bal_insert(array, &head, 0, 15);
    
    h = BSTinsert(&head, new_node(-1));
    h = BSTinsert(&head, new_node(18));
    h = BSTinsert(&head, new_node(10));
    h = BSTinsert(&head, new_node(-10));
     
    return 0;
}
