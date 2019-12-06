#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <cuda.h>
#include <iostream>
#include <iomanip>
#include <time.h>

using namespace std;
#define TILE 16


/* LU Decomposition using Shared Memory \
\           CUDA                        \            
\										\	
\ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/  


//Initialize a 2D matrix   
void initialize_matrices(double** a, double** l, double** u, int size)
{
  //for each row in the 2d array, initialize the values
  for (int i = 0; i < size; ++i)
  {
    a[i] = new double[size];
    l[i] = new double[size];
    u[i] = new double[size];
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Scale the index for threads to get pivot starting and ending points
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
__global__ void scaleIndex(double *matrix, int n, int index){
  int start=(index*n+index);
	int end=(index*n+n);
	
	for(int i= start+1 ; i<end; ++i){
		matrix[i]=(matrix[i]/matrix[start]);
	}

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Row elimination Kernel - takes matrix, dimension, currect row index, and block size
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
__global__ void elim(double *A, int n, int index, int bsize){
	extern __shared__ double pivot[];

	int idThread=threadIdx.x;
	int idBlock=blockIdx.x;
	int blockSize=bsize;


	if(idThread==0){
	     for(int i=index;i<n;i++) pivot[i]=A[(index*n)+i];
	}

	__syncthreads();
  //Varitables for pivot, row, start and end
	int pivotRow=(index*n);
	int currentRow=(((blockSize*idBlock) + idThread)*n);
	int start=currentRow+index;
	int end=currentRow+n;
  //If greater than pivot row, loop from start index + 1(next row) to end of column
	if(currentRow >pivotRow){
    for(int i= start+1; i<end; ++i){
        //Set the matrix value of next row and its column - pivot
        A[i]=A[i]-(A[start]*pivot[i-currentRow]);

             }
      }
}
//Randomly generated diagonal dominant (non-singular) matrix - 1D
void fillMatrix(double* a, int n){
  // Fill the matrix
   for (int i = 0; i <= (n*n); ++i) {
    a[i] =((rand()%10)+1);
  }

  //Make the matrix diagonally dominant to guarantee it is non-singular (invertible)
  int diagCount = 0;
  double sum = 0;
  for(int i = 0; i < n; ++i){
    //Iterate through the row, add all the values, remove the diagonal value
    for(int j = i*n; j < i*n + n; ++j){
      sum += abs(a[j]);
      //printf("%f +", sum);
    }
    ///Remove the diagonal value
    //i*n gives us the current row, then  add diagCount to get to correct column
    sum -= abs(a[i*n + diagCount]);
    //Add random value to the new sum, this guarantees diagonal is now larger than row sum
    a[i*n + diagCount] = sum + ((rand()%5)+1);
    ++diagCount;
    sum = 0;
  }

}

//----------------------------------------------------------------------- 
//Print 1D Matrix
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void printMatrix(double* a, int n){
    for(int i=0; i<(n*n); ++i){
           if(i%n==0)
       		   cout << endl << left << setw(9) << setprecision(3) << a[i] << left <<  setw(9);
           else cout << left << setw(9) << setprecision(3) << a[i] << left <<  setw(9);
         }
    printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    cout << endl;
}
//----------------------------------------------------------------------- 
//Print 2D Matrix
//----------------------------------------------------------------------- 
void print2D(double** matrix, int size)
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

int main(int argc, char** argv){
  //Matrix dimension will be n*n
  int n = atoi(argv[1]);

  srand(1);

  //Allocate A matrix, U, and L  for CPU
  double *a = new double[n*n];
  double *ret=new double[n*n];

  //Fill in diagonal dominant matrix, then print
  fillMatrix(a, n);
 

  //Allocate GPU memory for A matrix, get number of blocks
  double *da;
  int numblock = n/TILE + ((n%TILE)?1:0);

  double runtime;
  runtime = clock()/(double)CLOCKS_PER_SEC;
  cudaMalloc(&da, n*n* sizeof (double));
  cudaMemcpy(da, a, n*n*sizeof(double), cudaMemcpyHostToDevice);
  for(int i=0; i<n; ++i){
      scaleIndex<<<1,1>>>(da,n,i);
	    elim<<<numblock,TILE,n*sizeof(double)>>>(da,n,i,TILE);
   }
  cudaMemcpy(ret, da, n*n*sizeof(double),cudaMemcpyDeviceToHost );
  //printf("Returned Matrix\n");
  //printMatrix(ret,n);
  runtime = clock() - runtime;
  printf("For %u x %u Matrix\n",n,n);
  cout << "Runtime for LU Decomposition is: " << (runtime)/float(CLOCKS_PER_SEC) << endl;
  //Create 2D matrices for the L and U
  double** A = new double* [n];
  double** u = new double* [n];
  double** l = new double* [n];
 
  //Initialize these matrices, and transfer ret values into A
  initialize_matrices(A,u,l,n);
  for(int i = 0 ;i < n ; ++i){
    for(int j= 0; j < n; ++j){
      A[i][j]=ret[i*n+j];
    }
  }
  //Take values diagonal values from returned array and pull L and U
  for(int i=0; i<n; i++){
    for(int j=0; j<n; j++){
        //Find diagonals
        for(int k=0; k<n; k++){
            //If the outermost for loop is larger or equal to k, then grab L values
            if(i>=k)
              l[i][k] = A[i][k];
            //Else the rest of the array is zeroes
            else l[i][k] = 0;
            //If loops at diagonal then enter 1 for U, if j > k then we're on upper part 
            //of Matrix so fill in values, 
            if(k==j)u[k][j] = 1;
            else if(k<j)u[k][j] = A[k][j];
            else u[k][j] = 0.0;
        }
    }
  }
  //Print L and U if user specified
 if(atoi(argv[2]) == 1){
  printf("Matrix 'A' is:\n");
  printMatrix(a,n);  
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  printf("Matrix 'L' is:\n");
  print2D(l,n);
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  printf("Matrix 'U' is: \n");
  print2D(u,n);
 }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //Code from here on out is borrowed from Lab 1 to check for validity of L and U
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



  cudaFree(da);
  cudaFree(ret);
  delete[] a;
  delete[] ret; 

  return 0;
}
