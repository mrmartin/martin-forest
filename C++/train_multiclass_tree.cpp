#include <iostream>
#include <fstream>
#include <random>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

class Node {
  	public:
		Node();
		VectorXf w;
		int label=0;
		float entropy=0.0;
		int chosen_d=0;
		std::vector<Node> pos;
		std::vector<Node> neg;
	// Insertion operator
	friend std::ostream& operator<<(std::ostream& os, const Node& n)
	{
		// write out individual members of s with an end of line between each one
		os << n.label << '\n';
		os << n.entropy << '\n';
		os << n.chosen_d << '\n';
		os << n.w(0) << '\n';
		os << n.w(1) << '\n';
		if(!n.pos.empty()){
			os << "p" << '\n';
			os << n.pos.front();
		}
		if(!n.neg.empty()){
			os << "n" << '\n';
			os << n.neg.front();
		}
		return os;
	}
};

Node::Node(){
	VectorXf w2(2);
	w2(0)=0;
	w2(1)=0;
	w=w2;
}

float entropy(VectorXi labels, VectorXi unique_labels){
	//count how many there are of each class
	VectorXi counts(unique_labels.size());
	for(int i=0;i<unique_labels.size();i++){
		counts(i)=0;
		for(int j=0;j<labels.size();j++)
			if(labels(j)==unique_labels(i))
				counts(i)=counts(i)+1;
	}
	//std::cout << counts << endl << "-" << endl;
	float entropy=0;
	for(int i=0;i<counts.size();i++){
		if(counts(i)>0){
			float p = (float)counts(i)/labels.size();
			entropy+= log(p)*p;
		}
	}
	return -1*entropy;
}

std::vector<std::vector<int> > unconditioned_partition (int l){
	if(l<2)
		cout << "error, partitioning less than one set" << endl;
	std::vector<std::vector<int> > partitions;
	partitions.push_back(std::vector<int>());
	partitions.push_back(std::vector<int>());

	//put a random class in each
	int r1=rand()%l;
	int r2=rand()%l;
	while(r2==r1)
		r2=rand()%l;
	partitions[0].push_back(r1);
	partitions[1].push_back(r2);

	for (int j=0;j<l;j++){
		if(j!=r1 && j!=r2)
			switch(rand()%3){
				case 0: partitions[0].push_back(j);
					break;
				case 1: partitions[1].push_back(j);
					break;
				default: break;
			}
	}
	return partitions;
}

Node learn_node(MatrixXf csv, VectorXi labels, VectorXi unique_labels){
	int num_unique=unique_labels.size();
	//First, select a random combination of label subsets to create a binary classification problem
	std::vector<std::vector<int> > partitions = unconditioned_partition(num_unique);
	/*cout << "division: ";
	for(std::vector<int>::size_type i = 0; i != partitions[0].size(); i++) {
		printf("%d ",unique_labels[partitions[0][i]]);
	}
	printf(" - ");
	for(std::vector<int>::size_type i = 0; i != partitions[1].size(); i++) {
		printf("%d ",unique_labels[partitions[1][i]]);
	}*/
	int posrows=0;
	int negrows=0;
	for (int j=0;j<csv.rows();j++){
		//count rows for which we use the label
		for (int k=0;k<partitions[1].size();k++)
			if(labels(j)==unique_labels[partitions[1][k]])
				posrows++;
		for (int k=0;k<partitions[0].size();k++)
			if(labels(j)==unique_labels[partitions[0][k]])
				negrows++;
	}
	int activerows=posrows+negrows;

	MatrixXf X(activerows,2);
	for (int i=0;i<X.rows();i++)
		X(i,0)=1.0;
	VectorXf Y(activerows);
	int pos_references[activerows];
	int row_incrementer=0;
	//for this problem, for each dimension of X, find the best dividing value.
	for (int j=0;j<csv.rows();j++){
		//label each row
		for (int k=0;k<partitions[1].size();k++)
			if(labels(j)==unique_labels[partitions[1][k]]){
				pos_references[row_incrementer]=j;
				Y(row_incrementer++)=1;
			}
		for (int k=0;k<partitions[0].size();k++)
			if(labels(j)==unique_labels[partitions[0][k]]){
				pos_references[row_incrementer]=j;
				Y(row_incrementer++)=-1;
			}
	}
	//cout << "\nY:\n" << Y.transpose() << endl;

	MatrixXf X_all(csv.rows(),2);
	//the difference between these two lines is significant. The first only allows classifiers which lower the entropy against the previous entropy of the set (but the behavior for what happens otherwise is undefined). The second uses the best, which may be worse than not dividing the dataset at all. For a visualisation of this problem, think of the perceptron problem.
	//1:
	float lowest_entropy=entropy(labels,unique_labels);
	//2:
	//float lowest_entropy=std::numeric_limits<float>::max();
	VectorXf w_chosen(2);
	int d_chosen=-1;
	for (int i=0;i<csv.cols();i++){
		for (int j=0;j<X.rows();j++)
			X(j,1)=csv(pos_references[j],i);
		//cout << "\nX:" << X << endl;
		//cout << "\nY:" << Y.transpose() << endl;
		//learn linear classifier
		VectorXf w = X.colPivHouseholderQr().solve(Y);
		//cout << "The solution is:\n" << w.transpose() << endl;

		//calculate the entropy of this classifier, on all labels available here
		for (int j=0;j<X_all.rows();j++){
			X_all(j,0)=1;
			X_all(j,1)=csv(j,i);
		}
		VectorXf e = X_all*w;
		//VectorXf e = X*w;
		//cout << "w*X transpose is:\n" << e.transpose() << endl;
		//calculate the entropy of each branch. First, count number of elements in each branch
		posrows=0;
		negrows=0;
		for(int j=0;j<X_all.rows();j++)
			if(e(j)>0)
				posrows++;
			else
				negrows++;
		VectorXi labels_pos(posrows);
		VectorXi labels_neg(negrows);
		posrows=0;//becomes incrementer
		negrows=0;//becomes incrementer
		//place elements in each branch
		for(int j=0;j<X_all.rows();j++)
			if(e(j)>0)
				labels_pos(posrows++)=labels(j);
			else
				labels_neg(negrows++)=labels(j);

		/*cout << "positive labels:";
		for(int j=0;j<labels_pos.size();j++)
			cout << labels_pos(j) << ",";
		cout << endl;
		cout << "negative labels:";
		for(int j=0;j<labels_neg.size();j++)
			cout << labels_neg(j) << ",";
		cout << endl;*/
		//std::cout << "The entropy of this entire set is: " << entropy(labels,unique_labels) << ". Positive branch: " << entropy(labels_pos,unique_labels) << ", negative branch: " << entropy(labels_neg,unique_labels) << endl;

		//for this divider, for each parameter, calculate the resulting entropy
		//pick the lowest entropy
		if(lowest_entropy>entropy(labels_neg,unique_labels)+entropy(labels_pos,unique_labels)){
			w_chosen=w;
			d_chosen=i;
			lowest_entropy=entropy(labels_neg,unique_labels)+entropy(labels_pos,unique_labels);
		}
	}
	//cout << "The entropy of this set is: " << entropy(labels,unique_labels) << ", and the lowest entropy sum over divided classes is: " << lowest_entropy << ". The chosen dimension is " << d_chosen << " (if this is -1, nothing was chosen)" << endl;
	if(d_chosen==-1){
		lowest_entropy=-1;
		w_chosen(0)=10.0* ((double)rand() / RAND_MAX) -5;
		w_chosen(1)=10.0* ((double)rand() / RAND_MAX) -5;
		d_chosen=rand()%csv.cols();
		//cout << "Creating random classifier, with d_chosen " << d_chosen << " and w: " << w_chosen.transpose() << endl;
	}
	Node curnode;
	curnode.w=w_chosen;
	curnode.chosen_d=d_chosen;
	curnode.entropy=lowest_entropy;
	return curnode;
}

void traverse(Node node){
	cout << "traversing - ";
	//cout << "+" << n.pos << ", -" << n.neg << endl;//n.entropy << endl;
	if(!node.pos.empty())
		traverse(node.pos.front());
	if(!node.neg.empty())
		traverse(node.neg.front());
}

Node learn_tree(MatrixXf X, VectorXi labels, VectorXi unique_labels){
	Node parent;
	if(labels.size()==0)
		cout << "learn_tree called with labels size 0. size X is (" << X.rows() << " x " << X.cols() << ")" << endl;
	//cout << "creating a node on labels: " << labels.transpose() << endl;
	//if all the labels are the same, don't learn anything
	for(int i=1;i<labels.size();i++)
		if(labels(i-1)!=labels(i)){//need to learn
			//cout << "There are different labels here. Learning clasifier." << endl;//labels.transpose() << "\nX:\n" << X << endl;
			parent = learn_node(X,labels,unique_labels);
			//recreate relevant X
			MatrixXf X_dimension(X.rows(),2);
			for (int i=0;i<X.rows();i++){
				X_dimension(i,0)=1.0;
				X_dimension(i,1)=X(i,parent.chosen_d);
			}
			//create positive and negative X and labels
			VectorXf e = X_dimension*parent.w;
			int posrows=0;
			int negrows=0;
			for(int j=0;j<X.rows();j++)
				if(e(j)>0)
					posrows++;
				else
					negrows++;

			VectorXi labels_pos(posrows);
			MatrixXf X_pos(posrows,X.cols());
			VectorXi labels_neg(negrows);
			MatrixXf X_neg(negrows,X.cols());

			posrows=0;//becomes incrementer
			negrows=0;//becomes incrementer
			//place elements in each branch
			for(int j=0;j<X.rows();j++)
				if(e(j)>0){
					for(int k=0;k<X.cols();k++)
						X_pos(posrows,k)=X(j,k);
					labels_pos(posrows++)=labels(j);
				}else{
					for(int k=0;k<X.cols();k++)
						X_neg(negrows,k)=X(j,k);
					labels_neg(negrows++)=labels(j);
				}
			//cout << "labels: " << labels << "\nX:\n" << X << "\nX_pos:\n" << X_pos << "\nlabels_pos:\n" << labels_pos << "\nX_neg:\n" << X_neg << "\nlabels_neg:\n" << labels_neg << endl;
			//if either positive or negative labels are empty, simply replace the parent with the new classifier. They can't both be empty. This reflects a classifier that didn't help, and is discarted.
			if(negrows==0)
				parent=learn_tree(X_pos,labels_pos,unique_labels);
			else if(posrows==0)
				parent=learn_tree(X_neg,labels_neg,unique_labels);
			else{
				Node pos=learn_tree(X_pos,labels_pos,unique_labels);
				parent.pos.push_back(pos);

				/*Test the positive node results: Node tmp = *(parent.pos);
				cout << "label of positive child: " << tmp.label << endl;*/
				Node neg=learn_tree(X_neg,labels_neg,unique_labels);
				parent.neg.push_back(neg);

			}
			break;//stop for loop
		}else if(i==labels.size()-1){//all the same, don't learn
			parent.label = labels(0);
			parent.entropy=0;
			//cout << "created leaf with label " << parent.label << endl;
		}
	if(labels.size()==1){//all the same, don't learn
		parent.label = labels(0);
		parent.entropy=0;
		//cout << "Just one datapoint. Created leaf with label " << parent.label << endl;
	}

	/*if(!parent.pos.empty() && !parent.neg.empty()){
		cout << "returning node with offspring " << endl;
		traverse(parent);
		cout << endl;		
	}
	else
		cout << "returning leaf with label " << parent.label << endl;*/
	return parent;
}

void save(Node node, char* filename){
	ofstream file(filename);
	file << "1 tree" << endl;
	file << node;
	file.close();
}

Node load_node(ifstream* ifs){
	Node node;
	string line;
	getline (*ifs, line);
	node.label = atoi(line.c_str());
	getline (*ifs, line);
	node.entropy = atof(line.c_str());
	getline (*ifs, line);
	node.chosen_d = atoi(line.c_str());
	getline (*ifs, line);
	node.w(0) = atof(line.c_str());
	getline (*ifs, line);
	node.w(1) = atof(line.c_str());
	getline (*ifs, line);
	if(line == "p")
		node.pos.push_back(load_node(ifs));
	if(line == "n")
		node.neg.push_back(load_node(ifs));
	return node;
}

int main(int argc, char** argv)
{
	/*float nula=0;
	float minusnula=-1*nula;

	cout << "0: " << nula << ", -0: " << minusnula << endl;
	if(nula!=minusnula)
		cout << "nejsem nula!" << endl;
	else
		cout << "jsem nula" << endl;*/

	srand(time(NULL)); /* seed random number generator */
	char* filename;
	char* outfilename;
	if( argc != 3)
	{
		cout << "\trequires name of .csv file and forest .forest output file" << endl;
		return -1;
	}else{
		filename=argv[1];
		outfilename=argv[2];
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
	cout << "\nfile has " << rows << " rows with " << cols << " columns each, and " << incomplete_rows << " incomplete rows" << endl;

	file.clear();
	//and the second time to put everything into a matrix
	MatrixXf csv(rows-incomplete_rows,cols-1);
	VectorXi labels(rows-incomplete_rows);
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
			std::getline(ss, token, ',');
			labels(i) = atoi(token.c_str());
			for(int j=0;j<cols-1;j++){
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

	//what are the unique labels?
	VectorXi unique_all(rows-incomplete_rows);
	int num_unique=0;
	for(int i=0;i<rows-incomplete_rows;i++){
		//is this the first occurence of this label?
		bool first=true;
		for (int j=0;j<i && first;j++)
			if(labels(i)==labels(j))
				first=false;
		if(!first)
			unique_all(i)=0;
		else{
			unique_all(i)=labels(i);
			num_unique++;
		}
	}
	std::cout << endl;
	VectorXi unique_labels(num_unique);
	int counter=0;
	for(int i=0;i<rows-incomplete_rows;i++){
		if(unique_all(i)!=0){
			unique_labels(counter)=unique_all(i);
			++counter;
		}
	}
	std::cout << "There are " << num_unique << " unique labels: " << unique_labels.transpose() << endl;

	//calculate the entropy. Does not require values X, only labels
	std::cout << "The entropy of this set is: " << entropy(labels,unique_labels) << endl;

	cout << "\nStarting learning" << endl;
	Node root;
	root = learn_tree(csv,labels,unique_labels);

	cout << "... learning complete" << endl;

	save(root,outfilename);
	/*
	vector<Node> learning_buffer;
	learning_buffer.push_back(root);

	while(learning_buffer.size()>0){
		cout << "learning node ...";
		Node parent=learning_buffer.front();
		learning_buffer.pop_back();
		cout << " learned w: " << parent.w.transpose() << endl;
	}*/

	std::ifstream ifs(outfilename);
	getline (ifs, line);
	std::istringstream ss(line);
	
	std::getline(ss, token, ' ');
	cout << "number of trees: " << atoi(token.c_str()) << endl;
	Node node_from_file = load_node(&ifs);
	
	cout << node_from_file;
}
