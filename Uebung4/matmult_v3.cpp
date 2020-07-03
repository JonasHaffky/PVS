#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <string>

//alloc_mat, init_mat, print_mat, check
#include "matrixutilities.cpp"

#define MASTER 0			// taskid von erstem task
#define FROM_MASTER 1		// Nachrichtentypen
#define FROM_WORKER 2
#define D1 1000				// Zeilenanzahl von A und C
#define D2 1000				// Spaltenanzahl von A und Zeilenanzahl von B
#define D3 1000				// Spaltenanzahl von B und C


double start;

int main(int argc, char *argv[])
{
	int numtasks,		// Anzahl an Tasks
		taskid,			// Task ID
		numworkers, i,	// Anzahl an Arbeitern
		bsize, bpos,	// Zeilenabschnitt von Matrix A
		averow, extra,	// Berechnung von Zeilenabschnitten
		blksz;			// HIER KOMMT NOCH WAS HIN

	float **A, **B, **C, **C_serial;	// Matrizen

	MPI_Status status;			// Statusvariable

	/*
		Standard initializations
	*/

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	blksz = D1/numtasks;
	/*
		Exception catching
	*/

	// if(numtasks < 2 )
	// {
	// 	std::cout << "Need at least two tasks!" << std::endl;

	// 	MPI_Abort(MPI_COMM_WORLD, 0);

	// 	exit(1);
	// }

	/*
		Master task
	*/

	A = alloc_mat(D1, D2); init_mat(A, D1, D2);		// Speicher für Matrizen holen
	B = alloc_mat(D2, D3); init_mat(B, D2, D3);		// und initialisieren
	C = alloc_mat(D1, D3);
	C_serial = alloc_mat(D1, D3);


	
	MPI_Barrier(MPI_COMM_WORLD);	//barrier for time stamp

	start = MPI_Wtime();		// Zeitmessung starten

	

	MPI_Scatter(A, blksz * D2, MPI_FLOAT, C, blksz * D2, MPI_FLOAT, 0, MPI_COMM_WORLD);
	
	MPI_Bcast(B, D2 * D3, MPI_FLOAT, 0, MPI_COMM_WORLD);

	for(int i = 0; i < D1; i++)
		for(int j = 0; j < D3; j++)
			for(int k = 0; k < D2; k++)
				C[i][j] += A[i][k] * B[k][j];

	
	//MPI_Gather(C, blksz, MPI_FLOAT, A, blksz * D2, MPI_FLOAT, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);


	std::cout << "Used " << MPI_Wtime() - start << " seconds!" << std::endl; // Zeitmessung anhalten

	
	// if(taskid == MASTER)
	// {


	// start = MPI_Wtime();		// Zeitmessung starten

	// 		matmult_serial(A, B, C_serial, D1, D2, D3);
		
	// 		if(check(C, C_serial, D1, D3))
	// 		{
	// 			std::cout << "same!" << std::endl;
	// 		}
	// 		else
	// 		{
	// 			std::cout << "not same!" << std::endl;
	// 		}

	// std::cout << "Used " << MPI_Wtime() - start << " seconds!" << std::endl; // Zeitmessung anhalten

	// }



	MPI_Finalize();

}