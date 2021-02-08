#include <iostream>       // std::cout
#include <stack>          // std::stack
#include <vector>         // std::vector
#include <deque>          // std::deque
#define max(a, b) (a>b ? a : b)
#define min(a, b) (a<b ? a : b)
typedef struct node
{
   struct node *left;
   struct node *right;
   int value;
} node;

int tree_depth(node *head)
{
   int dl,dr;
    if (head== NULL) return 0;
    dl = tree_depth(head->left);
    dr = tree_depth(head->right);
    return (max(dr, dl)+1);
}
int shallowest_node(node *head)
{
   int dl,dr;
   if (head == NULL) return 0;
   if (head->left == NULL && head->right == NULL) return 1;
   if (head->left == NULL ) return 1+shallowest_node(head->right);
   if (head->right == NULL) return 1+shallowest_node(head->left);
    dl = shallowest_node(head->left);
    dr = shallowest_node(head->right);
    return (min(dl, dr)+1);
}
void BFS(node *head)
{
  std::deque<node *> q;
  if (head == NULL) goto out;
  q.push_front(head);
  while (!q.empty()) {
    node *x=q.back();
    q.pop_back();
    std::cout << x->value << "  " ;
    if (x->left) q.push_front(x->left);
    if (x->right) q.push_front(x->right);
  }
out:
  std::cout << '\n';
}
int main ()
{

  node *head=(node *)malloc(sizeof(node));
  head->value=1;
  head->left=(node *)malloc(sizeof(node));
  head->left->value=2;
  head->right=(node *)malloc(sizeof(node));
  head->right->value=3;
  head->left->left=(node *)malloc(sizeof(node));
  head->left->right=(node *)malloc(sizeof(node));
  head->right->left=(node *)malloc(sizeof(node));
  head->right->right=(node *)malloc(sizeof(node));
  head->left->left->left=NULL;
  head->left->left->right=NULL;
  head->left->right->left=NULL;
  head->left->right->right=NULL;
  head->right->left->left=NULL;
  head->right->left->right=NULL;
  head->right->right->left=NULL;
  head->right->right->right=NULL;
  head->left->left->value=4;
  head->left->right->value=5;
  head->right->left->value=6;
  head->right->right->value=7;

  BFS(head);

  std::stack<node *> s;
  s.push(head);
  while (!s.empty()) {
      node *x=s.top();
      s.pop();
      std::cout << x->value << "  " ;
      if (x->left) s.push(x->left);
      if (x->right) s.push(x->right);
  }
  std::cout << '\n';

  return 0;
}

