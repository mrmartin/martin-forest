#include <iostream>
#include <fstream>
#include <random>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

class Node {
  	public:
		float w;
		std::vector<int> label;
		float entropy=0.0;
		int chosen_d=0;
		std::vector<int> leaf_weight;
		std::vector<Node> pos;
		std::vector<Node> neg;
	// Insertion operator
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

/*Node learn_node(MatrixXf X, VectorXi labels, VectorXi unique_labels){
	int num_unique=unique_labels.size();
	//First, select a random combination of label subsets to create a binary classification problem
	std::vector<std::vector<int> > partitions = unconditioned_partition(num_unique);
	//using partitions, X, and labels, find the best dividing line for the first dimension
	//count which apply
	int relevant_rows=0;
	for(int j=0;j<labels.size();j++){
		for(int i=0;i<partitions[0].size();i++)
			if(labels(j)==unique_labels[partitions[0][i]])
				relevant_rows++;
		for(int i=0;i<partitions[1].size();i++)
			if(labels(j)==unique_labels[partitions[1][i]])
				relevant_rows++;
	}
	//cout << relevant_rows << " rows are relevant to this." << endl;
	float best_w=0;
	int best_d;
	int best_error=relevant_rows;
	for(int d=0;d<X.cols();d++){//for each dimension of X
		float values[relevant_rows];
		bool binary_labels[relevant_rows];
		int row_counter=0;
		for(int i=0;i<labels.size();i++){
			for(int j=0;j<partitions[0].size();j++)
				if(labels(i)==unique_labels[partitions[0][j]])
					binary_labels[row_counter++]=false;
			for(int j=0;j<partitions[1].size();j++)
				if(labels(i)==unique_labels[partitions[1][j]])
					binary_labels[row_counter++]=true;
		}
		row_counter=0;
		for(int i=0;i<labels.size();i++){
			for(int j=0;j<partitions[0].size();j++)
				if(labels(i)==unique_labels[partitions[0][j]])
					values[row_counter++]=X(i,d);
			for(int j=0;j<partitions[1].size();j++)
				if(labels(i)==unique_labels[partitions[1][j]])
					values[row_counter++]=X(i,d);
		}

		//print labels and values
		//cout << endl << "dimension " << d << ":" << endl;
		//for(int i=0;i<relevant_rows;i++)
		//	cout << binary_labels[i] << ", ";
		//cout << endl;
		//for(int i=0;i<relevant_rows;i++)
		//	cout << values[i] << ", ";
		//cout << endl;

		//now, bubble sort values, and move binary_labels accordingly
		for(int i=1;i<relevant_rows;i++){
			for(int j=0;j<relevant_rows-i;j++){
				if(values[j]>values[j+1]){//swap
					float tmp=values[j];
					values[j]=values[j+1];
					values[j+1]=tmp;

					bool btmp=binary_labels[j];
					binary_labels[j]=binary_labels[j+1];
					binary_labels[j+1]=btmp;
				}
			}
		}

		//print labels and values
		//cout << "sorted values :" << endl;
		//for(int i=0;i<relevant_rows;i++)
		//	cout << binary_labels[i] << ", ";
		//cout << endl;
		//for(int i=0;i<relevant_rows;i++)
		//	cout << values[i] << ", ";
		//cout << endl;

		//place w between every pair, and calculate errors
		for(int i=1;i<relevant_rows;i++){
			int error=0;
			for(int j=0;j<i;j++)
				error+=binary_labels[j]?0:1;
			for(int j=i;j<relevant_rows;j++)
				error+=binary_labels[j]?1:0;
			//cout << error << ",";
			//an error of 0 and an error of relevant_rows are both perfect, but the second would require flipping left and right. Since that doesn't matter, we can compare error to relevant_rows-error
			if(error<best_error || relevant_rows-error<best_error){//new best w
				best_w=(values[i]+values[i-1])/2.0;
				best_d=d;
				best_error=min(error,relevant_rows-error);
				//cout << "best error is now a measy " << best_error << endl;
			}
		}
	}
	cout << endl << "best divider is at " << best_w << " along dimension " << best_d << ", with an error of " << best_error << endl;

	Node curnode;
	curnode.w=best_w;
	curnode.chosen_d=best_d;
	//curnode.entropy=lowest_entropy;
	return curnode;
}*/

Node learn_node(MatrixXf X, VectorXi labels, VectorXi unique_labels){
	//cout << "things are cool" << endl;
	Node curnode;
	curnode.chosen_d=rand()%X.cols();
	//find min and max here
	float min=std::numeric_limits<float>::max();
	float max=std::numeric_limits<float>::min();
	for(int i=0;i<X.rows();i++){
		if(min>X(i,curnode.chosen_d))
			min=X(i,curnode.chosen_d);
		if(max<X(i,curnode.chosen_d))
			max=X(i,curnode.chosen_d);
	}
	curnode.w=(max-min)* ((double)rand() / RAND_MAX) +min;
	return curnode;
}

/*Node learn_node(MatrixXf csv, VectorXi labels, VectorXi unique_labels){
	int num_unique=unique_labels.size();
	//First, select a random combination of label subsets to create a binary classification problem
	std::vector<std::vector<int> > partitions = unconditioned_partition(num_unique);
	//cout << "division: ";
	//for(std::vector<int>::size_type i = 0; i != partitions[0].size(); i++) {
	//	printf("%d ",unique_labels[partitions[0][i]]);
	//}
	//printf(" - ");
	//for(std::vector<int>::size_type i = 0; i != partitions[1].size(); i++) {
	//	printf("%d ",unique_labels[partitions[1][i]]);
	//}
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

		//cout << "positive labels:";
		//for(int j=0;j<labels_pos.size();j++)
		//	cout << labels_pos(j) << ",";
		//cout << endl;
		//cout << "negative labels:";
		//for(int j=0;j<labels_neg.size();j++)
		//	cout << labels_neg(j) << ",";
		//cout << endl;
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
	//to avoid ridiculous recursion. Count attempts
	int attempts=0;
	while(d_chosen==-1){
		w_chosen(0)=4.0* ((double)rand() / RAND_MAX) -2;
		w_chosen(1)=4.0* ((double)rand() / RAND_MAX) -2;
		d_chosen=rand()%csv.cols();

		//calculate the entropy of this classifier, on all labels available here
		for (int j=0;j<X_all.rows();j++){
			X_all(j,0)=1;
			X_all(j,1)=csv(j,d_chosen);
		}
		VectorXf e = X_all*w_chosen;
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
		
		if(posrows>0 && negrows>0){
			//cout << "it took " << attempts << " attempts." << endl;
		}else
			d_chosen=-1;//try again
		if(attempts % 1000 == 999){
			cout << "tried " << attempts << ", and still trying" << endl;
			cout << "labels are: " << labels << endl << " and csv is " << endl << csv << endl;
		}
		attempts++;
	}
	Node curnode;
	curnode.w=w_chosen;
	curnode.chosen_d=d_chosen;
	curnode.entropy=lowest_entropy;
	return curnode;
}*/

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
					parent.label.push_back(unique_labels_local(j));
					parent.leaf_weight.push_back(unique_labels_local_counts(j));
				}
				parent.entropy=0;
				//cout << "created leaf with label " << parent.label << endl;
			}else{
				//cout << "There are different points here. Learning clasifier." << endl;//labels.transpose() << "\nX:\n" << X << endl;
				//try to separate them 1000 times
				int tries=0;
				int posrows=0;
				int negrows=0;
				while(tries++<1000 && (posrows==0 || negrows==0)){
					parent = learn_node(X,labels,unique_labels);
					//create positive and negative X and labels
					posrows=0;
					negrows=0;
					for(int j=0;j<X.rows();j++)
						if(X(j,parent.chosen_d)>parent.w)
							posrows++;
						else
							negrows++;
				}
				cout << "I really tried, man. I tried " << tries-1 << " times, and came up with pos #" << posrows << ", neg #" << negrows << ". Parent d and w are " << parent.chosen_d << " and " << parent.w << endl;
				VectorXi labels_pos(posrows);
				MatrixXf X_pos(posrows,X.cols());
				VectorXi labels_neg(negrows);
				MatrixXf X_neg(negrows,X.cols());

				posrows=0;//becomes incrementer
				negrows=0;//becomes incrementer
				//place elements in each branch
				for(int j=0;j<X.rows();j++)
					if(X(j,parent.chosen_d)>parent.w){
						for(int k=0;k<X.cols();k++)
							X_pos(posrows,k)=X(j,k);
						labels_pos(posrows++)=labels(j);
					}else{
						for(int k=0;k<X.cols();k++)
							X_neg(negrows,k)=X(j,k);
						labels_neg(negrows++)=labels(j);
					}
				//cout << "labels: " << labels << "\nX:\n" << X << "\nX_pos:\n" << X_pos << "\nlabels_pos:\n" << labels_pos << "\nX_neg:\n" << X_neg << "\nlabels_neg:\n" << labels_neg << endl;
				
				//cout << "entropy from parent: " << entropy(labels,unique_labels) << ", entropy to children: " << entropy(labels_pos,unique_labels) << ", " << entropy(labels_neg,unique_labels) << " (sum: " << (entropy(labels_pos,unique_labels)+entropy(labels_neg,unique_labels)) << ")" << endl;
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
			}
			break;//stop for loop
		}else if(i==labels.size()-1){//all the same, don't learn
			parent.label.push_back(labels(0));
			parent.leaf_weight.push_back(labels.size());
			parent.entropy=0;
			//cout << "created leaf with label " << parent.label << endl;
		}
	if(labels.size()==1){//all the same, don't learn
		parent.label.push_back(labels(0));
		parent.leaf_weight.push_back(1);
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

void save(std::vector<Node> forest, char* filename){
	ofstream file(filename);
	file << forest.size() << " trees" << endl;
	for(int i=0;i<forest.size();i++)
		file << forest[i] << "next tree" << endl;
	file.close();
}

Node load_node(ifstream* ifs){
	Node node;
	string line, token;
	//cout << "reading node: ";
	getline (*ifs, line);
	std::istringstream ss(line);
	std::getline(ss, token, ',');
	int m=atoi(token.c_str());
	for(int i=0;i<m;i++){
		std::getline(ss, token, ',');
		node.label.push_back(atoi(token.c_str()));
	}
	//cout << "label: " << line.c_str() << "->" << node.label;
	getline (*ifs, line);
	node.entropy = atof(line.c_str());
	//cout << ", entropy: " << line.c_str() << "->" << node.entropy;
	getline (*ifs, line);
	node.chosen_d = atoi(line.c_str());
	//cout << ", chosen_d: " << line.c_str() << "->" << node.chosen_d;
	getline (*ifs, line);
	//node.leaf_weight = atoi(line.c_str());
	std::istringstream ss_weights(line);
	std::getline(ss_weights, token, ',');
	m=atoi(token.c_str());
	for(int i=0;i<m;i++){
		std::getline(ss_weights, token, ',');
		node.leaf_weight.push_back(atoi(token.c_str()));
	}
	//cout << ", leaf_weight: " << line.c_str() << "->" << node.leaf_weight;
	getline (*ifs, line);
	node.w = atof(line.c_str());
	//cout << ", w(0): " << line.c_str() << "->" << node.w;
	getline (*ifs, line);
	if(line == "p"){//careful, a node must have either 0 or two children!
		//cout << "read a pos node:" << endl;
		//the order is always 1)positive, 2)negative.
		node.pos.push_back(load_node(ifs));
		//getline (*ifs, line);//this discarted line must always be 'n', otherwise something went awry
		//cout << "n is " << line.c_str() << endl;
		//cout << "read a neg node:" << endl;
		node.neg.push_back(load_node(ifs));
		//(Either can have an arbitrary substructure)
	}
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
	std::vector<Node> forest;
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
