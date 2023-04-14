#include "util.h"
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include "rep.h"

#ifndef MAX_THREADNUM
#define MAX_THREADNUM 128
#endif
extern double *a[MAX_THREADNUM], *b[MAX_THREADNUM], *c[MAX_THREADNUM];
int sizeperarray;

/* Preprocess
 * Driver invoke this for each `nthreads`
 * You might want to allocate resource in this routine
 */
void camp_preprocess(const Parameter *param) {
    int nthreads = omp_get_num_threads();
    // a = (double **)malloc(sizeof(double *) * nthreads);
    // b = (double **)malloc(sizeof(double *) * nthreads);
    // c = (double **)malloc(sizeof(double *) * nthreads);
#pragma omp parallel default(none) shared(param, a, b, c, sizeperarray)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        if (param->scale == strong) sizeperarray = param->size / nthreads;
        else if (param->scale == weak) sizeperarray = param->size;
        a[tid] = (double *)malloc(sizeof(double) * sizeperarray);
        b[tid] = (double *)malloc(sizeof(double) * sizeperarray);
        c[tid] = (double *)malloc(sizeof(double) * sizeperarray);
        for (size_t i = 0; i < sizeperarray; ++i) {
            a[tid][i] = 1.0;
            b[tid][i] = 2.0;
            c[tid][i] = 0.0;
        }
    }
}

static double camp_dotproduct(const Parameter *param) {
    double sum = 0;
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, a, b, c, sizeperarray) reduction(+:sum)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 0; i < sizeperarray; i += 1) {
            sum += a[tid][i] * b[tid][i];
        }
    }
    c[0][0] = sum;
    return TIMER() - starttime;
}

/* Kernel interface
 * Parameters:
    param: Parameter struct
    intensity: intensity for this run
    runtime: runtime
    mb: memory (MB) moved per thread
    mflop: floating point operations performed per thread
 * Driver has set affinity for you, and it will invoke kernel() for each `nthreads,intensity,repeat`
 * You should invoke omp parallel yourself.
 */
void camp_kernel(const Parameter *param, double intensity, double *runtime, double *mb, double *mflop) {
    double memperarray = param->size * sizeof(double) / 1000000.0;  /* MB */
    if (strncmp(param->kernel, "dotproduct", 10) == 0) {
        *mb = memperarray * 1;  // approximate
        *mflop = param->size / 1000000.0;  // approximate
        *runtime = camp_dotproduct(param);
    } else {
        fprintf(stderr, "Unsupported kernel\n");
        exit(-1);
    }
}
/* Postprocess
 * Driver invoke this for each `nthreads`
 * You might want to deallocate resource in this routine
 */
void camp_postprocess(const Parameter *param) {
#pragma omp parallel default(none) shared(param, a, b, c)
    {
        int tid = omp_get_thread_num();
        free(a[tid]);
        free(b[tid]);
        free(c[tid]);
    }
    // free(a);
    // free(b);
    // free(c);
}