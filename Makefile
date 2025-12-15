TARGETLINEAR = linear
TARGETMPI = mpi
TARGETCELLS = cells
TARGETGENERATE = generate
UTILS = utils.c

FILE ?= mdf.cls
COMPARE1 ?= linear.csv
COMPARE2 ?= gridLinear.csv

GCC = gcc
MPICC = mpicc
MPIRUN = mpirun

LIBS = -lm

np = 4

.PHONY: build, run, clean, generate, buildMpi, buildLinear

cleanLinear:
	rm -rf  $(TARGETLINEAR)

buildLinear: cleanLinear main.c
	$(GCC) main.c $(UTILS) -o $(TARGETLINEAR) $(LIBS)

runLinear: buildLinear
	./$(TARGETLINEAR) $(FILE)

cleanCells:
	rm -rf $(TARGETCELLS)

buildCells: cleanCells gridLinear.c
	$(GCC) gridLinear.c $(UTILS) -o $(TARGETCELLS) $(LIBS)

runGrid: buildCells
	./$(TARGETCELLS) $(FILE)

cleanMpi: 
	rm -rf $(TARGETMPI)

buildMpi: cleanMpi parallelMpi.c
	$(MPICC) parallelMpi.c $(UTILS) -o $(TARGETMPI)
	
runMpi: buildMpi
	$(MPIRUN) -np $(np) ./$(TARGETMPI) $(FILE)

clean:
	rm -rf $(TARGETLINEAR)
	rm -rf $(TARGETMPI)
	rm -rf $(TARGETCELLS)
	rm -rf $(TARGETGENERATE)

generate: generate.c 
	$(GCC) generate.c -o $(TARGETGENERATE)

compare:
	./compare.py $(COMPARE1) $(COMPARE2)

help:
	@echo "runLinear" 
	@echo "runGrid" 
	@echo "runMPI"
	@echo "clean"
	@echo "generate"
	@echo "compare"

.DEFAULT_GOAL := help