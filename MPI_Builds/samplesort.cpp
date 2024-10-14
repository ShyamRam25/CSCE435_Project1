#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <mpi.h>
#include <algorithm>  // for std::qsort, std::sort
#include "helper.h"
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#define SIZE 10

/**** Function Declaration Section ****/

static int intcompare(const void* i, const void* j) {
    if ((*(int*)i) > (*(int*)j))
        return (1);
    if ((*(int*)i) < (*(int*)j))
        return (-1);
    return (0);
}

int main(int argc, char* argv[]) {
    /* Variable Declarations */
    CALI_CXX_MARK_FUNCTION;

    int Numprocs, MyRank, Root = 0;
    int i, j, k, NoofElements, NoofElements_Bloc, NoElementsToSort;
    int count, temp;
    int* Input = nullptr;
    int* InputData = nullptr;
    int* Splitter = nullptr;
    int* AllSplitter = nullptr;
    int* Buckets = nullptr;
    int* BucketBuffer = nullptr;
    int* LocalBucket = nullptr;
    int* OutputBuffer = nullptr;
    int* Output = nullptr;
    std::ofstream fp;

    MPI_Status status;

    /**** Initialising ****/
    cali::ConfigManager mgr;
    mgr.start();

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &Numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);

    if (argc != 3) {
        if (MyRank == 0)
            std::cout << "Usage : run size" << std::endl;
        MPI_Finalize();
        return 0;
    }

    /**** Data Initialization ****/
    // Caliper region for data initialization
    CALI_MARK_BEGIN("data_init_runtime");
    if (MyRank == Root) {
        NoofElements = std::atoi(argv[1]);
        Input = new int[NoofElements];
        if (Input == nullptr) {
            std::cout << "Error: Cannot allocate memory" << std::endl;
        }

        std::string input = argv[2];
        if (input == "sorted") {
            Input = sortedArray(NoofElements);
        } else if (input == "random") {
            Input = randomArray(NoofElements);
        } else if (input == "reverse") {
            Input = reverseArray(NoofElements);
        } else if (input == "perturbed") {
            Input = perturbedArray(NoofElements);
        } else {
            std::cout << "Invalid input" << std::endl;
            return 0;
        }
        // // Print out the input array in the same format as above 
        // std::cout << "Input Array for Sorting: [";
        // for (i = 0; i < NoofElements; i++) {
        //     std::cout << Input[i];
        //     if (i != NoofElements - 1) // To avoid printing comma after the last element
        //         std::cout << ", ";
        // }
        // std::cout << "]" << std::endl;
    }
    CALI_MARK_END("data_init_runtime");

    /**** Sending Data ****/
    MPI_Bcast(&NoofElements, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if ((NoofElements % Numprocs) != 0) {
        if (MyRank == Root)
            std::cout << "Number of Elements are not divisible by Numprocs" << std::endl;
        MPI_Finalize();
        return 0;
    }

    NoofElements_Bloc = NoofElements / Numprocs;
    InputData = new int[NoofElements_Bloc];

    // Caliper region for communication
    CALI_MARK_BEGIN("comm");
    MPI_Scatter(Input, NoofElements_Bloc, MPI_INT, InputData, NoofElements_Bloc, MPI_INT, Root, MPI_COMM_WORLD);
    CALI_MARK_END("comm");

    /**** Sorting Locally ****/
    // Caliper region for computation
    CALI_MARK_BEGIN("comp");
    std::qsort(InputData, NoofElements_Bloc, sizeof(int), intcompare);
    CALI_MARK_END("comp");
    
    /**** Choosing Local Splitters ****/
    Splitter = new int[Numprocs - 1];
    int elements_per_proc = std::max(1, NoofElements / (Numprocs * Numprocs));
    for (i = 0; i < (Numprocs - 1); i++) {
        Splitter[i] = InputData[elements_per_proc * (i + 1)];
    }

    /**** Gathering Local Splitters at Root ****/
    AllSplitter = new int[Numprocs * (Numprocs - 1)];
    MPI_Gather(Splitter, Numprocs - 1, MPI_INT, AllSplitter, Numprocs - 1, MPI_INT, Root, MPI_COMM_WORLD);

    /**** Choosing Global Splitters ****/
    if (MyRank == Root) {
        std::qsort(AllSplitter, Numprocs * (Numprocs - 1), sizeof(int), intcompare);
        for (i = 0; i < Numprocs - 1; i++)
            Splitter[i] = AllSplitter[(Numprocs - 1) * (i + 1)];
    }

    /**** Broadcasting Global Splitters ****/
    MPI_Bcast(Splitter, Numprocs - 1, MPI_INT, Root, MPI_COMM_WORLD);

    /**** Creating Numprocs Buckets locally ****/
    Buckets = new int[(NoofElements_Bloc + 1) * Numprocs];

    j = 0;
    k = 1;

    for (i = 0; i < NoofElements_Bloc; i++) {
        if (j < (Numprocs - 1)) {
            if (InputData[i] < Splitter[j])
                Buckets[((NoofElements_Bloc + 1) * j) + k++] = InputData[i];
            else {
                Buckets[(NoofElements_Bloc + 1) * j] = k - 1;
                k = 1;
                j++;
                i--;
            }
        } else
            Buckets[((NoofElements_Bloc + 1) * j) + k++] = InputData[i];
    }
    Buckets[(NoofElements_Bloc + 1) * j] = k - 1;

    /**** Sending buckets to respective processors ****/

    BucketBuffer = new int[(NoofElements_Bloc + 1) * Numprocs];

    CALI_MARK_BEGIN("alltoall");
    MPI_Alltoall(Buckets, NoofElements_Bloc + 1, MPI_INT, BucketBuffer, NoofElements_Bloc + 1, MPI_INT, MPI_COMM_WORLD);
    CALI_MARK_END("alltoall");

    /**** Rearranging BucketBuffer ****/
    LocalBucket = new int[2 * NoofElements / Numprocs];

    count = 1;

    for (j = 0; j < Numprocs; j++) {
        k = 1;
        for (i = 0; i < BucketBuffer[(NoofElements / Numprocs + 1) * j]; i++)
            LocalBucket[count++] = BucketBuffer[(NoofElements / Numprocs + 1) * j + k++];
    }
    LocalBucket[0] = count - 1;

    /**** Sorting Local Buckets using qsort ****/
    NoElementsToSort = LocalBucket[0];
    std::qsort(&LocalBucket[1], NoElementsToSort, sizeof(int), intcompare);

    /**** Gathering sorted sub blocks at root ****/
    if (MyRank == Root) {
        OutputBuffer = new int[2 * NoofElements];
        Output = new int[NoofElements];
    }

    CALI_MARK_BEGIN("gather");
    MPI_Gather(LocalBucket, 2 * NoofElements_Bloc, MPI_INT, OutputBuffer, 2 * NoofElements_Bloc, MPI_INT, Root, MPI_COMM_WORLD);
    CALI_MARK_END("gather");

    /**** Rearranging output buffer ****/
    if (MyRank == Root) {
        count = 0;
        for (j = 0; j < Numprocs; j++) {
            k = 1;
            for (i = 0; i < OutputBuffer[(2 * NoofElements / Numprocs) * j]; i++)
                Output[count++] = OutputBuffer[(2 * NoofElements / Numprocs) * j + k++];
        }

        /**** Correctness Check ****/
        CALI_MARK_BEGIN("correctness_check");
        // Checking if the output is sorted
        bool is_sorted = std::is_sorted(Output, Output + NoofElements);
        if (is_sorted) {
            std::cout << "The output is correctly sorted." << std::endl;
        } else {
            std::cout << "The output is NOT sorted!" << std::endl;
        }
        CALI_MARK_END("correctness_check");

        /**** Printing the output ****/
        // fp.open("sort.out");
        // if (!fp) {
        //     std::cerr << "Can't Open Output File" << std::endl;
        //     return 0;
        // }

        // fp << "Number of Elements to be sorted: " << NoofElements << std::endl;
        // std::cout << "Number of Elements to be sorted: " << NoofElements << std::endl;

        // fp << "The sorted sequence is: [";
        // std::cout << "Sorted output sequence is: [";
        // for (i = 0; i < NoofElements; i++) {
        //     fp << Output[i];
        //     std::cout << Output[i];
        //     if (i != NoofElements - 1) { // Avoid printing comma after last element
        //         fp << ", ";
        //         std::cout << ", ";
        //     }
        // }
        // fp << "]" << std::endl;
        // std::cout << "]" << std::endl;

        // fp.close();
        delete[] Input;
        delete[] OutputBuffer;
        delete[] Output;
    }

    delete[] InputData;
    delete[] Splitter;
    delete[] AllSplitter;
    delete[] Buckets;
    delete[] BucketBuffer;
    delete[] LocalBucket;

    adiak::init(NULL);
    adiak::launchdate();    // launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", "Samplesort"); // The name of the algorithm you are using (e.g., "merge", "bitonic")
    adiak::value("programming_model", "mpi"); // e.g. "mpi"
    adiak::value("data_type", "int"); // The datatype of input elements (e.g., double, int, float)
    adiak::value("size_of_data_type", "4"); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
    adiak::value("input_size", NoofElements); // The number of elements in input dataset (1000)
    adiak::value("input_type", argv[2]); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
    adiak::value("num_procs", Numprocs); // The number of processors (MPI ranks)
    adiak::value("scalability", "strong"); // The scalability of your algorithm. choices: ("strong", "weak")
    adiak::value("group_num", "11"); // The number of your group (integer, e.g., 1, 10)
    adiak::value("implementation_source", "Handwritten"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").

    /**** Finalize ****/
    mgr.stop();
    mgr.flush();
    MPI_Finalize();
    return 0;
}
