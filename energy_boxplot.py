#!/usr/bin/env python3
# known issue: same FLOP can NOT be batched, maybe because use same exeutable?

from scripts.camp_utils import *
import matplotlib
import matplotlib.pyplot as plt
import re
import csv 
import time
import numpy as np

# -------------- parameter -----------------
# frequency = 1500000

config_tamplate = "config/example_tag_tamplate_boxplot"
config_tag_tamplate = "tagged_batch/energy_boxplot_{}"
batch_tamplate = "batch/archer2_tag_tamplate.slurm"
batch_tag_tamplate = "tagged_batch/archer2_energy_boxplot_{}.slurm"

# [[flop, nthread], ...]
# config_list = [[1,128],[2,128]]

boxplot_repeat = 10

# -------------- end -----------------

def execute(config_list: list, result_filename: str):
    make_dir_if_needed("tagged_batch","tagged_batch",echo=True)
    with open(result_filename, 'w', encoding='UTF8', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["repeat", "jobid", "energy(J)"])
        for config in config_list:
            tag = "{}".format(config)
            config_tag = config_tag_tamplate.format(tag)
            replace_cmd = "sed -e 's/$REPEAT/{}/g' {} > {}".format(config, config_tamplate, config_tag)
            if execute_shell(replace_cmd, echo=True) != 0:
                sys.stderr.write("Replace tamplate failed\n")
                return 1
            batch_tag = batch_tag_tamplate.format(tag)
            replace_cmd = "sed -e 's/$FREQUENCY/{}/g' -e 's/$CONFIG/{}/g' {} > {}".format(frequency, config_tag.split('/')[-1], batch_tamplate, batch_tag)
            if execute_shell(replace_cmd, echo=True) != 0:
                sys.stderr.write("Replace tamplate failed\n")
                return 1
            
            # run 10 time to get enough data for boxplot
            for i in range(boxplot_repeat):
                submit_cmd = "sbatch {} | grep -Po -m1 '\\d+'".format(batch_tag)
                # submit_cmd = "echo 'Submitted batch job 3391917' | grep -Po -m1 '\\d+'"
                retcode, retval = stdout_shell(submit_cmd, echo=True)
                writer.writerow([config,retval.strip('\n')])
                # time.sleep(10)

def extract(result_filename: str):
    with open(result_filename, 'r', encoding='UTF8', newline='') as f:
        reader = csv.reader(f)
        next(reader, None)
        config_list = []
        for row in reader:
            config_list.append(row)
        # print(config_list)

    with open(result_filename, 'w', encoding='UTF8', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["repeat", "jobid", "Runtime(s)", "energy(J)"])
        for config in config_list:
            jobid = config[1]
            query_cmd = "sacct -j {} --format=ElapsedRaw | grep -Po -m1 '\\d+'".format(jobid)
            retcode, retval = stdout_shell(query_cmd, echo=True)
            config.append(retval.strip('\n'))
            query_cmd = "sacct -j {} --format=ConsumedEnergyRaw | grep -Po -m1 '\\d+'".format(jobid)
            retcode, retval = stdout_shell(query_cmd, echo=True)
            config.append(retval.strip('\n'))
            writer.writerow(config)
    
    print("Extract energy by sacct, done")

def plot(result_filename_full: str):
    df = csv2df(result_filename_full)
    df = df.sort_values(by=["repeat"])
    runtime_list = get_elem_list("Runtime(s)", df, repeat=True)
    avg_runtime = []
    print(runtime_list)
    for i in range(len(repeat)):
        print(i*boxplot_repeat,(i+1)*boxplot_repeat,sum(runtime_list[i*boxplot_repeat:(i+1)*boxplot_repeat]))
        print(runtime_list[i*boxplot_repeat:(i+1)*boxplot_repeat])
        avg_runtime.append(sum(runtime_list[i*boxplot_repeat:(i+1)*boxplot_repeat])/boxplot_repeat)
    energy_list_full = get_elem_list("energy(J)", df, repeat=True)
    data = np.array(energy_list_full).reshape(len(repeat), -1)
    data = np.transpose(data)
    print(data)


    figure, ax = plt.subplots(1,1, constrained_layout=True)#,figsize=(5,4)) 

    # Creating plot
    bp = ax.boxplot(data)

    # ax.legend()
    ax.set_xticks([num+1 for num in range(len(avg_runtime))], labels=avg_runtime)
    print(avg_runtime)
    # plt.setp(ax.get_xticklabels(), rotation=45, ha="right", rotation_mode="anchor")

    plt.xlabel("Runtime (s)")
    plt.ylabel("Energy (J)")
    # plt.ylim([0, 10000])
    ax.set_ylim(bottom=0)

    # ax.set_aspect('auto')
    plt.tight_layout()
    # plt.show()
    pic_name = "rst_energy_boxplot_{}.png".format(frequency)
    plt.savefig(pic_name, dpi=300)
    print("Energy plot saved in: ", pic_name)
        
if __name__ == '__main__':
    frequency = sys.argv[1]
    command = sys.argv[2]

    repeat = [10, 50, 100]
    result_filename = "rst_energy_boxplot.csv"
    

    if command == "run":
        execute(repeat, result_filename)
    if command == "extract":
        # extract(result_filename)

        plot(result_filename)