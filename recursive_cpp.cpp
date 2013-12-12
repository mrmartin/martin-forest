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
		Node* pos=NULL;
		Node* neg=NULL;
};

Node learn_node(){
	Node curnode;
	VectorXf w2(2);
	w2(1)=1;w2(2)=2;
	curnode.w=w2;
	curnode.chosen_d=rand()%10;
	curnode.entropy=(double)rand() / MAX_RAND;
	return curnode;
}

void traverse(Node* n){
	Node node=*n;
	cout << "traversing - ";
	//cout << "+" << n.pos << ", -" << n.neg << endl;//n.entropy << endl;
	if(node.pos!=NULL)
		traverse(node.pos);
	if(node.neg!=NULL)
		traverse(node.neg);
}

Node learn_me(){
	Node parent;
	if(rand()%2==1){
		negrows=rand()%3;
		posrows=rand()%3;
		if(negrows==0)
			parent=learn_me();
		else if(posrows==0)
			parent=learn_me();
		else{
			Node pos=learn_me();
			parent.pos=&pos;

			Node neg=learn_me();
			parent.neg=&neg;
		}
		break;//stop for loop
	}else{
		parent.label = rand()%10;
		parent.entropy=0;
		//cout << "created leaf with label " << parent.label << endl;
	}

	if(parent.pos!=NULL && parent.neg!=NULL){
		cout << "returning node with offspring " << endl;
		traverse(&parent);
		cout << endl;		
	}
	else
		cout << "returning leaf with label " << parent.label << endl;
	return parent;
}

int main(int argc, char** argv)
{
	Node root;
	root = learn_me(csv,labels,unique_labels);

	cout << "learning complete" << endl;
	//traverse(root);
	cout << "nothing" << endl;
}
