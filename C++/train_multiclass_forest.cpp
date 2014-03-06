#include <iostream>
#include <fstream>
#include <random>
#include <sstream>
using namespace std;

class Node {
  	public:
		float w;
		vector<int> label;
		float entropy;
		int chosen_d;
		int weight;
		vector<int> leaf_weight;
		Node* pos;
		Node* neg;
	//Insertion operator
	friend ostream& operator<<(ostream& os, const Node& n)
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
        w=0.0f;
        entropy=0.0f;
        chosen_d=0;
		weight=0;
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

float entropy(int labels[], int labels_size, int unique_labels[], int unique_labels_size){
	//count how many there are of each class
	int counts[unique_labels_size];
	for(int i=0;i<unique_labels_size;i++){
		counts[i]=0;
		for(int j=0;j<labels_size;j++)
			if(labels[j]==unique_labels[i])
				counts[i]=counts[i]+1;
	}
	//cout << counts << endl << "-" << endl;
	float entropy=0.0f;
	for(int i=0;i<unique_labels_size;i++){
		if(counts[i]>0){
			float p = (float)counts[i]/labels_size;
			entropy+= log(p)*p;
		}
	}
	return -1*entropy;
}

vector<vector<int>> unconditioned_partition (int l){
	if(l<2)
		cout << "error, partitioning less than one set" << endl;
	vector<vector<int> > partitions;
	partitions.push_back(vector<int>());
	partitions.push_back(vector<int>());

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

Node* learn_node(double X[], int X_cols,int X_rows, int labels[], int labels_size, int unique_labels[], int unique_labels_size, float min[], float max[]){
	//cout << "things are cool" << endl;
	Node *curnode = new Node();
	int d=rand()%X_cols;
	curnode->chosen_d=d;
	//find min and max here
	/*float min=numeric_limits<float>::max();
	float max=numeric_limits<float>::min();
	for(int i=0;i<X.rows();i++){
		if(min>X(i,curnode.chosen_d))
			min=X(i,curnode.chosen_d);
		if(max<X(i,curnode.chosen_d))
			max=X(i,curnode.chosen_d);
	}*/
	curnode->w=(max[d]-min[d])* ((double)rand() / RAND_MAX) +min[d];
	return curnode;
}

//overload the learn_node function to reduce overhead. However, this function does not allow complex training beyond random classifiers
Node* learn_node(int X_cols, float min[], float max[]){
	Node *curnode = new Node();
	int d=rand()%X_cols;
	curnode->chosen_d=d;
	curnode->w=(max[d]-min[d])* ((double)rand() / RAND_MAX) +min[d];
	return curnode;
}

//this function recursively learns the tree, until reaching nodes where division is either impossible or all the labels are the same
Node* learn_tree(double X[], int X_cols, int X_rows, int labels[], int labels_size, int unique_labels[], int unique_labels_size, int selected_X[], int num_selected){
	/*cout << "selected_X is:" << endl;
	for (int i=0;i<num_selected;i++)
		cout << selected_X[i] << ", ";
	cout << endl;
	cout << "selected_X labels are:" << endl;
	for (int i=0;i<num_selected;i++)
		cout << labels[i] << ", ";
	cout << endl;
	*/
	Node *parent = new Node();
	if(labels_size==0)
		cout << "learn_tree called with labels size 0. size X is (" << X_rows << " x " << X_cols << ")" << endl;
	//cout << "creating a node on labels: " << labels.transpose() << endl;
	//if all the labels are the same, don't learn anything
	for(int i=1;i<labels_size;i++)
		if(labels[i-1]!=labels[i]){//need to learn
			//there are different labels, but the datapoints may be the same. Are they?
			bool indivisible=true;
			for(int k=1;indivisible && k<num_selected;k++)
				for(int j=0;indivisible && j<X_cols;j++)
					if(X[(selected_X[k]-1)*X_cols+j]!=X[selected_X[k]*X_cols+j])//compare every element of this one against every element of the previous point
						indivisible=false;

			if(indivisible){
				//find the unique labels for these points
				int unique_local[labels_size];
				int num_unique=0;
				
				for(int k=0;k<labels_size;k++){
					//is this the first occurence of this label?
					bool first=true;
					for (int j=0;j<k && first;j++)
						if(labels[k]==labels[j])
							first=false;
					if(!first)
						unique_local[k]=0;
					else{
						unique_local[k]=labels[k];
						num_unique++;
					}
				}
				int unique_labels_local[num_unique];
				int counter=0;
				for(int k=0;k<labels_size;k++){
					if(unique_local[k]!=0){
						unique_labels_local[counter]=unique_local[k];
						++counter;
					}
				}
				int unique_labels_local_counts[num_unique];
				for (int j=0;j<num_unique;j++){
					unique_labels_local_counts[j]=0;
					for (int k=0;k<labels_size;k++)
						if(unique_labels_local[j]==labels[k])
							unique_labels_local_counts[j]=unique_labels_local_counts[j]+1;
				}
				//cout << "There are " << num_unique << " indistinguishable labels." << endl;
				//cout << endl << X << endl;

				for(int j=0;j<num_unique;j++){
					parent->label.push_back(unique_labels_local[j]);
					parent->leaf_weight.push_back(unique_labels_local_counts[j]);
				}
				parent->entropy=entropy(labels,labels_size,unique_labels,unique_labels_size);
				parent->weight=labels_size;
				//cout << "created leaf with several labels" << endl;
			}else{
				//cout << "There are different points here. Learning clasifier." << endl;//labels.transpose() << "\nX:\n" << X << endl;
				//try to separate them 1000 times
				float min[X_cols];
				float max[X_cols];
				for(int d=0;d<X_cols;d++){
					min[d]=numeric_limits<float>::max();
					max[d]=numeric_limits<float>::min();
					for(int i=0;i<num_selected;i++){
						if(min[d]>X[selected_X[i]*X_cols+d])
							min[d]=X[selected_X[i]*X_cols+d];
						if(max[d]<X[selected_X[i]*X_cols+d])
							max[d]=X[selected_X[i]*X_cols+d];
					}
				}

				int tries=0;
				int posrows=0;
				int negrows=0;
				float parent_entropy=entropy(labels,labels_size,unique_labels,unique_labels_size);
				float entropy_sum=parent_entropy;
				while(tries++<500 && (entropy_sum>=parent_entropy)){
					delete parent;
					parent = learn_node(X_cols,min,max);
					parent->entropy=parent_entropy;
					parent->weight=labels_size;
					//create positive and negative X and labels
					posrows=0;
					negrows=0;
					for(int j=0;j<num_selected;j++)
						if(X[selected_X[j]*X_cols+parent->chosen_d]>parent->w)
							posrows++;
						else
							negrows++;

					int labels_pos[posrows];
					//double X_pos[posrows*X_cols];
					int labels_neg[negrows];
					//double X_neg[negrows*X_cols];

					posrows=0;//becomes incrementer
					negrows=0;//becomes incrementer
					//place elements in each branch
					for(int j=0;j<num_selected;j++)
						if(X[selected_X[j]*X_cols+parent->chosen_d]>parent->w){
							//for(int k=0;k<X_cols;k++)
							//	X_pos[posrows*X_cols+k]=X[j*X_cols+k];
							labels_pos[posrows++]=labels[j];
						}else{
							//for(int k=0;k<X_cols;k++)
							//	X_neg[negrows*X_cols+k]=X[j*X_cols+k];
							labels_neg[negrows++]=labels[j];
						}
					entropy_sum=entropy(labels_pos,posrows,unique_labels,unique_labels_size)+entropy(labels_neg,negrows,unique_labels,unique_labels_size);
				}
				//cout << "I really tried, man. I tried " << tries-1 << " times, and came up with pos #" << posrows << ", neg #" << negrows << ". Parent d and w are " << parent->chosen_d << " and " << parent->w << ", and the entropy has gone from " << parent_entropy << " to " << entropy_sum << endl;
				int* labels_pos = new int[posrows];
				//double* X_pos = new double[posrows*X_cols];
				int* selected_pos = new int[posrows];
				int* labels_neg = new int[negrows];
				//double* X_neg = new double[negrows*X_cols];
				int* selected_neg = new int[negrows];
				if(posrows+negrows!=num_selected)
					cout << "error: selcted positive rows + selected negative rows != total rows under consideration." << endl;

				posrows=0;//becomes incrementer
				negrows=0;//becomes incrementer
				//place elements in each branch
				for(int j=0;j<num_selected;j++)
					if(X[selected_X[j]*X_cols+parent->chosen_d]>parent->w){
						selected_pos[posrows]=selected_X[j];
						labels_pos[posrows++]=labels[j];
					}else{
						selected_neg[negrows]=selected_X[j];
						labels_neg[negrows++]=labels[j];
					}
				//if either positive or negative labels are empty, simply replace the parent with the new classifier. They can't both be empty. This reflects a classifier that didn't help, and is discarted.
				if(negrows==0){
					delete parent;
					parent=learn_tree(X,X_cols,posrows,labels_pos,posrows,unique_labels,unique_labels_size,selected_pos,posrows);
                    			delete[] selected_pos;
                    			delete[] labels_pos;
				}else if(posrows==0){
					delete parent;
					parent=learn_tree(X,X_cols,negrows,labels_neg,negrows,unique_labels,unique_labels_size,selected_neg,negrows);
                    			delete[] selected_neg;
                    			delete[] labels_neg;
				}else{
					Node* pos=learn_tree(X,X_cols,posrows,labels_pos,posrows,unique_labels,unique_labels_size,selected_pos,posrows);
					parent->pos=pos;
					//cout << "pos created and put into tree, now deleting arrays" << endl;
                   			delete[] selected_pos;
					delete[] labels_pos;
					//cout << "pos arrays deleted, learning neg tree (size " << negrows << ")" << endl;

					Node* neg=learn_tree(X,X_cols,negrows,labels_neg,negrows,unique_labels,unique_labels_size,selected_neg,negrows);
					parent->neg=neg;
                    			delete[] selected_neg;
                    			delete[] labels_neg;
				}
			}
			break;//stop for loop
		}else if(i==labels_size-1){//all the same, don't learn. This creates a leaf (no children)
			parent->label.push_back(labels[0]);
			parent->leaf_weight.push_back(labels_size);
			parent->entropy=0.0f;
			parent->weight=labels_size;
			//cout << "all the same, created leaf with label " << labels[0] << endl;
		}
	if(labels_size==1){//There is only onw datapoint, so create a leaf
		parent->label.push_back(labels[0]);
		parent->leaf_weight.push_back(1);
		parent->entropy=0.0f;
		parent->weight=labels_size;
		//cout << "Just one datapoint. Created leaf with label " << labels[0] << endl;
	}
	return parent;
}

void save(vector<Node*> forest, char* filename){
	ofstream file(filename);
	file << forest.size() << " trees" << endl;
	for(int i=0;i<forest.size();i++)
		file << *forest[i] << "next tree" << endl;
	file.close();
}

// returns count of non-overlapping occurrences of 'sub' in 'str'
int countSubstring(const string& str, const string& sub)
{
    if (sub.length() == 0) return 0;
    int count = 0;
    for (size_t offset = str.find(sub); offset != string::npos;
	 offset = str.find(sub, offset + sub.length()))
    {
        ++count;
    }
    return count;
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
	cout << "reading file to check size and inconsistencies " << filename << endl;
	//read the file twice. Once to count elements per line and #lines
	ifstream file (filename); 
	string value, line;
	int rows=0;
	int cols=0;
	int incomplete_rows=0;
	getline (file, line);
	while ( file.good() )
	{
		if(line.find("?")!=string::npos)//count(line.begin(), line.end(), '?')>0)
			incomplete_rows++;
		int elements;
		rows++;
		elements=countSubstring(line, ",");
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
	int X_rows=rows-incomplete_rows;
	int X_cols=cols-1;
	cout << "creating double array of size " << rows * cols << endl;
	double* csv = new double[X_rows*X_cols];
	//VectorXi labels(rows-incomplete_rows);
	int labels[rows-incomplete_rows];
	int labels_size = rows-incomplete_rows;
	string incomplete[incomplete_rows];
	file.seekg(0, ios::beg);

	string token;
	int i=0;
	int incomplete_i=0;
	cout << "reading training file into double array." << endl;
	getline (file, line);
	while ( file.good() )
	{
		istringstream ss(line);
		if(line.find("?")!=string::npos)//count(line.begin(), line.end(), '?')>0)//contains '?'
			incomplete[incomplete_i++]=line;
		else{
			getline(ss, token, ',');
			labels[i] = atoi(token.c_str());
			for(int j=0;j<cols-1;j++){
				getline(ss, token, ',');
				//cout << "(" << j << ", " << i << "):" << token << ", at X[" << i*X_cols+j << "]" << '\n';
				csv[i*X_cols+j] = atof(token.c_str());
			}
			++i;
		}
		getline (file, line);
	}
	//cout << "X from file is:\n" << csv << endl;
	//cout << "Y from file is:\n" << labels << endl;

	//what are the unique labels?
	int unique_all[rows-incomplete_rows];
	int num_unique=0;
	for(int i=0;i<rows-incomplete_rows;i++){
		//is this the first occurence of this label?
		bool first=true;
		for (int j=0;j<i && first;j++)
			if(labels[i]==labels[j])
				first=false;
		if(!first)
			unique_all[i]=0;
		else{
			unique_all[i]=labels[i];
			num_unique++;
		}
	}
	cout << endl;
	int unique_labels[num_unique];
	for (int i=0;i<num_unique;i++)
		unique_labels[i]=0;
	int counter=0;
	for(int i=0;i<rows-incomplete_rows;i++){
		if(unique_all[i]!=0){
			unique_labels[counter]=unique_all[i];
			++counter;
		}
	}
	cout << "There are " << num_unique << " unique labels: " << endl;//unique_labels.transpose() << endl;
	for(int i=0;i<num_unique;i++)
		cout << unique_labels[i] << " ";
	cout << endl;

	//calculate the entropy. Does not require values X, only labels
	cout << "The entropy of this set is: " << entropy(labels,labels_size,unique_labels,num_unique) << endl;

	cout << "\nStarting learning " << num_trees << " trees" << endl;
	vector<Node*> forest;
	int selected_X[X_rows];
	for (int i=0;i<X_rows;i++)
		selected_X[i]=i;//initialize with all points
	// learn entire forest, then save it
	/* for (int i=0;i<num_trees;i++){
		forest.push_back(learn_tree(csv,X_cols,X_rows,labels,labels_size,unique_labels,num_unique,selected_X,X_rows));
	}
	cout << "... learning complete" << endl;

	//cout << forest[1] << endl;
	save(forest,outfilename); */

	// learn each tree on its own, save it, and erase it from memory
	ofstream forestfile(outfilename);
	forestfile << num_trees << " trees" << endl;
	for (int i=0;i<num_trees;i++){
		Node *parent = learn_tree(csv,X_cols,X_rows,labels,labels_size,unique_labels,num_unique,selected_X,X_rows);
		forestfile << *parent << "next tree" << endl;
		delete parent;
		cout << "learned tree " << i << " of " << num_trees << endl;
	}
	forestfile.close();
	/*
	vector<Node> learning_buffer;
	learning_buffer.push_back(root);

	while(learning_buffer.size()>0){
		cout << "learning node ...";
		Node parent=learning_buffer.front();
		learning_buffer.pop_back();
		cout << " learned w: " << parent.w.transpose() << endl;
	}*/

	/*ifstream ifs(outfilename);
	getline (ifs, line);
	istringstream ss(line);
	
	getline(ss, token, ' ');
	cout << "number of trees: " << atoi(token.c_str()) << endl;
	Node node_from_file = load_node(&ifs);
	cout << "second tree:" << endl;
	node_from_file = load_node(&ifs);
	
	cout << node_from_file;*/
}
