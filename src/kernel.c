#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include <time.h>
#include <stdio.h>
#include "util.h"

/* STREAM TRAID kernel, 1 operation on 1 element, move 24 byte, do 2 FLOP */
#define TRAID_BYTEPEROPS 12
#define TRAID_NOPS(intensity) (round(intensity * TRAID_BYTEPEROPS))

double *a[128], *b[128], *c[128];
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

/* operational intensity granularity: 1/12 */
static double camp_contig(const Parameter *param, float intensity, size_t stride_size) {
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, stride_size, sizeperarray)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 0; i < sizeperarray; i += stride_size) {
            for (int cnt = 0; cnt < nops; ++cnt) {
                a[tid][i] = b[tid][i] + c[tid][i] * scalar;
            }
        }
    }
    return TIMER() - starttime;
}

/*
 * we didn't put stencil in one function because we don't know how much it would affect performance
 * if we put branch statement inside loops. We try to keep the inner loop simple.
 */

static double camp_stencil5(const Parameter *param, float intensity) {
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    int n = (int)sqrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, n, sizeperarray)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                for (int cnt = 0; cnt < nops; ++cnt) {
                    a[tid][i*n + j] =
                        b[tid][i*n + j] + c[tid][i*n + j] * scalar;
                    a[tid][i*n + j+1] =
                        b[tid][i*n + j+1] + c[tid][i*n + j+1] * scalar;
                    a[tid][i*n + j-1] =
                        b[tid][i*n + j-1] + c[tid][i*n + j-1] * scalar;
                    a[tid][(i+1)*n + j] =
                        b[tid][(i+1)*n + j] + c[tid][(i+1)*n + j] * scalar;
                    a[tid][(i-1)*n + j] =
                        b[tid][(i-1)*n + j] + c[tid][(i-1)*n + j] * scalar;
                }
            }
        }
    }
    return TIMER() - starttime;
}

static double camp_stencil9(const Parameter *param, float intensity) {
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    int n = (int)sqrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, n, sizeperarray)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                for (int cnt = 0; cnt < nops; ++cnt) {
                    a[tid][i*n + j] =
                        b[tid][i*n + j] + c[tid][i*n + j] * scalar;
                    a[tid][(i-1)*n + j-1] =
                        b[tid][(i-1)*n + j-1] + c[tid][(i-1)*n + j-1] * scalar;
                    a[tid][(i-1)*n + j] =
                        b[tid][(i-1)*n + j] + c[tid][(i-1)*n + j] * scalar;
                    a[tid][(i-1)*n + j+1] =
                        b[tid][(i-1)*n + j+1] + c[tid][(i-1)*n + j+1] * scalar;
                    a[tid][i*n + j-1] =
                        b[tid][i*n + j-1] + c[tid][i*n + j-1] * scalar;
                    a[tid][i*n + j+1] =
                        b[tid][i*n + j+1] + c[tid][i*n + j+1] * scalar;
                    a[tid][(i+1)*n + j-1] =
                        b[tid][(i+1)*n + j-1] + c[tid][(i+1)*n + j-1] * scalar;
                    a[tid][(i+1)*n + j] =
                        b[tid][(i+1)*n + j] + c[tid][(i+1)*n + j] * scalar;
                    a[tid][(i+1)*n + j+1] =
                        b[tid][(i+1)*n + j+1] + c[tid][(i+1)*n + j+1] * scalar;
                }
            }
        }
    }
    return TIMER() - starttime;
}

static double camp_stencil7(const Parameter *param, float intensity) {
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    int n = (int)cbrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, n, sizeperarray)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                for (size_t k = 1; k < n - 1; ++k) {
                    for (int cnt = 0; cnt < nops; ++cnt) {
                        a[tid][i*n*n + j*n + k] =
                            b[tid][i*n*n + j*n + k] + c[tid][i*n*n + j*n + k] * scalar;
                        a[tid][(i-1)*n*n + j*n + k] =
                            b[tid][(i-1)*n*n + j*n + k] + c[tid][(i-1)*n*n + j*n + k] * scalar;
                        a[tid][(i+1)*n*n + j*n + k] =
                            b[tid][(i+1)*n*n + j*n + k] + c[tid][(i+1)*n*n + j*n + k] * scalar;
                        a[tid][i*n*n + (j-1)*n + k] =
                            b[tid][i*n*n + (j-1)*n + k] + c[tid][i*n*n + (j-1)*n + k] * scalar;
                        a[tid][i*n*n + (j+1)*n + k] =
                            b[tid][i*n*n + (j+1)*n + k] + c[tid][i*n*n + (j+1)*n + k] * scalar;
                        a[tid][i*n*n + j*n + k-1] =
                            b[tid][i*n*n + j*n + k-1] + c[tid][i*n*n + j*n + k-1] * scalar;
                        a[tid][i*n*n + j*n + k+1] =
                            b[tid][i*n*n + j*n + k+1] + c[tid][i*n*n + j*n + k+1] * scalar;
                    }
                }
            }
        }
    }
    return TIMER() - starttime;
}

static double camp_stencil13(const Parameter *param, float intensity) {
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    int n = (int)cbrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, n, sizeperarray)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 2; i < n - 2; ++i) {
            for (size_t j = 2; j < n - 2; ++j) {
                for (size_t k = 2; k < n - 2; ++k) {
                    for (int cnt = 0; cnt < nops; ++cnt) {
                        a[tid][i*n*n + j*n + k] =
                            b[tid][i*n*n + j*n + k] + c[tid][i*n*n + j*n + k] * scalar;
                        a[tid][(i-1)*n*n + j*n + k] =
                            b[tid][(i-1)*n*n + j*n + k] + c[tid][(i-1)*n*n + j*n + k] * scalar;
                        a[tid][(i+1)*n*n + j*n + k] =
                            b[tid][(i+1)*n*n + j*n + k] + c[tid][(i+1)*n*n + j*n + k] * scalar;
                        a[tid][(i-2)*n*n + j*n + k] =
                            b[tid][(i-2)*n*n + j*n + k] + c[tid][(i-2)*n*n + j*n + k] * scalar;
                        a[tid][(i+2)*n*n + j*n + k] =
                            b[tid][(i+2)*n*n + j*n + k] + c[tid][(i+2)*n*n + j*n + k] * scalar;
                        a[tid][i*n*n + (j-1)*n + k] =
                            b[tid][i*n*n + (j-1)*n + k] + c[tid][i*n*n + (j-1)*n + k] * scalar;
                        a[tid][i*n*n + (j+1)*n + k] =
                            b[tid][i*n*n + (j+1)*n + k] + c[tid][i*n*n + (j+1)*n + k] * scalar;
                        a[tid][i*n*n + (j-2)*n + k] =
                            b[tid][i*n*n + (j-2)*n + k] + c[tid][i*n*n + (j-2)*n + k] * scalar;
                        a[tid][i*n*n + (j+2)*n + k] =
                            b[tid][i*n*n + (j+2)*n + k] + c[tid][i*n*n + (j+2)*n + k] * scalar;
                        a[tid][i*n*n + j*n + k-1] =
                            b[tid][i*n*n + j*n + k-1] + c[tid][i*n*n + j*n + k-1] * scalar;
                        a[tid][i*n*n + j*n + k+1] =
                            b[tid][i*n*n + j*n + k+1] + c[tid][i*n*n + j*n + k+1] * scalar;
                        a[tid][i*n*n + j*n + k-2] =
                            b[tid][i*n*n + j*n + k-2] + c[tid][i*n*n + j*n + k-2] * scalar;
                        a[tid][i*n*n + j*n + k+2] =
                            b[tid][i*n*n + j*n + k+2] + c[tid][i*n*n + j*n + k+2] * scalar;
                    }
                }
            }
        }
    }
    return TIMER() - starttime;
}

/* access array sizeperarray times, do not gurantee access each element */
static double camp_random(const Parameter *param, float intensity) {
    srand(time(NULL));
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, sizeperarray)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 0; i < sizeperarray; ++i) {
            int pos = rand() % sizeperarray;
            for (int cnt = 0; cnt < nops; ++cnt) {
                a[tid][pos] = b[tid][pos] + c[tid][pos] * scalar;
            }
        }
    }
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
    double memperarray = param->size * sizeof(double) * 1.0E-06;  /* MB */
    if (strcmp(param->kernel, "contig") == 0) {
        *mb = memperarray * 3.0;
        *mflop = *mb * intensity;
        *runtime = camp_contig(param, intensity, 1);
    } else if (strncmp(param->kernel, "stride", 6) == 0) {
        if (strlen(param->kernel) < 7) { fprintf(stderr, "Please provide stride size\n"); exit(-1); }
        size_t stride_size = atoi(param->kernel + 6);
        *mb = memperarray * 3.0 / stride_size;
        *mflop = *mb * intensity;
        *runtime = camp_contig(param, intensity, stride_size);
    } else if (strncmp(param->kernel, "stencil", 7) == 0) {
        if (strlen(param->kernel) < 8) { fprintf(stderr, "Please provide stencil cell size\n"); exit(-1); }
        size_t cell_size = atoi(param->kernel + 7);
        *mb = memperarray * 3.0 * cell_size;  // approximate
        *mflop = *mb * intensity;  // approximate
        switch (cell_size) {
            case 5:
                *runtime = camp_stencil5(param, intensity);
                break;
            case 9:
                *runtime = camp_stencil9(param, intensity);
                break;
            case 7:
                *runtime = camp_stencil7(param, intensity);
                break;
            case 13:
                *runtime = camp_stencil13(param, intensity);
                break;
            default:
                fprintf(stderr, "Unsupported stencil cell size\n");
                exit(-1);
        }
    } else if (strcmp(param->kernel, "random") == 0) {
        *mb = memperarray * 3.0;
        *mflop = *mb * intensity;
        *runtime = camp_random(param, intensity);
    } else {
        fprintf(stderr, "Unsupported kernel\n");
        exit(-1);
    }
    int n;
#pragma omp parallel default(none) shared(n)
    {
    #pragma omp single
        {    
            n = omp_get_num_threads();
        }
    }
    if (param->scale == weak) {
        *mb = *mb * n;
        *mflop = *mb * n;
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