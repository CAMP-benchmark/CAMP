CFLAGS = -fopenmp -Ofast -march=znver2 -Rpass=loop-vectorize -mllvm -force-vector-interleave=1
default:
	cc simple_kernel.c $(CFLAGS) -o kernel_ADD -DADD
	cc simple_kernel.c $(CFLAGS) -o kernel_TMP -DTMP
	cc simple_kernel.c $(CFLAGS) -o kernel_NOTMP -DNOTMP
	cc simple_kernel.c $(CFLAGS) -o kernel_MORE -DMORE

.PHONE: pat

# module load perftools
# module load perftools-base
KERNEL=NOTMP  # ADD/TMP/NOTMP/MORE
hwpc:
	cc simple_kernel.c $(CFLAGS) -c -D$(KERNEL) -o kernel_hwpc.o 
	cc kernel_hwpc.o $(CFLAGS) -o kernel_hwpc_$(KERNEL) 
	pat_build -w kernel_hwpc_$(KERNEL)

clean:
	rm kernel_*