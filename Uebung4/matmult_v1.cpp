#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>

#include <iostream>
#include <string>

//alloc_mat, init_mat, print_mat, check
#include "matrixutilities.cpp"

#define MASTER 0			// taskid von erstem task
#define FROM_MASTER 1		// Nachrichtentypen
#define FROM_WORKER 2

int main(int argc, char *argv[])
{
	/*
		Standard initializations
	*/

	int numtasks,		// Anzahl an Tasks
		taskid,			// Task ID
		numworkers, i,	// Anzahl an Arbeitern
		bsize, bpos,	// Zeilenabschnitt von Matrix A
		averow, extra;	// Berechnung von Zeilenabschnitten

	int D1, D2, D3;

	float **A, **B, **C, **C_serial;	// Matrizen

	/*
		Time measuring
	*/

	double 	start_overall, end_overall, overall,
			start_distribute, end_distribute, distribute,
			start_gather, end_gather, gather,
			computation;

	MPI_Status status;			// Statusvariable

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	/*
	extension for variable matricy-sizes
	*/

	if(argc != 4 )
	{
		std::cout 	<< "Usage:" << argv[0]
					<< "<int> for D1"	// Zeilenanzahl von A und C
					<< "<int> for D2"  // Spaltenanzahl von A und Zeilenanzahl von B
					<< "<int> for D3"  // Spaltenanzahl von B und C
					<< std::endl;

		MPI_Abort(MPI_COMM_WORLD, 0);

		exit(1);
	}

	/*
	for(int x = 1; x <= argc; ++x)
	{
		std::cout << "Testing" << std::endl;
		
		if(argv[x] < 0)
		{
			std::cout 	<< "Usage:" << argv[0]
						<< "<int> for D1"	// Zeilenanzahl von A und C
						<< "<int> for D2"  // Spaltenanzahl von A und Zeilenanzahl von B
						<< "<int> for D3"  // Spaltenanzahl von B und C
						<< "integers musst be greater than 0!"
						<< std::endl;
			MPI_Abort(MPI_COMM_WORLD, 0);
			exit(1);
		}
	}
	*/

	D1 = std::atoi(argv[1]);
	D2 = std::atoi(argv[2]);
	D3 = std::atoi(argv[3]);

	/*
		Exception catching
	*/

	if(numtasks < 2 )
	{
		std::cout << "Need at least two tasks!" << std::endl;

		MPI_Abort(MPI_COMM_WORLD, 0);

		exit(1);
	}

	/*
		Master task
	*/

	if(taskid == MASTER)
	{

		std::cout << "[Task " << taskid << " of " << numtasks << "] Hello from MASTER" << std::endl;

		std::cout << "Matrix-Multiplication started with " << numtasks << " tasks!" << std::endl;

		start_overall = MPI_Wtime();

		A = alloc_mat(D1, D2); init_mat(A, D1, D2);		// Speicher für Matrizen holen
		B = alloc_mat(D2, D3); init_mat(B, D2, D3);		// und initialisieren
		C = alloc_mat(D1, D3);

		C_serial = alloc_mat(D1, D3);

		start_distribute = MPI_Wtime();

		numworkers = numtasks-1;	// Anzahl der Arbeiter
		averow = D1 / numworkers;	// Mittlere Blockgröße
		extra = D1 % numworkers;	// Restzeilen

		for (i = 1, bpos = 0; i <= numworkers; i++, bpos += bsize)
		{
			if(i > extra) // Restzeilen aufteilen
			{
				bsize = averow;
			}
			else
			{
				bsize = averow + 1;
			}

			// Senden der Matrixblöcke an die Arbeiter
			//message("Sending " + NumberToString(bsize) + " rows to task " + NumberToString(i));

			std::cout << "[Task " << taskid << " of " << numtasks << "] Sending " << bsize << " rows to task " << i << std::endl;

			//MPI_SEND(data, amount of data, data type, task id, XXX, region)
			MPI_Send(&bpos,		1,			MPI_INT,	i,	1,	MPI_COMM_WORLD);
			MPI_Send(&bsize,	1,			MPI_INT,	i,	1,	MPI_COMM_WORLD);
			MPI_Send(A[bpos],	bsize*D2,	MPI_FLOAT,	i,	1,	MPI_COMM_WORLD);
			MPI_Send(B[0],		D2*D3,		MPI_FLOAT,	i,	1,	MPI_COMM_WORLD);
		}

		end_distribute = MPI_Wtime();
		start_gather = MPI_Wtime();

		/*
			retrieving the worker results
		*/
		for (i = 1; i <= numworkers; i++)	// Empfangen der Ergebnisse von den Arbeitern
		{

			MPI_Recv(&bpos,		1,			MPI_INT,	i,	2,	MPI_COMM_WORLD,	&status);
			MPI_Recv(&bsize,	1,			MPI_INT,	i,	2,	MPI_COMM_WORLD,	&status);
			MPI_Recv(C[bpos],	bsize*D3,	MPI_FLOAT,	i,	2,	MPI_COMM_WORLD,	&status);
			
			//message("Received results from task " + i); DOES NOT WORK BECAUSE OF BARRIER

			std::cout << "[Task " << taskid << " of " << numtasks << "] Received results from task " << i << std::endl;

			/* wtf received, eceived, ceived
			std::cout 	<< "[TASK " << taskid << " of " << numworkers << "] "
						<< "Received results from task " + i
						<< std::endl;
			*/
		}
		
		end_overall = MPI_Wtime();
		end_gather = MPI_Wtime();

		overall = end_overall - start_overall;

		distribute = end_distribute - start_distribute;

		gather = end_gather - start_gather;

		computation = overall - (distribute + gather);

		std::cout 	<< "The distribution took: " 	<< distribute 	<< " seconds!" << std::endl
					<< "The computation took: " 	<< computation 	<< " seconds!" << std::endl
					<< "The gathering took: " 		<< gather 		<< " seconds!" << std::endl
				 	<< "The whole process took: " 	<< overall 		<< " seconds!" << std::endl;

		/* 
			Ergebnis überprüfen
		*/

		std::cout << "Starting serial Matrix multiplication for comparison!" << std::endl;

		matmult_serial(A, B, C_serial, D1, D2, D3);

		if(check(C, C_serial, D1, D3))
		{
			std::cout << "The matrices are the same!" << std::endl;
		}
		else
		{
			std::cout << "The matrices are not the same!" << std::endl;
		}

	}

	/*
		Worker task
	*/

	if(taskid > MASTER)
	{
		std::cout << "[Task " << taskid << " of " << numtasks << "] Hello from a task!" << std::endl;

		MPI_Recv(&bpos,		1,	MPI_INT,	0,	1,	MPI_COMM_WORLD,	&status);
		MPI_Recv(&bsize,	1,	MPI_INT,	0,	1,	MPI_COMM_WORLD,	&status);
		
		A = alloc_mat(bsize,	D2);	// Speicher für die Matrixblöcke holen
		B = alloc_mat(D2,		D3);
		C = alloc_mat(bsize,	D3);

		MPI_Recv(A[0],	bsize*D2,	MPI_FLOAT,	0,	1,	MPI_COMM_WORLD,	&status);
		MPI_Recv(B[0],	D2*D3,		MPI_FLOAT,	0,	1,	MPI_COMM_WORLD,	&status);
		
		for(int i = 0; i < bsize; i++)
			for(int j = 0; j < D3; j++)
				for(int k = 0; k < D2; k++)
					C[i][j] += A[i][k] * B[k][j];
		
		MPI_Send(&bpos,		1,			MPI_INT,	0,	2,	MPI_COMM_WORLD);
		MPI_Send(&bsize,	1,			MPI_INT,	0,	2,	MPI_COMM_WORLD);
		MPI_Send(C[0],		bsize*D3,	MPI_FLOAT,	0,	2,	MPI_COMM_WORLD);
	}

	/*
		Finalization
	*/
	
	MPI_Finalize();
}