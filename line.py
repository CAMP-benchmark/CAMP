from scripts.camp_utils import *
import matplotlib
import matplotlib.pyplot as plt
import re
import csv 
import time
import numpy as np

pic_name = "Dotproduct"
def plot(result_filename: str):

    df = csv2df(result_filename)
    best_thread_list = []
    df = df.sort_values(by=["intensity", "nthreads"])
    OIs = get_elem_list("intensity", df)
    nthreads = get_elem_list("nthreads", df)
    data = get_elem_list("bandwidth(MB/s)", df, repeat=True)
    data = [value/1000 for value in data]

    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.plot(range(len(nthreads)),data,marker = 'o',color='b', label="Dot product")

    ax.legend(loc='lower right')
    ax.set_xticks(range(len(nthreads)), labels=nthreads)
    plt.setp(ax.get_xticklabels(), rotation=45, ha="right", rotation_mode="anchor")

    plt.xlabel("nthreads")
    plt.ylabel("Bandwidth (GB/s)")
    # plt.ylim([0, 10000])
    ax.set_ylim(bottom=0)

    # ax.set_aspect('auto')
    plt.tight_layout()
    # plt.show()
    plt.savefig("{}.png".format(pic_name), dpi=300)


if __name__ == '__main__':
    filename = sys.argv[1]
    plot(filename)
    