TARGET = main
TARGETMPI = parallel

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
buildLinear: gridLinear.c
	$(GCC) gridLinear.c -o gridLinear $(LIBS)
	./gridLinear

run: build
	./$(TARGET)

clean:
	rm -rf $(TARGET)

generate: generate.c 
	$(GCC) generate.c -o generate
help:
	

.DEFAULT_GOAL := run