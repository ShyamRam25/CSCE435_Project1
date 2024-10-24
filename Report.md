# CSCE 435 Group project

## 0. Group number: 11

## 1. Group members:
1. Shyam Ramachandran
2. Andrew Lin
3. Sydney Lipar
4. Nai-Yun Wu

Our team will communicate via text messaging. 

## 2. Project topic (e.g., parallel sorting algorithms)

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

- Bitonic Sort: Bitonic sort is a comparison-based parallel sorting algorithm that first creates a bitonic sequence then sorts the sequence. A bitonic sequence is a sequence such that the elemts are first monotonically increasing, then monotonically decreasing. Then, the algorithm repeatedly merges these bitonic sequences to produce a sorted sequence. It is important to note that bitonic sort can only be done if there are 2<sup>n</sup> number of elements for some integer n. The time complexity is O(log<sup>2</sup>n) and the space complexity is O(nlog<sup>2</sup>n). It is important to note that while the number of comparisons is more than other sorting algorithms, bitonic sort is still more efficient than quicksort. 
  
- Sample Sort: Sample sort is a divide-and-conquer sorting algorithm that works a little differently than other generic partition-based sorting algorithms. This is partially due to the fact that Sample sort is commonly used in parallel systems, so regular sorting algorithms performance can be severely throttled when an array is non-uniform. Sample sort gets around this by choosing a sample S and getting a range of buckets by sorting the sample and choosing a number of elements from the result. These elements (called splitters) go on to divide the array into approximately equal-sized buckets. 

- Merge Sort: Merge sort is a divide-and-conquer algorithm that works by recursively splitting an array into smaller subarrays until each subarray has one element, and is therefore inherently sorted. Then, it merges these subarrays back together in a sorted order. When implementing Merge sort in parallel, the recursive calls stay almost the same but the merging process is implemented in parallel. First, the input array is divided across multiple processors and each processor sorts its own subarray. Next, the processors communicate with eachother via MPI to perform a parralel merge operation.
  
- Radix Sort: Radix Sort is a non-comparative sorting algorithm that sorts numbers by processing individual digits. It works by sorting numbers digit by digit, starting from the least significant digit to the most significant digit. For each digit, the algorithm uses a stable sorting technique like counting sort to ensure that the relative order of numbers is preserved. Parallel algorithm can be implemented by distributing the globe array into smaller chucks between worker processes. Perform radix sort on the local arrays. Finally merge them back together in the master process.

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

Bitonic Sort:

  1. If number of elements isn't power of 2, pad list with infinity until the size is a power of 2
     
  2. Create the bitonic sequence
  
  3. Merge the bitonic sequences
  
  4.  Recursively merge until completely sorted. Repeat process for each subsequence

    def bitonicSort(loc_arr, size):

      n = len(loc_arr)
      total_size = n * size
  
      // Bitonic sequence creation
      for k=2 to k=total_size by powers of 2:
        for j=k/2 to j=1 by division by 2:
  
          // Local comparison and swap
          for i=0 to i=n-1:
            partner = i XOR j // Get partner index for comparison
            
            if partner >= 0 and partner < total_size:
              partner_value = mpiExchangeValue(loc_arr[i], partner)
              
              if loc_arr[i] > partner_value:
                swap(loc_arr[i], partner_value)
                
      return loc_arr
    

Sample Sort:
Assuming n threads and k as a random sampling constant, the algorithm goes as follows: 
  1. Randomly sample n * k elements from the input
    
  2. Share these samples with every processor (MPI_AllGather) or MPI_BCast
    
  3. Sort the samples and select the k-th, 2*k-th, etc. elements as pivots
    
  4. Split the data into buckets defined by these pivots
  
  5. Send the b_th bucket to the b_th processor (MPI_ToAll[v])
     
  6.  Sort the buckets in parallel


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

We will be testing with both an increasing number of processors and an increasing problem size.
    Each program will be tested in 4 different runs as well
    
    With a fully random array
    With a fully sorted array
    With a reverse sorted array
    With a sorted array, but with 1% of elements swapped, internally and externally

### 3a. Caliper instrumentation
Please use the caliper build `/scratch/group/csce435-f24/Caliper/caliper/share/cmake/caliper` 
(same as lab2 build.sh) to collect caliper files for each experiment you run.

Your Caliper annotations should result in the following calltree
(use `Thicket.tree()` to see the calltree):
```
main
|_ data_init_X      # X = runtime OR io
|_ comm
|    |_ comm_small
|    |_ comm_large
|_ comp
|    |_ comp_small
|    |_ comp_large
|_ correctness_check
```

Required region annotations:
- `main` - top-level main function.
    - `data_init_X` - the function where input data is generated or read in from file. Use *data_init_runtime* if you are generating the data during the program, and *data_init_io* if you are reading the data from a file.
    - `correctness_check` - function for checking the correctness of the algorithm output (e.g., checking if the resulting data is sorted).
    - `comm` - All communication-related functions in your algorithm should be nested under the `comm` region.
      - Inside the `comm` region, you should create regions to indicate how much data you are communicating (i.e., `comm_small` if you are sending or broadcasting a few values, `comm_large` if you are sending all of your local values).
      - Notice that auxillary functions like MPI_init are not under here.
    - `comp` - All computation functions within your algorithm should be nested under the `comp` region.
      - Inside the `comp` region, you should create regions to indicate how much data you are computing on (i.e., `comp_small` if you are sorting a few values like the splitters, `comp_large` if you are sorting values in the array).
      - Notice that auxillary functions like data_init are not under here.
    - `MPI_X` - You will also see MPI regions in the calltree if using the appropriate MPI profiling configuration (see **Builds/**). Examples shown below.

All functions will be called from `main` and most will be grouped under either `comm` or `comp` regions, representing communication and computation, respectively. You should be timing as many significant functions in your code as possible. **Do not** time print statements or other insignificant operations that may skew the performance measurements.

### **Nesting Code Regions Example** - all computation code regions should be nested in the "comp" parent code region as following:
```
CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_small");
sort_pivots(pivot_arr);
CALI_MARK_END("comp_small");
CALI_MARK_END("comp");

# Other non-computation code
...

CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_large");
sort_values(arr);
CALI_MARK_END("comp_large");
CALI_MARK_END("comp");
```

### **Calltree Example**:
```
# MPI Mergesort
4.695 main
├─ 0.001 MPI_Comm_dup
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Finalized
├─ 0.000 MPI_Init
├─ 0.000 MPI_Initialized
├─ 2.599 comm
│  ├─ 2.572 MPI_Barrier
│  └─ 0.027 comm_large
│     ├─ 0.011 MPI_Gather
│     └─ 0.016 MPI_Scatter
├─ 0.910 comp
│  └─ 0.909 comp_large
├─ 0.201 data_init_runtime
└─ 0.440 correctness_check
```

### **Samplesort Calltree**:
```
0.545 main
├─ 0.000 MPI_Init
├─ 0.000 data_init_runtime
├─ 0.009 comm
│  ├─ 0.005 MPI_Bcast
│  ├─ 0.001 comm_small
│  │  └─ 0.001 MPI_Scatter
│  ├─ 0.002 MPI_Alltoall
│  └─ 0.000 MPI_Gather
├─ 0.002 comp
│  └─ 0.002 comp_large
├─ 0.001 MPI_Gather
├─ 0.000 MPI_Bcast
├─ 0.001 comp_large
├─ 0.000 correctness_check
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.003 MPI_Comm_dup
```


### **Mergesort Calltree**:
```
0.566 main
├─ 0.004 MPI_Comm_dup
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Finalized
├─ 0.000 MPI_Init
├─ 0.000 MPI_Initialized
├─ 0.008 comm
│  └─ 0.008 comm_large
│     ├─ 0.002 MPI_Recv
│     ├─ 0.007 MPI_Scatter
│     └─ 0.001 MPI_Send
├─ 0.002 comp
│  └─ 0.002 comp_large
├─ 0.000 correctness_check
└─ 0.000 data_init_runtime
```


### **Radixsort Calltree**:
```
2.875 main
├─ 2.743 main
│  ├─ 0.000 MPI_Init
│  ├─ 0.000 data_init_runtime
│  ├─ 0.007 comm
│  │  └─ 0.007 comm_large
│  │     ├─ 0.004 MPI_Bcast
│  │     ├─ 0.001 MPI_Scatter
│  │     └─ 0.002 MPI_Gather
│  ├─ 0.002 comp
│  │  └─ 0.002 comp_large
│  └─ 0.008 correctness_check
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.121 MPI_Comm_dup
```

### **Bitonicsort Calltree**:
```
0.567 main
├─ 0.561 main
│  ├─ 0.000 MPI_Init
│  ├─ 0.000 data_init_runtime
│  ├─ 0.002 comm
│  │  └─ 0.002 comm_large
│  │     ├─ 0.002 MPI_Scatter
│  │     └─ 0.000 MPI_Gather
│  ├─ 0.030 comp
│  │  └─ 0.030 comp_large
│  │     └─ 0.028 comp_small
│  │        ├─ 0.019 comm_small
│  │        │  └─ 0.000 MPI_Sendrecv
│  │        └─ 0.009 MPI_Barrier
│  └─ 0.000 correctness_check
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.000 MPI_Comm_dup
```

### 3b. Collect Metadata

Have the following code in your programs to collect metadata:

```
adiak::init(NULL);
adiak::launchdate();    // launch date of the job
adiak::libraries();     // Libraries used
adiak::cmdline();       // Command line used to launch the job
adiak::clustername();   // Name of the cluster
adiak::value("algorithm", algorithm); // The name of the algorithm you are using (e.g., "merge", "bitonic")
adiak::value("programming_model", programming_model); // e.g. "mpi"
adiak::value("data_type", data_type); // The datatype of input elements (e.g., double, int, float)
adiak::value("size_of_data_type", size_of_data_type); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
adiak::value("input_size", input_size); // The number of elements in input dataset (1000)
adiak::value("input_type", input_type); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
adiak::value("num_procs", num_procs); // The number of processors (MPI ranks)
adiak::value("scalability", scalability); // The scalability of your algorithm. choices: ("strong", "weak")
adiak::value("group_num", group_number); // The number of your group (integer, e.g., 1, 10)
adiak::value("implementation_source", implementation_source); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").
```

They will show up in the `Thicket.metadata` if the caliper file is read into Thicket.

### **See the `Builds/` directory to find the correct Caliper configurations to get the performance metrics.** They will show up in the `Thicket.dataframe` when the Caliper file is read into Thicket.

### Samplesort Metadata Image

<img width="893" alt="Screenshot 2024-10-16 at 12 12 42 PM" src="https://github.com/user-attachments/assets/8cb51b37-9263-44c3-bcc1-f72ccaf5359a">


### Mergesort Metadata Image

<img width="893" alt="samplesort_metadata_image" src="/image/mergesort_metadata.png">


### Radixsort Metadata Image

<img width="893" src="/image/radix_metadata.png">


### Bitonicsort Metadata Image

<img width="893" src="/image/bitonic_metadata.png">



## 4. Performance evaluation

Include detailed analysis of computation performance, communication performance. 
Include figures and explanation of your analysis.

### 4a. Vary the following parameters
For input_size's:
- 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28

For input_type's:
- Sorted, Random, Reverse sorted, 1%perturbed

MPI: num_procs:
- 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024

This should result in 4x7x10=280 Caliper files for your MPI experiments.

### 4b. Hints for performance analysis

To automate running a set of experiments, parameterize your program.

- input_type: "Sorted" could generate a sorted input to pass into your algorithms
- algorithm: You can have a switch statement that calls the different algorithms and sets the Adiak variables accordingly
- num_procs: How many MPI ranks you are using

When your program works with these parameters, you can write a shell script 
that will run a for loop over the parameters above (e.g., on 64 processors, 
perform runs that invoke algorithm2 for Sorted, ReverseSorted, and Random data).  

### 4c. You should measure the following performance metrics
- `Time`
    - Min time/rank
    - Max time/rank
    - Avg time/rank
    - Total time
    - Variance time/rank

### Samplesort

#### **Note: 512 and 1024 processors did not work because of HPRC Hydra Errors, so all graphs are scaled to 256 processors**

<img width="500" alt="Screenshot 2024-10-16 at 12 12 42 PM" src="https://github.com/user-attachments/assets/6d9dcea5-9ac4-4c39-a142-8a8d09f8b988">

The computation time steadily decreases in my algorithm since an increase in the number of processes will more efficiently do the swaps and sorting needed. The algorithm nicely scales as processor numbers increase. 

<img width="500" alt="Screenshot 2024-10-16 at 12 12 42 PM" src="https://github.com/user-attachments/assets/0c5a773e-cb6d-499a-82e6-6d4c64ec0eda">

The speedup and weak scaling show much of the same. Speedup graphs compare the effectiveness of different programs (Different array sizes) against that of a base case: two processors. In accordance with Amdahl's law, my algorithms correctly speedup. 

<img width="500" alt="Screenshot 2024-10-16 at 12 12 42 PM" src="https://github.com/user-attachments/assets/ce8aedea-652e-4576-a0ee-79bb15e0e221">

Despite how lopsided the graph looks, Weak scaling measures how the algorithm performs with differing problem sizes. In my algorithm, the largest array size time trumps that of the others, and causes a long spike for the smallest processor size. 

<img width="500" alt="Screenshot 2024-10-16 at 12 12 42 PM" src="https://github.com/user-attachments/assets/9fe5f26c-8dfb-44b1-9032-bd022fc285ab">

The communication plot is very standard, especially for the algorithm's use on larger arrays. As the number of processors increases, my Samplesort algorithm has to spread data among a larger number of buckets, and gather them an increasing amount. 

<img width="500" alt="Screenshot 2024-10-16 at 12 12 42 PM" src="https://github.com/user-attachments/assets/fac87bb6-7eaa-45fd-ac4f-4b18dd93da3d">
<img width="500" alt="Screenshot 2024-10-16 at 12 12 42 PM" src="https://github.com/user-attachments/assets/6ba0d1bb-10f8-4e6f-a878-92d251ffe72a">

Lastly, much like in previous lab programs, the overall time for computing the algorithm goes down as # of processors increases in larger sample sizes, but will go up in smaller sizes. This is due to the communication and initialization pieces of the algorithm, which often trump the computation part in smaller array sizes, causing the overall time to increase as processor numbers do. 

### Merge Sort

#### **Note: 512 and 1024 processors did not work because of HPRC Hydra Errors, so all graphs are scaled to 256 processors**

<img width="500" alt="Strong Scaling Computation" src="/image/StrongScalingComp2.png">

<img width="500" alt="Strong Scaling Communication" src="/image/StrongScalingComm2.png">

As the number of processors increased from 2 to 64, the average time spent in computation sharply decreases. This makese sense because the work of the computation is distributed across more and more processors. As we continue to add more processors, we see diminishing returns as there is too much overhead to warrant distributing the array among more processors. Meanwhile, the average time spent in communication generally increases as the number of processors increase.

<img width="500" alt="Speedup" src="/image/Speedup2.png">

The speedup for computation increases sharply at first as the workload is efficiently distributed across more processors, but then gradually decreases due to diminishing returns, where each processor receives less work, and overheads such as synchronization and communication start to outweigh the benefits. 

<img width="500" alt="Weak Scaling" src="/image/WeakScaling2.png">

The benefits of parallel computing can clearly be seen as the problem size becomes increasinly large. Computation time benefitted most across multiple processors for an array size of 2^28. This makes sense as larger problem sizes allow each processor to have a substantial amount of work to do, maximizing the efficiency of parallelism.


### Radix Sort

#### **Note: Some of the cali files failed to generate especially with some of the 2^28 array size and 1024 processor. This is because my program rely on only the root process to do the merging part at the end of the sort when all the worker threads send local sorted arrays back to root

Strong Scaling: The computation time decreases as the number of processor increases and the array size stays constant. As the number of processor doubles, the computation time is cut almost in half everytime.
<img width="978" alt="Screenshot 2024-10-24 at 2 15 45 PM" src="https://github.com/user-attachments/assets/60b36ce7-73ef-4f44-bb6f-5d8247d1affa">

The communication time increases when the number of the processor increases, this is because more communication between processes are done when there are more processors.
<img width="621" alt="Screenshot 2024-10-24 at 2 18 25 PM" src="https://github.com/user-attachments/assets/62dd86a1-cae2-4e02-be48-70aeeca6c2c4">

Speed up: The speed up is significant when we first introducing more processors, each processes are being used efficiently. However, as more and more processors are added, we spent more time on 
municating between each processes, which affects the efficiently of the overall runtime. 
com<img width="668" alt="Screenshot 2024-10-24 at 2 19 22 PM" src="https://github.com/user-attachments/assets/d615eb5a-cd3b-4ea5-8048-9998437ecc03">

### Bitonic Sort


<img width="500" src="/image/bitonic_avg_268435456.png">

Average times vs Number of Processors for Array Size 268435456

The average times show a rapid decrease as the number of processors increases. This drop is expected as adding more processors initially reduces the computation time significantly due to division of work. However, beyond a certain number of processors, communication overhead costs start to dominate, leading to diminishing returns. 

<img width="500" src="/image/bitonic_total_67108864.png">

Total times vs Number of Processors for Array Size 67108864

The total time increases steadily up to 128 processors, but for larger processors counts, the performance either levels off or slightly increases. This is because the higher processor counts lead to a point where the cost of communication outweights the benefits of parallel computation. 

<img width="500" src="/image/bitonic_min_16777216.png">

Minimum times vs Number of Processors for Array Size 16777216

Minimum time drops quickly as the number of processors increase. These minimum times highlight the most efficient process. The rapid decline is expected as the workload is distributed. 

<img width="500" src="/image/bitonic_max_268435456.png">

Maximum times vs Number of Processors for Array Size 268435456

There is a steep decline between 16 and 64 processors. The maximum times represent the worst-case performance. The flatening of the graph may be due to diminishing returns, where adding more processors does not significantly reduce computation time. 

## 5. Presentation
Plots for the presentation should be as follows:
- For each implementation:
    - For each of comp_large, comm, and main:
        - Strong scaling plots for each input_size with lines for input_type (7 plots - 4 lines each)
        - Strong scaling speedup plot for each input_type (4 plots)
        - Weak scaling plots for each input_type (4 plots)

Analyze these plots and choose a subset to present and explain in your presentation.

## 6. Final Report
Submit a zip named `TeamX.zip` where `X` is your team number. The zip should contain the following files:
- Algorithms: Directory of source code of your algorithms.
- Data: All `.cali` files used to generate the plots seperated by algorithm/implementation.
- Jupyter notebook: The Jupyter notebook(s) used to generate the plots for the report.
- Report.md
