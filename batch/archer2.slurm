#!/bin/bash

# Slurm job options (name, compute nodes, job time)
#SBATCH --job-name=CAMP
#SBATCH --time=1:20:0
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=128

# Replace [budget code] below with your project code (e.g. t01)
#SBATCH --account=ta094-wenqingpen
#SBATCH --partition=standard
#SBATCH --qos=standard

# Load the Python module
module load cray-python

# If using a virtual environment
source /work/ta094/ta094/wenqingpeng/pyenv-camp/bin/activate

export OMP_NUM_THREADS=128
export OMP_PROC_BIND=true
# export CRAY_OMP_CHECK_AFFINITY=TRUE
# export PAT_RT_SUMMARY=0
# export PAT_RT_PERFCTR=CORE_TO_L2_CACHEABLE_REQUEST_ACCESS_STATUS:LS_RD_BLK_C,PAPI_L2_DCM,PAPI_FP_OPS,mem_bw


# --cpu-freq=2250000
srun --hint=nomultithread --unbuffered ./camp config/example