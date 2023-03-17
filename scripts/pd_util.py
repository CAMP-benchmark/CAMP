import pandas as pd

# csv title :'kernel, memory(MiB),     MFLOP, nthreads, intensity, runtime(s), bandwidth(MB/s),          mflops, repeat, raw_runtime(colon-seperated)'

def csv2df(filename: str) -> pd.DataFrame:
    """Get pandas.Dataframe from csv file"""
    df = pd.read_csv(filename)
    df.columns = df.columns.str.strip()
    return df

def get_elem_list(title: str, df_all: pd.DataFrame) -> list:
    """Get list of elements in column named `title`"""
    list = []
    for index, row in df_all.iterrows():
        elem = row[title]
        if elem not in list:
            list.append(elem)

    return list
    
def get_sub_df(title: str, df_all: pd.DataFrame) -> dict:
    """
    Get sub DaraFrame with same value in `title` column
    """
    values = get_elem_list(title, df_all)
    dfs = {}
    for value in values:
        dfs[value] = df_all.loc[df_all[title] == value]
        dfs[value].reset_index(drop=True, inplace=True)
    
    return dfs

if __name__ == '__main__':
    filename = '../src/ERT_ugly_alpha.csv'
    df = csv2df(filename)
    dfs = get_sub_df("kernel", df)
    for title in dfs:
        print(title)
        print(dfs[title].to_string())