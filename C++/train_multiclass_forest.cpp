#include <iostream>
#include <fstream>
#include <random>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

class Node {
  	public:
		float w=0.0f;
		std::vector<int> label;
		float entropy=0.0f;
		int chosen_d=0;
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
	float entropy=0.0f;
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

Node* learn_node(MatrixXf X, VectorXi labels, VectorXi unique_labels, float min[], float max[]){
	//cout << "things are cool" << endl;
	Node *curnode = new Node();
	int d=rand()%X.cols();
	curnode->chosen_d=d;
	//find min and max here
	/*float min=std::numeric_limits<float>::max();
	float max=std::numeric_limits<float>::min();
	for(int i=0;i<X.rows();i++){
		if(min>X(i,curnode.chosen_d))
			min=X(i,curnode.chosen_d);
		if(max<X(i,curnode.chosen_d))
			max=X(i,curnode.chosen_d);
	}*/
	curnode->w=(max[d]-min[d])* ((double)rand() / RAND_MAX) +min[d];
	return curnode;
}

Node* learn_tree(MatrixXf X, VectorXi labels, VectorXi unique_labels){
	Node *parent = new Node();
	if(labels.size()==0)
		cout << "learn_tree called with labels size 0. size X is (" << X.rows() << " x " << X.cols() << ")" << endl;
	//cout << "creating a node on labels: " << labels.transpose() << endl;
	//if all the labels are the same, don't learn anything
	for(int i=1;i<labels.size();i++)
		if(labels(i-1)!=labels(i)){//need to learn
			//there are different labels, but the datapoints may be the same. Are they?
			bool indivisible=true;
			for(int k=1;indivisible && k<X.rows();k++)
				for(int j=0;indivisible && j<X.cols();j++)
					if(X(k-1,j)!=X(k,j))//compare every element of this one against every element of the previous point
						indivisible=false;

			if(indivisible){
				//find the unique labels for these points
				VectorXi unique_local(labels.size());
				int num_unique=0;
				
				for(int k=0;k<labels.size();k++){
					//is this the first occurence of this label?
					bool first=true;
					for (int j=0;j<k && first;j++)
						if(labels(k)==labels(j))
							first=false;
					if(!first)
						unique_local(k)=0;
					else{
						unique_local(k)=labels(k);
						num_unique++;
					}
				}
				VectorXi unique_labels_local(num_unique);
				int counter=0;
				for(int k=0;k<labels.size();k++){
					if(unique_local(k)!=0){
						unique_labels_local(counter)=unique_local(k);
						++counter;
					}
				}
				VectorXi unique_labels_local_counts(num_unique);
				for (int j=0;j<num_unique;j++){
					unique_labels_local_counts(j)=0;
					for (int k=0;k<labels.size();k++)
						if(unique_labels_local(j)==labels(k))
							unique_labels_local_counts(j)=unique_labels_local_counts(j)+1;
				}
				//std::cout << "There are " << num_unique << " indistinguishable labels: " << unique_labels_local.transpose() << ", and their counts: " << unique_labels_local_counts.transpose() << endl;
				//cout << endl << X << endl;

				for(int j=0;j<num_unique;j++){
					parent->label.push_back(unique_labels_local(j));
					parent->leaf_weight.push_back(unique_labels_local_counts(j));
				}
				parent->entropy=entropy(labels,unique_labels);
				//cout << "created leaf with label " << parent.label << endl;
			}else{
				//cout << "There are different points here. Learning clasifier." << endl;//labels.transpose() << "\nX:\n" << X << endl;
				//try to separate them 1000 times
				float min[X.cols()];
				float max[X.cols()];
				for(int d=0;d<X.cols();d++){
					min[d]=std::numeric_limits<float>::max();
					max[d]=std::numeric_limits<float>::min();
					for(int i=0;i<X.rows();i++){
						if(min[d]>X(i,d))
							min[d]=X(i,d);
						if(max[d]<X(i,d))
							max[d]=X(i,d);
					}
				}

				int tries=0;
				int posrows=0;
				int negrows=0;
				float parent_entropy=entropy(labels,unique_labels);
				float entropy_sum=parent_entropy;
				while(tries++<500 && (entropy_sum>=parent_entropy)){
					delete parent;
					parent = learn_node(X,labels,unique_labels,min,max);
					parent->entropy=parent_entropy;
					//create positive and negative X and labels
					posrows=0;
					negrows=0;
					for(int j=0;j<X.rows();j++)
						if(X(j,parent->chosen_d)>parent->w)
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
						if(X(j,parent->chosen_d)>parent->w){
							for(int k=0;k<X.cols();k++)
								X_pos(posrows,k)=X(j,k);
							labels_pos(posrows++)=labels(j);
						}else{
							for(int k=0;k<X.cols();k++)
								X_neg(negrows,k)=X(j,k);
							labels_neg(negrows++)=labels(j);
						}
					entropy_sum=entropy(labels_pos,unique_labels)+entropy(labels_neg,unique_labels);
				}
				//cout << "I really tried, man. I tried " << tries-1 << " times, and came up with pos #" << posrows << ", neg #" << negrows << ". Parent d and w are " << parent->chosen_d << " and " << parent->w << ", and the entropy has gone from " << parent_entropy << " to " << entropy_sum << endl;
				VectorXi labels_pos(posrows);
				MatrixXf X_pos(posrows,X.cols());
				VectorXi labels_neg(negrows);
				MatrixXf X_neg(negrows,X.cols());

				posrows=0;//becomes incrementer
				negrows=0;//becomes incrementer
				//place elements in each branch
				for(int j=0;j<X.rows();j++)
					if(X(j,parent->chosen_d)>parent->w){
						for(int k=0;k<X.cols();k++)
							X_pos(posrows,k)=X(j,k);
						labels_pos(posrows++)=labels(j);
					}else{
						for(int k=0;k<X.cols();k++)
							X_neg(negrows,k)=X(j,k);
						labels_neg(negrows++)=labels(j);
					}
				//if either positive or negative labels are empty, simply replace the parent with the new classifier. They can't both be empty. This reflects a classifier that didn't help, and is discarted.
				if(negrows==0){
					delete parent;
					parent=learn_tree(X_pos,labels_pos,unique_labels);
				}else if(posrows==0){
					delete parent;
					parent=learn_tree(X_neg,labels_neg,unique_labels);
				}else{
					Node* pos=learn_tree(X_pos,labels_pos,unique_labels);
					parent->pos=pos;

					Node* neg=learn_tree(X_neg,labels_neg,unique_labels);
					parent->neg=neg;
				}
			}
			break;//stop for loop
		}else if(i==labels.size()-1){//all the same, don't learn
			parent->label.push_back(labels(0));
			parent->leaf_weight.push_back(labels.size());
			parent->entropy=0.0f;
		}
	if(labels.size()==1){//all the same, don't learn
		parent->label.push_back(labels(0));
		parent->leaf_weight.push_back(1);
		parent->entropy=0.0f;
		//cout << "Just one datapoint. Created leaf with label " << parent.label << endl;
	}
	return parent;
}

void save(std::vector<Node*> forest, char* filename){
	ofstream file(filename);
	file << forest.size() << " trees" << endl;
	for(int i=0;i<forest.size();i++)
		file << *forest[i] << "next tree" << endl;
	file.close();
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
	int num_trees;
	if( argc != 4)
	{
		cout << "\trequires name of .csv file, number of trees, and forest .forest output file" << endl;
		return -1;
	}else{
		filename=argv[1];
		num_trees=atoi(argv[2]);
		outfilename=argv[3];
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
	for (int i=0;i<num_unique;i++)
		unique_labels(i)=0;
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

	cout << "\nStarting learning " << num_trees << " trees" << endl;
	std::vector<Node*> forest;
	for (int i=0;i<num_trees;i++){
		forest.push_back(learn_tree(csv,labels,unique_labels));
	}
	cout << "... learning complete" << endl;

	//cout << forest[1] << endl;
	save(forest,outfilename);
	/*
	vector<Node> learning_buffer;
	learning_buffer.push_back(root);

	while(learning_buffer.size()>0){
		cout << "learning node ...";
		Node parent=learning_buffer.front();
		learning_buffer.pop_back();
		cout << " learned w: " << parent.w.transpose() << endl;
	}*/

	/*std::ifstream ifs(outfilename);
	getline (ifs, line);
	std::istringstream ss(line);
	
	std::getline(ss, token, ' ');
	cout << "number of trees: " << atoi(token.c_str()) << endl;
	Node node_from_file = load_node(&ifs);
	cout << "second tree:" << endl;
	node_from_file = load_node(&ifs);
	
	cout << node_from_file;*/
}
