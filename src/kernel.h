#include "util.h"

/* Preprocess
 * Driver invoke this for each `nthreads`
 * You can allocate resource in this routine
 */
extern void camp_preprocess(const Parameter *param);
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
extern void camp_kernel(const Parameter *param, double intensity, double *runtime, double *mb, double *mflop);
/* Postprocess
 * Driver invoke this for each `nthreads`
 * You can deallocate resource in this routine
 */
extern void camp_postprocess(const Parameter *param);