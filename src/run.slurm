#!/bin/bash

# Slurm job options (name, compute nodes, job time)
#SBATCH --job-name=CAMP
#SBATCH --time=0:05:0
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=128

# Replace [budget code] below with your project code (e.g. t01)
#SBATCH --account=[budget code]
#SBATCH --partition=standard
#SBATCH --qos=short

export OMP_NUM_THREADS=128
export OMP_PROC_BIND=true
# export CRAY_OMP_CHECK_AFFINITY=TRUE
# export PAT_RT_SUMMARY=0
# export PAT_RT_PERFCTR=CORE_TO_L2_CACHEABLE_REQUEST_ACCESS_STATUS:LS_RD_BLK_C,PAPI_L2_DCM,PAPI_FP_OPS,mem_bw


srun --hint=nomultithread --unbuffered build/camp -s 1024000000 -r 3 -t 8,16,32,64,128 -i 0.02,0.04,0.08,0.1,0.16,0.32,0.64,1.28,2.56 -f result_hardcode.csv -k contig

# Load the Python module
module load cray-python

# If using a virtual environment
source /work/ta094/ta094/wenqingpeng/pyenv-camp/bin/activate

python ../scripts/mesh.py result_hardcode.csv