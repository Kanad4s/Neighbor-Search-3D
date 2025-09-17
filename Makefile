TARGET = main

GCC = gcc

LIBS = -lm

.PHONY: build, run, clean

build: main.c
	$(GCC) main.c -o $(TARGET) $(LIBS)

run: build
	./$(TARGET)

clean:
	rm -rf $(TARGET)

.DEFAULT_GOAL := run