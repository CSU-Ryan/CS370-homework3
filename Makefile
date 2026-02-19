CC := gcc
SRC_PATH := ./src
BIN_PATH := ./bin

all: build

build:
	mkdir -p bin
	$(CC) -o $(BIN_PATH)/checker.o $(SRC_PATH)/Checker.c
	$(CC) -o $(BIN_PATH)/coordinator.o $(SRC_PATH)/Coordinator.c

clean:
	rm -r $(BIN_PATH)/*