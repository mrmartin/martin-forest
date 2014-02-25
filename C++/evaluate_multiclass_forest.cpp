#include <iostream>
#include <fstream>
#include <random>
#include <sstream>
using namespace std;

static int num_labels=0;

class Node {
  	public:
		float w;
		std::vector<int> label;
		float entropy;
		int chosen_d;
		int weight;
		std::vector<int> leaf_weight;
		Node* pos;
		Node* neg;
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
		entropy=0.0;
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

vector<int*> eval_tree(Node *curnode, float datapoint[]){
	//cout << "evaluating for datapoint: " << datapoint.transpose() << endl << "curnode->w: " << curnode->w << " curnode->d: " << curnode->chosen_d << endl;
	if(curnode->leaf_weight.size()>0){//is a leaf?
		vector<int*> label_and_weight;
		for(int i=0;i<curnode->label.size();i++){
			int* this_label = new int(2);
			this_label[0]=curnode->label[i];
			this_label[1]=curnode->leaf_weight[i];
			label_and_weight.push_back(this_label);
		}
		return label_and_weight;
	}else{//a decision node
		//apply linear classifier
		if(datapoint[curnode->chosen_d]>curnode->w)//pos
			return eval_tree(curnode->pos,datapoint);
		else//neg
			return eval_tree(curnode->neg,datapoint);
	}
}

//evaluate for datapoint with some unknowns
vector<int*> eval_tree_unknowns(Node *curnode, float datapoint[], bool known[]){
	//cout << "evaluating for datapoint: " << datapoint.transpose() << endl << "curnode->w: " << curnode->w << " curnode->d: " << curnode->chosen_d << endl;
	if(curnode->leaf_weight.size()>0){//is a leaf?
		vector<int*> label_and_weight;
		for(int i=0;i<curnode->label.size();i++){
			int* this_label = new int(2);
			this_label[0]=curnode->label[i];
			this_label[1]=curnode->leaf_weight[i];
			label_and_weight.push_back(this_label);
		}
		return label_and_weight;
	}else{//a decision node
		if(known[curnode->chosen_d]){
			//apply linear classifier
			if(datapoint[curnode->chosen_d]>curnode->w)//pos
				return eval_tree_unknowns(curnode->pos,datapoint,known);
			else//neg
				return eval_tree_unknowns(curnode->neg,datapoint,known);
		}else{
			//don't know which, so combine the two
			vector<int*> pos=eval_tree_unknowns(curnode->pos,datapoint,known);
			vector<int*> neg=eval_tree_unknowns(curnode->neg,datapoint,known);
			//combine silly way, without checking for duplicates
			vector<int*> combined;
			
			for(int j=0;j<pos.size();j++)
				combined.push_back(pos.at(j));
			for(int j=0;j<neg.size();j++)
				combined.push_back(neg.at(j));
			return combined;
		}
	}
}

//evaluate for datapoint with some unknowns
float* eval_tree_importance(Node *curnode, float* datapoint, bool* known, int numfeatures){
	//cout << "evaluating for datapoint: " << datapoint.transpose() << endl << "curnode->w: " << curnode->w << " curnode->d: " << curnode->chosen_d << endl;
	//cout << "another node with size " << curnode->leaf_weight.size() << endl;
	if(curnode->leaf_weight.size()>0){//is a leaf?
		float* zeros = new float[numfeatures];
		for (int i=0;i<numfeatures;i++)
			zeros[i]=0;
		return zeros;
	}else{//a decision node
		//cout << "looking at " << curnode->chosen_d << endl;
		if(known[curnode->chosen_d]){
			//cout << "applying linear classifier ... ";
			//apply linear classifier
			if(datapoint[curnode->chosen_d]>curnode->w){//pos
				//cout << "going to pos" << endl;
				return eval_tree_importance(curnode->pos,datapoint,known,numfeatures);
			}else{//neg
				//cout << "going to neg" << endl;
				return eval_tree_importance(curnode->neg,datapoint,known,numfeatures);
			}
		}else{
			//cout << "It's unknown. clculating importance and adding it to pos_en[" << curnode->chosen_d << "]" << endl;
			//don't know which, so combine the two
			float* pos_en = eval_tree_importance(curnode->pos,datapoint,known,numfeatures);
			float* neg_en = eval_tree_importance(curnode->neg,datapoint,known,numfeatures);
			for (int i=0;i<numfeatures;i++)
				pos_en[i] = pos_en[i]+neg_en[i];
			delete[] neg_en;
			pos_en[curnode->chosen_d]=pos_en[curnode->chosen_d]+(curnode->entropy-((curnode->pos->entropy*curnode->pos->weight)+(curnode->neg->entropy*curnode->neg->weight))/curnode->weight);
			//cout << "(importance) if I knew " << curnode->chosen_d << " the entropy would (on average) go down by " << (curnode->entropy-((curnode->pos->entropy*curnode->pos->weight)+(curnode->neg->entropy*curnode->neg->weight))/curnode->weight) << endl;

			//cout << "returning " << pos_en+neg_en+1 << endl;
			return pos_en;
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
	srand(time(NULL)); /* seed random number generator */
	char* test_csv_filename;
	char* forest_filename;
	char* output_classification_filename;
	char* usefulness_filename;
	usefulness_filename=NULL;
	int num_trees;
	if( argc != 4 && argc != 5 )
	{
		cout << "\tusage: " << endl << argv[0] << " test_data.csv learned.forest output.classification" << endl << "or" << endl << argv[0] << " test_data.csv learned.forest output.classification usefulness.entropy" << endl;
		return -1;
	}else{
		test_csv_filename=argv[1];
		forest_filename=argv[2];
		output_classification_filename=argv[3];
		if(argc==5)
			usefulness_filename=argv[4];
			
	}
	string token, line;

	std::ifstream ifs(forest_filename);
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

	cout << "reading file " << test_csv_filename << endl;
	//read the file twice. Once to count elements per line and #lines
	ifstream file (test_csv_filename); 
	int rows=0;
	int cols=0;
	int incomplete_rows=0;
	getline (file, line);
	while ( file.good() )
	{
		if(line.find("?")!=string::npos)//if(std::count(line.begin(), line.end(), '?')>0)
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
	cout << "\nfile has " << rows << " rows with " << cols << " columns each, including " << incomplete_rows << " incomplete rows" << endl;

	file.clear();

	//save results to in csv format
	ofstream classification(output_classification_filename);
	//print labels, to know which order they are in
	classification << unique_labels[0];
	for (int j=1;j<unique_labels.size();j++)
		classification << "," << unique_labels[j];
	classification << endl;

	ofstream usefulness;
	if(usefulness_filename!=NULL)
		//save usefulness of each information in csv format
		usefulness.open(usefulness_filename,ios::binary);

	file.seekg(0, ios::beg);

	int i=0;
	int incomplete_i=0;
	getline (file, line);
	while ( file.good() )
	{
		float* total_importance;
		if(usefulness_filename!=NULL){
			total_importance = new float[cols];
			for (int i=0;i<cols;i++)
				total_importance[i] = 0;
		}
		std::istringstream ss(line);
		float datapoint_res[unique_labels.size()];
		float datapoint[cols];
		for (int j=0;j<unique_labels.size();j++)
			datapoint_res[j]=0.0f;
		if(line.find(" ")!=string::npos)
			cout << "Warning! There are spaces in your test csv." << endl;
		if(line.find("?")!=string::npos){//count(line.begin(), line.end(), '?')>0)//contains '?'
			//incomplete[incomplete_i++]=line;
			//cout << line << endl;
			bool known[cols];
			for(int j=0;j<cols;j++){
				std::getline(ss, token, ',');
				//std::cout << j << " - " << token;
				if(token.compare("?") != 0){
					known[j]=true;
					datapoint[j] = atof(token.c_str());
				}else{
					known[j]=false;
					datapoint[j] = 0.0f;
				}
				//cout << known[j] << "," << endl;
			}

			for (int j=0;j<num_trees;j++){
				vector<int*> tmp_res = eval_tree_unknowns(forest[j],datapoint,known);

				//cout << tmp_res.transpose() << endl;

				for (int k=0;k<unique_labels.size();k++)
					for (int m=0;m<tmp_res.size();m++)
						if(tmp_res.at(m)[0]==unique_labels[k])
							datapoint_res[k]=datapoint_res[k]+tmp_res.at(m)[1];
				for(int k=0;k<tmp_res.size();k++)
					delete[] tmp_res.at(k);

				
				if(usefulness_filename!=NULL){//calculate importance
					float* importance = eval_tree_importance(forest[j],datapoint, known, cols);
					for (int i=0;i<cols;i++){
						//cout << importance[i] << ", ";
						total_importance[i] += importance[i];
					}
					//cout << endl;
					delete[] importance;
					//cout << "importance is :" << importance << endl;
				}
			}
		}
		else{
			for(int j=0;j<cols;j++){
				std::getline(ss, token, ',');
				//std::cout << "(" << j << ", " << i << "):" << token << '\n';
				datapoint[j] = atof(token.c_str());
			}
			//cout << datapoint.transpose() << endl;
			
			for (int j=0;j<num_trees;j++){
				vector<int*> tmp_res = eval_tree(forest[j],datapoint);

				//cout << tmp_res.transpose() << endl;

				for (int k=0;k<unique_labels.size();k++)
					for (int m=0;m<tmp_res.size();m++)
						if(tmp_res.at(m)[0]==unique_labels[k])
							datapoint_res[k]=datapoint_res[k]+tmp_res.at(m)[1];
				for(int k=0;k<tmp_res.size();k++)
					delete[] tmp_res.at(k);
			}
			
		}
		classification << datapoint_res[0];
		for (int j=1;j<unique_labels.size();j++)
			classification << "," << datapoint_res[j];
		classification << endl;

		if(usefulness_filename!=NULL){
			usefulness << total_importance[i];
			for (int i=1;i<cols;i++)
				usefulness << "," << total_importance[i];
			usefulness << endl;
		}
		getline (file, line);
	}
	classification.close();
	if(usefulness_filename!=NULL)
		usefulness.close();	
}
