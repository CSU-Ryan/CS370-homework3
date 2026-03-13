CC := gcc
SRC_PATH := .
BIN_PATH := .

all: build

build:
	$(CC) -o $(BIN_PATH)/checker $(SRC_PATH)/Checker.c
	$(CC) -o $(BIN_PATH)/coordinator $(SRC_PATH)/Coordinator.c

clean:
	rm -f $(BIN_PATH)/checker
	rm -f $(BIN_PATH)/coordinator