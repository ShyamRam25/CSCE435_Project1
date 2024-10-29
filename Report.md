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

#### **Plots for the report are detailed below**



## 6. Final Report
Submit a zip named `TeamX.zip` where `X` is your team number. The zip should contain the following files:
- Algorithms: Directory of source code of your algorithms.
- Data: All `.cali` files used to generate the plots seperated by algorithm/implementation.
- Jupyter notebook: The Jupyter notebook(s) used to generate the plots for the report.
- Report.md

### Samplesort Plots

Some things to note about the Samplesort plots are the spikes in communication, especially for smaller array sizes. Shown in the weak scaling and speedup plots, the communication can greatly vary because of the Gather and Broadcast functions being called, with Samplesort using different buckets to sort the array in "smaller" chunks. As the array gets a larger size the communication overhead is taken over by the speed of computation, leading to a cleaner speedup in the main algorithm (Shown in the last few plots). Samplesort also uses a standardized sorting algorithm to sort each individual bucket, which causes it to have almost all array types to perform similarly, with only random having a slight discrepency with smaller processor numbers. 

<img width="500" src="https://github.com/user-attachments/assets/efc606b8-38d3-42a0-8454-16bedf0ab580">
<img width="500" src="https://github.com/user-attachments/assets/57b06c6c-9606-4434-a777-a3666cf1f7fc">
<img width="500" src="https://github.com/user-attachments/assets/06faebec-0170-4b53-be78-29d9949a5455">
<img width="500" src="https://github.com/user-attachments/assets/0530057d-25ae-4b1b-83bc-c57001a1efdc">
<img width="500" src="https://github.com/user-attachments/assets/6721a480-e6fb-470e-96d0-e9b7a00fe45b">
<img width="500" src="https://github.com/user-attachments/assets/2b6fce84-577a-40b8-9fd4-b0ef91e7694f">
<img width="500" src="https://github.com/user-attachments/assets/40093def-e45d-4f66-bee4-d5df8fa74491">
<img width="500" src="https://github.com/user-attachments/assets/d9527853-7a5b-42c6-b053-721a8c8a6c53">
<img width="500" src="https://github.com/user-attachments/assets/2338492e-07ed-44c8-ad91-4417c9d55b05">
<img width="500" src="https://github.com/user-attachments/assets/f539dcbd-5f90-4434-9bb7-eefdf3718e13">
<img width="500" src="https://github.com/user-attachments/assets/1fc3bd5f-3ab3-4a9c-b844-8662cc04ffc3">
<img width="500" src="https://github.com/user-attachments/assets/8057b4f5-1d87-43dd-93cb-e886b68f5f7a">
<img width="500" src="https://github.com/user-attachments/assets/dbe22811-e24a-4bc4-b554-3234aae30fb8">
<img width="500" src="https://github.com/user-attachments/assets/ab984ff4-b369-4e41-81c9-27cca823a27d">
<img width="500" src="https://github.com/user-attachments/assets/e2629958-64f5-4ba9-ac2e-075f52a2826b">
<img width="500" src="https://github.com/user-attachments/assets/21a2c439-8c39-4a5d-b7cd-85d23abbbab3">
<img width="500" src="https://github.com/user-attachments/assets/3e481e10-bc6d-4da1-b326-f5993f049bbb">
<img width="500" src="https://github.com/user-attachments/assets/bb2b99a6-ce1d-4349-bb5f-fee8dea6a9b8">
<img width="500" src="https://github.com/user-attachments/assets/48ef20d0-4a32-4470-a2f1-e0ab8db89219">
<img width="500" src="https://github.com/user-attachments/assets/c6e9a964-2443-4f9c-aaf3-25d0d0cf8a70">
<img width="500" src="https://github.com/user-attachments/assets/08de9634-cbf4-4309-87c9-947cd66163d1">
<img width="500" src="https://github.com/user-attachments/assets/c7d326e0-7b43-4591-bd0c-dd88224b51cf">
<img width="500" src="https://github.com/user-attachments/assets/6df7577d-e1c1-4f56-a819-8b8019c4f92b">
<img width="500" src="https://github.com/user-attachments/assets/4b497d11-faf4-459a-a299-9efeef590124">
<img width="500" src="https://github.com/user-attachments/assets/aff6f242-8be2-4797-bf20-c6ac5b02a49b">
<img width="500" src="https://github.com/user-attachments/assets/4a4138a7-ee4a-4273-b7a7-d999e5c12532">
<img width="500" src="https://github.com/user-attachments/assets/5d3e7121-9f34-4529-95f2-0baafb0a33db">
<img width="500" src="https://github.com/user-attachments/assets/5391f091-adc4-452e-9d59-19da4d6064ac">
<img width="500" src="https://github.com/user-attachments/assets/d1a4c84a-d521-458f-b7e8-5593881677e8">
<img width="500" src="https://github.com/user-attachments/assets/eba4ed09-f969-4240-a77f-69de35f52ed5">
<img width="500" src="https://github.com/user-attachments/assets/a5d93c05-9d34-417d-9f04-39620f153f69">
<img width="500" src="https://github.com/user-attachments/assets/d9a50ffd-e893-4b56-974e-cbc29f824979">
<img width="500" src="https://github.com/user-attachments/assets/b4d51a72-8d1e-4ee3-8a57-54a9cfc22c11">
<img width="500" src="https://github.com/user-attachments/assets/61388a1e-2f6a-4dc0-9d8d-4b7b79d5b4e4">
<img width="500" src="https://github.com/user-attachments/assets/617aca0a-cda7-4d0f-b6d9-fbeb45c8718c">
<img width="500" src="https://github.com/user-attachments/assets/935da9f2-19a5-43f3-b0bd-d89972b9763b">
<img width="500" src="https://github.com/user-attachments/assets/7de918f2-5a4e-4c2c-aee5-997ddc263c67">
<img width="500" src="https://github.com/user-attachments/assets/bf4dac9a-e423-4989-9657-da9b62705f1b">
<img width="500" src="https://github.com/user-attachments/assets/65c86331-975f-435e-854d-3824a5435c6d">
<img width="500" src="https://github.com/user-attachments/assets/05094aa7-01e6-4b57-a54b-26daeac6e808">
<img width="500" src="https://github.com/user-attachments/assets/08b53a48-2884-48c3-8e1c-e8656b1c34d9">
<img width="500" src="https://github.com/user-attachments/assets/8b956f86-028e-4ae0-9d3d-b502bd8cb168">
<img width="500" src="https://github.com/user-attachments/assets/671f9125-1c01-432f-ad7c-5e50093b0c12">
<img width="500" src="https://github.com/user-attachments/assets/004cb4c7-016b-4fd4-bd33-cf152a5f75a3">


### Mergesort Plots
<img width="500" src="https://github.com/user-attachments/assets/cea18931-d4fa-4793-8c22-dd4d5e01d198">
<img width="500" src="https://github.com/user-attachments/assets/f568daab-8f06-42ab-aef7-9f29cd2ed37b">
<img width="500" src="https://github.com/user-attachments/assets/ff920f99-56a0-4784-9fd9-dcb2dbdfd60b">
<img width="500" src="https://github.com/user-attachments/assets/51993a76-fc8f-47b0-8edd-82c48627ab5d">
<img width="500" src="https://github.com/user-attachments/assets/db4567c1-7e14-452b-bf99-d01a3224b9ce">
<img width="500" src="https://github.com/user-attachments/assets/3c0ffa63-f14f-47bc-89c8-e6ae17a09d0e">
<img width="500" src="https://github.com/user-attachments/assets/faccfa92-f1d4-489a-a96b-d59f7a1b1225">

<img width="500" src="https://github.com/user-attachments/assets/3d360660-f13f-4446-86a9-5949906cbc8a">
<img width="500" src="https://github.com/user-attachments/assets/04745f8b-017b-4754-a708-680aee4ac859">
<img width="500" src="https://github.com/user-attachments/assets/dc10c40a-bdb9-4e91-b22b-612687405ba9">
<img width="500" src="https://github.com/user-attachments/assets/a0e70a4c-a5a9-4761-81ae-3c5f3f63fd39">

<img width="500" src="https://github.com/user-attachments/assets/c94737cf-9850-4fd5-b15f-929fbae178f1">
<img width="500" src="https://github.com/user-attachments/assets/3cd64a5d-3a52-45f8-88f7-5549f5fd4798">
<img width="500" src="https://github.com/user-attachments/assets/3fac64e5-90bf-4329-b51d-b5e11f67c87e">
<img width="500" src="https://github.com/user-attachments/assets/727c16b8-c7b3-4d85-8ed4-86f28d7fdf4e">



<img width="500" src="https://github.com/user-attachments/assets/d0482c0a-2c0c-40e1-9ca0-ea834e016042">
<img width="500" src="https://github.com/user-attachments/assets/469b0db2-8814-4e75-8aef-57fb977d170d">
<img width="500" src="https://github.com/user-attachments/assets/7bcef8a3-9c79-482e-bec3-05c52e4d6d1c">
<img width="500" src="https://github.com/user-attachments/assets/a35926b3-5a89-43b5-90a1-ecef39cb9ad8">
<img width="500" src="https://github.com/user-attachments/assets/fabfed0c-7722-47f0-b176-e9a1737bb33f">
<img width="500" src="https://github.com/user-attachments/assets/c1146b23-b468-4409-a58c-a803b26a1532">
<img width="500" src="https://github.com/user-attachments/assets/e713b835-f8be-48da-bac6-bb62e4e775ad">

<img width="500" src="https://github.com/user-attachments/assets/deb2ce6b-03f4-4d5a-90e3-0257c2bbd075">
<img width="500" src="https://github.com/user-attachments/assets/8b2cb827-02fb-43c1-b0b9-c07d3289a435">
<img width="500" src="https://github.com/user-attachments/assets/1ff7b64c-f1c6-4966-af67-a7a1542f3132">
<img width="500" src="https://github.com/user-attachments/assets/32c720d8-6885-4305-960f-fc6bef755917">

<img width="500" src="https://github.com/user-attachments/assets/53f60424-d286-498b-8a15-9a1fc814d950">
<img width="500" src="https://github.com/user-attachments/assets/3b50a073-9b77-4343-9d5d-2aca5512e66e">
<img width="500" src="https://github.com/user-attachments/assets/847cdc53-c3cf-41bb-be2c-77f0b02c2d06">
<img width="500" src="https://github.com/user-attachments/assets/02fc754c-7071-4f0a-9c58-8ad03a4f6434">



<img width="500" src="https://github.com/user-attachments/assets/828043a9-70fc-4fdb-b8ee-e2be05d4b036">
<img width="500" src="https://github.com/user-attachments/assets/eb8b7185-2661-4654-aed5-a7c0ef1f7268">
<img width="500" src="https://github.com/user-attachments/assets/25adf730-43dc-4a82-8d31-21abf791372f">
<img width="500" src="https://github.com/user-attachments/assets/9ad86138-6222-4d02-98a7-ecc09b19c746">
<img width="500" src="https://github.com/user-attachments/assets/90136150-0e75-42e0-a09c-653a5db15793">
<img width="500" src="https://github.com/user-attachments/assets/a93ca324-93b4-4a1f-8e7f-b378b06ba783">
<img width="500" src="https://github.com/user-attachments/assets/15a1876c-f04a-4ea0-bf09-3b3c1c8d8ebc">

<img width="500" src="https://github.com/user-attachments/assets/6b6bb8e3-45cf-4c47-81b8-08e9f8f4e37d">
<img width="500" src="https://github.com/user-attachments/assets/c6589599-99d5-4873-9726-9382fdd32637">
<img width="500" src="https://github.com/user-attachments/assets/d386572a-fcda-4daa-a2c8-c0bf2e157b48">
<img width="500" src="https://github.com/user-attachments/assets/23a0cbb6-072c-4a7c-b42d-0066a15f539b">

<img width="500" src="https://github.com/user-attachments/assets/e4ae2050-d231-426b-bb8f-6fd049db7a36">
<img width="500" src="https://github.com/user-attachments/assets/57c67b13-c770-44fc-9d2c-6b84578dcd14">
<img width="500" src="https://github.com/user-attachments/assets/cc98e999-824a-423e-8ee5-6f061c899159">
<img width="500" src="https://github.com/user-attachments/assets/5be5763f-776c-4944-9039-18cf0636078c">



<img width="500" src="https://github.com/user-attachments/assets/34496131-f031-4ef2-91cc-f735bcb3f74c">
<img width="500" src="https://github.com/user-attachments/assets/f080eb22-8d13-40b5-8efe-c561c1872e8e">
<img width="500" src="https://github.com/user-attachments/assets/6f865cb8-5ee7-4e73-9eb1-69d6584b11cc">
<img width="500" src="https://github.com/user-attachments/assets/89b969fa-c32a-4f72-8ffa-6bd25cf253aa">
<img width="500" src="https://github.com/user-attachments/assets/6f8080b5-5eca-4e75-85d3-8f4a45a8509c">
<img width="500" src="https://github.com/user-attachments/assets/c3518cb4-88d9-4bc1-b45a-406bd3ce708a">
<img width="500" src="https://github.com/user-attachments/assets/2ea02225-51ce-4b94-825f-b1cc57071b97">

<img width="500" src="https://github.com/user-attachments/assets/2a68c5dd-a743-42ee-ac2c-be4d7527c14c">
<img width="500" src="https://github.com/user-attachments/assets/ccf2acaa-7d1f-429a-8e27-4cf5a445cdd3">
<img width="500" src="https://github.com/user-attachments/assets/bc9efb33-8091-4b3e-a3a1-2ba5810bb3dd">
<img width="500" src="https://github.com/user-attachments/assets/ae432dd1-234e-458e-9840-2de7158fc926">



<img width="500" src="https://github.com/user-attachments/assets/7441cc3f-a97b-4175-b136-66ece83361f7">
<img width="500" src="https://github.com/user-attachments/assets/85d5220a-1c9a-4ba3-a691-942fbe149934)">
<img width="500" src="https://github.com/user-attachments/assets/a5cb0358-5d23-4d3e-942e-2b6f5750e379)">
<img width="500" src="https://github.com/user-attachments/assets/611f40ce-45cc-4f76-a06c-8d4d2dbb69a3)">
<img width="500" src="https://github.com/user-attachments/assets/71f193b8-d79c-4bf2-ace5-39f7d376c85b)">
<img width="500" src="https://github.com/user-attachments/assets/2c9abfb3-abc4-4f07-b227-7ac56d7454cd)">
<img width="500" src="https://github.com/user-attachments/assets/94b29230-45ae-44a1-801c-2d390b550737)">

<img width="500" src="https://github.com/user-attachments/assets/a1d78759-48cb-4fa4-b265-c25d3d626f91">
<img width="500" src="https://github.com/user-attachments/assets/cddeab45-f523-4ca5-9564-a1c6edba20d7">
<img width="500" src="https://github.com/user-attachments/assets/d435f388-a03b-402c-9f19-1cb74fde7444">
<img width="500" src="https://github.com/user-attachments/assets/785ca75e-17ab-4fa3-bb7a-03d667a35ab5">



<img width="500" src="https://github.com/user-attachments/assets/704a1848-1024-4bd9-8799-f2982aec59e4">
<img width="500" src="https://github.com/user-attachments/assets/a9f571c1-348d-4900-9b65-8c51bc5ee4a1">
<img width="500" src="https://github.com/user-attachments/assets/bdeb5a1d-e1c6-4ad6-8af2-64b11584d9d6">
<img width="500" src="https://github.com/user-attachments/assets/f83906a1-7dfa-4f7e-a2d8-29861d89b07a">
<img width="500" src="https://github.com/user-attachments/assets/56948f25-dd6b-4786-b930-a9ca30f3a53b">
<img width="500" src="https://github.com/user-attachments/assets/04358031-d576-4e22-b79a-b5b77146e93a">
<img width="500" src="https://github.com/user-attachments/assets/ac0c4ac0-112e-4604-923b-e22abe1f1ec4">

<img width="500" src="https://github.com/user-attachments/assets/7417327c-a0a7-42f8-81d7-a685b99456f6">
<img width="500" src="https://github.com/user-attachments/assets/922d2c57-1190-44e4-b81d-2150fe31a964">
<img width="500" src="https://github.com/user-attachments/assets/6452906b-356f-47c6-8bfa-aa0e0a2a54b9">
<img width="500" src="https://github.com/user-attachments/assets/1f4a210b-03ff-438b-8fd4-2d56fa6da489">



### Bitonic Sort Plots
<img width="500" src="/image/bitonic/comp_strong_max_65536.png">
<img width="500" src="/image/bitonic/comp_strong_max_242144.png">
<img width="500" src="/image/bitonic/comp_strong_max_1048576.png">
<img width="500" src="/image/bitonic/comp_strong_max_4194304.png">
<img width="500" src="/image/bitonic/comp_strong_max_16777216.png">
<img width="500" src="/image/bitonic/comp_strong_max_67108864.png">
<img width="500" src="/image/bitonic/comp_strong_max_268435456.png">





### Radix Sort Plots
The reason my speed up for main is 0 (first 4 graphs) after 128 processors is because I used the total time for main functions to calculate the speed up, so the communication time dominates the performance of main once there are more than 128 processors, in the following graphs 5-8 I used Max time to calculate the speed up for main, which made more sense for the result of speed up.

Other observations: 
1. the speed up graph for computational time showed that the speed up is increasing rapidly up to 128 processors, the benefit of parallelism is reduced because my algorithm is only using the root process to sort all the local sorted arrays once they are gathered to the root processor.
2. The overall performance of the algorithm is still heavily affected by the communication time
![(Main: Speedup (total time) vs Number of Processors for sorted](https://github.com/user-attachments/assets/1e2c28fd-943b-4ba9-8c9a-85d8e22f1059)
![(Main: Speedup (total time) vs Number of Processors for reverse](https://github.com/user-attachments/assets/907ca6f3-3de8-456c-aa09-adbefd78b008)
![(Main: Speedup (total time) vs Number of Processors for random](https://github.com/user-attachments/assets/f7668443-c57f-4062-9a9b-50ef36abb044)
![(Main: Speedup (total time) vs Number of Processors for perturbed](https://github.com/user-attachments/assets/509a9a56-f457-4e10-8b6a-41ac5b3848e3)


![(Main: Speedup vs Number of Processors for sorted](https://github.com/user-attachments/assets/31133788-cf3b-4559-9c0d-ce127c7022d3)
![(Main: Speedup vs Number of Processors for reverse](https://github.com/user-attachments/assets/8a7029fb-b6e3-4f9e-bb18-0472377418bd)
![(Main: Speedup vs Number of Processors for random](https://github.com/user-attachments/assets/2399a7ac-6c56-401c-953a-9e3feb738c32)
![(Main: Speedup vs Number of Processors for perturbed](https://github.com/user-attachments/assets/783235cd-abb3-4fc8-a26e-46e6e772a254)

![(Main: Weak Scaling vs Number of Processors for sorted](https://github.com/user-attachments/assets/4c33259d-c618-4ef1-bd21-ea31d69c9ec2)
![(Main: Weak Scaling vs Number of Processors for reverse](https://github.com/user-attachments/assets/8193afa6-5303-4409-9761-a5eb586e933e)
![(Main: Weak Scaling vs Number of Processors for random](https://github.com/user-attachments/assets/e25f3623-5c5f-4c6c-9638-b2d4e9a12ff1)
![(Main: Weak Scaling vs Number of Processors for perturbed](https://github.com/user-attachments/assets/bda6b6a1-2b7b-4107-ba0f-abe9ccdd7f74)
![(Main: Total Time vs Number of Processors for 268435456](https://github.com/user-attachments/assets/bd80b33b-35fc-44e3-bbef-e5a70332576e)
![(Main: Total Time vs Number of Processors for 67108864](https://github.com/user-attachments/assets/f14ff53b-2461-41a3-bb0e-c87146923d9d)
![(Main: Total Time vs Number of Processors for 16777216](https://github.com/user-attachments/assets/1372158c-a94a-42a6-8777-39d5d97889a6)
![(Main: Total Time vs Number of Processors for 4194304](https://github.com/user-attachments/assets/141efe4d-79f6-48c1-acd5-3d4b90e1132b)
![(Main: Total Time vs Number of Processors for 1048576](https://github.com/user-attachments/assets/2d9ad167-c675-4e0e-872f-8350d95196df)
![(Main: Total Time vs Number of Processors for 262144](https://github.com/user-attachments/assets/a78b9973-97c4-48f7-b195-8b42a4854c49)
![(Main: Total Time vs Number of Processors for 65536](https://github.com/user-attachments/assets/cd7b677e-915a-42eb-ac1d-d27f98ca2eb9)
![(Main: Speedup vs Number of Processors for sorted](https://github.com/user-attachments/assets/0f1f0bfc-270a-44d7-9d8e-e2921d5de7e7)
![(Main: Speedup vs Number of Processors for reverse](https://github.com/user-attachments/assets/1b2f6400-6dd7-45e5-93d5-9a8b56cc9699)
![(Main: Speedup vs Number of Processors for random](https://github.com/user-attachments/assets/4010f5b1-ed74-449e-b5d7-9bdc946e1446)
![(Main: Speedup vs Number of Processors for perturbed](https://github.com/user-attachments/assets/06991150-5af5-4643-b0df-357b6733ebdb)
![(Main: Max Time vs Number of Processors for 268435456](https://github.com/user-attachments/assets/2a00e611-041c-4a9a-921f-65d0e96eaa53)
![(Main: Max Time vs Number of Processors for 67108864](https://github.com/user-attachments/assets/e4a89669-36ea-44a0-8213-b9e4e59e3082)
![(Main: Max Time vs Number of Processors for 16777216](https://github.com/user-attachments/assets/aef16510-b872-4abf-8120-c9df882f16b2)
![(Main: Max Time vs Number of Processors for 4194304](https://github.com/user-attachments/assets/873e4545-f304-4925-815b-4aed7af6adaa)
![(Main: Max Time vs Number of Processors for 1048576](https://github.com/user-attachments/assets/6c17a5c5-b771-49e0-97d4-b3541e1f8af7)
![(Main: Max Time vs Number of Processors for 262144](https://github.com/user-attachments/assets/d3d74368-de73-453d-8e31-007dc603901e)
![(Main: Max Time vs Number of Processors for 65536](https://github.com/user-attachments/assets/f29e6a99-24b6-495b-9105-70fd89aa5138)
![(Main: Avg Time vs Number of Processors for 268435456](https://github.com/user-attachments/assets/aa8bb539-e141-4513-93cd-ccf3ba93466d)
![(Main: Avg Time vs Number of Processors for 67108864](https://github.com/user-attachments/assets/482cf6a2-fce7-40b4-9c0e-d160c3cb432c)
![(Main: Avg Time vs Number of Processors for 16777216](https://github.com/user-attachments/assets/dcd1f065-3413-4290-a6bb-1d747193f043)
![(Main: Avg Time vs Number of Processors for 4194304](https://github.com/user-attachments/assets/5e020919-d2d3-4c8f-b248-bd5b70989534)
![(Main: Avg Time vs Number of Processors for 1048576](https://github.com/user-attachments/assets/f408177f-8080-4d2c-8ddd-bad0664b012d)
![(Main: Avg Time vs Number of Processors for 262144](https://github.com/user-attachments/assets/77cc6593-5a8f-43d3-8242-85dd74cf7e27)
![(Main: Avg Time vs Number of Processors for 65536](https://github.com/user-attachments/assets/de12ee7a-7845-4168-b659-90f940bfb08c)
![(Comp_Large: Weak Scaling vs Number of Processors for sorted](https://github.com/user-attachments/assets/6a972e0a-7707-43fe-a456-59f92e54ae89)
![(Comp_Large: Weak Scaling vs Number of Processors for reverse](https://github.com/user-attachments/assets/9846585d-36d0-4029-aa54-1cf16591f906)
![(Comp_Large: Weak Scaling vs Number of Processors for random](https://github.com/user-attachments/assets/5a95373f-0e31-4d7b-8a4b-7b3919494259)
![(Comp_Large: Weak Scaling vs Number of Processors for perturbed](https://github.com/user-attachments/assets/a310fc13-3606-4ede-b6d3-48f21fc834df)
![(Comp_Large: Total Time vs Number of Processors for 268435456](https://github.com/user-attachments/assets/191ad3a1-7339-49a3-b5b9-16e9f250d4b1)
![(Comp_Large: Total Time vs Number of Processors for 67108864](https://github.com/user-attachments/assets/a34439d2-593e-47ac-b306-53be1b4b2925)
![(Comp_Large: Total Time vs Number of Processors for 16777216](https://github.com/user-attachments/assets/dfa74f2a-d8c9-41b1-96cb-99368b890f4e)
![(Comp_Large: Total Time vs Number of Processors for 4194304](https://github.com/user-attachments/assets/29beab90-781a-4568-b9bd-5fa32d43c49a)
![(Comp_Large: Total Time vs Number of Processors for 1048576](https://github.com/user-attachments/assets/0a71d69a-e4c0-4cf0-83b8-59c4d485933e)
![(Comp_Large: Total Time vs Number of Processors for 262144](https://github.com/user-attachments/assets/c28a5f59-2971-42ec-8bae-ef3f08f8ceee)
![(Comp_Large: Total Time vs Number of Processors for 65536](https://github.com/user-attachments/assets/fdbc00b4-fe61-4edc-b6c0-eccace450c40)
![(Comp_Large: Speedup vs Number of Processors for sorted](https://github.com/user-attachments/assets/b48779fb-3bd3-499d-afbe-f76d53cefaef)
![(Comp_Large: Speedup vs Number of Processors for reverse](https://github.com/user-attachments/assets/233db646-5fb4-4785-9b90-b00dca985a62)
![(Comp_Large: Speedup vs Number of Processors for random](https://github.com/user-attachments/assets/b6344473-cb3b-4442-9ad5-438950ad901f)
![(Comp_Large: Speedup vs Number of Processors for perturbed](https://github.com/user-attachments/assets/38502268-efd5-4809-b981-be53a09f89d3)
![(Comp_Large: Max Time vs Number of Processors for 268435456](https://github.com/user-attachments/assets/d7a36d6c-d2bc-4b5b-aa57-435dfb9f9cc5)
![(Comp_Large: Max Time vs Number of Processors for 67108864](https://github.com/user-attachments/assets/a1713e8e-1388-48f8-9c72-9b7dcb5829a3)
![(Comp_Large: Max Time vs Number of Processors for 16777216](https://github.com/user-attachments/assets/d7964a45-22ea-459e-8f0a-ba709df4733f)
![(Comp_Large: Max Time vs Number of Processors for 4194304](https://github.com/user-attachments/assets/8ee1983a-b3b4-43db-be51-a400d13d77e3)
![(Comp_Large: Max Time vs Number of Processors for 1048576](https://github.com/user-attachments/assets/7c2c7738-c66b-4302-a2d4-63e66588b03d)
![(Comp_Large: Max Time vs Number of Processors for 262144](https://github.com/user-attachments/assets/21110989-d6d0-40e5-a8b9-554f2f79bf3c)
![(Comp_Large: Max Time vs Number of Processors for 65536](https://github.com/user-attachments/assets/fb7923c1-2024-4b37-825d-872b715f4c16)
![(Comp_Large: Average Time vs Number of Processors for 268435456](https://github.com/user-attachments/assets/59001c3f-49f5-4381-8f33-50bfea772f78)
![(Comp_Large: Average Time vs Number of Processors for 67108864](https://github.com/user-attachments/assets/f9dd4906-81fd-4cc1-a477-1f40d6d8c629)
![(Comp_Large: Average Time vs Number of Processors for 16777216](https://github.com/user-attachments/assets/86d365a4-b715-412c-af67-20fe0781a1a2)
![(Comp_Large: Average Time vs Number of Processors for 4194304](https://github.com/user-attachments/assets/4d4ee68a-d408-4876-ae03-ddd5b8bc4b6d)
![(Comp_Large: Average Time vs Number of Processors for 1048576](https://github.com/user-attachments/assets/7783d6fd-ff10-434a-b46c-df28ca213560)
![(Comp_Large: Average Time vs Number of Processors for 262144](https://github.com/user-attachments/assets/74448506-0792-4d42-9b04-d1ce1244c155)
![(Comp_Large: Average Time vs Number of Processors for 65536](https://github.com/user-attachments/assets/81c46543-5a92-49a7-9bbc-837ec7e0bf4f)
![(Comm: Weak Scaling vs Number of Processors for sorted](https://github.com/user-attachments/assets/b1b89212-f147-4b8d-abf4-b2ca1d5b68df)
![(Comm: Weak Scaling vs Number of Processors for reverse](https://github.com/user-attachments/assets/669eb112-4e27-4d80-9385-45eb46ca79a2)
![(Comm: Weak Scaling vs Number of Processors for random](https://github.com/user-attachments/assets/be885db8-893c-4a92-be91-d747c3b36597)
![(Comm: Weak Scaling vs Number of Processors for perturbed](https://github.com/user-attachments/assets/6a71493d-23e2-493e-820a-ffae52a7ca8d)
![(Comm: Total Time vs Number of Processors for 268435456](https://github.com/user-attachments/assets/18036c8b-8a08-4b26-93f8-867718721fcb)
![(Comm: Total Time vs Number of Processors for 67108864](https://github.com/user-attachments/assets/b51b30b6-4e04-48ab-90d1-7dceb94035c3)
![(Comm: Total Time vs Number of Processors for 16777216](https://github.com/user-attachments/assets/98586fe8-1435-4b95-9127-d9ce7d545ccc)
![(Comm: Total Time vs Number of Processors for 4194304](https://github.com/user-attachments/assets/6a38b65d-2e19-4633-b36f-3a5d140b77ac)
![(Comm: Total Time vs Number of Processors for 1048576](https://github.com/user-attachments/assets/fe4b168f-8f65-4e8a-a847-f4cd133face2)
![(Comm: Total Time vs Number of Processors for 262144](https://github.com/user-attachments/assets/50761b39-665a-4dcd-bc77-51c238f30b29)
![(Comm: Total Time vs Number of Processors for 65536](https://github.com/user-attachments/assets/3bb39594-4186-437a-91a9-ed7fea1dca15)
![(Comm: Speedup vs Number of Processors for sorted](https://github.com/user-attachments/assets/e73ede23-772a-48cb-8514-b4d397fdb901)
![(Comm: Speedup vs Number of Processors for reverse](https://github.com/user-attachments/assets/ff995edb-05af-4600-b993-c231daa46602)
![(Comm: Speedup vs Number of Processors for random](https://github.com/user-attachments/assets/6901df0d-3826-47ae-869d-a980ae091c2a)
![(Comm: Speedup vs Number of Processors for perturbed](https://github.com/user-attachments/assets/e54d784e-4892-44c8-8c5c-7b64ead2c22b)
![(Comm: Max Time vs Number of Processors for 268435456](https://github.com/user-attachments/assets/e4a713eb-09f7-4920-b68d-e60aaa85f312)
![(Comm: Max Time vs Number of Processors for 67108864](https://github.com/user-attachments/assets/e70d571d-8e96-43c8-862a-c840cdb613c2)
![(Comm: Max Time vs Number of Processors for 16777216](https://github.com/user-attachments/assets/ba9587ed-cb5e-4c42-a558-5c12f7e2f3d2)
![(Comm: Max Time vs Number of Processors for 4194304](https://github.com/user-attachments/assets/d2d3706c-73f6-41af-afe7-5cd014d0b061)
![(Comm: Max Time vs Number of Processors for 1048576](https://github.com/user-attachments/assets/094bedc3-71e4-4c06-b990-544893d05e6a)
![(Comm: Max Time vs Number of Processors for 262144](https://github.com/user-attachments/assets/eec09f56-1bbf-461c-b41f-6e78e68b220a)
![(Comm: Max Time vs Number of Processors for 65536](https://github.com/user-attachments/assets/e2b250f9-79fe-4c77-8a95-22e9900559f7)
![(Comm: Avg Time vs Number of Processors for 268435456](https://github.com/user-attachments/assets/f75e6531-4830-4b1e-a390-4d7b23859788)
![(Comm: Avg Time vs Number of Processors for 67108864](https://github.com/user-attachments/assets/34e8ed88-d105-4ee6-a49e-ad9b7926a89b)
![(Comm: Avg Time vs Number of Processors for 16777216](https://github.com/user-attachments/assets/2e918827-c13e-4d18-9d4e-64a7044792cc)
![(Comm: Avg Time vs Number of Processors for 4194304](https://github.com/user-attachments/assets/ef32b08a-778b-4e3f-bc54-68594b8216d6)
![(Comm: Avg Time vs Number of Processors for 1048576](https://github.com/user-attachments/assets/49f5ae5c-4891-4c04-94c2-9e3d97966863)
![(Comm: Avg Time vs Number of Processors for 262144](https://github.com/user-attachments/assets/3c0aced0-84f1-475b-a56f-f0e282c82b6b)
![(Comm: Avg Time vs Number of Processors for 65536](https://github.com/user-attachments/assets/39c23d58-f921-4811-897e-37eea6c5b160)



