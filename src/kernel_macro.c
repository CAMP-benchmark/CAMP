#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include <time.h>
#include <stdio.h>
#include "util.h"
#include "rep.h"

/* STREAM TRAID kernel, 1 operation on 1 element, move 24 byte, do 2 FLOP */
#define TRAID_BYTEPEROPS 12
#define TRAID_NOPS(intensity) (round(intensity * TRAID_BYTEPEROPS))
/* TMP kernel:
    tmp = a[i] + b[i]
    a[i] = -tmp
    b[i] = -tmp
    c[i] = -tmp
 * 1 FLOP, 3*8*2 Bytes
 */
// #define TMP_KERNEL(tmp, a, b, c) (tmp) = (a) + (b); (a) = (tmp); (b) = (tmp); (c) = (tmp)
#define ADD_KERNEL(a, b, c) ((a) = (b) + (c))
#define TRAID_KERNEL(a, b, c) ((a) = (a) * (b) + (c))
#define TMP_BYTEPEROPS 48
#define TMP_NOPS(intensity) (round(intensity * TMP_BYTEPEROPS))

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

/* operational intensity granularity: 1/48 */
static double camp_contig(const Parameter *param, float intensity, size_t stride_size) {
    int nops = TMP_NOPS(intensity);
    printf(" [nops is %d] ", FLOP);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, a, b, c, stride_size, sizeperarray)
    {
        double const_tmp = 1.0, alpha = 2.0;
        int tid = omp_get_thread_num();
        
        for (size_t i = 0; i < sizeperarray; i += 1) {
            double beta = const_tmp;
            
#if (FLOP & 1) == 1 /* add 1 flop */
    ADD_KERNEL(beta, a[tid][i], alpha);
#endif
#if (FLOP & 2) == 2 /* add 2 flops */
      TRAID_KERNEL(beta, a[tid][i], alpha);
#endif
#if (FLOP & 4) == 4 /* add 4 flops */
      REP2(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 8) == 8 /* add 8 flops */
      REP4(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 16) == 16 /* add 16 flops */
      REP8(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 32) == 32 /* add 32 flops */
      REP16(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 64) == 64 /* add 64 flops */
      REP32(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 128) == 128 /* add 128 flops */
      REP64(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 256) == 256 /* add 256 flops */
      REP128(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 512) == 512 /* add 512 flops */
      REP256(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 1024) == 1024 /* add 1024 flops */
      REP512(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif

            a[tid][i] = -beta;
            b[tid][i] = -beta;
            c[tid][i] = -beta;
        }
    }
    return TIMER() - starttime;
}

/* operational intensity granularity: 1/48 */
static double camp_stride(const Parameter *param, float intensity, size_t stride_size) {
    int nops = TMP_NOPS(intensity);
    printf(" [nops is %d] ", FLOP);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, a, b, c, stride_size, sizeperarray)
    {
        double const_tmp = 1.0, alpha = 2.0;
        int tid = omp_get_thread_num();
        
        for (size_t i = 0; i < sizeperarray; i += stride_size) {
            double beta = const_tmp;
            
#if (FLOP & 1) == 1 /* add 1 flop */
    ADD_KERNEL(beta, a[tid][i], alpha);
#endif
#if (FLOP & 2) == 2 /* add 2 flops */
      TRAID_KERNEL(beta, a[tid][i], alpha);
#endif
#if (FLOP & 4) == 4 /* add 4 flops */
      REP2(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 8) == 8 /* add 8 flops */
      REP4(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 16) == 16 /* add 16 flops */
      REP8(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 32) == 32 /* add 32 flops */
      REP16(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 64) == 64 /* add 64 flops */
      REP32(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 128) == 128 /* add 128 flops */
      REP64(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 256) == 256 /* add 256 flops */
      REP128(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 512) == 512 /* add 512 flops */
      REP256(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif
#if (FLOP & 1024) == 1024 /* add 1024 flops */
      REP512(TRAID_KERNEL(beta, a[tid][i], alpha));
#endif

            a[tid][i] = -beta;
            b[tid][i] = -beta;
            c[tid][i] = -beta;
        }
    }
    return TIMER() - starttime;
}


/* operational intensity granularity: 1/12 */
static double camp_contig_traid(const Parameter *param, float intensity, size_t stride_size) {
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

#define WRITE_BACK(a,b,c,tid,i,beta) a[tid][i] = -beta; b[tid][i] = -beta; c[tid][i] = -beta

/*
 * we didn't put stencil in one function because we don't know how much it would affect performance
 * if we put branch statement inside loops. We try to keep the inner loop simple.
 */
// #define STENCIL5_ADD(a, tid, i, j, n, beta, alpha) ADD_KERNEL(beta, a[tid][i*n + j], alpha); ADD_KERNEL(beta, a[tid][i*n + j+1], alpha); ADD_KERNEL(beta, a[tid][i*n + j-1], alpha); ADD_KERNEL(beta, a[tid][(i+1)*n + j], alpha); ADD_KERNEL(beta, a[tid][(i-1)*n + j], alpha)
// #define STENCIL5_TRAID(a, tid, i, j, n, beta, alpha) TRAID_KERNEL(beta, a[tid][i*n + j], alpha); TRAID_KERNEL(beta, a[tid][i*n + j+1], alpha); TRAID_KERNEL(beta, a[tid][i*n + j-1], alpha); TRAID_KERNEL(beta, a[tid][(i+1)*n + j], alpha); TRAID_KERNEL(beta, a[tid][(i-1)*n + j], alpha)
#define STENCIL5_ADD(a, tid, i, j, n, beta, alpha) (beta = a[tid][i*n + j] + a[tid][i*n + j+1] + a[tid][i*n + j-1] + a[tid][(i-1)*n + j] + a[tid][(i+1)*n + j] + alpha)
#define STENCIL5_TRAID(a, tid, i, j, n, beta, alpha) (beta += (a[tid][i*n + j+1]*alpha + a[tid][i*n + j-1]*alpha + a[tid][(i-1)*n + j]*alpha + a[tid][(i+1)*n + j]*alpha) + a[tid][i*n + j]*alpha)
static double camp_stencil5(const Parameter *param, float intensity) {
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    int n = (int)sqrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, n, sizeperarray)
    {
        double const_tmp = 1.0, alpha = 2.0;
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                double beta = const_tmp;
#if (FLOP & 1) == 1 /* add 1 flop */
    STENCIL5_ADD(a, tid, i, j, n, beta, alpha);
#endif
#if (FLOP & 2) == 2 /* add 2 flops */
    STENCIL5_TRAID(a, tid, i, j, n, beta, alpha);
#endif
#if (FLOP & 4) == 4 /* add 4 flops */
    REP2(STENCIL5_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 8) == 8 /* add 8 flops */
    REP4(STENCIL5_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 16) == 16 /* add 16 flops */
    REP8(STENCIL5_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 32) == 32 /* add 32 flops */
    REP16(STENCIL5_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 64) == 64 /* add 64 flops */
    REP32(STENCIL5_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 128) == 128 /* add 128 flops */
    REP64(STENCIL5_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 256) == 256 /* add 256 flops */
    REP128(STENCIL5_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 512) == 512 /* add 512 flops */
    REP256(STENCIL5_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 1024) == 1024 /* add 1024 flops */
    REP512(STENCIL5_TRAID(a, tid, i, j, n, beta, alpha));
#endif
                // WRITE_BACK(a,b,c,tid,i*n + j,beta);
                // WRITE_BACK(a,b,c,tid,i*n + j+1,beta);
                // WRITE_BACK(a,b,c,tid,i*n + j-1,beta);
                // WRITE_BACK(a,b,c,tid,(i+1)*n + j,beta);
                // WRITE_BACK(a,b,c,tid,(i-1)*n + j,beta);
                a[tid][i*n + j] = -beta; a[tid][i*n + j-1] = -beta; a[tid][i*n + j+1] = -beta; a[tid][(i-1)*n + j] = -beta; a[tid][(i+1)*n + j] = -beta;
                a[tid][i*n + j] = -beta; b[tid][i*n + j-1] = -beta; b[tid][i*n + j+1] = -beta; b[tid][(i-1)*n + j] = -beta; b[tid][(i+1)*n + j] = -beta;
                c[tid][i*n + j] = -beta; c[tid][i*n + j-1] = -beta; c[tid][i*n + j+1] = -beta; c[tid][(i-1)*n + j] = -beta; c[tid][(i+1)*n + j] = -beta;
            }
        }
    }
    return TIMER() - starttime;
}
// #define STENCIL9_ADD(a, tid, i, j, n, beta, alpha) ADD_KERNEL(beta, a[tid][i*n + j], alpha); ADD_KERNEL(beta, a[tid][i*n + j+1], alpha); ADD_KERNEL(beta, a[tid][i*n + j-1], alpha); ADD_KERNEL(beta, a[tid][(i+1)*n + j], alpha); ADD_KERNEL(beta, a[tid][(i-1)*n + j], alpha); ADD_KERNEL(beta, a[tid][(i+1)*n + j+1], alpha); ADD_KERNEL(beta, a[tid][(i+1)*n + j-1], alpha); ADD_KERNEL(beta, a[tid][(i-1)*n + j+1], alpha); ADD_KERNEL(beta, a[tid][(i-1)*n + j-1], alpha)
// #define STENCIL9_TRAID(a, tid, i, j, n, beta, alpha) TRAID_KERNEL(beta, a[tid][i*n + j], alpha); TRAID_KERNEL(beta, a[tid][i*n + j+1], alpha); TRAID_KERNEL(beta, a[tid][i*n + j-1], alpha); TRAID_KERNEL(beta, a[tid][(i+1)*n + j], alpha); TRAID_KERNEL(beta, a[tid][(i-1)*n + j], alpha); TRAID_KERNEL(beta, a[tid][(i+1)*n + j+1], alpha); TRAID_KERNEL(beta, a[tid][(i+1)*n + j-1], alpha); TRAID_KERNEL(beta, a[tid][(i-1)*n + j+1], alpha); TRAID_KERNEL(beta, a[tid][(i-1)*n + j-1], alpha)
#define STENCIL9_ADD(a, tid, i, j, n, beta, alpha) (beta = a[tid][i*n + j] + a[tid][i*n + j+1] + a[tid][i*n + j-1] + a[tid][(i-1)*n + j] + a[tid][(i+1)*n + j] + a[tid][(i+1)*n + j+1] + a[tid][(i-1)*n + j-1] + a[tid][(i-1)*n + j+1] + a[tid][(i+1)*n + j-1] + alpha)
#define STENCIL9_TRAID(a, tid, i, j, n, beta, alpha) (beta += (a[tid][i*n + j+1]*alpha + a[tid][i*n + j-1]*alpha + a[tid][(i-1)*n + j]*alpha + a[tid][(i+1)*n + j]*alpha + a[tid][(i-1)*n + j+1]*alpha + a[tid][(i+1)*n + j-1]*alpha + a[tid][(i-1)*n + j-1]*alpha + a[tid][(i+1)*n + j+1]*alpha) + a[tid][i*n + j]*alpha)
static double camp_stencil9(const Parameter *param, float intensity) {
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    int n = (int)sqrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, n, sizeperarray)
    {
        double const_tmp = 1.0, alpha = 2.0;
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                double beta = const_tmp;
#if (FLOP & 1) == 1 /* add 1 flop */
    STENCIL9_ADD(a, tid, i, j, n, beta, alpha);
#endif
#if (FLOP & 2) == 2 /* add 2 flops */
    STENCIL9_TRAID(a, tid, i, j, n, beta, alpha);
#endif
#if (FLOP & 4) == 4 /* add 4 flops */
    REP2(STENCIL9_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 8) == 8 /* add 8 flops */
    REP4(STENCIL9_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 16) == 16 /* add 16 flops */
    REP8(STENCIL9_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 32) == 32 /* add 32 flops */
    REP16(STENCIL9_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 64) == 64 /* add 64 flops */
    REP32(STENCIL9_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 128) == 128 /* add 128 flops */
    REP64(STENCIL9_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 256) == 256 /* add 256 flops */
    REP128(STENCIL9_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 512) == 512 /* add 512 flops */
    REP256(STENCIL9_TRAID(a, tid, i, j, n, beta, alpha));
#endif
#if (FLOP & 1024) == 1024 /* add 1024 flops */
    REP512(STENCIL9_TRAID(a, tid, i, j, n, beta, alpha));
#endif
                WRITE_BACK(a,b,c,tid,i*n + j,beta);
                WRITE_BACK(a,b,c,tid,i*n + j+1,beta);
                WRITE_BACK(a,b,c,tid,i*n + j-1,beta);
                WRITE_BACK(a,b,c,tid,(i+1)*n + j,beta);
                WRITE_BACK(a,b,c,tid,(i-1)*n + j,beta);
                WRITE_BACK(a,b,c,tid,(i+1)*n + j+1,beta);
                WRITE_BACK(a,b,c,tid,(i+1)*n + j-1,beta);
                WRITE_BACK(a,b,c,tid,(i-1)*n + j+1,beta);
                WRITE_BACK(a,b,c,tid,(i-1)*n + j-1,beta);
            }
        }
    }
    return TIMER() - starttime;
}

#define STENCIL7_ADD(a, tid, i, j, k, n, beta, alpha) (beta = a[tid][i*n*n + j*n + k] + a[tid][i*n*n + j*n + k+1] + a[tid][i*n*n + j*n + k-1] + a[tid][i*n*n + (j+1)*n + k] + a[tid][i*n*n + (j-1)*n + k] + a[tid][(i+1)*n*n + j*n + k] + a[tid][(i-1)*n*n + j*n + k] + alpha)
#define STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha) (beta += (a[tid][i*n*n + j*n + k+1]*alpha + a[tid][i*n*n + j*n + k-1]*alpha + a[tid][i*n*n + (j+1)*n + k]*alpha + a[tid][i*n*n + (j-1)*n + k]*alpha + a[tid][(i+1)*n*n + j*n + k]*alpha + a[tid][(i-1)*n*n + j*n + k]*alpha) + a[tid][i*n*n + j*n + k]*alpha)
static double camp_stencil7(const Parameter *param, float intensity) {
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    int n = (int)cbrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, n, sizeperarray)
    {
        double const_tmp = 1.0, alpha = 2.0;
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                for (size_t k = 1; k < n - 1; ++k) {
                    double beta = const_tmp;
#if (FLOP & 1) == 1 /* add 1 flop */
    STENCIL7_ADD(a, tid, i, j, k, n, beta, alpha);
#endif
#if (FLOP & 2) == 2 /* add 2 flops */
    STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha);
#endif
#if (FLOP & 4) == 4 /* add 4 flops */
    REP2(STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 8) == 8 /* add 8 flops */
    REP4(STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 16) == 16 /* add 16 flops */
    REP8(STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 32) == 32 /* add 32 flops */
    REP16(STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 64) == 64 /* add 64 flops */
    REP32(STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 128) == 128 /* add 128 flops */
    REP64(STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 256) == 256 /* add 256 flops */
    REP128(STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 512) == 512 /* add 512 flops */
    REP256(STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 1024) == 1024 /* add 1024 flops */
    REP512(STENCIL7_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
                    WRITE_BACK(a,b,c,tid,i*n*n + j*n + k,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + j*n + k+1,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + j*n + k-1,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + (j+1)*n + k,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + (j-1)*n + k,beta);
                    WRITE_BACK(a,b,c,tid,(i+1)*n*n + j*n + k,beta);
                    WRITE_BACK(a,b,c,tid,(i-1)*n*n + j*n + k,beta);
                }
            }
        }
    }
    return TIMER() - starttime;
}

#define STENCIL13_ADD(a, tid, i, j, k, n, beta, alpha) (beta = a[tid][i*n*n + j*n + k] + a[tid][i*n*n + j*n + k+1] + a[tid][i*n*n + j*n + k-1] + a[tid][i*n*n + (j+1)*n + k] + a[tid][i*n*n + (j-1)*n + k] + a[tid][(i+1)*n*n + j*n + k] + a[tid][(i-1)*n*n + j*n + k] + a[tid][i*n*n + j*n + k+2] + a[tid][i*n*n + j*n + k-2] + a[tid][i*n*n + (j+2)*n + k] + a[tid][i*n*n + (j-2)*n + k] + a[tid][(i+2)*n*n + j*n + k] + a[tid][(i-2)*n*n + j*n + k] + alpha)
#define STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha) (beta += (a[tid][i*n*n + j*n + k+1]*alpha + a[tid][i*n*n + j*n + k-1]*alpha + a[tid][i*n*n + (j+1)*n + k]*alpha + a[tid][i*n*n + (j-1)*n + k]*alpha + a[tid][(i+1)*n*n + j*n + k]*alpha + a[tid][(i-1)*n*n + j*n + k]*alpha + a[tid][i*n*n + j*n + k+2]*alpha + a[tid][i*n*n + j*n + k-2]*alpha + a[tid][i*n*n + (j+2)*n + k]*alpha + a[tid][i*n*n + (j-2)*n + k]*alpha + a[tid][(i+2)*n*n + j*n + k]*alpha + a[tid][(i-2)*n*n + j*n + k]*alpha) + a[tid][i*n*n + j*n + k]*alpha)
static double camp_stencil13(const Parameter *param, float intensity) {
    int nops = TRAID_NOPS(intensity);
    double scalar = 3.0;
    int n = (int)cbrt((double)sizeperarray);
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, n, sizeperarray)
    {
        double const_tmp = 1.0, alpha = 2.0;
        int tid = omp_get_thread_num();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                for (size_t k = 1; k < n - 1; ++k) {
                    double beta = const_tmp;
#if (FLOP & 1) == 1 /* add 1 flop */
    STENCIL13_ADD(a, tid, i, j, k, n, beta, alpha);
#endif
#if (FLOP & 2) == 2 /* add 2 flops */
    STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha);
#endif
#if (FLOP & 4) == 4 /* add 4 flops */
    REP2(STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 8) == 8 /* add 8 flops */
    REP4(STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 16) == 16 /* add 16 flops */
    REP8(STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 32) == 32 /* add 32 flops */
    REP16(STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 64) == 64 /* add 64 flops */
    REP32(STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 128) == 128 /* add 128 flops */
    REP64(STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 256) == 256 /* add 256 flops */
    REP128(STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 512) == 512 /* add 512 flops */
    REP256(STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
#if (FLOP & 1024) == 1024 /* add 1024 flops */
    REP512(STENCIL13_TRAID(a, tid, i, j, k, n, beta, alpha));
#endif
                    WRITE_BACK(a,b,c,tid,i*n*n + j*n + k,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + j*n + k+1,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + j*n + k-1,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + (j+1)*n + k,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + (j-1)*n + k,beta);
                    WRITE_BACK(a,b,c,tid,(i+1)*n*n + j*n + k,beta);
                    WRITE_BACK(a,b,c,tid,(i-1)*n*n + j*n + k,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + j*n + k+2,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + j*n + k-2,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + (j+2)*n + k,beta);
                    WRITE_BACK(a,b,c,tid,i*n*n + (j-2)*n + k,beta);
                    WRITE_BACK(a,b,c,tid,(i+2)*n*n + j*n + k,beta);
                    WRITE_BACK(a,b,c,tid,(i-2)*n*n + j*n + k,beta);
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
    size_t pos[sizeperarray];
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, sizeperarray, pos)
    {
        for (size_t i = 0; i < sizeperarray; ++i) {
            pos[i] = rand() % sizeperarray;
        }
    }
    double starttime = TIMER();
#pragma omp parallel default(none) shared(param, nops, scalar, a, b, c, sizeperarray, pos)
    {
        double const_tmp = 1.0, alpha = 2.0;
        int tid = omp_get_thread_num();
        for (size_t i = 0; i < sizeperarray; i += 1) {
            double beta = const_tmp;
            
#if (FLOP & 1) == 1 /* add 1 flop */
    ADD_KERNEL(beta, a[tid][pos[i]], alpha);
#endif
#if (FLOP & 2) == 2 /* add 2 flops */
      TRAID_KERNEL(beta, a[tid][pos[i]], alpha);
#endif
#if (FLOP & 4) == 4 /* add 4 flops */
      REP2(TRAID_KERNEL(beta, a[tid][pos[i]], alpha));
#endif
#if (FLOP & 8) == 8 /* add 8 flops */
      REP4(TRAID_KERNEL(beta, a[tid][pos[i]], alpha));
#endif
#if (FLOP & 16) == 16 /* add 16 flops */
      REP8(TRAID_KERNEL(beta, a[tid][pos[i]], alpha));
#endif
#if (FLOP & 32) == 32 /* add 32 flops */
      REP16(TRAID_KERNEL(beta, a[tid][pos[i]], alpha));
#endif
#if (FLOP & 64) == 64 /* add 64 flops */
      REP32(TRAID_KERNEL(beta, a[tid][pos[i]], alpha));
#endif
#if (FLOP & 128) == 128 /* add 128 flops */
      REP64(TRAID_KERNEL(beta, a[tid][pos[i]], alpha));
#endif
#if (FLOP & 256) == 256 /* add 256 flops */
      REP128(TRAID_KERNEL(beta, a[tid][pos[i]], alpha));
#endif
#if (FLOP & 512) == 512 /* add 512 flops */
      REP256(TRAID_KERNEL(beta, a[tid][pos[i]], alpha));
#endif
#if (FLOP & 1024) == 1024 /* add 1024 flops */
      REP512(TRAID_KERNEL(beta, a[tid][pos[i]], alpha));
#endif

            a[tid][pos[i]] = -beta;
            b[tid][pos[i]] = -beta;
            c[tid][pos[i]] = -beta;
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
        *mb = memperarray * 6.0;
        *mflop = *mb * intensity;
        *runtime = camp_contig(param, intensity, 1);
    } else if (strncmp(param->kernel, "stride", 6) == 0) {
        if (strlen(param->kernel) < 7) { fprintf(stderr, "Please provide stride size\n"); exit(-1); }
        size_t stride_size = atoi(param->kernel + 6);
        *mb = memperarray * 6.0 / stride_size;
        *mflop = *mb * intensity;
        *runtime = camp_stride(param, intensity, stride_size);
    } else if (strncmp(param->kernel, "stencil", 7) == 0) {
        if (strlen(param->kernel) < 8) { fprintf(stderr, "Please provide stencil cell size\n"); exit(-1); }
        size_t cell_size = atoi(param->kernel + 7);
        *mb = memperarray * 6.0 * cell_size;  // approximate
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
        *mb = memperarray * 6.0;
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