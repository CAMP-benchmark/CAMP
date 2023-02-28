#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <float.h>
#include <limits.h>
#ifndef SINGLE
#include <omp.h>
#endif

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

#if defined(ADD)
#define KERNEL c[i] = a[i] + b[i]
double traffic = 4;
#elif defined(TMP) 
#define KERNEL double tmp = a[i] + b[i]; a[i] = tmp; b[i] = tmp; c[i] = tmp
double traffic = 6;
#elif defined(NOTMP)
#define KERNEL c[i] = a[i] + b[i]; a[i] = c[i]; b[i] = c[i]
double traffic = 6;
#elif defined(MORE)
#define KERNEL double tmp = a[i] + b[i]; a[i] = tmp; b[i] = tmp; c[i] = tmp; d[i] = tmp
double traffic = 8;
#endif


double timer()
{
/* struct timeval { long        tv_sec;
            long        tv_usec;        };

struct timezone { int   tz_minuteswest;
             int        tz_dsttime;      };     */

    struct timeval tp;
    struct timezone tzp;
    int i;

    i = gettimeofday(&tp,&tzp);
    return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

int main() {
    size_t cache_per4cores = 16E6;
    size_t array_size = 4 * cache_per4cores * 128/4;
#ifdef SINGLE
    array_size = cache_per4cores * 4 * 4;  // memory inside one NUMA region is limited
#endif
    int ntrials = 5;
    double time[3] = {FLT_MAX, 0, 0};  // {shortest, average, longest}

    double *a = malloc(sizeof(double) * array_size);
    double *b = malloc(sizeof(double) * array_size);
    double *c = malloc(sizeof(double) * array_size);
    double *d = malloc(sizeof(double) * array_size);
#ifndef SINGLE
#pragma omp parallel for 
#endif
    for (size_t i = 0; i < array_size; ++i) {
        a[i] = 1.0;
        b[i] = 2.0;
        c[i] = 3.0;
        d[i] = 1.0;
    }
    


    for (int t = 0; t < ntrials; ++t) {
        double start_time = timer();
#ifndef SINGLE
#pragma omp parallel for 
#endif
        for (size_t i = 0; i < array_size; ++i) {
            KERNEL;
        }
        double end_time = timer();
        
        double runtime = end_time - start_time;
        time[0] = MIN(time[0], runtime);
        time[1] = time[1] + runtime;
        time[2] = MAX(time[2], runtime);
    }
    time[1] = time[1] / ntrials;

    printf(
        "Array size is: %ld\n"
        "Number of trails is: %d\n"
        "Time {min, avg, max}: {%.4f, %.4f, %.4f}\n"
        "Traffic is %f Bytes per iteration\n"
        "Bandwidth {max, avg, min} is {%.4f, %.4f, %.4f}\n",
        array_size, ntrials, time[0], time[1], time[2], traffic, 
        array_size * 8 * traffic / time[0],
        array_size * 8 * traffic / time[1],
        array_size * 8 * traffic / time[2]
    );
}