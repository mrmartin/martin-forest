#include <iostream>
#include <fstream>
#include <random>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

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

int main(int argc, char** argv)
{
	srand(time(NULL)); /* seed random number generator */
	char* filename;
	if( argc != 2)
	{
		cout << "\trequires name of .csv file" << endl;
		return -1;
	}else{
		filename=argv[1];
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
	std::cout << "X from file is:\n" << csv << endl;
	std::cout << "Y from file is:\n" << labels.transpose() << endl;

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

	cout << "\nStarting learning" << endl;
	vector<vector<int>> partitions=unconditioned_partition(num_unique);
	cout << "division: ";
	for(std::vector<int>::size_type i = 0; i != partitions[0].size(); i++) {
		printf("%d ",unique_labels[partitions[0][i]]);
	}
	printf(" - ");
	for(std::vector<int>::size_type i = 0; i != partitions[1].size(); i++) {
		printf("%d ",unique_labels[partitions[1][i]]);
	}cout << endl;

	MatrixXf X = csv;
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
	cout << relevant_rows << " rows are relevant to this." << endl;

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
		cout << endl << "dimension " << d << ":" << endl;
		for(int i=0;i<relevant_rows;i++)
			cout << binary_labels[i] << ", ";
		cout << endl;
		for(int i=0;i<relevant_rows;i++)
			cout << values[i] << ", ";
		cout << endl;

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
		cout << "sorted values :" << endl;
		for(int i=0;i<relevant_rows;i++)
			cout << binary_labels[i] << ", ";
		cout << endl;
		for(int i=0;i<relevant_rows;i++)
			cout << values[i] << ", ";
		cout << endl;

		float w=0;
		int best_error=relevant_rows;
		//place w between every pair, and calculate errors
		for(int i=1;i<relevant_rows;i++){
			int error=0;
			for(int j=0;j<i;j++)
				error+=binary_labels[j]?0:1;
			for(int j=i;j<relevant_rows;j++)
				error+=binary_labels[j]?1:0;
			cout << error << ",";
			//an error of 0 and an error of relevant_rows are both perfect, but the second would require flipping left and right. Since that doesn't matter, we can compare error to relevant_rows-error
			if(error<best_error || relevant_rows-error<best_error){//new best w
				w=(values[i]+values[i-1])/2.0;
				best_error=min(error,relevant_rows-error);
			}
		}
		cout << endl << "best divider is at " << w << ", with an error of " << best_error << endl;
	}
	cout << "... learning complete" << endl;
}
