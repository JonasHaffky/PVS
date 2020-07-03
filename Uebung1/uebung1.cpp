# include <stdio.h>
# include <omp.h>

int main(int argc, char* argv[])
{
    int numThreads;
    int threadID;
    float start, end;

    start = omp_get_wtime();

    #pragma omp parallel num_threads(23)
    {
        threadID = omp_get_thread_num();
        printf("Hello from thread %d\n", threadID);

        if (threadID == 0)
        {
            numThreads = omp_get_num_threads();
            printf("Number of threads: %d\n", numThreads);
        }
    }
    end = omp_get_wtime();
    printf("This task took %f seconds\n", end-start);

    return 0;
}