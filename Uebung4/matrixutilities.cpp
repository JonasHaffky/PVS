// allocate space for empty matrix A[row][col]
// access to matrix elements possible with:
// - A[row][col]
// - A[0][row*col]

float **alloc_mat(int row, int col)
{
	float **A1, *A2;

	A1 = (float **)calloc(row, sizeof(float *));		// pointer on rows
	A2 = (float *)calloc(row*col, sizeof(float));		// all matrix elements
	for (int i = 0; i < row; i++)
		A1[i] = A2 + i*col;

	return A1;
}

// ---------------------------------------------------------------------------
// random initialisation of matrix with values [0..9]

void init_mat(float **A, int row, int col)
{
	for (int i = 0; i < row*col; i++)
		A[0][i] = (float)(rand() % 10);
}

// ---------------------------------------------------------------------------
// DEBUG FUNCTION: printout of all matrix elements

void print_mat(float **A, int row, int col, char *tag)
{
	int i, j;

	printf("Matrix %s:\n", tag);
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++) 
			printf("%6.1f   ", A[i][j]);
		printf("\n"); 
	}
}

//Vergleich von zwei Matrizen
bool check(float **A,float **B, int row, int col)
{
	for(int i = 0; i < row; ++i)
		for (int j = 0; j < col; ++j)
			if(A[i][j] != B[i][j])
			{
				return false;
				break;
			}
	return true;
}

void matmult_serial(float **A, float **B, float **C, int D1, int D2, int D3)
{
	for(int i = 0; i < D1; i++)
		for(int j = 0; j < D3; j++)
			for(int k = 0; k < D2; k++)
				C[i][j] += A[i][k] * B[k][j];
	
	std::cout << "Serial multiplication: Done!" << std::endl;
}

//prints out a message and the taskid
void message(std::string message)
{
	int task_id;

	MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

	int size;

	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/*
		still under construction
	*/

	//MPI_Barrier(MPI_COMM_WORLD);
	
	std::cout 	<< "[TASK " << task_id << " of " << size << "] "
				<< message
				<< std::endl;
}

#include <sstream>

template <typename T>
std::string NumberToString(T number)
{
	std::ostringstream os;
	os << number;
	return os.str();
}