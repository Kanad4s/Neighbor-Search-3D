TARGET = main

GCC = gcc

LIBS = -lm

.PHONY: build, run, clean, generate

build: main.c
	$(GCC) main.c -o $(TARGET) $(LIBS)

run: build
	./$(TARGET)

clean:
	rm -rf $(TARGET)

generate: generate.c 
	$(GCC) generate.c -o generate

.DEFAULT_GOAL := run