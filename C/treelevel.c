	void levelOrder(Node * root){
      //Write your code here
  	    queue<Node *> s;
        if (root) s.push(root);
        while (!s.empty()) {
            Node *n = s.front();
            s.pop();
            cout << (n->data) << " ";
            if (n->left) s.push(n->left);
            if (n->right) s.push(n->right);
        }
  
	}
