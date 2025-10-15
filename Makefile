TARGET = main
TARGETMPI = parallel
TARGETLINEAR = gridLinear
TARGETGENERATE = generate

GCC = gcc
MPICC = mpicc
MPIRUN = mpirun

LIBS = -lm

np = 4

.PHONY: build, run, clean, generate, buildMpi, buildLinear

build: main.c
	$(GCC) main.c -o $(TARGET) $(LIBS)

buildMpi: parallelMpi.c
	$(MPICC) parallelMpi.c -o $(TARGETMPI)
	$(MPIRUN) -np $(np) ./$(TARGETMPI)

buildLinear: gridLinear.c clean
	$(GCC) gridLinear.c -o $(TARGETLINEAR) $(LIBS)
	./gridLinear

run: build
	./$(TARGET)

clean:
	rm -rf $(TARGET)
	rm -rf $(TARGETMPI)
	rm -rf $(TARGETLINEAR)
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