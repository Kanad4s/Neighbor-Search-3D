TARGET = main
TARGETMPI = parallel

GCC = gcc
MPICC = mpicc

LIBS = -lm

.PHONY: build, run, clean, generate, buildMpi

build: main.c
	$(GCC) main.c -o $(TARGET) $(LIBS)

buildMpi: parallelMpi.c
	$(MPICC) parallelMpi.c -o $(TARGETMPI)
	mpirun -np ./(TARGETMPI)

run: build
	./$(TARGET)

clean:
	rm -rf $(TARGET)

generate: generate.c 
	$(GCC) generate.c -o generate

.DEFAULT_GOAL := run