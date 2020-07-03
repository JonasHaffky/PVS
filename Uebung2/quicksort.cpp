#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define NUM 32767                                             // Elementanzahl

// ---------------------------------------------------------------------------
// Vertausche zwei Zahlen im Feld v an der Position i und j

void swap(float *v, int i, int j)
{
    float t = v[i]; 
    v[i] = v[j];
    v[j] = t;
}

// ---------------------------------------------------------------------------
// Methode, die prüft ob der Vektor richtig sortiert wurde

bool isSorted(float *v)
{
    int i;
    for(i=0;i<NUM;i++){
        if(v[i]<v[i+1]){
            return true;
        }else{
            return false;
        }
    }
}

// ---------------------------------------------------------------------------
// Serielle Version von Quicksort (Wirth) 

void quicksort(float *v, int start, int end) 
{
    int i = start, j = end;
    float pivot;

    pivot = v[(start + end) / 2];                         // mittleres Element
    do {
        while (v[i] < pivot)
            i++;
        while (pivot < v[j])
            j--;
        if (i <= j) {               // wenn sich beide Indizes nicht beruehren
            swap(v, i, j);
            i++;
            j--;
        }
   } while (i <= j);
   if (start < j)                                        // Teile und herrsche
       quicksort(v, start, j);                      // Linkes Segment zerlegen
   if (i < end)
       quicksort(v, i, end);                       // Rechtes Segment zerlegen
}

// ---------------------------------------------------------------------------
// Hauptprogramm

int main(int argc, char *argv[])
{
    float *v;                                                         // Feld                               
    int iter;                                                // Wiederholungen
    float start, end;                                       //variables for timer             

    if (argc != 2) {                                      // Benutzungshinweis
        printf ("Vector sorting\nUsage: %s <NumIter>\n", argv[0]); 
        return 0;
    }
    iter = atoi(argv[1]);                               
    v = (float *) calloc(NUM, sizeof(float));        // Speicher reservieren

    printf("Perform vector sorting %d times...\n", iter);
    start = omp_get_wtime();
    for (int i = 0; i < iter; i++) {               // Wiederhole das Sortieren
        for (int j = 0; j < NUM; j++)      // Mit Zufallszahlen initialisieren
            v[j] = (float)rand();

        quicksort(v, 0, NUM-1);                                  // Sortierung
        if(isSorted(v)==false){
            printf("Das war wohl nichts mit dem sortieren!");
        }
    }
    end = omp_get_wtime();
    printf("This task took %f seconds\n", end-start);
    printf ("\nDone.\n");
    return 0;
}