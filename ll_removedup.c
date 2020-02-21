          Node* removeDuplicates(Node *head)
          {
            //Write your code here
              Node *h = head;
              Node *p = head;
              if (h==NULL) return h;
              int val = h->data;
              h=h->next;
              while (h)
                  {
                  Node *n= h->next;
                  if (h->data == val) {//head=remove(head, h);
                      p->next=h->next;
                      delete h;
                  } else {p=h;}
                  //p=h
                  h=n;
              }
              if (head->next) head->next = removeDuplicates(head->next);
              return head;
          }
Node *remove(Node *head, Node *n) 
    {
    if (head==NULL) return NULL;
    Node *next= head->next;
    if (n==head) {
        delete n;
        return next;
    }
    head->next = remove(next, n);
    return head;
}
