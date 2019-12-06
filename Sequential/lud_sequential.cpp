#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iomanip>
using namespace std;

void print_matrix(float**, int);

//print the matrix out
void print_matrix(float** matrix, int size)
{
	//for each row...
	for (int i = 0; i < size; i++)
	{
		//for each column
		for (int j = 0; j < size; j++)
		{
			//print out the cell
			cout << left << setw(9) << setprecision(3) << matrix[i][j] << left <<  setw(9);
		}
		//new line when ever row is done
		cout << endl;
	}
}

//do LU decomposition
//a is the matrix that will be split up into l and u
//array size for all is size x size
void l_u_d(float** a, float** l, float** u, int size)
{
	//for each column...
	for (int i = 0; i < size; i++)
	{
		//for each row....
		for (int j = 0; j < size; j++)
		{
			//if j is smaller than i, set l[j][i] to 0
			if (j < i)
			{
				l[j][i] = 0;
				continue;
			}
			//otherwise, do some math to get the right value
			l[j][i] = a[j][i];
			for (int k = 0; k < i; k++)
			{
				//deduct from the current l cell the value of these 2 values multiplied
				l[j][i] = l[j][i] - l[j][k] * u[k][i];
			}
		}
		//for each row...
		for (int j = 0; j < size; j++)
		{
			//if j is smaller than i, set u's current index to 0
			if (j < i)
			{
				u[i][j] = 0;
				continue;
			}
			//if they're equal, set u's current index to 1
			if (j == i)
			{
				u[i][j] = 1;
				continue;
			}
			//otherwise, do some math to get the right value
			u[i][j] = a[i][j] / l[i][i];
			for (int k = 0; k < i; k++)
			{
				u[i][j] = u[i][j] - ((l[i][k] * u[k][j]) / l[i][i]);
			}
			
		}
	}
}

//initialize the matrices
void initialize_matrices(float** a, float** l, float** u, int size)
{
	//for each row in the 2d array, initialize the values
	for (int i = 0; i < size; ++i)
	{
		a[i] = new float[size];
		l[i] = new float[size];
		u[i] = new float[size];
	}
}

//fill the array with random values (done for a)
void random_fill(float** matrix, int size)
{
	//fill a with random values
	cout << "Producing random values " << endl;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			matrix[i][j] = ((rand()%10)+1) ;
		}
	}

	//Ensure the matrix is diagonal dominant to guarantee invertible-ness
	//diagCount well help keep track of which column the diagonal is in
	int diagCount = 0;
    float sum = 0;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			//Sum all column vaalues
			sum += abs(matrix[i][j]);
		}
		//Remove the diagonal  value from the sum
		sum -= abs(matrix[i][diagCount]);
		//Add a random value to the sum and place in diagonal position
		matrix[i][diagCount] = sum + ((rand()%5)+1);
		++diagCount;
        sum = 0;
	}
}


int main(int argc, char** argv)
{
	double runtime;
	//seed rng
	srand(1);

	//size of matrix
	int size = atoi(argv[1]);

	//initalize matrices
	float** a = new float* [size];
	float** l = new float* [size];
	float** u = new float* [size];
	initialize_matrices(a, l, u, size);
	//fill a with random values
	random_fill(a, size);
	//print A
	cout << "A Matrix: " << endl;
	print_matrix(a, size);
	//do LU decomposition
	runtime = clock()/(double)CLOCKS_PER_SEC;
	l_u_d(a, l, u, size);
	runtime = (clock()/(double)CLOCKS_PER_SEC ) - runtime;
	//print l and u
	cout << "L Matrix: " << endl;
	print_matrix(l, size);
	cout << "U Matrix:" << endl;
	print_matrix(u, size);
	cout << "Runtime: " << runtime << "seconds"<< endl;
	return 0;
}

