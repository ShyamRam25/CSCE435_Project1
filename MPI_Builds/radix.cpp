#include <iostream>
#include <vector>
#include <mpi.h>
#include <algorithm>
#include <cmath>


#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include "helper.h"

#define MAX_DIGIT 10 // Radix for base 10 numbers

using namespace std;

// Function to find the maximum value in the array
int getMax(vector<int>& ary) {
    return *max_element(ary.begin(), ary.end());
}


// A function to do counting sort of ary[] according to the digit represented by exp
void countingSort(vector<int>& ary, int exp) {
    int n = ary.size();
    vector<int> sorted_ary(n); // Output array
    int count[MAX_DIGIT] = {0};

    // Count occurrences of digits
    for (int i = 0; i < n; i++) {
        count[(ary[i] / exp) % MAX_DIGIT]++;
    }

    
    // Calculate the cumulative count
    for (int i = 1; i < MAX_DIGIT; i++) {
        count[i] += count[i - 1];
    }

    // Build the output array
    for (int i = n - 1; i >= 0; i--) {
        sorted_ary[count[(ary[i] / exp) % MAX_DIGIT] - 1] = ary[i];
        count[(ary[i] / exp) % MAX_DIGIT]--;
    }

    // Copy the output array to arr[], so that arr now contains sorted numbers
    for (int i = 0; i < n; i++) {
        ary[i] = sorted_ary[i];
    }
}


// Radix Sort function
void radixSort(vector<int>& ary) {
    int m = getMax(ary); // Find the maximum number to know the number of digits

    // Do counting sort for every digit. Note that instead of passing the digit number,
    // exp is passed. exp is 10^i where i is the current digit number.
    for (int exp = 1; m / exp > 0; exp *= 10) {
        countingSort(ary, exp);
    }
}


bool checkSorted(vector<int> &ary) {
    for(int i = 1; i < ary.size(); i++) {
        if (ary[i] < ary[i-1]) {
            return false;
        }
    }
    return true;
}


// Merge 2 arrays
vector<int> merge(const vector<int>& left, const vector<int>& right) {
    vector<int> result;
    int i = 0, j = 0;

    while (i < left.size() && j < right.size()) {
        if (left[i] < right[j]) {
            result.push_back(left[i]);
            i++;
        } else {
            result.push_back(right[j]);
            j++;
        }
    }

    // Append remaining elements
    while (i < left.size()) {
        result.push_back(left[i]);
        i++;
    }
    while (j < right.size()) {
        result.push_back(right[j]);
        j++;
    }

    return result;
}


// Function to merge multiple sorted arrays on the root process
vector<int> mergeSortedArrays(const vector<vector<int>>& arrays) {
    if (arrays.empty()) return {};

    vector<int> merged = arrays[0];
    for (int i = 1; i < arrays.size(); i++) {
        merged = merge(merged, arrays[i]);
    }
    return merged;
}



int main(int argc, char* argv[]) {
    CALI_CXX_MARK_FUNCTION;
    cali::ConfigManager mgr;
    mgr.start();

    const char* main = "main";
    const char* data_init_runtime = "data_init_runtime";
    const char* correctness_check = "correctness_check";
    const char* comm = "comm";
    const char* comm_large = "comm_large";
    const char* comp = "comp";
    const char* comp_large = "comp_large";

    CALI_MARK_BEGIN(main);
    MPI_Init(&argc, &argv);

    int rank, processor_count, ary_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processor_count);

    if (argc != 3) {
        if (rank == 0)
            std::cout << "Usage : run size" << std::endl;
        MPI_Finalize();
        return 0;
    }


    // Building original array
    CALI_MARK_BEGIN(data_init_runtime);
    vector<int> ary;
    std::string input = argv[2];
    if (rank == 0){
        ary_size = std::atoi(argv[1]);
        ary.resize(ary_size);

        if (input == "sorted") { 
            int* ptr = sortedArray(ary_size);  // Get the array pointer
            ary = std::vector<int>(ptr, ptr + ary_size);  // Convert pointer to vector
        } else if (input == "random") {
            int* ptr = randomArray(ary_size);  // Get the array pointer
            ary = std::vector<int>(ptr, ptr + ary_size);  // Convert pointer to vector
        } else if (input == "reverse") {
            int* ptr = reverseArray(ary_size);  // Get the array pointer
            ary = std::vector<int>(ptr, ptr + ary_size);  // Convert pointer to vector
        } else if (input == "perturbed") {
            int* ptr = perturbedArray(ary_size);  // Get the array pointer
            ary = vector<int>(ptr, ptr + ary_size);  // Convert pointer to vector
        } else {
            std::cout << "Invalid input" << std::endl;
            return 0;
        }
        //Print out the input array in same format as above 
       
    }
    else {
        ary.resize(ary_size);
    }
    CALI_MARK_END(data_init_runtime);

    
    // Broadcast the size of the array to all processes
    CALI_MARK_BEGIN(comm);
    CALI_MARK_BEGIN(comm_large);
    MPI_Bcast(&ary_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END(comm_large);
    CALI_MARK_END(comm);



    // Find out the size of each chuck of array for the processes
    int chunk_size = ary_size / (processor_count);
    vector<int> local_ary(chunk_size);


    // Scatter date to all processes
    CALI_MARK_BEGIN(comm);
    CALI_MARK_BEGIN(comm_large);
    MPI_Scatter(ary.data(), chunk_size, MPI_INT, local_ary.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END(comm_large);
    CALI_MARK_END(comm);


    // Sorting
    CALI_MARK_BEGIN(comp);
    CALI_MARK_BEGIN(comp_large);
    radixSort(local_ary);
    CALI_MARK_END(comp_large);
    CALI_MARK_END(comp);


    vector<int> global_ary;
    if (rank == 0) {
        global_ary.resize(ary_size);
    }

    CALI_MARK_BEGIN(comm);
    CALI_MARK_BEGIN(comm_large);
    MPI_Gather(local_ary.data(), chunk_size, MPI_INT, global_ary.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END(comm_large);
    CALI_MARK_END(comm);


     // Merge sorted subarrays at the root process
    if (rank == 0) {
        vector<vector<int>> subarrays(processor_count);
        for (int i = 0; i < processor_count; i++) {
            subarrays[i] = vector<int>(global_ary.begin() + i * chunk_size, global_ary.begin() + (i + 1) * chunk_size);
        }


        CALI_MARK_BEGIN(comp);
        CALI_MARK_BEGIN(comp_large);
        vector<int> merged_array = mergeSortedArrays(subarrays);
        CALI_MARK_END(comp_large);
        CALI_MARK_END(comp);


        // Check if sorted
        CALI_MARK_BEGIN(correctness_check);
        if(checkSorted(merged_array)) {
            cout << "Array is sorted" << endl;
        } else {
            cout << "Array is not sorted" << endl;
        }
        CALI_MARK_END(correctness_check);

    }

    adiak::init(NULL);
    adiak::launchdate();    // launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", "radix"); // The name of the algorithm you are using (e.g., "merge", "bitonic")
    adiak::value("programming_model", "mpi"); // e.g. "mpi"
    adiak::value("data_type", "int"); // The datatype of input elements (e.g., double, int, float)
    adiak::value("size_of_data_type", 4); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
    adiak::value("input_size", ary_size); // The number of elements in input dataset (1000)
    adiak::value("input_type", input); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
    adiak::value("num_procs", processor_count); // The number of processors (MPI ranks)
    adiak::value("scalability", "strong"); // The scalability of your algorithm. choices: ("strong", "weak")
    adiak::value("group_num", 11); // The number of your group (integer, e.g., 1, 10)
    adiak::value("implementation_source", "handwritten"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").

    CALI_MARK_END(main);
    mgr.stop();
    mgr.flush();
   
    MPI_Finalize();
}