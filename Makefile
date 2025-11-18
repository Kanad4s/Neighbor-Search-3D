TARGETLINEAR = linear
TARGETMPI = mpi
TARGETCELLS = cells
TARGETGENERATE = generate

FILE ?= mdf.cls

GCC = gcc
MPICC = mpicc
MPIRUN = mpirun

LIBS = -lm

np = 4

.PHONY: build, run, clean, generate, buildMpi, buildLinear

cleanLinear:
	rm -rf  $(TARGETLINEAR)

buildLinear: cleanLinear main.c
	$(GCC) main.c -o $(TARGETLINEAR) $(LIBS)

runLinear: buildLinear
	./$(TARGETLINEAR) $(FILE)

cleanCells:
	rm -rf $(TARGETCELLS)

buildCells: cleanCells gridLinear.c
	$(GCC) gridLinear.c -o $(TARGETCELLS) $(LIBS)

runCells: buildCells
	./$(TARGETCELLS) $(FILE)

cleanMpi: 
	rm -rf $(TARGETMPI)

buildMpi: cleanMpi parallelMpi.c
	$(MPICC) parallelMpi.c -o $(TARGETMPI)
	
runMpi: buildMpi
	$(MPIRUN) -np $(np) ./$(TARGETMPI) $(FILE)

clean:
	rm -rf $(TARGETLINEAR)
	rm -rf $(TARGETMPI)
	rm -rf $(TARGETCELLS)
	rm -rf $(TARGETGENERATE)

generate: generate.c 
	$(GCC) generate.c -o $(TARGETGENERATE)
help:
	@echo "build"
	@echo "buildMpi"
	@echo "buildLinear" 
	@echo "run"
	@echo "clean"
	@echo "generate"

.DEFAULT_GOAL := help