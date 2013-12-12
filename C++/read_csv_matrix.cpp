#include <iostream>
#include <fstream>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;
int main(int argc, char** argv)
{
	char* filename;
	if( argc != 2)
	{
		cout << "\trequires name of .csv file argument" << endl;
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
	std::cout << "matrix from file is:\n" << csv << endl;



	/*VectorXf v(cols);
	for (int j=0; j<v.size(); ++j) // loop over columns
		v(j)=j+1;
	cout << "Here is the vector v':\n" << v.transpose() << endl;
	VectorXf w = csv.colPivHouseholderQr().solve(v);
	cout << "The solution is:\n" << w.transpose() << endl;

	//now, find w*m
	VectorXf e = csv*w;
	cout << "w*m transpose is:\n" << e.transpose() << endl;*/
}
