/* Hidden stub code will pass a root argument to the function below. Complete the function to solve the challenge. Hint: you may want to write one or more helper functions.  

The Node struct is defined as follows:
	struct Node {
		int data;
		Node* left;
		Node* right;
	}
*/
bool checkLeft(Node* root, int val) {
        if (root==NULL) return true;
        if (root->data >= val) return false;
        return (checkLeft(root->left, root->data) && checkLeft(root->right, val/*root->data*/));
    }
bool checkRight(Node* root, int val) {
        if (root==NULL) return true;
        if (root->data <= val) return false;
        return (checkRight(root->left, val/*root->data*/) && checkRight(root->right, root->data));
    }
bool checkBST(Node* root) {
        if (root == NULL ) return true;
        if (!(checkLeft(root->left, root->data))) return false;
        if (!(checkRight(root->right, root->data))) return false;
        //return true;
        return (checkBST(root->left) && checkBST(root->right) );		
}
