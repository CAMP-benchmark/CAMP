import sys,operator
from .ert_utils import *
from functools import reduce

def text_list_2_string(text_list):
  return reduce(operator.add,[t+" " for t in text_list])

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
    made_results = make_dir_if_needed(self.results_dir,"results",False)
    
    return 0

  def build(self):
    print("  Building CAMP core code...")

    command_prefix =                                                       \
      self.dict["CONFIG"]["CC"]                                                + \
      self.dict["CONFIG"]["CFLAGS"]                                            + \
      ["-I%s/src" % self.exe_path]                                   + \
      ["-DFLOP=%d" % self.flop]

    targets = ["main","util","driver"]
    targets.append(self.dict["CONFIG"]["KERNEL"][0])
    for src in targets:
      command = command_prefix + \
                ["-c","%s/src/%s.c" % (self.exe_path,src)] + \
                ["-o","%s/%s.o" % (self.flop_dir,src)]
      if execute_noshell(command, True) != 0:
        sys.stderr.write("Compiling driver, %s, failed\n" % src)
        return 1

    command = self.dict["CONFIG"]["LD"]      + \
              self.dict["CONFIG"]["LDFLAGS"]

    command += ["%s/%s.o" % (self.flop_dir, obj) for obj in targets] + \
                self.dict["CONFIG"]["LDLIBS"]                                  + \
                ["-o","%s/%s" % (self.flop_dir,self.dict["CONFIG"]["KERNEL"][0])]

    if execute_noshell(command, True) != 0:
      sys.stderr.write("Linking code failed\n")
      return 1

    return 0