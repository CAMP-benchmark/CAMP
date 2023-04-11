import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from scripts.camp_utils import *
import sys

def bandwidth_diff(df_1: pd.DataFrame, df_1: pd.DataFrame):
    """get percentage mesh of same kernel"""
    df = df_1.sort_values(by=["intensity", "nthreads"])
    OIs = get_elem_list("intensity", df)
    nthreads = get_elem_list("nthreads", df)
    OI_dfs = get_sub_df("intensity", df)
    bandwidth1 = np.zeros(shape=(len(OIs), len(nthreads)))  # row is OI, column is nthreads
    for i, OI in enumerate(OI_dfs):
        sub_df = OI_dfs[OI]
        bandwidth1[i] = sub_df["bandwidth(MB/s)"].to_numpy()
    
    df = df_2.sort_values(by=["intensity", "nthreads"])
    OIs = get_elem_list("intensity", df)
    nthreads = get_elem_list("nthreads", df)
    OI_dfs_1 = get_sub_df("intensity", df)
    bandwidth2 = np.zeros(shape=(len(OIs), len(nthreads)))  # row is OI, column is nthreads
    for i, OI in enumerate(OI_dfs):
        sub_df = OI_dfs[OI]
        bandwidth2[i] = sub_df["bandwidth(MB/s)"].to_numpy()

    diff = bandwidth1 - bandwidth2
    