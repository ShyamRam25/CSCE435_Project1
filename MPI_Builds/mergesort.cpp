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
bool isSorted(int array[], int size); 


// Main function

int main(int argc, char* argv[]) {
    CALI_CXX_MARK_FUNCTION;


    //define caliper regions
    const char* data_init_runtime = "data_init_runtime";
    const char* comm = "comm";
    const char* comm_large = "comm_large";
    const char* comp = "comp";
    const char* comp_large = "comp_large";
    const char* correctness_check = "correctness_check";

    // MPI_Init(&argc, &argv);
    // MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    cali::ConfigManager mgr;
    mgr.start();


    // command line arguments
    int NUM = atoi(argv[1]);
    std::string type = argv[2];
    int*a;
    CALI_MARK_BEGIN(data_init_runtime);

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
    CALI_MARK_END(data_init_runtime);
    
    
    int i, a_size = NUM, local_size;
    int numtasks, rank;
    int* global = new int[NUM];
    int* comp_array;
    MPI_Status Stat;


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    // Local array for each process
    int* local = new int[NUM / numtasks];
    srand(static_cast<unsigned int>(time(NULL)));

    CALI_MARK_BEGIN(comm);
    CALI_MARK_BEGIN(comm_large);
    // Scatter and split array evenly for each process
    MPI_Scatter(a, NUM / numtasks, MPI_INT, local, NUM / numtasks, MPI_INT, 0, MPI_COMM_WORLD);
    local_size = NUM / numtasks;
    CALI_MARK_END(comm_large);
    CALI_MARK_END(comm);

    // Parent process
    if (rank == 0) {

        // Sequential merge sort
        CALI_MARK_BEGIN(comp);
        CALI_MARK_BEGIN(comp_large);
        mergesort(local, 0, local_size - 1);
        CALI_MARK_END(comp_large);
        CALI_MARK_END(comp);

        // Push sorted local array to global array
        l2g(global, local, local_size);
        int j, recv_size = local_size;
        int* buff = new int[recv_size];


        for (j = 0; j < numtasks - 1; j++) {
            CALI_MARK_BEGIN(comm);
            CALI_MARK_BEGIN(comm_large);
            // Receive sorted array from child process
            MPI_Recv(buff, recv_size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &Stat);
            CALI_MARK_END(comm_large);
            CALI_MARK_END(comm);

            // Merge received array and global array together
            CALI_MARK_BEGIN(comp);
            CALI_MARK_BEGIN(comp_large);
            comp_array = mergeArrays(global, buff, local_size, recv_size);
            local_size += recv_size;

            // Pointer to Array
            p2a(global, comp_array, local_size);
            delete[] comp_array;
            CALI_MARK_END(comp_large);
            CALI_MARK_END(comp);
        }

        //std::cout << "Sorted global array (Process " << rank << "):\n";
        //printArray(global, local_size);
        //std::cout << "Time spent (Parallel): " << time_spent << " seconds\n";

        //correctness check
        CALI_MARK_BEGIN(correctness_check);
        if (!isSorted(global, local_size)) {
            std::cout << "Array is NOT sorted correctly.\n";
        }
        CALI_MARK_END(correctness_check);

        delete[] buff;
    }
    // Child process
    else {
        CALI_MARK_BEGIN(comp);
        CALI_MARK_BEGIN(comp_large);
        mergesort(local, 0, local_size - 1);
        CALI_MARK_END(comp_large);
        CALI_MARK_END(comp);

        // Send sorted array to parent process
        CALI_MARK_BEGIN(comm);
        CALI_MARK_BEGIN(comm_large);
        MPI_Send(local, local_size, MPI_INT, 0, 0, MPI_COMM_WORLD); 
        CALI_MARK_END(comm_large);
        CALI_MARK_END(comm);
    }

    delete[] local;
    delete[] a;
    delete[] global;

    // Adiak annotations
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();
    adiak::value("algorithm", "merge");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "int");
    adiak::value("size_of_data_type", sizeof(int));
    adiak::value("input_size", NUM);
    adiak::value("input_type", type);
    adiak::value("num_procs", numtasks);
    adiak::value("scalability", "strong");
    adiak::value("group_num", 11);
    adiak::value("implementation_source", "handwritten");

    mgr.stop();
    mgr.flush();

    MPI_Finalize();

    return 0;
}


void mergesort(int array[], int left, int right) {
    if (left < right) {
        int middle = (left + right) / 2;
        mergesort(array, left, middle);
        mergesort(array, middle + 1, right);
        merge(array, left, middle, right);
    }
}

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

    delete[] temp;
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

bool isSorted(int array[], int size) {
    for (int i = 1; i < size; i++) {
        if (array[i] < array[i - 1]) {
            return false;
        }
    }
    return true;
}
