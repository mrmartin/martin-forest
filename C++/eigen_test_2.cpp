#include <iostream>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;
int main(int argc, char** argv)
{
	int size;
	if( argc != 2)
	{
		cout << "\trequires number of elements argument" << endl;
		return -1;
	}else{
		size=atoi(argv[1]);
	}

	MatrixXf m(size,size); // a (size)x(size+1)-matrix of int's
	for (int j=0; j<m.cols(); ++j) // loop over columns
		for (int i=0; i<m.rows(); ++i) // loop over rows
			m(i,j) = i+j*m.rows(); // to access matrix coefficients,
			// use operator()(int,int)
	VectorXf v(size);
	for (int j=0; j<v.size(); ++j) // loop over columns
		v(j)=j;
	cout << "Here is the matrix m:\n" << m << endl;
	cout << "Here is the vector v':\n" << v.transpose() << endl;
	VectorXf w = m.colPivHouseholderQr().solve(v);
	cout << "The solution is:\n" << w.transpose() << endl;

	//now, find w*m
	VectorXf e = m*w;
	cout << "w*m transpose is:\n" << e.transpose() << endl;
}
