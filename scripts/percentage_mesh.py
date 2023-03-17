import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from pd_util import *

def get_kernels_percent_mesh(df: pd.DataFrame):
    """get percentage mesh for multiple kernels"""
    kernel_dfs = get_sub_df("kernel", df)
    for kernel in kernel_dfs:
        get_percent_mesh(kernel_dfs[kernel], kernel)

def get_percent_mesh(df: pd.DataFrame, pic_name: str):
    """get percentage mesh of same kernel"""
    OIs = get_elem_list("intensity", df)
    nthreads = get_elem_list("nthreads", df)
    OI_dfs = get_sub_df("intensity", df)
    bandwidth = np.zeros(shape=(len(OIs), len(nthreads)))  # row is OI, column is nthreads
    for i, OI in enumerate(OI_dfs):
        sub_df = OI_dfs[OI].sort_values(by="nthreads")
        bandwidth[i] = sub_df["bandwidth(MB/s)"].to_numpy()

    plot_data = np.flip(bandwidth, axis=0)  # upside down

    # percentage of max threads
    for i,oi_line in enumerate(plot_data):
        line_base = oi_line[-1]
        for j,value in enumerate(oi_line):
            plot_data[i][j] = plot_data[i][j] / line_base

        fig = plt.figure()

    ax = fig.add_subplot(111)
    mycolors = 'Blues'
    cm = plt.colormaps.get_cmap(mycolors)

    normalize = matplotlib.colors.Normalize(vmin=np.min(plot_data), vmax=np.max(plot_data))
    lognorm = matplotlib.colors.Normalize(vmin=0.1, vmax=10)
    # plot = ax.pcolormesh(plot_data, norm=normalize, cmap=cm)
    plot = ax.imshow(plot_data,cmap=cm)
    ax.set_aspect('auto')

    # note max bandwidth
    for i in range(len(OIs)):
        max = -1
        pos = 0
        for j in range(len(nthreads)):
            if plot_data[i, j] > max:
                max = plot_data[i, j]
                pos = j
        text = ax.text(pos, i, round(plot_data[i, pos],2),
                       ha="center", va="center", color="w", fontsize = 'x-small')

    # label axis
    ypos =   [0,   3,   6,   9,   12,  15,  18,  21]
    yvalue = [0.1, 0.7, 1.3, 1.9, 3.0, 4.5, 7.0, 10]
    yvalue.reverse()
    # ax.set_yticks(xpos)
    # ax.set_yticklabels(xvalue)
    # ax.set_yticks(ypos, labels=yvalue)
    ax.set_xticks(range(len(nthreads)), labels=nthreads)
    plt.setp(ax.get_xticklabels(), rotation=45, ha="right",
         rotation_mode="anchor")
    # ax.set_yticks(range(len(nthreads)))
    # ax.set_yticklabels(nthreads)
    # ax.set_title("Contiguous access bandwidth")
    plt.xlabel("nthreads")
    plt.ylabel("Operational intensity")

    # color bar
    cbar = fig.colorbar(plot, ax=ax)
    # Create colorbar
    # cbar = ax.figure.colorbar(plot, ax=ax, **cbar_kw)
    cbar.ax.set_ylabel("Bandwidth improvement over fully populated", rotation=-90, va="bottom")

    # plt.imshow(plot_data)
    plt.tight_layout()
    # plt.show()
    plt.savefig("{}.png".format(pic_name), dpi=300)


filename = '../src/ERT_ugly_alpha.csv'
get_kernels_percent_mesh(csv2df(filename))