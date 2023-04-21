import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from scripts.camp_utils import *
import sys



pic_name = "value_best_contig"

def plot():
    df = csv2df("rst_macro_cyclic_contig_10/result.csv")
    best_thread_list = []
    best_bandwidth = []
    full_bandwidth = []
    df = df.sort_values(by=["intensity", "nthreads"])
    OIs = get_elem_list("intensity", df)
    nthreads = get_elem_list("nthreads", df)
    OI_dfs = get_sub_df("intensity", df)
    bandwidth = np.zeros(shape=(len(OIs), len(nthreads)))  # row is OI, column is nthreads
    for i, OI in enumerate(OI_dfs):
        sub_df = OI_dfs[OI]
        bandwidth[i] = sub_df["bandwidth(MB/s)"].to_numpy()
        max_idx = np.argmax(bandwidth[i])
        full_bandwidth.append(bandwidth[i][-1])
        best_bandwidth.append(np.amax(bandwidth[i]))
        best_thread_list.append(nthreads[max_idx])


    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.plot(range(len(OIs)),full_bandwidth,marker = 'o',color='r', label="Full")
    ax.plot(range(len(OIs)),best_bandwidth,marker = 'o',color='b', label="Underpopulate")

    ax.legend()
    ax.set_xticks(range(len(OIs)), labels=OIs)
    plt.setp(ax.get_xticklabels(), rotation=45, ha="right", rotation_mode="anchor")

    plt.xlabel("Operational intensity")
    plt.ylabel("Bandwidth (MB/s)")
    # plt.ylim([0, 10000])

    # ax.set_aspect('auto')
    plt.tight_layout()
    # plt.show()
    plt.savefig("{}.png".format(pic_name), dpi=300)

plot()