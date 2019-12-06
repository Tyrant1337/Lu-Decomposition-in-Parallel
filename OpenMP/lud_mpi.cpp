#include <iostream>
#include <iomanip>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string>

using namespace std;

void transpose_matrix(float * matrix, float * transposed_matrix, int size)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++ )
		{
			transposed_matrix[i * size + j] = matrix[j * size + i];
		}
	}
}

void get_column(float* matrix, float* column, int size, int column_index)
{
	for (int i = 0; i < size; i++)
	{
		column[i] = matrix[i * size + column_index];
	}
}

//col is of length size*cols_per_thread
void get_all_columns(float* matrix, float* col, int size, int process_num, int cols_per_thread)
{
	int index;
	for (int sub_col = 0; sub_col < cols_per_thread; sub_col++)
	{
		for (int row_index = 0; row_index < size; row_index++)
		{
			col[sub_col * size + row_index] = matrix[(row_index * size) + (process_num * cols_per_thread) + sub_col];
			//index = sub_col*size + row_index ;
			//cout << index << endl; //": " << col[index] << endl;
			//index = (row_index * size) + (process_num * cols_per_thread) + sub_col;
			//cout << index << ": " << matrix[index] << endl;
			
		}
	}
}

//the size of the rows should be size*rows_per_thread
void get_all_rows(float* matrix, float* row, int size, int process_num, int rows_per_thread)
{
	for (int sub_row = 0; sub_row < rows_per_thread; sub_row++)
	{
		for (int col_index = 0; col_index < size; col_index++)
		{
			row[sub_row * size + col_index] = matrix[ (process_num * rows_per_thread * size) + ( sub_row * size )   + col_index ];
		}
	}
}

void get_row(float* matrix, float* row, int size, int row_index)
{
	for (int i = 0; i < size; i++)
	{
		row[i] = matrix[row_index * size + i ];
	}
}


//print the matrix out
void print_matrix(float* matrix, int size)
{
	//for each row...
	for (int i = 0; i < size; i++)
	{
		//for each column
		for (int j = 0; j < size; j++)
		{
			//print out the cell
			cout << left << setw(9) << setprecision(3) << matrix[i*size + j] << left <<  setw(9);
			//cout << matrix[i * size + j] << " ";
			//cout << i << " " << j << endl;
		}
		//new line when ever row is done
		cout << endl;
	}
}

void fill_process_row(float* matrix, float* process_row, int size, int process_num)
{
	for(int i = 0; i < size; i++)
	{
		process_row[i] = matrix[process_num * size + i];
	}
}

//fill the array with random values (done for a)
void zero_fill(float* matrix, int size)
{
	//fill a with random values
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			matrix[i * size + j] = 0;
		}
	}
}

//do LU decomposition
//a is the matrix that will be split up into l and u
//array size for all is size x size
void l_u_d(float* a, float* l, float* u, int size, int process_num, int num_of_processes)
{


	int rows_per_thread = size/num_of_processes;

	//for sending whole matrices to each process
	float* a2 = new float[size*size];
	float* l2 = new float[size*size];
	float* u2 = new float[size*size];

	//for process 0 to receive the a matrix back (for testing)
	//float* a3 = new float[size*size];
	
	//these are for the rows that each will calculate
	float* l3 = new float[size*rows_per_thread];
	float* u3 = new float[size*rows_per_thread];
	
	//for returning the transpose of u
	//take the transpose of it to get u
	float* ut = new float[size*size];

	if (process_num == 0)
	{
		a2 = a;
		zero_fill(ut, size);
		
	}
	
	//initialize all indexes to 0
	zero_fill(l2, size);
	zero_fill(u2, size);
	

	//send a2 from process 0 to each other thread
	MPI_Bcast(a2, size*size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	//no need to bcast l and u right now, they're empty
	
	//for each column...
	for (int i = 0; i < size; i++)
	{

		//first round of stuff	
		//indenting for clarity, still in same scope (used to be a for loop here)

		//New loop for handling multiple rows per thread
		//	
		for (int j = process_num*rows_per_thread; j < (process_num + 1)*rows_per_thread; j++)
		{

			//here, process_num corresponds to row
			if (j < i)
			{
				l2[j*size + i] = 0;
			}
			else
			{
				//otherwise, do some math to get the right value
				l2[j*size + i] = a2[j*size + i];
				for (int k = 0; k < i; k++)
				{
					//deduct from the current l cell the value of these 2 values multiplied
					l2[j*size + i] = l2[j * size + i] - l2[j*size + k] * u2[k * size + i];
				}
			}

		}
		
		//only updated l in the above, so need to gather l back to process 0
		//first, get the row each process is working with
		get_all_rows(l2, l3, size, process_num, rows_per_thread);
		//now send that row in a gather to process 0's l2 
		MPI_Gather(l3, size*rows_per_thread, MPI_FLOAT, l2, size*rows_per_thread, MPI_FLOAT, 0, MPI_COMM_WORLD);
		//now bcast out the new l2 to each thread
			
		//if (process_num == 0)
		//{
			//cout << "l2 round " << i << endl;
			//print_matrix(l2, size);
		//}
		

		MPI_Bcast(l2, size*size, MPI_FLOAT, 0, MPI_COMM_WORLD);
		//cout << process_num << " reached here 0" << endl;
		//if (process_num == 0)
		//{
			//cout << "l2 round " << i << endl;
			//print_matrix(l2, size);
		//}
		
		//second round of stuff	

		//new for loop for going through number of processes
		for (int j = process_num*rows_per_thread; j < (process_num + 1)*rows_per_thread; j++)
		{

			//here, process_num refers to column 
			if (j < i)
			{
				u2[i*size + j] = 0;
			}
			//if they're equal, set u's current index to 1
			else if (j == i)
			{
				u2[i*size + j] = 1;
			}
			else
			{
				//if (process_num == 1)
				//{
					//cout << "l2 round new middle" << i << endl;
					//print_matrix(l2, size);
				//}
				//otherwise, do some math to get the right value
				u2[i*size + j] = a2[i*size + j] / l2[i* size + i];
				for (int k = 0; k < i; k++)
				{
					u2[i* size + j] = u2[i* size + j] - ((l2[i * size + k] * u2[k * size + j]) / l2[i * size + i]);
				}
			}
		}

		//if (process_num == 1)
		//{
			//cout << "u2 middle round " << i << endl;
			//print_matrix(u2, size);
		//}
		

		//cout << process_num << " reached here 1" << endl;
		//only updated u2, so do a gather back to 
		//but have to gather columns, 
		get_all_columns(u2, u3, size, process_num, rows_per_thread);
		//now send the columns to process 0's ut matrix
		MPI_Gather(u3, size*rows_per_thread, MPI_FLOAT, ut, size*rows_per_thread, MPI_FLOAT, 0, MPI_COMM_WORLD);
		//cout << process_num << " reached here 2" << endl;
		//process 0 transposes ut to and puts it in its u2
		//if (process_num == 0)
		//{
			//cout << "process 0 reached here 3" << endl;
			//print_matrix(ut, size);
			transpose_matrix(ut, u2, size);
			//cout << "u2 round " << i << endl;
			//print_matrix(u2, size);
			//cout << "still going " << i << endl;
		//}
		//cout << process_num << " reached here 4" << endl;
		//now send the updated u2 back out to the other threads
		MPI_Bcast(u2, size*size, MPI_FLOAT, 0, MPI_COMM_WORLD);
		//cout << "reached here 2" << endl;
		
		

	}
	

	//MPI_Barrier(MPI_COMM_WORLD);
	
	
	if (process_num == 0)
	{
		//print l and u
		cout << "L Matrix: " << endl;
		//print_matrix(l2, size);
		cout << "U Matrix:" << endl;
		//print_matrix(u2, size);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	

	
}

/*
//initialize the matrices
void initialize_matrices(float** a, float** l, float** u, int size)
{
	//for each row in the 2d arrays, initialize the values in each of the three arrays
	for (int i = 0; i < size; ++i)
	{
		a[i] = new float[size];
		l[i] = new float[size];
		u[i] = new float[size];
	}
}
*/

//fill the array with random values (done for a)
void random_fill(float* matrix, int size, int max)
{
	//fill a with random values
	cout << "Producing random values " << endl;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			matrix[i * size + j] = ((rand()%10)+1) ;
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
			sum += abs(matrix[i * size + j]);
		}
		//Remove the diagonal  value from the sum
		sum -= abs(matrix[i * size + diagCount]);
		//Add a random value to the sum and place in diagonal position
		matrix[i * size + diagCount] = sum + ((rand()%5)+1);
		++diagCount;
        sum = 0;
	}

}


int main(int argc, char* argv[])
{
	//seed rng
	srand(1);

	//size of matrix, hardcoded for now
	int size = atoi(argv[1]);;

	//initalize matrices
	float* a, * l, * u;
	a = new float[size * size];
	l = new float[size * size];
	u = new float[size * size];
	double run_time;
	//fill a with random values
	//Mpi fun stuff
	int num_of_processes, process_num;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_of_processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_num);
	if (process_num == 0)
	{
		random_fill(a, size, 6);
		//print A
		cout << "A Matrix: " << endl;
		//print_matrix(a, size);
		//cout << "Number of processes: " << num_of_processes << endl;
		run_time = MPI_Wtime();
	}	
	//cout << process_num << endl;
	//do LU decomposition
	l_u_d(a, l, u, size, process_num, num_of_processes);
	if (process_num == 0)
	{
		run_time = MPI_Wtime() - run_time;
		cout << fixed << setprecision(3) << run_time;
	}
	MPI_Finalize();
	return 0;
	
}

