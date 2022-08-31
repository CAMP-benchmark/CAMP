#include "util.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

FILE *fp;
int quiet = 0;

/* -------------------- time -------------------- */
/* get seconds from Epoch */
double getsecond()
{
    struct timeval tp;
    int i;

    i = gettimeofday(&tp,NULL);
    return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

/* get current time in string format */
char *getnow() {
    time_t now;
    time(&now);
    return ctime(&now);
}

size_t gettimetag(char buffer[], size_t n) {
    time_t now = time(NULL);
    struct tm *time = localtime(&now);
    size_t ret = strftime(buffer, n, "%F-%H_%M_%S", time);
    return ret;
}

/* -------------------- file -------------------- */
void write_init(char *filename) {
    if ((fp = fopen(filename, "a")) == NULL) {
        fprintf(stderr, "Can not open file: %s\n", filename);
        exit(-1);
    }
    // fprintf(fp, "\n%s\n", getnow());
    // printf("| Result goes to file: %s\n\n", filename);
}

void write_final() {
    fclose(fp);
}

void write_record(char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(fp, format, args);

    va_end(args);
}

void verbose(const char *format, ...) {
    if (!quiet) {
        va_list args;
        va_start(args, format);

        vprintf(format, args);

        va_end(args);
        fflush(stdout);
    }
}

/* -------------------- sequence -------------------- */
/* 
 * split string to array of string by delimiter
 * return 2D NULL-end Iliffe vector
 * remember to free with vec_free()
 * ref: https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
 */
char **split(const char *target, char delimiter) {
    char delim[2] = "\0"; /* ['\0', '\0'] */
    delim[0] = delimiter;
    char str[strlen(target) + 1];
    strcpy(str, target);

    size_t cnt = 1;
    for (char *ch = str; *ch != '\0'; ++ch) {
        if (*ch == delimiter) ++cnt;
    }

    char **result = (char **)malloc(sizeof(char *) * (cnt + 1));

    if (result) {
        size_t idx = 0;
        char *token = strtok(str, delim);
        while (token) {
            while (*token == ' ') ++token;
            result[idx] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
            strcpy(result[idx], token);
            token = strtok(NULL, delim);
            ++idx;
        }
        if (idx > cnt) {
            fprintf(stderr, "!! split(): overflow\n");
            exit(-1);
        }
        result[idx] = NULL;
    }

    return result;
}

/* free 2D NULL-end Iliffe vector */
void vec_free(void *vec) {
    size_t idx = 0;
    void *ptr;
    while ((ptr = ((void **)vec)[idx++]) != NULL) {
        free(ptr);
    }
    free(vec);
}

Sequence *seq_alloc() {
    Sequence *seq = (Sequence *)malloc(sizeof(Sequence));
    seq->size = 0;
    seq->vals = NULL;
    return seq;
}

void seq_free(Sequence *seq) {
    if (seq->vals != NULL) free(seq->vals);
    free(seq);
}

#define ZOOMSIZE 10000  /* convert float to int to avoid loop over float numbers */

/* parse_seq funcs support format:
 *   1. comma seprate values: "1, 2, 3" => [1, 2, 3]
 *   2. consecutive values with interval of 1: "1, 3-5, 7" => [1, 3, 4, 5, 7]
 *   3. expression [<ops><begin>:<end>:<operand>], <ops> only support '*' or '+': 
 *      "*2:8:2" => [2, 4, 8], "+0.1:0.3:0.1" => [0.1, 0.2, 0.3]
 *   (1) numbers must be positive
 *   (2) percision lower than 0.0001 may be ignored
 */
void parse_seq(Sequence *seq, const char *seq_str) {
    if (seq->vals != NULL) free(seq->vals);
    seq->vals = NULL;
    seq->size = 0;
    char **tokens = split(seq_str, ',');
    char *pos;
    for (size_t idx = 0; tokens[idx] != NULL; ++idx) {
        if ((pos = strchr(tokens[idx], '*')) != NULL) {
            char **nums = split(pos + 1, ':');
            size_t cnt = 0;
            float start = atof(nums[0]), end = atof(nums[1]), inc = atof(nums[2]);
            if (inc <= 1) { fprintf(stderr, "! invalid parse result\n"); exit(-1); }
            for (int i = start * ZOOMSIZE; i <= end * ZOOMSIZE; i *= inc) {
                ++cnt;
            }
            seq->vals = (float *)realloc(seq->vals, sizeof(float) * (seq->size + cnt));
            for (int i = start * ZOOMSIZE; i <= end * ZOOMSIZE; i *= inc) {
                seq->vals[seq->size++] = i / ZOOMSIZE;
            }
            vec_free(nums);
        } else if ((pos = strchr(tokens[idx], '+')) != NULL) {
            char **nums = split(pos + 1, ':');
            size_t cnt = 0;
            float start = atof(nums[0]), end = atof(nums[1]), inc = atof(nums[2]);
            if (inc <= 0) { fprintf(stderr, "! invalid parse result\n"); exit(-1); }
            for (int i = start * ZOOMSIZE; i <= end * ZOOMSIZE; i += inc * ZOOMSIZE) {
                ++cnt;
            }
            seq->vals = (float *)realloc(seq->vals, sizeof(float) * (seq->size + cnt));
            for (int i = start * ZOOMSIZE; i <= end * ZOOMSIZE; i += inc * ZOOMSIZE) {
                seq->vals[seq->size++] = (float)i / ZOOMSIZE;
            }
            vec_free(nums);
        } else if ((pos = strchr(tokens[idx], '-')) != NULL) {
            *pos = '\0';
            float start = atof(tokens[idx]);
            float end = atoi(pos + 1);
            seq->vals = (float *)realloc(seq->vals, sizeof(float) * (seq->size + (end - start + 1)));
            for (int i = start * ZOOMSIZE; i <= end * ZOOMSIZE; i += ZOOMSIZE) {
                seq->vals[seq->size++] = i / ZOOMSIZE;
            }
        } else {
            seq->vals = (float *)realloc(seq->vals, sizeof(float) * (seq->size + 1));
            seq->vals[seq->size++] = atof(tokens[idx]);
        }
    }

    vec_free(tokens);
}
/* -------------------- affinity -------------------- */
void parse_distribution(int *dis, const char *dis_str) {
    if (strcmp(dis_str, "cyclic") == 0) {
        *dis = cyclic;
    } else if (strcmp(dis_str, "block") == 0) {
        *dis = block;
    } else if (strcmp(dis_str, "spread") == 0) {
        *dis = spread;
    } else if (strcmp(dis_str, "cpu-bind") == 0) {
        *dis = cpu_bind;
    } else if (strcmp(dis_str, "none") == 0) {
        *dis = none;
    } else {
        fprintf(stderr, "Unsupported distribution");
        exit(-1);
    }
}

/* -------------------- affinity -------------------- */

/* Borrowed from util-linux-2.13-pre7/schedutils/taskset.c */
#define __USE_GNU  /* TODO: should use _GNU_SOURCE */
#include <sched.h>

static char *cpuset_to_cstr(cpu_set_t *mask, char *str)
{
  char *ptr = str;
  int i, j, entry_made = 0;
  for (i = 0; i < CPU_SETSIZE; i++) {
    if (CPU_ISSET(i, mask)) {
      int run = 0;
      entry_made = 1;
      for (j = i + 1; j < CPU_SETSIZE; j++) {
        if (CPU_ISSET(j, mask)) run++;
        else break;
      }
      if (!run)
        sprintf(ptr, "%d,", i);
      else if (run == 1) {
        sprintf(ptr, "%d,%d,", i, i + 1);
        i++;
      } else {
        sprintf(ptr, "%d-%d,", i, i + run);
        i += run;
      }
      while (*ptr != 0) ptr++;
    }
  }
  ptr -= entry_made;
  *ptr = 0;
  return(str);
}

/* 
 * print threads affinity,
 * borrowed from archer2/xthi
 */
void print_affinity(int tid) {
    cpu_set_t coremask;
    char clbuf[7 * CPU_SETSIZE];

    memset(clbuf, 0, sizeof(clbuf));
    sched_getaffinity(0, sizeof(coremask), &coremask);
    cpuset_to_cstr(&coremask, clbuf);

    verbose("Thread %3d, (affinity = %4s) \n",
	    tid, clbuf);
}

/* bind current thread to cpus [begin, begin + n) */
static int bind_cpus(int begin, int n) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int end = begin + n; begin < end; ++begin) {
        CPU_SET(begin, &mask);
    }
    if (sched_setaffinity(0, sizeof(mask), &mask) != 0) {
        fprintf(stderr, "Cann't set CPU affinity to CPU [%d, %d)", begin, begin + n);
        return -1;
    }
    return 0;
}

/* set affinity according to param->distribution */
void camp_set_affinity(int tid, int nthreads, const Parameter *param) {
    int pos = 0;
    int id = tid;
    int level = param->hierarchy->size;
    float *hierarchy = param->hierarchy->vals;
    if (level < 1) {
        fprintf(stderr, "Too few hierarchy\n");
        exit(-1);
    }
    int ncores = (int)hierarchy[0];
    int cores_this_level = ncores;
    switch (param->distribution) {
        case cyclic: {
            for (int l = 1; l < level; ++l) {
                cores_this_level = (int)hierarchy[l];
                int nthislevel = (int)hierarchy[l-1] / cores_this_level;
                pos += (id % nthislevel) * cores_this_level;
                id /= nthislevel;
            }
            break;
        }
        case block: {
            cores_this_level = (int)hierarchy[level - 1];
            pos = (tid / cores_this_level) * cores_this_level;
            break;
        }
        case spread: {
            cores_this_level = ncores / nthreads;  // subpartitions
            pos = tid * cores_this_level;
            break;
        }
        case cpu_bind:
            if (tid > param->cpus->size) {
                fprintf(stderr, "Provided CPU list too small\n");
                exit(-1);
            }
            cores_this_level = 1;
            pos = (int)param->cpus->vals[tid];
            break;
        case none:
            return;
        default: {
            fprintf(stderr, "Unsupported distribution\n");
            exit(-1);
        }
    }
    bind_cpus(pos, cores_this_level);
}

/* -------------------- miscellany -------------------- */

static void report_to(FILE *fp, char *exe, Parameter *param) {
    fprintf(fp, "| %s", getnow());
    fprintf(fp, "| Executeable: %s\n", exe);
    fprintf(fp, "| Kernel: %s\n", param->kernel);
    fprintf(fp, "| Result filename: %s\n", param->filename);
    fprintf(fp, "| Array size: %ld doubles/thread\n", param->size);
    fprintf(fp, "| Each run Repeat %d times\n", param->repeat);
    fprintf(fp, "| Num threads: [");
    for (size_t i = 0; i < param->threads->size; ++i) {
        if (i > 0) fprintf(fp, ", ");
        fprintf(fp, "%d", (int)param->threads->vals[i]);
    }
    fprintf(fp, "]\n| Operational intensity: [");
    for (size_t i = 0; i < param->intensity->size; ++i) {
        if (i > 0) fprintf(fp, ", ");
        fprintf(fp, "%.2f", param->intensity->vals[i]);
    }
    fprintf(fp, "]\n| Hierarchy: [");
    for (size_t i = 0; i < param->hierarchy->size; ++i) {
        if (i > 0) fprintf(fp, ", ");
        fprintf(fp, "%d", (int)param->hierarchy->vals[i]);
    }
    fprintf(fp, "]\n| Distribution: ");
    switch (param->distribution) {
        case block:
            fprintf(fp, "block\n");
            break;
        case cyclic:
            fprintf(fp, "cyclic\n");
            break;
        case spread:
            fprintf(fp, "stride\n");
            break;
        case cpu_bind:
            fprintf(fp, "cpu-bind\n");
            break;
        case none:
            fprintf(fp, "none\n");
            break;
    }
    fprintf(fp, LINE);
}

void report(char *exe, Parameter *param) {
    report_to(stdout, exe, param);
    if (!quiet) {
        FILE *fp = fopen("history.txt", "a");
        report_to(fp, exe, param);
        fclose(fp);
    }
}