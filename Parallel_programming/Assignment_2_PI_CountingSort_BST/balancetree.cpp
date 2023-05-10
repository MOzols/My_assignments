#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <omp.h>
#include <iostream>

struct bstnode {
	 bstnode(int key) : key(key) { left = right = nullptr; }
	~bstnode() { delete left; delete right; }
	int key;
	bstnode *left, *right;
};

inline int max(int a, int b) { return a>b ? a : b; }

//returns the height of the tree
int height(bstnode* root) {
	if(!root) return 0;
	int lefth,righth;
	lefth = height(root->left);
	righth = height(root->right);
	return max(lefth,righth)+1;
}

//returns true if the tree is height-balanced; false otherwise
bool isbalanced(bstnode* root) {
	if(!root) return true;
	int lefth,righth;
	lefth = height(root->left);
	righth = height(root->right);
	return abs(lefth-righth)<2 && isbalanced(root->left) && isbalanced(root->right);
}
//with thread pool parallel variant of isbalanced
bool isbalanced_p(bstnode* root){
    if(!root) return true;
    int lefth, righth;
    //Creating pool of threads which will access height function
    //in height function separate task is not created because in this function
    //created pool will work in parallel calling height function.
    //No reason to create task in height because it will not give more threads
    //beside already existing ones.
    #pragma omp task
    {
        lefth = height(root->left);
        righth = height(root->right);
    }
    //call taskwait so that result is created in right order
    #pragma omp taskwait
    return abs(lefth-righth)<2 && isbalanced_p(root->left) && isbalanced_p(root->right);
}

//return a pointer to a balanced BST of which keys are in [lower,upper]
bstnode* buildbalanced(int lower, int upper) {
  if (upper<lower) return nullptr;
  int key = (upper+lower)/2;
  bstnode* root = new bstnode(key);
  root->left = buildbalanced(lower, key-1);
  root->right = buildbalanced(key+1, upper);
  return root;
}

#define power_of_two(exp) (1<<(exp))

int main() {
	int nlevels = 12;
	bstnode *root = buildbalanced(1, power_of_two(nlevels)-1);

	printf("\nisbalanced(root) = %c", isbalanced(root)?'Y':'N');

	//creating parallelism and then single so that only one thread calls isbalanced_p
	#pragma omp parallel
	#pragma omp single
    printf("\nisbalanced_p(root) = %c\n", isbalanced_p(root)?'Y':'N');

	delete root;
	return EXIT_SUCCESS;
}
