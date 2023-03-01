
TARGET=$(strip $(ARCH))

# assumes 
# PrgEnv-cray
ifeq ($(TARGET), archer2-cray)
CC=cc
CPPFLAGS = 
CFLAGS = -O2 -march=znver2 -Rpass=loop-vectorize -fopenmp #-fno-vectorize #-g
LDFLAGS = -fopenmp -fsanitize=address

# assumes 
# module load gnuX  
# where is the version e.g. 10.2.0
else ifeq ($(TARGET), ngio-gnu)
CC=gcc
CPPFLAGS =
CFLAGS = -O2 -fno-tree-vectorize -fopenmp
LDFLAGS = -fopenmp -lm

else ifeq ($(TARGET), )
arch-missing:
	@echo ERROR: You must specify ARCH \(for options see \"make usage\"\)
	@echo
	@exit 1
else
arch-invalid:
	@echo ERROR: You must provide a valid ARCH \(for options see \"make usage\"\)
	@echo
	@exit 1
endif

KERNELSRC = kernel.c

SRC = main.c util.c driver.c
SRC += $(KERNELSRC)
INC = driver.h util.h
OBJDIR = build

EXE = build/camp
ifneq ($(KERNELSRC), kernel.c)
  EXE = build/camp-custom
endif
OBJ = $(SRC:%.c=$(OBJDIR)/%.o)
# OBJ = $(SRC:%.c=%.o)

all: $(EXE)
	

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(EXE)

$(OBJDIR)/%.o : %.c Makefile $(INC)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


.PHONY: clean pat cleanpat run usage

clean:
	rm $(OBJ) $(EXE)

cleanpat:
	rm *pat

pat:
	pat_build -T mem_read_ram $(EXE)
	sbatch hwpc.slurm

run:
	build/camp -k stride4 -s 16000000 -r 10 -t 1,2 -i +0:0.3:0.1

usage:
	$(info Usage: ARCH=arch make)
	$(info Set ARCH to one of the following values:)
	$(info archer2-cray | ngio-gnu)

