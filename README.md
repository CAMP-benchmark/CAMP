# :camping: Configurable App for Memory Probing

## Introduction
CAMP is a synthetic micro-benchmark for assessing deep memory hierarchies. This tool is insprired by [STREAM](https://www.cs.virginia.edu/stream/) and [adept](https://github.com/EPCCed/adept-kernel-openmp) and complement it by introducing variable operational intensity and access patterns. Underpopulation and placement suggestions for a high-performance computing (HPC) application are provided by finding a configuration that yields the best
performance.

### Objectives
CAMP is originally introduced in the hope of mimicing a real scientific computation program. CAMP would run on real machines and find the best configuration to launch the program, for example, placement and underpopulation configuration. However we found it difficult to develop a memory model that represent the memory behaviour of a real program. But we found this piece of code is helpful to observe the underpopulation effect for programs with different features, such as different patterns and operational intensity.


## Usage
### TL;DR
```c
cd src
mkdir build
make
build/camp <optional arguments>
```

### Configurations
Parameters are input through command line arguments. Posible arguments are:

| Name (`--<n>`) | Shortcut (`-<s>`) | Type     | Usage                       | Default          |
| -------------- | ----------------- | -------- | --------------------------- | ---------------- |
| kernel         | k                 | string   | name of kernel              | contig           |
| filename       | f                 | string   | result filename             | <time>.csv       |
| threads        | t                 | Sequence | list of nthreads to test    | 1,2,+4:128:8     |
| intensity      | i                 | Sequence | list of intensity to test   | +0:1:0.1,+2:10:1 |
| size           | s                 | int      | array size                  | 16000000         |
| repeat         | r                 | int      | num of times to run         | 3                |
| distribution   | d                 | string   | threads affinity            | cyclic           |
| strong         |                   |          | strong scaling              | default          |
| weak           |                   |          | weak scaling                |                  |
| cpus           | c                 | Sequence | allowed CPUs (for cpu-bind) |                  |
| hierarchy      | l                 | Sequence | memory hierarchy            | 128,64,16,8,4    |
| quiet          |                   |          | reduce log                  | false            |
| help           |                   |          | print help                  |                  |

Sequence is a ourselves-define format: string literal can be parsed to a list of number:

| String format                 | Example              | Parsed list           |
| ----------------------------- | -------------------- | --------------------- |
| "\<values\>"                    | "1,2"                | [1,2]                 |
| "\<start>-\<end>"               | "3-5"                | [3,4,5]               |
| <\* or +>\<start>:\<end>:\<increase>" | +0.1:0.5:0.2,*8:32:2 | [0.1,0.3,0.5,8,16,32] |

An example:

```c
build/camp -k contig -s 1024000000 -r 3 -t *8:64:2 -i 0.1,1.0 -f result.csv -d cyclic -l 128,64,16,4
```

The execution use contiguous pattern, kernel use arrays sized 1024000000, we parallel the exection with [8,16,32,64] threads, and try operation intentity 0,1 and 1.0, result stored in result.csv, use cyclic threads distribution, each region in a memory level has [128,64,16,4] cores, each test repeat 3 times.

### Submit job to HPC cluster
A [sample](src/run.slurm) of slurm configuration file is given. Replace your buddget information and `sbatch run.slurm` instead of running the executable directly.

### Result
The result is dumped into a csv file whose name is configured in `-f` parameter. The first line in the csv file decribes the data in each column. Runtime, bandwidth and FLOPS are provided based on minimun runtime. The colon-seperated raw runtime values are provided in the last column.

### Custom kernel
Provide your kenrel by implementing interfaces in [kernel.h](src/kernel.h). Compile to get the kernel.o and link with other objects. An easier way to do this is change the [Makefile](src/Makefile) where `KERNELSRC = <your-source-file>` and remake.