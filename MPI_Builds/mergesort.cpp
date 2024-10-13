#include "mpi.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <limits.h>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include "helper.h"

// Function declarations
void mergesort(int array[], int left, int right);
void merge(int array[], int left, int middle, int right);
int* mergeArrays(int a[], int b[], int n, int m);
void p2a(int a[], int* b, int size);
void l2g(int a[], int b[], int size);
void printArray(int array[], int size);

// Add caliper annotations
// Lab 2- mrg start, calicxxmarkfunction, and addiak 
// mgr.stop, flush, finalize

// Main function

int main(int argc, char* argv[]) {
    
    //get input from command line
    //format is mpirun -np 4 ./mergesort 16 sorted

    //get value for num
    int NUM = atoi(argv[1]);
    std::cout << "NUM aka number of elements in array: " << NUM << std::endl;

    //get value for type
    std::string type = argv[2];
    int*a;

    if (type == "sorted") {
        a = sortedArray(NUM);
    }
    else if (type == "random") {
        a = randomArray(NUM);
    }
    else if (type == "reverse") {
        a = reverseArray(NUM);
    }
    else if (type == "perturbed") {
        a = perturbedArray(NUM);
    }
    else {
        std::cout << "Invalid type\n";
        return 0;
    }
    printArray(a,NUM);
    
    
    int i, a_size = NUM, local_size;
    int numtasks, rank;
    int* global = new int[NUM];
    int* comp;
    MPI_Status Stat;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    // Local array for each process
    int* local = new int[NUM / numtasks];
    srand(static_cast<unsigned int>(time(NULL)));


    std::cout << "Original array:\n";
    //printArray(a, NUM);

    // Scatter and split array evenly for each process
    MPI_Scatter(a, NUM / numtasks, MPI_INT, local, NUM / numtasks, MPI_INT, 0, MPI_COMM_WORLD);
    local_size = NUM / numtasks;

    // Parent process
    if (rank == 0) {
        double begin, end, time_spent;
        begin = MPI_Wtime();

        // Sequential merge sort
        mergesort(local, 0, local_size - 1);

        // Push sorted local array to global array
        l2g(global, local, local_size);
        int j, recv_size = local_size;
        int* buff = new int[recv_size];

        for (j = 0; j < numtasks - 1; j++) {
            // Receive sorted array from child process
            MPI_Recv(buff, recv_size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &Stat);

            // Merge received array and global array together
            comp = mergeArrays(global, buff, local_size, recv_size);
            local_size += recv_size;

            // Pointer to Array
            p2a(global, comp, local_size);
            delete[] comp; // Free memory for merged array
        }

        end = MPI_Wtime();
        time_spent = end - begin;

        std::cout << "Sorted global array (Process " << rank << "):\n";
        //printArray(global, local_size);
        std::cout << "Time spent (Parallel): " << time_spent << " seconds\n";

        begin = MPI_Wtime();
        // Time sequential merge sort on the same set of numbers
        mergesort(a, 0, NUM - 1);
        end = MPI_Wtime();
        time_spent = end - begin;
        std::cout << "Sorted global array (Process " << rank << "):\n";
        //printArray(global, local_size);
        std::cout << "Time spent (Non-Parallel): " << time_spent << " seconds\n";

        delete[] buff; // Free memory
    }
    // Child process
    else {
        mergesort(local, 0, local_size - 1);
        // Send sorted array to parent process
        MPI_Send(local, local_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    delete[] local;   // Free local array
    delete[] a;       // Free original array
    delete[] global;  // Free global array

    MPI_Finalize();
    return 0;
}

// Merge sort
void mergesort(int array[], int left, int right) {
    if (left < right) {
        int middle = (left + right) / 2;
        mergesort(array, left, middle);
        mergesort(array, middle + 1, right);
        merge(array, left, middle, right);
    }
}

// Merge function
void merge(int array[], int left, int middle, int right) {
    int* temp = new int[right - left + 1];
    int i = left, j = middle + 1, k = 0;

    while (i <= middle && j <= right) {
        if (array[i] <= array[j])
            temp[k++] = array[i++];
        else
            temp[k++] = array[j++];
    }
    while (i <= middle)
        temp[k++] = array[i++];
    while (j <= right)
        temp[k++] = array[j++];

    for (k = 0; k < right - left + 1; k++) {
        array[left + k] = temp[k];
    }

    delete[] temp; // Free temporary array
}

// Merge two arrays
int* mergeArrays(int a[], int b[], int n, int m) {
    int* c = new int[n + m];
    int i = 0, j = 0, k = 0;

    while (i < n && j < m) {
        if (a[i] <= b[j]) {
            c[k++] = a[i++];
        } else {
            c[k++] = b[j++];
        }
    }

    while (i < n) {
        c[k++] = a[i++];
    }
    while (j < m) {
        c[k++] = b[j++];
    }
    return c;
}

// Pointer to Array
void p2a(int a[], int* b, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = b[i];
    }
}

// Local to Global
void l2g(int a[], int b[], int size) {
    for (int i = 0; i < size; i++)
        a[i] = b[i];
}

// Print array function
void printArray(int array[], int size) {
    for (int i = 0; i < size; i++) {
        std::cout << array[i] << " ";
    }
    std::cout << "\n";
}
