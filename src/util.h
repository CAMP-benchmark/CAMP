#ifndef UTIL_INC
#define UTIL_INC

#include <stdlib.h>

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

#ifndef TIMER
#define TIMER omp_get_wtime
#endif

extern int quiet;

typedef struct Sequence {
    float *vals;
    size_t size;
} Sequence;

/* Distribution of threads */
enum Distribution {
    cyclic,  /* spread threads across each level of hierarchy, higher level first */
    block,  /* file the last hierarchy unit before moving the the next */
    spread,  /* like OMP_PROC_BIND:spread, but only use floor(P/T) */
    cpu_bind,  /* explictly bind to core */
    none  /* use omp defalut */
};

/* Scaling of workload */
enum Scaling {
    strong,  /* the size of memory scales */
    weak  /* do not scale */
};

typedef struct Parameter {
    char *kernel;  // name of kernel
    char *filename;  // name of file to store result
    Sequence *threads;  // number of threads
    Sequence *intensity;  // operational intensity
    size_t size;  // size of array (number of doubles) per core
    int distribution;  // distribution of threads
    /* 
     * number of cores per hierarchical level, from high to low
     * eg: {cores/machine, c/socket, c/NUMA, c/CCD, c/CCX}
     * TODO: get through code
     */
    Sequence *hierarchy;
    Sequence *cpus;
    int scale;
    int repeat;  // repeat each config
} Parameter;

#define LINE "======================================================\n"

double getsecond();
char *getnow();
size_t gettimetag(char buffer[], size_t n);

void write_init(char *filename);
void write_final();
void write_record(char *format, ...);

void verbose(const char *format, ...);

Sequence *seq_alloc();
void seq_free(Sequence *seq);
void parse_seq(Sequence *seq, const char *seq_str);

void parse_distribution(int *dis, const char *dis_str);

void print_affinity(int tid);
void camp_set_affinity(int tid, int nthreads, const Parameter *param);

void report(char *exe, Parameter *param);
#endif  // UTIL_INC