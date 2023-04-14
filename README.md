# :camping: Configurable App for Memory Probing

## Introduction
CAMP is a synthetic micro-benchmark for assessing deep memory hierarchies. This tool is insprired by [STREAM](https://www.cs.virginia.edu/stream/) and [adept](https://github.com/EPCCed/adept-kernel-openmp) and complement it by introducing variable operational intensity and access patterns. Underpopulation and placement suggestions for a high-performance computing (HPC) application are provided by finding a configuration that yields the best
performance.

### Objectives
CAMP is originally introduced in the hope of mimicing a real scientific computation program. CAMP would run on real machines and find the best configuration to launch the program, for example, placement and underpopulation configuration. However we found it difficult to develop a memory model that represent the memory behaviour of a real program. But we found this piece of code is helpful to observe the underpopulation effect for programs with different features, such as different patterns and operational intensity.

## Usage
### Python
Required Python packages can be found in `requirements.txt`. Code has been tested in Python version `3.8.5` and packages version
```
numpy==1.24.2
matplotlib==3.7.1
pandas==1.5.3
```
You may would like to use `virtualenv` to manage Python packages by 
```
pip install --user virtualenv # if you don't have virtualenv
mkdir /work/t01/t01/auser/<<name of your virtual environment>>  # Create the folder
virtualenv -p /opt/cray/pe/python/3.8.5.0/bin/python /work/t01/t01/asuser/<<name of your virtual environment>>  # -p flag means use this python interpreter
source /work/t01/t01/auser/<<name of your virtual environment>>/bin/activate
pip install -r requirements.txt
```

### Configure
Create a configure file in `config/` folder to specify details to build and run. See the sample for details. Each line in configure file is a key-value pair seperated by a space. Allowed values is listed in [legacy Usage](#Configurations)

### Execute CAMP
```
chmod +x ./camp
./camp config_file [option]
```
| option | Usage |
|-------|----------|
| (default) | build,run,plot |
| build-only | only build executables |
| run-only | only run executables |
| plot-only | only plot results |

### Batch
You may would like to perform the above `./camp` execution in a batch file. Sample batch files can be found in `batch/` folder to submit jobs to a batch system.

## Energy Consumption Analysis
A script is provided to analysis the energy consumption. We compare the energy between using underpopulated number of threads (result from above) and using fullypopulated (all available) number of threads.

After completing above experiments, you would find a file named `<result folder>/result.csv` which contains bandwidth data. The scripts gather these data and re-run by submit as seperated Slurm job so that we can collect energy data with `sacct`. One would also collect energy analysis using different CPU frequency.
 
```
./energy <result.csv> <frequency> <option: run/extract/plot>
```

To use this script, you shall first submit jobs to Slurm by e.g., `./energy rst_macro_cyclic_contig_10/result.csv 2250000 run`. Once all jobs are finished, `./energy rst_macro_cyclic_contig_10 2250000 extract` to collect energy data from Slurm and plot.

## Lagecy Usage
Above usege is based on the refactor ispired by [Empirical Roofline Toolkit](https://crd.lbl.gov/divisions/amcr/computer-science-amcr/par/research/roofline/software/ert/). Please refer to the below instruction if you would like to use the lagency version. Lagecy version use loop to implement variable operational intensity instead of C macro and vectorization is off, people argue that this limits the usage of results.

### TL;DR
```shell
cd src
mkdir build
ARCH=<target_architecture(see `make usage`)> make
build/camp <optional arguments>
```

### Configurations
Parameters are input through command line arguments. Posible arguments are:

| Name (`--<n>`) | Shortcut (`-<s>`) | Type     | Usage                       | Values(Default)          |
| -------------- | ----------------- | -------- | --------------------------- | ---------------- |
| kernel         | k                 | string   | name of kernel              | contig(default),stride<n>,stencil<5/9/7/13>,random           |
| filename       | f                 | string   | result filename             | .csv       |
| threads        | t                 | Sequence | list of nthreads to test    | 1,2,+4:128:8     |
| intensity      | i                 | Sequence | list of intensity to test   | +0:1:0.1,+2:10:1 |
| size           | s                 | int      | array size                  | 16000000         |
| repeat         | r                 | int      | num of times to run         | 3                |
| distribution   | d                 | string   | threads placement            | cyclic(default),block,spread,none           |
| strong         |                   |          | strong scaling              | default          |
| weak           |                   |          | weak scaling                |                  |
| cpus           | c                 | Sequence | allowed CPUs (for cpu-bind) |                  |
| hierarchy      | l                 | Sequence | memory hierarchy (n cores in each hierarchy)            | 128,64,16,8,4    |
| quiet          |                   |          | reduce log                  | false            |
| help           |                   |          | print help                  |                  |

Threads placement:
| Option | Description |
|----|----|
| cyclic | From top hierarchy to lowest, at each level put next thread in next region cyclicly. Creating a `spread` flavor placement in each hierarchy. |
| block | Fill a region in lowest hierarchy before moving to the next. |
| spread | OpenMP `spread` |
| cpu_bind | Explicitly set allowed CPUS |
| none | do nothing |

In fact, you may use `OMP_PLACES` and `OMP_PROC_BIND` to achieve the same effects, but tediously.

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