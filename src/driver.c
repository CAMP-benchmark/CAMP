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
    write_record("kernel, memory(MiB),     MFLOP, nthreads, intensity, runtime(s), bandwidth(MB/s),          mflops, repeat, raw_runtime(colon-seperated)\n");

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
            double min_runtime = DBL_MAX, this_runtime;
            verbose("%.2f ", intensity);

            /* main */
            double raw_vals[param->repeat];
            for (int repeat = 0; repeat < param->repeat; ++repeat) {
                camp_kernel(param, intensity, &this_runtime, &mb, &mflop);
                raw_vals[repeat] = this_runtime;
                min_runtime = MIN(min_runtime, this_runtime);
            }
            
            if (intensity_iter == 0 && min_runtime < 0.01)
                printf("Warning: each thread run less than 0.01 second (%.4fs), "
                       "should increase <repeat> to get a resonable runtime\n", min_runtime);
            write_record("%s, %11.2f, %9.2f,       %d, %9.2f, %10.6f, %15.4f, %15.4f,      %d, ",
                param->kernel, mb * 1.0E6 / 1024 / 1024, mflop, nthreads, intensity,
                min_runtime, mb / min_runtime, mflop / min_runtime, param->repeat);
            for (int repeat = 0; repeat < param->repeat; ++repeat) {
                if (repeat > 0) write_record(":");
                write_record("%.6f", raw_vals[repeat]);
            }
            write_record("\n");
        }  /* for intensity */
        
        camp_postprocess(param);
        verbose("\n");
    }  /* for threads */
}  /* loop */
