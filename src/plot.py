import sys
from scritps.mesh import *
if len(sys.argv) != 2:
    sys.stderr.write("\n--- need filename ---\n\n")
    sys.exit(1)
sys.stdout.flush()
filename = sys.argv[1]
df = csv2df(filename)
kernel_dfs = get_sub_df("kernel", df)
for kernel in kernel_dfs:
    get_mesh(df=kernel_dfs[kernel], pic_name=filename.replace(".csv", "_"+kernel), percentage=False)