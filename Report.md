# CSCE 435 Group project

## 0. Group number: 11

## 1. Group members:
1. Shyam Ramachandran
2. Andrew Lin
3. Sydney Lipar
4. Nai-Yun Wu

## 2. Project topic (e.g., parallel sorting algorithms)

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

- Bitonic Sort:
  
- Sample Sort: Sample sort is a divide-and-conquer sorting algorithm that works a little differently than other generic partition-based sorting algorithms. This is partially due to the fact that Sample sort is commonly used in parallel systems, so regular sorting algorithms performance can be severely throttled when an array is non-uniform. Sample sort gets around this by choosing a sample S and getting a range of buckets by sorting the sample and choosing a number of elements from the result. These elements (called splitters) go on to divide the array into approximately equal-sized buckets. 

- Merge Sort: Merge sort is a divide-and-conquer algorithm that works by recursively splitting an array into smaller subarrays until each subarray has one element, and is therefore inherently sorted. Then, it merges these subarrays back together in a sorted order. When implementing Merge sort in parallel, the recursive calls stay almost the same but the merging process is implemented in parallel. First, the input array is divided across multiple processors and each processor sorts its own subarray. Next, the processors communicate with eachother via MPI to perform a parralel merge operation.
  
- Radix Sort:

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

Sample Sort:
Assuming n threads and k as a random sampling constant, the algorithm goes as follows: 
  1. Randomly sample n * k elements from the input
  2. Sort the samples and select the k-th, 2*k-th, etc. elements as pivots
  3. Split the data into buckets defined by these pivots
  4. Sort the buckets in parallel


def sampleSort(A, k, p):

    # Step 1
    S = [S1, ..., S(p-1)k] # Create a random sample from A
    sort S
    splitters = [s0, ..., sp-1, sp]
    buckets = [-∞, Sk, S2k, ..., S(p-1)k, ∞]
    Map buckets onto splitters
    # Step 2
    for each element in A:
        find j such that s[j-1] < a <= s[j]
        place a in bucket b[j]
    # Step 3
    return concatenate(sampleSort(b1), ..., sampleSort(bk))


Merge Sort:

  1. Distribute array across processors
  2. Each processor sorts its own subarray
  3. Perform parallel merge
  4. Gather final sorted array on processor 0

def parallelMergeSort(A, lo, hi, B, proc_rank, num_procs):

    // Step 1
    local_len := (hi - lo + 1) / num_procs
    local_A := MPI_Scatter(A, local_len, proc_rank)

    // Step 2
    local_sorted := mergeSort(local_A, 0, local_len - 1)

    // Step 3
    step := 1
    while step < num_procs do
        if proc_rank % (2 * step) == 0 then
            if proc_rank + step < num_procs then
                recv_array := MPI_Recv(proc_rank + step)
                merged_array := parallelMerge(local_sorted, recv_array)
                local_sorted := merged_array
        else
            MPI_Send(local_sorted, proc_rank - step)
            break
        step := step * 2

    // Step 4
    if proc_rank == 0 then
        B := local_sorted


    
### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)
