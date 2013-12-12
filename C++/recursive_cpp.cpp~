#include <iostream>
#include <fstream>
#include <random>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

class Node {
  	public:
		VectorXf w;
		int chosen_d;
		float entropy;
		int label;
		
		Node* pos=NULL;
		Node* neg=NULL;
Node()
{
pos = neg = NULL;
}
~Node()
{
if(pos) delete pos;
if(neg) delete neg;
}

protected:
Node(const Node &n); // copying proibited
};

void traverse(Node* n){
	cout << "trying to get node at " << n << endl;
	Node &node=*n;
	//cout << "traversing - ";
	cout << "ok. My offspring hang out at +" << (*n).pos << ", and -" << (*n).neg << endl;//n.entropy << endl;
	if(node.pos!=NULL){
		cout << "node.pos!=NULL" << endl;
		traverse(node.pos);
	}if(node.neg!=NULL)
		traverse(node.neg);
}

Node *learn_node(){
	Node *curnode = new Node();
	VectorXf w2(2);
	w2(1)=1;w2(0)=2;
	curnode->w=w2;
	curnode->chosen_d=rand()%10;
	curnode->entropy=(double)rand() / RAND_MAX;
	return curnode;
}

Node *learn_tree(){
	Node* parent = NULL;
	if(rand()%2==1){
		parent = learn_node();
		int negrows=rand()%3;
		int posrows=rand()%3;
		if(negrows==0) {
			delete parent;
			parent=learn_tree();
		} else if(posrows==0) {
			delete parent;
			parent=learn_tree();
		}else{
			//parent = new Node();
			Node *pos=learn_tree();
			parent->pos=pos;

			Node *neg=learn_tree();
			parent->neg=neg;
		}
	}else{
		parent = new Node();
		parent->label = rand()%10;
		parent->entropy=0;
		//cout << "created leaf with label " << parent.label << endl;
	}

	if(parent->pos!=NULL && parent->neg!=NULL){
		cout << "returning node with offspring " << endl;
		traverse(parent);
		cout << endl;		
	}
	else
		cout << "returning leaf with label " << parent->label << endl;
	return parent;
}

int main(int argc, char** argv)
{
	srand(0);
	srand(time(NULL)); /* seed random number generator */
	Node *root;
	root = learn_tree();

	cout << "learning complete" << endl;
	//traverse(root);
	cout << "nothing" << endl;
	delete root;
}
