#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

// Basic type definitions:

enum color_t { BLACK, RED };

struct RBnode {     // node of red–black tree
  RBnode* parent;   // == NULL if root of the tree
  RBnode* child[2]; // == NIL if child is empty
    // Index is:
    //   LEFT  := 0, if (key < parent->key)
    //   RIGHT := 1, if (key > parent->key)
  enum color_t color;
  int key;
};

#define NIL   NULL // null pointer  or  pointer to sentinel node
#define LEFT  0
#define RIGHT 1
#define left  child[LEFT]
#define right child[RIGHT]

struct RBtree { // red–black tree
  RBnode* root; // == NIL if tree is empty
};

// Get the child direction (∈ { LEFT, RIGHT })
//   of the non-root non-NIL  RBnode* N:
#define childDir(N) ( N == (N->parent)->right ? RIGHT : LEFT )

RBnode* RotateDirRoot(
    RBtree* T,   // red–black tree
    RBnode* P,   // root of subtree (may be the root of T)
    int dir) {   // dir ∈ { LEFT, RIGHT }
  RBnode* G = P->parent;
  RBnode* S = P->child[1-dir];
  RBnode* C;
  assert(S != NIL); // pointer to true node required
  C = S->child[dir];
  P->child[1-dir] = C; if (C != NIL) C->parent = P;
  S->child[  dir] = P; P->parent = S;
  S->parent = G;
  if (G != NULL)
    G->child[ P == G->right ? RIGHT : LEFT ] = S;
  else
    T->root = S;
  return S; // new root of subtree
}

#define RotateDir(N,dir) RotateDirRoot(T,N,dir)
#define RotateLeft(N)    RotateDirRoot(T,N,LEFT)
#define RotateRight(N)   RotateDirRoot(T,N,RIGHT)

int main() {
return 0;
}
