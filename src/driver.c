#include <omp.h>
#include <stdio.h>
#include <float.h>

#include "util.h"
#include "kernel.h"


/*
 * 1. loop number of threads
 * 2. loop operational intensity
 * 3. loop repeat
 */
void loop(const Parameter *param) {
    write_record("kernel,memory(MiB),nthreads,intensity,runtime(s),bandwidth(MB/s),mflops\n");

    for (size_t threads_iter = 0; threads_iter < param->threads->size; ++threads_iter) {
        int nthreads = (int)param->threads->vals[threads_iter];
        omp_set_dynamic(0);
        omp_set_num_threads(nthreads);
#pragma omp parallel default(none) shared(nthreads, param, stderr)
    {
    #pragma omp single
        {
            int threadget = omp_get_num_threads();
            if (threadget != nthreads) {
                fprintf(stderr, "Num threads %d is not avaliable, only get %d\n", nthreads, threadget);
                exit(-1);
            }
        }
        /* set up thread affinity */
        int tid = omp_get_thread_num();
        camp_set_affinity(tid, nthreads, param);
        print_affinity(tid);
    }
        verbose("=> Running with threads: %d. ==> Intensity: ", nthreads);

        camp_preprocess(param);

        for (size_t intensity_iter = 0; intensity_iter < param->intensity->size; ++intensity_iter) {
            double intensity = param->intensity->vals[intensity_iter];
            double mb, mflop;
            double runtime = DBL_MAX, this_runtime;
            verbose("%.2f ", intensity);

            /* main */
            for (int repeat = 0; repeat < param->repeat; ++repeat) {
                camp_kernel(param, intensity, &this_runtime, &mb, &mflop);
                runtime = MIN(runtime, this_runtime);
            }
            
            if (intensity_iter == 0 && runtime < 0.01)
                printf("Warning: each thread run less than 0.01 second (%.4fs), "
                       "should increase <repeat> to get a resonable runtime\n", runtime);
            write_record("%s, %.2f, %d, %.2f, %f, %f, %f\n",
                param->kernel, mb * 1.0E6 / 1024 / 1024, nthreads, intensity,
                runtime, mb / runtime, mflop / runtime);
        }  /* for intensity */
        
        camp_postprocess(param);
        verbose("\n");
    }  /* for threads */
}  /* loop */
