CPPFLAGS = 
CFLAGS = -O2 -march=znver2 -Rpass=loop-vectorize -fopenmp -fno-vectorize #-g
LDFLAGS = -fopenmp #-fsanitize=address

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

$(EXE): $(OBJ)
	cc $(OBJ) $(LDFLAGS) -o $(EXE)

$(OBJDIR)/%.o : %.c Makefile $(INC)
	cc $(CPPFLAGS) $(CFLAGS) -c $< -o $@


.PHONY: clean pat cleanpat run

clean:
	rm $(OBJ) $(EXE)

cleanpat:
	rm *pat

pat:
	pat_build -T mem_read_ram $(EXE)
	sbatch hwpc.slurm

run:
	build/main -k stride4 -s 16000000 -r 10 -t 1,2 -i +0:0.3:0.1
