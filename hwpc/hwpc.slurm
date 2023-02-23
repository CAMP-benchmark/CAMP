#!/bin/bash

# Slurm job options (name, compute nodes, job time)
#SBATCH --job-name=CAMP
#SBATCH --time=00:20:00
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=128

# Replace [budget code] below with your project code (e.g. t01)
#SBATCH --account=ta094-wenqingpen
#SBATCH --partition=standard
#SBATCH --qos=short

export OMP_NUM_THREADS=16
export OMP_PROC_BIND=SPREAD
# export CRAY_OMP_CHECK_AFFINITY=TRUE
export PAT_RT_SUMMARY=0
export PAT_RT_PERFCTR=CORE_TO_L2_CACHEABLE_REQUEST_ACCESS_STATUS:LS_RD_BLK_C,PAPI_L2_DCM,PAPI_FP_OPS,mem_bw


module load perftools-base
module load perftools

srun --hint=nomultithread --unbuffered kernel_hwpc_ADD+pat
srun --hint=nomultithread --unbuffered kernel_hwpc_TMP+pat
srun --hint=nomultithread --unbuffered kernel_hwpc_NOTMP+pat
srun --hint=nomultithread --unbuffered kernel_hwpc_MORE+pat