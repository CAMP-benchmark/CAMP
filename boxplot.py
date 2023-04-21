from scripts.camp_utils import *
import matplotlib
import matplotlib.pyplot as plt
import re
import csv 
import time
import numpy as np


def plot(result_filename: str):
    data = []

    df = csv2df(result_filename)
    best_thread_list = []
    df = df.sort_values(by=["intensity", "nthreads"])
    OIs = get_elem_list("intensity", df)
    nthreads = get_elem_list("nthreads", df)

    # dotproduct
    rawtime = get_elem_list("raw_runtime(colon-seperated)", df, repeat=True)

    # contig
    OI_dfs = get_sub_df("intensity", df)
    for i, OI in enumerate(OI_dfs):
        sub_df = OI_dfs[OI]
        rawtime = get_elem_list("raw_runtime(colon-seperated)", sub_df, repeat=True)
        # bandwidth[i] = sub_df["bandwidth(MB/s)"].to_numpy()
        break
    
    for time in rawtime:
        timelist = time.split(":")
        timelist = [float(time) for time in timelist]
        data.append(timelist)
    print(data)

    figure, ax = plt.subplots(1,1, constrained_layout=True)#,figsize=(5,4)) 

    # Creating plot
    bp = ax.boxplot(data)

    ax.set_ylim(bottom=0)
    ax.set_xticklabels(nthreads)
    plt.setp(ax, xlabel = "nthreads")
    plt.setp(ax, ylabel = "Time (s)")

    # plt.show()
    plt.savefig("boxplot.png", dpi=300)


if __name__ == '__main__':
    filename = sys.argv[1]
    plot(filename)
    