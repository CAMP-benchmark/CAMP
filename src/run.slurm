#!/bin/bash

# Slurm job options (name, compute nodes, job time)
#SBATCH --job-name=CAMP
#SBATCH --time=10:20:0
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=128

# Replace [budget code] below with your project code (e.g. t01)
#SBATCH --account=[budget code]
#SBATCH --partition=standard
#SBATCH --qos=lowpriority

export OMP_NUM_THREADS=128
export OMP_PROC_BIND=true
# export CRAY_OMP_CHECK_AFFINITY=TRUE
# export PAT_RT_SUMMARY=0
# export PAT_RT_PERFCTR=CORE_TO_L2_CACHEABLE_REQUEST_ACCESS_STATUS:LS_RD_BLK_C,PAPI_L2_DCM,PAPI_FP_OPS,mem_bw


# ================ placement ================
# srun --hint=nomultithread /work/d180/d180/a2144962/hwloc/bin/bin/hwloc-ls
# ==== range
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:4 -i 0.1 -f placement/range/contig-none.csv -k contig -d none
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:4 -i 0.1 -f placement/range/contig-cyclicL3.csv -k contig -d cyclic -l 128,64,16,8,4
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:4 -i 0.1 -f placement/range/contig-cyclicL3_1.csv -k contig -d cyclic -l 128,64,16,8,4,1
# ==== socket
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:8 -i 0.1 -f placement/proc/contig-cyclicsocket.csv -k contig -d cyclic -l 128,64,16,8,4
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:8 -i 0.1 -f placement/proc/contig-cyclicnosocket.csv -k contig -d cyclic -l 128,16,8,4
# ==== NUMA
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:8 -i 0.1 -f placement/proc/contig-cyclicnuma.csv -k contig -d cyclic -l 128,64,16,8,4
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:8 -i 0.1 -f placement/proc/contig-cyclicnonuma.csv -k contig -d cyclic -l 128,64,8,4
# ==== CCD
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:8 -i 0.1 -f placement/proc/contig-cyclicccd.csv -k contig -d cyclic -l 128,64,16,8,4
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:8 -i 0.1 -f placement/proc/contig-cyclicnoccd.csv -k contig -d cyclic -l 128,64,16,4
# ==== L3
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:8 -i 1.0 -f placement/mem/contig1.0-cyclicL3.csv -k contig -d cyclic -l 128,16,8,4
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:8 -i 1.0 -f placement/mem/contig1.0-cyclicCCD_1.csv -k contig -d cyclic -l 128,16,8,1
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t +8:64:8 -i 1.0 -f placement/mem/contig1.0-cyclicNUMA_1.csv -k contig -d cyclic -l 128,16,1
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 8,16 -i 1.0 -f placement/mem/contig1.0-cyclicNUMA_L3.csv -k contig -d cyclic -l 128,16,4,1
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 24 -i 1.0 -f placement/mem/contig1.0-cyclicNUMA_L3.csv -k contig -d cpu-bind -c 0,1,4,16,17,20,32,33,36,48,49,52,64,65,68,80,81,84,96,97,100,112,113,116
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 32 -i 1.0 -f placement/mem/contig1.0-cyclicNUMA_L3.csv -k contig -d cpu-bind -c 0,1,4,5,16,17,20,21,32,33,36,37,48,49,52,53,64,65,68,69,80,81,84,85,96,97,100,101,112,113,116,117
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 40 -i 1.0 -f placement/mem/contig1.0-cyclicNUMA_L3.csv -k contig -d cpu-bind -c 0,1,2,4,5,16,17,18,20,21,32,33,34,36,37,48,49,50,52,53,64,65,66,68,69,80,81,82,84,85,96,97,98,100,101,112,113,114,116,117
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 48 -i 1.0 -f placement/mem/contig1.0-cyclicNUMA_L3.csv -k contig -d cpu-bind -c 0,1,2,4,5,6,16,17,18,20,21,22,32,33,34,36,37,38,48,49,50,52,53,54,64,65,66,68,69,70,80,81,82,84,85,86,96,97,98,100,101,102,112,113,114,116,117,118
srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 56,64 -i 1.0 -f placement/mem/contig1.0-cyclicNUMA_L3.csv -k contig -d cyclic -l 128,16,1

# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:8 -i 0.1 -f placement/mem/contig1.0-cyclicCCD.csv -k contig -d cyclic -l 128,16,8
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t +8:64:8 -i 0.1 -f placement/mem/contig1.0-cyclicNUMA.csv -k contig -d cyclic -l 128,16

# ================ verify ================
# for i in 0 1 2 3 4 5 6 7 8 9
# do
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 1,2,4,+8:128:4 -i 0.1 -f verify/std/contig-std-$i.csv -k contig
# done

# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t +1:128:1 -i 0.1 -f verify/contig-cyclic-128.csv -k contig -d cyclic
# srun --hint=nomultithread --unbuffered build/camp-noop -s 1024000000 -r 3 -t +1:128:1 -i 0.1 -f verify/contig-cyclic-noop-128.csv -k contig -d cyclic
# srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t +1:128:1 -i +0.1:2.1:0.2,+2.5:5:0.5,+6:10:1 -f contig-cyclic.csv -k contig -d cyclic

# ================ pattern ================
srun --hint=nomultithread --unbuffered build/camp -s 16000000 -r 3 -t +8:128:8 -i 0.1,0.5,1.1 -f pattern/weak_contig-cyclic.csv -k contig -d cyclic --weak
srun --hint=nomultithread --unbuffered build/camp -s 16000000 -r 3 -t +8:128:8 -i 0.1,0.5,1.1 -f pattern/weak_stride4-cyclic.csv -k stride4 -d cyclic --weak
srun --hint=nomultithread --unbuffered build/camp -s 16000000 -r 3 -t +8:128:8 -i 0.1,0.5,1.1 -f pattern/weak_stencil9-cyclic.csv -k stencil9 -d cyclic --weak
srun --hint=nomultithread --unbuffered build/camp -s 16000000 -r 3 -t +8:128:8 -i 0.1,0.5,1.1 -f pattern/weak_stencil5-cyclic.csv -k stencil5 -d cyclic --weak
# srun --hint=nomultithread --unbuffered build/camp -s 16000000 -r 3 -t +8:128:8 -i 0.1,0.5,1.1 -f pattern/weak_tstencil9-cyclic.csv -k stencil9 -d cyclic --weak
# srun --hint=nomultithread --unbuffered build/camp -s 16000000 -r 3 -t +8:128:8 -i 0.1,0.5,1.1 -f pattern/weak_tstencil5-cyclic.csv -k stencil5 -d cyclic --weak

# ================ size ================
# for size in 128000000 64000000 32000000 16000000
# do
# srun --hint=nomultithread --unbuffered build/camp -s $size -r 3 -t +8:128:8 -i 0.1,0.3,0.5,1.0 -f size1/contig-cyclic-$size.csv -k contig -d cyclic
# srun --hint=nomultithread --unbuffered build/camp -s $size -r 3 -t +8:128:8 -i 0.1,0.3,0.5,1.0 -f size1/stencil5-cyclic-$size.csv -k stencil5 -d cyclic
# done