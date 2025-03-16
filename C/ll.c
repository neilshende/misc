#include <stdlib.h>
#include <stdio.h>

typedef struct node {
int data;
struct node * next;
} node;
int iscyclic(node *head)
{
   node *tortoise = head;
   node *hare = head;
   while (hare != NULL && hare->next != NULL) {
      tortoise = tortoise->next;
      hare = hare->next->next;
      if (hare == tortoise || hare->next == tortoise) {
         printf("Loop detected.\n");
         return 1;
      }
   }
   printf("There is no loop.\n");
   return 0;
}
node * llremove(node * head, node * what) {
   if (what == NULL) return head;
   if (head == NULL) return head;
   if (head == what) {
      node * next = head->next;
      free(head);
      return next;
   }
   head->next = llremove(head->next, what);
   return head;
}
node * find(node *head, int data) {
   node * jj = head;
   while (jj != NULL) {
      if (jj->data==data) return jj;
      jj = jj->next;
   }
   return NULL;
}
node * insert(node *head, node *where, node *what) {
   if (head == NULL) return head;
   if (head == where) {
      what->next = where;
      return what;
   }
   head->next = insert(head->next, where, what);
   return head;
}
void insertAfter(node *where, node *what) {
   what->next = where->next;
   where->next = what;
}
node * nthfromlast(node *head, int n)
{
   int i;
   node *a, *b;
   a = head;
   if (head == NULL) return NULL;
   for (i = 0; i < n; i++) {
       if (a->next != NULL) a=a->next; else return NULL;
   }
   b = head;
   while (a != NULL) {
       a= a->next;
       b= b->next;
   }
   return b;
}
node *reverse(node *head)
{
    node* prev   = NULL;
    node* current = head;
    node* next;
    while (current != NULL)
    {
        next  = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}
node * recreverse(node *curr, node *prev)
{
    if (curr == NULL) return NULL:
    if (curr->next == NULL) {
      curr->next = prev;
      return curr;
    }
    {
       node *t = recreverse(curr->next, curr);
       curr->next = prev;
       return t;
    }
}
int main()
{
int i;
node * head = malloc(sizeof(node));
head->data = 911;
head->next = NULL;
node * last = head;
for (i = 0; i <100; i++) {
   node * ii = malloc(sizeof(node));
   ii->data = i;
   head = insert(head, last, ii);
//   last = ii;
}//for
for (i = 90; i < 110; i++) {
   head = llremove(head, find(head, i));
}
head = recreverse(head, NULL); //or reverse(head)
node * jj = head;
while (jj != NULL) {
   printf("%d \n", jj->data);
   jj=jj->next;
}//while
   return iscyclic(head); 
}//main

