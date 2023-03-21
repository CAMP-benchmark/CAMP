import sys,operator,os,time
from scripts.camp_utils import *
from scripts.mesh import *
from functools import reduce

def text_list_2_string(text_list):
  return reduce(operator.add,[t+" " for t in text_list])

first = True

class camp_core:
  def __init__(self, config_filename):
    self.dict = {}
    self.configure_filename = config_filename

  def configure(self):
    print("Reading configuration from '%s'..." % self.configure_filename)

    try:
      configure_file = open(self.configure_filename,"r")
    except IOError:
      sys.stderr.write("Unable to open '%s'...\n" % self.configure_filename)
      return 1

    self.dict["CONFIG"] = {}

    for line in configure_file:
      line = line[:-1]

      if len(line) > 0 and line[0] != "#":
        line = line.split()
        if len(line) > 0:
          target = line[0]
          value = line[1:]

          if len(target) > 0:
            self.dict["CONFIG"][target] = value

    self.results_dir = self.dict["CONFIG"]["RESULTS"][0]
    if os.path.exists(self.results_dir):
      t = time.localtime()
      self.results_dir += "_{}_{}".format(t.tm_hour,t.tm_min)
    made_results = make_dir_if_needed(self.results_dir,"results",False)
    self.result_csvname = "%s/result.csv" % (self.results_dir)
    
    return 0

  def build(self):
    print("  Building CAMP core code...")

    command_prefix =                                                       \
      self.dict["CONFIG"]["CC"]                                                + \
      self.dict["CONFIG"]["CFLAGS"]                                            + \
      ["-I%s/src" % self.exe_path]                                   + \
      ["-DFLOP=%d" % self.flop]

    targets = ["main","util","py_driver","array"]
    targets.append(self.dict["CONFIG"]["KERNEL"][0])
    for src in targets:
      command = command_prefix + \
                ["-c","%s/src/%s.c" % (self.exe_path,src)] + \
                ["-o","%s/%s.o" % (self.flop_dir,src)]
      if execute_noshell(command) != 0:
        sys.stderr.write("Compiling driver, %s, failed\n" % src)
        return 1

    command = self.dict["CONFIG"]["LD"]      + \
              self.dict["CONFIG"]["LDFLAGS"]

    command += ["%s/%s.o" % (self.flop_dir, obj) for obj in targets] + \
                self.dict["CONFIG"]["LDLIBS"]                                  + \
                ["-o","%s/%s" % (self.flop_dir,self.dict["CONFIG"]["KERNEL"][0])]

    if execute_noshell(command) != 0:
      sys.stderr.write("Linking code failed\n")
      return 1

    return 0

  def run(self):
    # write csv title
    global first
    if (first):
      result_csv = open(self.result_csvname, "a")  # append mode
      result_csv.write("kernel, memory(MiB),     MFLOP, nthreads, intensity, runtime(s), bandwidth(MB/s),          mflops, repeat, raw_runtime(colon-seperated)\n")
      result_csv.close()
      first = False

    base_command = list_2_string(self.dict["CONFIG"]["CAMP_RUN"])
    base_command = base_command.replace("CAMP_RESULT", self.result_csvname)
    base_command = base_command.replace("CAMP_MEM", self.dict["CONFIG"]["MEM"][0])
    base_command = base_command.replace("CAMP_EXE", "%s/%s" % (self.flop_dir,self.dict["CONFIG"]["KERNEL"][0]))
    base_command = base_command.replace("CAMP_NTHREADS", self.dict["CONFIG"]["OPENMP_THREADS"][0])
    base_command = base_command.replace("CAMP_FLOP", str(self.flop/48))

    if execute_shell(base_command) != 0:
      sys.stderr.write("Unable to complete %s, experiment\n" % (self.results_dir))
      return 1
  
    return 0


  def plot(self):
    filename = self.result_csvname
    df = csv2df(filename)
    kernel_dfs = get_sub_df("kernel", df)
    for kernel in kernel_dfs:
      get_mesh(df=kernel_dfs[kernel], pic_name=self.results_dir+"/percent_mesh", percentage=True)
      get_mesh(df=kernel_dfs[kernel], pic_name=self.results_dir+"/mesh", percentage=False)