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
  
- Radix Sort: Radix Sort is a non-comparative sorting algorithm that sorts numbers by processing individual digits. It works by sorting numbers digit by digit, starting from the least significant digit to the most significant digit. For each digit, the algorithm uses a stable sorting technique like counting sort to ensure that the relative order of numbers is preserved. Parallel algorithm can be implemented by distributing the globe array into smaller chucks between worker processes. Perform radix sort on the local arrays. Finally merge them back together in the master process.

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

Sample Sort:
Assuming n threads and k as a random sampling constant, the algorithm goes as follows: 
  1. Randomly sample n * k elements from the input
  2. Share these samples with every processor (MPI_AllGather) or MPI_BCast
  3. Sort the samples and select the k-th, 2*k-th, etc. elements as pivots
  4. Split the data into buckets defined by these pivots
  5. Send the b_th bucket to the b_th processor (MPI_ToAll[v])
  6. Sort the buckets in parallel


def sampleSort(A, n, num_processors):

    num_elements_bloc = n / num_processors
    
    #Send Data
    MPI_BCast(n, 1, MPI_INT, 0, MPI_COMM, WORLD)
    Take input data and validate
    MPI_Scatter(Input, num_elements_bloc)
    
    #Sort locally
    qsort(A, num_elements_bloc, ...)
    MPI_Gather(A, n, ...)
    
    #Choose local splitters
    Splitters = [...]
    
    #Gather local splitters
    AllSplitters = [...]
    MPI_Gather(Splitter, numprocs-1, AllSplitter, ...)
    
    #Choose global splitters
    if Rank == 0:
      qsort(AllSplitter)
      Splitter[i] = AllSplitter[(num_processors-1) * (i + 1)]
    
    #Broadcast global splitters
    MPI_BCast(Splitter, ...)
    
    #Create num_processors buckets
    buckets = [...]
    
    #Send buckets to respective processors
    MPI_Alltoall(Buckets, ...)
    #Sort local buckets
    qsort(LocalBucket, ...)
    MPI_Gather(LocalBucket)

    #Print out results


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


Radix Sort:

  1. Broadcast the size of the global array(the array we are sorting)
  MPI_bcast(&global_array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  2. Determine the size of the local matrix in each process
  local_size = global_size / num_of_processes;

  3. Create local arrays with the local size
  vector<int> local_arr(local_size);

  4. Distribute the data to all worker processes
  MPI_Scatter(global_array.data(), local_size, MPI_INT, local_arr.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

  5. Perform radix sort in each worker process
  radix_sort(local_arr);

  6. Gather sorted local arrays to master
  MPI_Gather(local_arr.data(), local_size, MPI_INT, glocal_arr.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

  7. Merge the gathered data



    
### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)
