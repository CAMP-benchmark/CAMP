module load perftools
srun --ntasks=1 --partition=standard --qos=short --account=ta094-wenqingpen papi_avail

# run below command on login node 
# pat_help counters rome groups

# note: 
#   1. these flags make hwpc works `cc simple_kernel.c -c -Ofast -march=znver2 -Rpass=loop-vectorize -DSINGLE -D$(KERNEL) -o kernel_hwpc.o -fopenmp`
#   2. PAPI_FP_OPS not work when loop unroll factor is not 4