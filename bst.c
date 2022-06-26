#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <iostream>
using namespace std;

int stack[1000];

int *sp = stack;

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))

typedef enum ORDER
{
   PREORDER = -1,
   INORDER = 0,
   POSTORDER = 1
} ORDER;
typedef struct node
{
   struct node *left;
   struct node *right;
   int value;
} node;
node *merge(node *head, node *subhead);

bool isHeap(node *head)
{
   if (head == NULL) return true;
   if (head->left == NULL && head->right == NULL) return true;
   if (head->left && head->right) return head->value < head->right->value
                                         && head->value < head->left->value
                                         && isHeap(head->right)
                                         && isHeap(head->left);
   if (head->left == NULL) return (head->value < head->right->value) && isHeap(head->right);
   if (head->right == NULL) return (head->value < head->left->value) && isHeap(head->left);
   // can't reach here
   return false;
}
bool isBST(node *head)
{
   if (head == NULL) return true;
   if (head->left == NULL && head->right == NULL) return true;
   if (head->left && head->right) return head->value < head->right->value
                                         && head->value > head->left->value
                                         && isBST(head->right)
                                         && isBST(head->left);
   if (head->left == NULL) return (head->value < head->right->value) && isBST(head->right);
   if (head->right == NULL) return (head->value > head->left->value) && isBST(head->left);
   // can't reach here
   return false;
}
int depth(node *head) 
{
//depth of tree. == depth of deepest node.
   if (head==NULL) return 0;
   if (head->left==NULL && head->right==NULL) return 1;
   int l = depth(head->left);
   int r = depth(head->right);
   return (l>r?l:r)+1;
}
int shallow_depth(node *head)
{
//depth of shallowest leaf
//same as above except change 0 to MAXINT and max to min.
   if (head==NULL) return 0;//??100000;
   if (head->left==NULL && head->right==NULL) return 1;
   int l = shallow_depth(head->left);
   int r = shallow_depth(head->right);
   return (l<r?l:r)+1;
}

bool isAVL(node *head)
{
  if (head==NULL) return true;
  return (depth(head->left)-depth(head->right) <= 1);
}

int size(node *node)
{
   if (node==NULL) return 0;
   int r = size(node->right);
   int l = size(node->left);
   return r+l+1;
}

int balance(node *node)
{
   if (node==NULL) return 0;
   int r = size(node->right);
   int l = size(node->left);
   return r-l;
}

int bint[15] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

node *bal_insert(int *a, int start, int end)
{
   if (start>end) return NULL;
   node *n= (node *)malloc(sizeof(node));
   int mid = (start+end)/2;
   n->value=a[mid];
//   n->left = n->right = NULL;
   n->left=bal_insert(a, start, mid-1);
   n->right=bal_insert(a, mid+1, end);
   return n;
}

int least(struct node* head)
{
   if (head->left == NULL) return head->value;
   return (least(head->left));
}

//inorder print
void printInorder(struct node* node)
{
    if (node == NULL)
        return;
    printInorder(node->left);
    cout <<"  "<< node->value;
    printInorder(node->right);
}

//Lowest Common Ancestor
node * LCA(node *head, node *a, node *b)
{
    if (head == NULL) return NULL;
    if (head->value > a->value && head->value > b->value) {
       return LCA(head->left, a, b);
    } else if (head->value < a->value && head->value < b->value) {
       return LCA(head->right, a, b);
    } else {
       return head;
    }
}
node * rotateright(node *oldroot)
{
   node *newroot = oldroot->left;
   oldroot->left = newroot->right;
   newroot->right = oldroot;
   return newroot;
}
node * deletenode(node *x)
{
/* responsibility of the caller to hang the returned pointer to correct node.
 * e.g. y->left = deletenode (y->left);
 */
   if (x->left == NULL && x->right == NULL) {
      free(x);
      return NULL;
   }
   if (x->right == NULL) {
       node *xl =x->left;
       free(x);
       return xl;
   }
   if (x->left == NULL) {
      node *xr = x->right;
      free(x);
      return xr;
   }
   {
#if 0
       node *xm = merge(x->left, x->right);
       free(x);
       return xm;
#endif
      x->value = x->left->value;
      x->left = deletenode(x->left);
      return x;
   }
}

node *findu(node *head, int value)
{
// tree not assumed to be search tree.
   if (head == NULL) return NULL;
   if (head->value == value) return head;
   node *nl = findu(head->left, value);
   if (nl!=NULL) return nl;
   return findu(head->right, value);
}
       
node * find(node * head, int value)
{
   if (head == NULL) return NULL;
   if (head->value == value) return head;
   if (head->value > value) {
       return find(head->left, value);
   } else {
       return find(head->right, value);
   }
}
node *insert(node *head, int value)
{
   if (head == NULL) {
      node * n= (node *)malloc(sizeof(node));
      n->value = value;
      n->left = n->right = NULL;
      return n;
   }
   if (head->value == value) {
       return head;
   }
   if (head->value > value) {
      head->left=insert(head->left, value);
   } else {
      head->right=insert(head->right, value);
   }
   return head;
}
node *insert(node *head, node *newnode)
{
   if (head == NULL) return newnode;
   if (head->value == newnode->value) {
       if (head != newnode) {
          free(newnode);
          printf("Duplicate value. Freeing newnode.\n");
       } else {
          printf("Node already in BST.\n");
       }
       return head;
   }
   if (head->value > newnode->value) {
      head->left=insert(head->left, newnode);
   } else {
      head->right=insert(head->right, newnode);
   }
   return head;
}
node *merge(node *head, node *subhead)
{
    node *finder;
    if (head == NULL) return subhead;
    if (subhead == NULL) return head;
    finder = find(head, subhead->value);
    if (finder==subhead) {
       printf("Subhead already branch of head.\n");
       return head;
    }
    if (subhead->left) {
        head = merge(head, subhead->left);
        subhead->left = NULL;
    }
    if (subhead->right) {
       head = merge(head, subhead->right);
       subhead->right = NULL;
    }
    return insert(head, subhead);
}

void printbst(node *head, ORDER o)
{
    if (head==NULL) return;
    switch (o) {
    case PREORDER:
       printf("%d\n", head->value);
       printbst(head->left, o);
       printbst(head->right, o);
       break;

    case INORDER:
       printbst(head->left, o);
       printf("%d\n", head->value);
       printbst(head->right, o);
       break;

    case POSTORDER:
       printbst(head->left, o);
       printbst(head->right, o);
       printf("%d\n", head->value);
       break;
    }
}
int path(node *head, int val) 
{
   if (head ==  NULL) {push(sp, 0); return 0;}
   push(sp, head->value);
   if (head->value == val) return 1;
   if (path(head->left, val)) { return 1; } else { (void)pop(sp); }
   if (path(head->right, val)) { return 1; } else { (void)pop(sp); }
   return 0;
}
int main(void)
{
   node *head = NULL;
   node *subhead = NULL;
   node *a, *b, *c;
   int i;
   for(i=10; i>0; i--) {
      node *newnode = (node *)malloc(sizeof(node));
      newnode->value= i;
      newnode->left = newnode->right = NULL;
      head = insert(head, newnode);
   }
   printf("printing the tree\n");
   printbst(head, INORDER);
   printf("Find 4 %p\n", a=find(head, 4));
   for (i=0; i<4; i++) {
      head = rotateright(head);
   }
   printf("Find 8 %p\n", b=find(head, 8));
   printf("printing the tree after rotate\n");
   printbst(head, PREORDER);

   c = LCA(head, a, b);
   printf("Lowest Common Ancestor in BST. LCA->val = %d\n", c->value);

   for (i=100; i>80; i--) {
      node *newnode = (node *)malloc(sizeof(node));
      newnode->value= i;
      newnode->left = newnode->right = NULL;
      subhead = insert(subhead, newnode);
   }
   head = merge(head, subhead);
   printf("printing the tree after merge\n");
   printbst(head, POSTORDER);

   printf("mergine with child branch, should emit lot of errors.\n");
   head = merge(head, c);
   printf("printing again.\n");
   printbst(head, POSTORDER);
   a = find(head, 95);
   printf("Find 95 %p.\n", a);
   printf("adding duplicate node.\n");
   b = (node *)malloc(sizeof(node));
   b->value = 95;
   b->left = b->right = NULL;
   head = insert(head, b);

   node *nn = bal_insert(bint, 0, 14);

   printf("Balanced\n");
   printbst(nn, INORDER);


   printf("PATH\n");
   if (!path(nn, 1)) {printf("not FOUND\n");} else {
   while (sp>stack) printf("%d ",pop(sp));
   printf("\n");
   }

   cout << "\nprint inOrder\n";
   printInorder(head);

   cout << "\nLeast is " << least(head) << endl;
   cout << "isBST is " << (isBST(head) ? "true" : "false") << endl;
   cout << "isAVL is " << (isAVL(head) ? "true" : "false") << endl;
   return 0;
}
