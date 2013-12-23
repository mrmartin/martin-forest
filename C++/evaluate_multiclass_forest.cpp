#include <iostream>
#include <fstream>
#include <random>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

static int num_labels=0;

class Node {
  	public:
		float w;
		std::vector<int> label;
		float entropy=0.0;
		int chosen_d=0;
		int weight=0;
		std::vector<int> leaf_weight;
		Node* pos=NULL;
		Node* neg=NULL;
	//Insertion operator
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
		os << n.weight << '\n';
		//os << t << "," << n.leaf_weight << '\n';
		os << n.leaf_weight.size();
		for(int i=0;i<n.leaf_weight.size();i++)
			os  << "," << n.leaf_weight[i];
		os << '\n';		

		os << n.w << '\n';
		if(n.label.empty()){//not a leaf
			os << "p" << '\n';
			os << *(n.pos);

			os << "n" << '\n';
			os << *(n.neg);
		}
		return os;
	}
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

Node *load_node(ifstream* ifs){
	Node *node = new Node();
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
	node->weight = atoi(line.c_str());
	getline (*ifs, line);
	std::istringstream ss_weights(line);
	std::getline(ss_weights, token, ',');
	m=atoi(token.c_str());
	for(int i=0;i<m;i++){
		std::getline(ss_weights, token, ',');
		node->leaf_weight.push_back(atoi(token.c_str()));
	}
	getline (*ifs, line);
	node->w = atof(line.c_str());
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
	//cout << "evaluating for datapoint: " << datapoint.transpose() << endl << "curnode->w: " << curnode->w << " curnode->d: " << curnode->chosen_d << endl;
	if(curnode->leaf_weight.size()>0){//is a leaf?
		MatrixXi label_and_weight(curnode->label.size(),2);
		for(int i=0;i<curnode->label.size();i++){
			label_and_weight(i,0)=curnode->label[i];
			label_and_weight(i,1)=curnode->leaf_weight[i];
		}
		return label_and_weight;
	}else{//a decision node
		//apply linear classifier
		if(datapoint(curnode->chosen_d)>curnode->w)//pos
			return eval_tree(curnode->pos,datapoint);
		else//neg
			return eval_tree(curnode->neg,datapoint);
	}
}

//evaluate for datapoint with some unknowns
MatrixXi eval_tree(Node *curnode, VectorXf datapoint, bool known[]){
	//cout << "evaluating for datapoint: " << datapoint.transpose() << endl << "curnode->w: " << curnode->w << " curnode->d: " << curnode->chosen_d << endl;
	if(curnode->leaf_weight.size()>0){//is a leaf?
		MatrixXi label_and_weight(curnode->label.size(),2);
		for(int i=0;i<curnode->label.size();i++){
			label_and_weight(i,0)=curnode->label[i];
			label_and_weight(i,1)=curnode->leaf_weight[i];
		}
		return label_and_weight;
	}else{//a decision node
		if(known[curnode->chosen_d]){
			//apply linear classifier
			if(datapoint(curnode->chosen_d)>curnode->w)//pos
				return eval_tree(curnode->pos,datapoint,known);
			else//neg
				return eval_tree(curnode->neg,datapoint,known);
		}else{
			//don't know which, so combine the two
			MatrixXi pos=eval_tree(curnode->pos,datapoint,known);
			MatrixXi neg=eval_tree(curnode->neg,datapoint,known);
			//cout << "at unknown " << curnode->chosen_d << "(weight " << curnode->weight << "), entropy is " << curnode->entropy << " and could go to " << curnode->pos->entropy << "(weight " << curnode->pos->weight << ") or " << curnode->neg->entropy << "(weight " << curnode->neg->weight << ")." << endl;
			//cout << "pos:" << endl << pos << endl << "neg:" << endl << neg << endl;
			//cout << "mean weighted child entropy is " << ((curnode->pos->entropy*curnode->pos->weight)+(curnode->neg->entropy*curnode->neg->weight))/curnode->weight << endl;
			cout << "if I knew " << curnode->chosen_d << " the entropy would (on average) go down by " << (curnode->entropy-((curnode->pos->entropy*curnode->pos->weight)+(curnode->neg->entropy*curnode->neg->weight))/curnode->weight) << endl;

			int unique_labels=pos.rows()+neg.rows();
			//assume that pos and neg both don't contain duplicates
			for(int i=0;i<pos.rows();i++){
				for (int j=0;j<neg.rows();j++){
					if(pos(i,0)==neg(j,0))
						unique_labels--;
				}
			}
			MatrixXi combined(unique_labels,2);
			for(int i=0;i<pos.rows();i++){
				combined(i,0)=pos(i,0);
				combined(i,1)=pos(i,1);
			}
			//cout << " combined " << endl << combined << endl;
			int currow=pos.rows();
			for(int i=0;i<neg.rows();i++){
				bool isinpos=false;
				for(int j=0;j<pos.rows();j++)
					if(pos(j,0)==neg(i,0))
						isinpos=true;
				if(!isinpos){
					combined(currow,0)=neg(i,0);
					combined(currow++,1)=0;
				}
			}
			//cout << " combined " << endl << combined << endl;

			for(int i=0;i<neg.rows();i++)
				for(int j=0;j<unique_labels;j++)
					if(combined(j,0)==neg(i,0))
						combined(j,1)=combined(j,1)+neg(i,1);
			//cout << " really combined " << endl << combined << endl;
			return combined;
		}
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
	char* ffffilename;
	ffffilename=NULL;
	int num_trees;
	if( argc != 4 && argc != 5 )
	{
		cout << "\tusage: " << endl << argv[0] << " test_data.csv learned.forest output.classification" << endl << "or" << endl << argv[0] << " test_data.csv learned.forest output.classification usefulness.entropy" << endl;
		return -1;
	}else{
		filename=argv[1];
		ffilename=argv[2];
		fffilename=argv[3];
		if(argc==5)
			ffffilename=argv[4];
			
	}
	string token, line;

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
	//cout << "second tree: " << *forest[1] << endl;

	vector<int> unique_labels = get_unique_labels(forest.front());

	num_labels=unique_labels.size();

	cout << "unique labels: " << endl;
	for (int i=0;i<unique_labels.size();i++)
		cout << unique_labels[i] << " - ";
	cout << endl;

	cout << "reading file " << filename << endl;
	//read the file twice. Once to count elements per line and #lines
	ifstream file (filename); 
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

	//save results to in csv format
	ofstream classification(fffilename);
	//print labels, to know which order they are in
	classification << unique_labels[0];
	for (int j=1;j<unique_labels.size();j++)
		classification << "," << unique_labels[j];
	classification << endl;

	ofstream usefulness;
	if(ffffilename!=NULL)
		//save usefulness of each information in csv format
		usefulness.open(ffffilename,ios::binary);

	if(ffffilename!=NULL)
		usefulness << "zilch" << endl;


	file.seekg(0, ios::beg);

	int i=0;
	int incomplete_i=0;
	getline (file, line);
	while ( file.good() )
	{
		std::istringstream ss(line);
		VectorXi datapoint_res(unique_labels.size());
		VectorXf datapoint(cols);
		for (int j=0;j<unique_labels.size();j++)
			datapoint_res(j)=0.0f;
		if(std::count(line.begin(), line.end(), '?')>0){//contains '?'
			//incomplete[incomplete_i++]=line;
			//cout << line << endl;
			bool known[cols];
			for(int j=0;j<cols;j++){
				std::getline(ss, token, ',');
				//std::cout << j << " - " << token << '\n';
				if(token.compare("?") != 0){
					known[j]=true;
					datapoint(j) = atof(token.c_str());
				}else{
					known[j]=false;
					datapoint(j) = 0.0f;
				}
			}
			//cout << datapoint.transpose() << endl;

			for (int j=0;j<num_trees;j++){
				MatrixXi tmp_res = eval_tree(forest[j],datapoint,known);

				for (int k=0;k<unique_labels.size();k++)
					for (int m=0;m<tmp_res.rows();m++)
						if(tmp_res(m,0)==unique_labels[k])
							datapoint_res(k)=datapoint_res(k)+tmp_res(m,1);
			}
		}
		else{
			for(int j=0;j<cols;j++){
				std::getline(ss, token, ',');
				//std::cout << "(" << j << ", " << i << "):" << token << '\n';
				datapoint(j) = atof(token.c_str());
			}
			//cout << datapoint.transpose() << endl;
			
			for (int j=0;j<num_trees;j++){
				MatrixXi tmp_res = eval_tree(forest[j],datapoint);

				for (int k=0;k<unique_labels.size();k++)
					for (int m=0;m<tmp_res.rows();m++)
						if(tmp_res(m,0)==unique_labels[k])
							datapoint_res(k)=datapoint_res(k)+tmp_res(m,1);
			}
		}
		classification << datapoint_res(0);
		for (int j=1;j<datapoint_res.size();j++)
			classification << "," << datapoint_res(j);
		classification << endl;
		getline (file, line);
	}
	classification.close();	
	if(ffffilename!=NULL)
		usefulness.close();	
}
