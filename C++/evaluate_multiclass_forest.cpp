#include <iostream>
#include <fstream>
#include <random>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

class Node {
  	public:
		VectorXf w;
		std::vector<int> label;
		float entropy=0.0;
		int chosen_d=0;
		std::vector<int> leaf_weight;
		Node* pos=NULL;
		Node* neg=NULL;
	/* Insertion operator
	friend std::ostream& operator<<(std::ostream& os, const Node& n)
	{
		int t=3;
		// write out individual members of s with an end of line between each one
		//os << t << "," << n.label << '\n';
		os << n.label.size();
		for(int i=0;i<n.label.size();i++)
			os << "," << n.label[i];
		os << '\n';

		os << n.entropy << '\n';
		os << n.chosen_d << '\n';
		//os << t << "," << n.leaf_weight << '\n';
		os << n.leaf_weight.size();
		for(int i=0;i<n.leaf_weight.size();i++)
			os  << "," << n.leaf_weight[i];
		os << '\n';		

		os << n.w(0) << '\n';
		os << n.w(1) << '\n';
		if(n.label.empty()){//not a leaf
			os << "p" << '\n';
			os << n->pos;

			os << "n" << '\n';
			os << n->neg;
		}
		return os;
	}*/
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

/*Node::Node(){
	pos = neg = NULL;
	VectorXf w2(2);
	w2(0)=0;
	w2(1)=0;
	w=w2;
}*/

Node *load_node(ifstream* ifs){
	Node *node = new Node();
	VectorXf w2(2);
	w2(0)=0;
	w2(1)=0;
	node->w=w2;
	string line, token;
	//cout << "reading node: " << endl;
	getline (*ifs, line);
	std::istringstream ss(line);
	std::getline(ss, token, ',');
	int m=atoi(token.c_str());
	for(int i=0;i<m;i++){
		std::getline(ss, token, ',');
		node->label.push_back(atoi(token.c_str()));
	}
	getline (*ifs, line);
	node->entropy = atof(line.c_str());
	getline (*ifs, line);
	node->chosen_d = atoi(line.c_str());
	getline (*ifs, line);
	std::istringstream ss_weights(line);
	std::getline(ss_weights, token, ',');
	m=atoi(token.c_str());
	for(int i=0;i<m;i++){
		std::getline(ss_weights, token, ',');
		node->leaf_weight.push_back(atoi(token.c_str()));
	}
	getline (*ifs, line);
	node->w(0) = atof(line.c_str());
	getline (*ifs, line);
	node->w(1) = atof(line.c_str());
	//cout << endl;
	getline (*ifs, line);
	if(line == "p"){//careful, a node must have either 0 or two children!
		//the order is always 1)positive, 2)negative.
		node->pos = load_node(ifs);
		node->neg = load_node(ifs);
		//(Either can have an arbitrary substructure)
	}
	return node;
}

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

MatrixXi eval_tree(Node *curnode, VectorXf datapoint){
	//cout << "evaluating for datapoint: " << datapoint.transpose() << endl << "curnode: " << endl << curnode << endl;
	if(curnode->leaf_weight.size()>0){//is a leaf?
		MatrixXi label_and_weight(curnode->label.size(),2);
		for(int i=0;i<curnode->label.size();i++){
			label_and_weight(i,0)=curnode->label[i];
			label_and_weight(i,1)=curnode->leaf_weight[i];
		}
		return label_and_weight;
	}else{//a decision node
		//recreate relevant X
		VectorXf X_dimension(2);
		X_dimension(0)=1.0;
		X_dimension(1)=datapoint(curnode->chosen_d);

		//cout << X_dimension.transpose() << " times " << curnode.w.transpose() << endl;
		//cout << "that's easy, e is just " << X_dimension.dot(curnode.w) << endl;

		//apply linear classifier
		if(X_dimension.dot(curnode->w)>0.0)//pos
			return eval_tree(curnode->pos,datapoint);
		else//neg
			return eval_tree(curnode->neg,datapoint);
	}
}

//return a vector of unique labels in the leaves of a decision tree
vector<int> get_unique_labels(Node *curnode){
	if(curnode->leaf_weight.size()>0){//leaf node
		vector<int> leaf;
		for(int i=0;i<curnode->label.size();i++)
			leaf.push_back(curnode->label[i]);
		return leaf;
	}else{//decision node
		vector<int> pos, neg;
		pos=get_unique_labels(curnode->pos);
		neg=get_unique_labels(curnode->neg);
		//combine the label vectors. We can assume that pos and neg don't contain duplicates
		for (int i=0;i<pos.size();i++){
			bool in_neg=false;
			for (int j=0;j<neg.size();j++)
				if(pos[i]==neg[j])//already there
					in_neg=true;
			if(!in_neg)//last element
				neg.push_back(pos[i]);
		}
		return neg;
	}
}

int main(int argc, char** argv)
{
	srand(time(NULL)); /* seed random number generator */
	char* filename;
	char* ffilename;
	char* fffilename;
	int num_trees;
	if( argc != 4)
	{
		cout << "\trequires name of .csv file, forest .forest input file, and .classification output" << endl;
		return -1;
	}else{
		filename=argv[1];
		ffilename=argv[2];
		fffilename=argv[3];
	}
	cout << "reading file " << filename << endl;
	//read the file twice. Once to count elements per line and #lines
	ifstream file (filename); 
	string value, line;
	int rows=0;
	int cols=0;
	int incomplete_rows=0;
	getline (file, line);
	while ( file.good() )
	{
		if(std::count(line.begin(), line.end(), '?')>0)
			incomplete_rows++;
		int elements;
		rows++;
		elements=std::count(line.begin(), line.end(), ',');
		//cout << elements << endl;
		if(cols==0)
			cols=elements;
		else if(elements>0 && elements!=cols){
			cout << "bad input file, unequal rows (misplaced comma?)" << endl;
			return -1;
		}
		getline (file, line);
	}
	cols++;
	cout << "\nfile has " << rows << " rows with " << cols << " columns each, including " << incomplete_rows << " incomplete rows" << endl;

	file.clear();
	//and the second time to put everything into a matrix
	MatrixXf csv(rows-incomplete_rows,cols);
	string incomplete[incomplete_rows];
	file.seekg(0, ios::beg);

	std::string token;
	int i=0;
	int incomplete_i=0;
	getline (file, line);
	while ( file.good() )
	{
		std::istringstream ss(line);
		if(std::count(line.begin(), line.end(), '?')>0)//contains '?'
			incomplete[incomplete_i++]=line;
		else{
			for(int j=0;j<cols;j++){
				std::getline(ss, token, ',');
				//std::cout << "(" << j << ", " << i << "):" << token << '\n';
				csv(i,j) = atof(token.c_str());
			}
			++i;
		}
		getline (file, line);
	}
	//std::cout << "X from file is:\n" << csv << endl;
	//std::cout << "Y from file is:\n" << labels << endl;

	std::ifstream ifs(ffilename);
	getline (ifs, line);
	std::istringstream ss(line);
	
	std::getline(ss, token, ' ');
	num_trees = atoi(token.c_str());
	std::vector<Node*> forest;
	cout << "There are " << num_trees << " trees, reading..." << endl;
	for (int i=0;i<num_trees;i++)
		forest.push_back(load_node(&ifs));

	cout << "Loaded forest, number of trees: " << forest.size() << endl;
	//cout << "first tree: " << forest.front() << endl;
	//cout << "second tree: " << forest[1] << endl;

	vector<int> unique_labels = get_unique_labels(forest.front());

	cout << "unique labels: " << endl;
	for (int i=0;i<unique_labels.size();i++)
		cout << unique_labels[i] << " - ";
	cout << endl;

	MatrixXi result_weights(csv.rows(),unique_labels.size());
	for (int i=0;i<result_weights.rows();i++)
		for (int j=0;j<result_weights.cols();j++)
			result_weights(i,j)=0;
	for (int i=0;i<csv.rows();i++){
		VectorXf datapoint(cols);
		for (int j=0;j<cols;j++)
			datapoint(j)=csv(i,j);
		//cout << "evaluating " << num_trees << " trees on point " << i << "/" << csv.rows();
		for (int j=0;j<num_trees;j++){
			MatrixXi tmp_res = eval_tree(forest[j],datapoint);
			//cout << ".";
			//cout.flush();

			//if(tmp_res.rows()>1)
			//	cout << "tree " << j << " and point " << i << ": " << endl << tmp_res << endl;//label: " << tmp_res(0) << ", weight: " << tmp_res(1) << endl;
			for (int k=0;k<unique_labels.size();k++)
				for (int m=0;m<tmp_res.rows();m++)
					if(tmp_res(m,0)==unique_labels[k]){
						result_weights(i,k)=result_weights(i,k)+tmp_res(m,1);
						//skip rest: k=unique_labels.size();
					}
		}
		VectorXi datapoint_res(unique_labels.size());
		for (int j=0;j<unique_labels.size();j++)
			datapoint_res(j)=result_weights(i,j);
		//cout << endl << "point " << i << ", " << datapoint_res.transpose() << endl;
	}
	//cout << "results:" << endl << result_weights << endl;
	
	//save results to in csv format
	ofstream classification(fffilename);
	//print labels, to know which order they are in
	classification << unique_labels[0];
	for (int j=1;j<unique_labels.size();j++)
		classification << "," << unique_labels[j];
	classification << endl;

	for(int i=0;i<result_weights.rows();i++){
		classification << result_weights(i,0);
		for (int j=1;j<result_weights.cols();j++)
			classification << "," << result_weights(i,j);
		classification << endl;
	}
	classification.close();
}
