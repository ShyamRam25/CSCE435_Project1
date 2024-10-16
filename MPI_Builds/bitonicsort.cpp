#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include <algorithm>
#include "helper.h"

const char* comp_small = "comp_small";
const char* comm_small = "comm_small";

void exchange_data(int* localArray, int localSize, int partner, int rank, int direction) {
    CALI_MARK_BEGIN(comm_small); 
    int* recvBuffer = new int[localSize];

    MPI_Sendrecv(localArray, localSize, MPI_INT, partner, 0,
                 recvBuffer, localSize, MPI_INT, partner, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int* combinedArray = new int[2 * localSize];
    std::copy(localArray, localArray + localSize, combinedArray);
    std::copy(recvBuffer, recvBuffer + localSize, combinedArray + localSize);

    if (direction == 1) { 
        std::sort(combinedArray, combinedArray + 2 * localSize);
    } 
    else {  
        std::sort(combinedArray, combinedArray + 2 * localSize, std::greater<int>());
    }

    if (rank < partner) {
        std::copy(combinedArray, combinedArray + localSize, localArray);
    } 
    else {
        std::copy(combinedArray + localSize, combinedArray + 2 * localSize, localArray);
    }

    delete[] combinedArray;
    delete[] recvBuffer; 
    CALI_MARK_END(comm_small);
}

void bitonic_sort(int* localArray, int localSize, int rank, int num_procs) {
    std::sort(localArray, localArray + localSize);

    for (int size = 2; size <= num_procs; size *= 2) {
        int direction = ((rank / size) % 2 == 0) ? 1 : 0; 

        for (int subSize = size / 2; subSize > 0; subSize /= 2) {
            CALI_MARK_BEGIN(comp_small);
            int partner = rank ^ subSize;
            if (partner < num_procs) {
                exchange_data(localArray, localSize, partner, rank, direction);
            }
            MPI_Barrier(MPI_COMM_WORLD);
            CALI_MARK_END(comp_small);
        }
    }
}

void printArray(int* array, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}


// argc = 3
// argv = [./bitonicsort, arraySize, arrayType]

int main(int argc, char* argv[]) {
    CALI_CXX_MARK_FUNCTION;

    int num_procs, rank;

    const char* main = "main";
    const char* data_init_runtime = "data_init_runtime";
    const char* comm = "comm";
    const char* comm_small = "comm_small";
    const char* comm_large = "comm_large";
    const char* comp = "comp";
    const char* comp_small = "comp_small";
    const char* comp_large = "comp_large";
    const char* correctness_check = "correctness_check";

    cali::ConfigManager mgr;
    mgr.start();
    CALI_MARK_BEGIN(main);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int n = atoi(argv[1]);
    std::string arrayType = argv[2];

    CALI_MARK_BEGIN(data_init_runtime);
    int* globalArray = nullptr;
    int localSize = n / num_procs;
    int* localArray = new int[localSize];

    if (rank == 0) {
        if (arrayType == "sorted") {
            globalArray = sortedArray(n);
        }
        else if (arrayType == "random") {
            globalArray = randomArray(n);
        }
        else if (arrayType == "reverse") {
            globalArray = reverseArray(n);
        }
        else if (arrayType == "perturbed") {
            globalArray = perturbedArray(n);
        }
        else {
            std::cout << "Invalid input" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        // printArray(globalArray, n);
    }
    CALI_MARK_END(data_init_runtime);
    
    CALI_MARK_BEGIN(comm);
    CALI_MARK_BEGIN(comm_large);
    MPI_Scatter(globalArray, localSize, MPI_INT, localArray, localSize, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END(comm_large);
    CALI_MARK_END(comm);

    CALI_MARK_BEGIN(comp);
    CALI_MARK_BEGIN(comp_large);
    bitonic_sort(localArray, localSize, rank, num_procs);
    CALI_MARK_END(comp_large);
    CALI_MARK_END(comp);

    CALI_MARK_BEGIN(comm);
    CALI_MARK_BEGIN(comm_large);
    MPI_Gather(localArray, localSize, MPI_INT, globalArray, localSize, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END(comm_large);
    CALI_MARK_END(comm);

    if (rank == 0) {
        CALI_MARK_BEGIN(correctness_check);
        bool isSorted = true;
        for (int i = 1; i < n; i++) {
            if (globalArray[i-1] > globalArray[i]) {
                isSorted = false;
                break;
            }
        }
        printf("Array is%s sorted\n", isSorted ? "" : " NOT");
        // printArray(globalArray, n);
        CALI_MARK_END(correctness_check);
    }

    if (rank == 0) {
        delete[] globalArray;
    }
    delete[] localArray;


    CALI_MARK_END(main);

    adiak::init(NULL);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();
    adiak::value("algorithm", "bitonic");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "int");
    adiak::value("size_of_data_type", sizeof(int));
    adiak::value("input_size", n); 
    adiak::value("input_type", arrayType); 
    adiak::value("num_procs", num_procs); 
    adiak::value("scalability", "strong");
    adiak::value("group_num", 11); 
    adiak::value("implementation_source", "handwritten");

    mgr.stop();
    mgr.flush();

    MPI_Finalize();
    return 0;
}

