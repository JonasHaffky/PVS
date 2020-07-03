#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MASTER 0        //taskid vom ersten task
#define FROM_MASTER 1   //Nachrichtentypen
#define FROM_MASTER 2
#define D1 1000         //Zeilenanzahl von A und C
#define D2 1000         //Spaltenanzahl von A und Zeilenanzhal von B
#define D3 1000         //Spaltenanzahl von B und C



// ---------------------------------------------------------------------------
// allocate space for empty matrix A[row][col]
// access to matrix elements possible with:
// - A[row][col]
// - A[0][row*col]

float **alloc_mat(int row, int col)
{
    float **A1, *A2;

	A1 = (float **)calloc(row, sizeof(float *));		// pointer on rows
	A2 = (float *)calloc(row*col, sizeof(float));    // all matrix elements
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

// ---------------------------------------------------------------------------

int main (int argc, char *argv[])
{
    int numtask,                //Anzahl an Tasks
        taskid,
        numworkers, i, j, k,          //Anzahl an Arbeitern
        bsize, bpos,            //Zeilenabschnitt von Matrix A
        averow, extra;          //Berechnung von Zeilenabschnitten
    float **A, **B, **C, **D;   //Matrizen
    MPI_Status status;          //Statusvariable

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtask);
    if (numtask < 2){
        printf("Need at least two tasks!\n");
        MPI_Abort(MPI_COMM_WORLD, 0); exit(1);
    }

    //***********************Master Task*********************************************
    if (taskid == MASTER){      //Master
        printf("MatMult started with %d tasks.\n", numtask);
        A = alloc_mat(D1, D2); init_mat(A, D1, D2);     //Speicher für Matrizen holen und Initialisieren
        B = alloc_mat(D2, D3); init_mat(B, D2, D3);
        C = alloc_mat(D1, D3);

        float start = MPI_Wtime();            //Zeitmessung starten
        numworkers = numtask - 1;       //Anzahl der Arbeiter
        averow = D1 / numworkers;       //Mittlere Blockgröße
        extra = D1 % numworkers;        //Restzeilen

        for (i = 1, bpos = 0; i <= numworkers; i++, bpos += bsize){
            if (i > extra){             //Restzeilen aufteilen
                bsize = averow;
            } else {
                bsize = averow + 1;
            }
            printf("Sending %d rows to task %d\n", bsize, i);
            MPI_Send(&bpos, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            MPI_Send(&bsize, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            MPI_Send(A[bpos], bsize*D2, MPI_FLOAT, i, 1, MPI_COMM_WORLD);
            MPI_Send(B[0], D2*D3, MPI_FLOAT, i, 1, MPI_COMM_WORLD);
        }
        for (i = 1; i <= numworkers; i++){      //Empfangen der Ergebnisse der Arbeiter
            MPI_Recv(&bpos, 1, MPI_INT,i, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(&bsize, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(C[bpos], bsize*D3, MPI_FLOAT, i, 2, MPI_COMM_WORLD, &status);
            printf("Received results from task %d\n", i);
        }
        printf("\nUsed %f seconds.\n", MPI_Wtime()-start);
    }

    //***********************Arbeiter Task*******************************************
    if (taskid > MASTER){           //Worker
        MPI_Recv(&bpos, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&bsize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        A = alloc_mat(bsize, D2);           //Speicher für die Matrixblöcke holen
        B = alloc_mat(D2, D3);
        C = alloc_mat(bsize, D3);
        MPI_Recv(A[0], bsize*D2, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(B[0], D2*D3, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, &status);

        for ( i = 0; i < bsize; i++){
            for (j = 0; j < D3; j++){
                for (k = 0; k < D2; k++){
                    C[i][j] += A[i][k] * B[k][j];
                }
            }
        }

        MPI_Send(&bpos, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&bsize, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(C[0], bsize*D3, MPI_FLOAT, 0, 2, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}