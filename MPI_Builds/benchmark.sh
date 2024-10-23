#!/bin/bash


# Grace properties
cores_per_node=32
mem_per_node=32  # Adjusted to reflect the new memory specification (32G)


# Job file replacements
replace_proc="NUM-PROCESSES"
replace_nodes="NUM_NODES"
replace_tasks="NUM_TASKS"
replace_mem="MEM_NEEDED"


# Input combinations
input_sizes=(65536 262144 1048576 4194304 16777216 67108864 268435456)  # 2^16 to 2^28
input_types=("random" "sorted" "reverse" "perturbed")
num_processes=(2 4 8 16 32 64 128 256 512 1024)


# Loop over input sizes, types, and number of processes
for size in "${input_sizes[@]}"; do
   for type in "${input_types[@]}"; do
        p=4

        # Determine number of nodes needed
        num_nodes=$(($p / $cores_per_node))
        if (($p % $cores_per_node > 0)); then
            num_nodes=$(($num_nodes + 1));
        fi


        # Determine tasks per node needed
        num_tasks=$(($p / $num_nodes))
        if (($p % $num_nodes > 0)); then
            num_tasks=$(($num_tasks + 1));
        fi


        # Determine memory needed per node
        per_process_memory=3
        mem_needed=$(($num_tasks * $per_process_memory))


        # Create altered job file
        sed "s/$replace_nodes/$num_nodes/; s/$replace_tasks/$num_tasks/; s/$replace_mem/${mem_needed}G/" mpi.grace_job > temp.grace_job
        
        # Dispatch job with input parameters: size, processes, and type
        sbatch temp.grace_job $size $p $type
        
        # Remove altered job file
        rm temp.grace_job
    done
done
