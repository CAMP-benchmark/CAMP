#include "util.h"
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>

double *a[128];
double *b[128];
int sizeperarray;

/* Preprocess
 * Driver invoke this for each `nthreads`
 * You might want to allocate resource in this routine
 */
void camp_preprocess(const Parameter *param) {
#pragma omp parallel default(none) shared(param, a, b, sizeperarray)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        if (param->scale == strong) sizeperarray = param->size / nthreads;
        else if (param->scale == weak) sizeperarray = param->size;

        a[tid] = (double *)malloc(sizeof(double) * sizeperarray);
        b[tid] = (double *)malloc(sizeof(double) * sizeperarray);
        for (size_t i = 0; i < sizeperarray; ++i) {
            a[tid][i] = 1.0;
            b[tid][i] = 1.1;
        }
    }
}


static double camp_stencil5(const Parameter *param, float intensity) {
    int n = (int)sqrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, a, b, sizeperarray, n)
    {
        double scalar = 3.0;
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                b[tid][i*n + j] = (
                    a[tid][i*n + (j+1)] + a[tid][i*n + (j-1)] +
                    a[tid][(i+1)*n + j] + a[tid][(i-1)*n + j]
                ) * scalar;
            }
        }

        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                a[tid][i*n + j] = b[tid][i*n + j];
            }
        }
    }
    return TIMER() - starttime;
}

static double camp_stencil9(const Parameter *param, float intensity) {
    int n = (int)sqrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, a, b, sizeperarray, n)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                double sum = 0;
                sum += a[tid][i*n + j];
                sum += a[tid][i*n + j+1];
                sum += a[tid][i*n + j-1];
                sum += a[tid][(i+1)*n + j];
                sum += a[tid][(i-1)*n + j];
                sum += a[tid][(i+1)*n + j-1];
                sum += a[tid][(i-1)*n + j-1];
                sum += a[tid][(i+1)*n + j+1];
                sum += a[tid][(i-1)*n + j+1];
                b[tid][i*n + j] = sum;
            }
        }
    }
    return TIMER() - starttime;
}

static double camp_stencil7(const Parameter *param, float intensity) {
    int n = (int)cbrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, a, b, sizeperarray, n)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                for (size_t k = 1; k < n - 1; ++k) {
                    double sum = 0;
                    sum += a[tid][i*n*n + j*n + k];
                    sum += a[tid][i*n*n + j*n + k+1];
                    sum += a[tid][i*n*n + j*n + k-1];
                    sum += a[tid][i*n*n + (j+1)*n + k];
                    sum += a[tid][i*n*n + (j-1)*n + k];
                    sum += a[tid][(i+1)*n*n + j*n + k];
                    sum += a[tid][(i-1)*n*n + j*n + k];
                    b[tid][i*n*n + j*n + k] = sum;
                }
            }
        }
    }
    return TIMER() - starttime;
}

static double camp_stencil13(const Parameter *param, float intensity) {
    int n = (int)cbrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, a, b, sizeperarray, n)
    {
        int tid = omp_get_thread_num();
        for (size_t i = 2; i < n - 2; ++i) {
            for (size_t j = 2; j < n - 2; ++j) {
                for (size_t k = 2; k < n - 2; ++k) {
                    double sum = 0;
                    sum += a[tid][i*n*n + j*n + k];
                    sum += a[tid][i*n*n + j*n + k+1];
                    sum += a[tid][i*n*n + j*n + k-1];
                    sum += a[tid][i*n*n + (j+1)*n + k];
                    sum += a[tid][i*n*n + (j-1)*n + k];
                    sum += a[tid][(i+1)*n*n + j*n + k];
                    sum += a[tid][(i-1)*n*n + j*n + k];
                    sum += a[tid][i*n*n + j*n + k+2];
                    sum += a[tid][i*n*n + j*n + k-2];
                    sum += a[tid][i*n*n + (j+2)*n + k];
                    sum += a[tid][i*n*n + (j-2)*n + k];
                    sum += a[tid][(i+2)*n*n + j*n + k];
                    sum += a[tid][(i-2)*n*n + j*n + k];
                    b[tid][i*n*n + j*n + k] = sum;
                }
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
    double memperarray = param->size * sizeof(double) / 1000000.0;  /* MB */
    if (strncmp(param->kernel, "stencil", 7) == 0) {
        if (strlen(param->kernel) < 8) { fprintf(stderr, "Please provide stencil cell size\n"); exit(-1); }
        size_t cell_size = atoi(param->kernel + 7);
        *mb = memperarray * cell_size;  // approximate
        *mflop = param->size * cell_size / 1000000.0;  // approximate
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
    #pragma omp parallel default(none) shared(param, a, b)
    {
        int tid = omp_get_thread_num();
        free(a[tid]);
        free(b[tid]);
    }
}