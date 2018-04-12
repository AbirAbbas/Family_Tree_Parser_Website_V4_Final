CC = gcc
CFLAGS = -Wall -std=c11 -g 

BIN = ./bin/
SRC = ./src/
INC = ./include/
ROOT = ./../

all: $(BIN)LinkedListAPI.o $(BIN)GEDCOMparser.o $(BIN)GEDCOMutilities.o
	$(CC) -shared -o $(ROOT)libparser.so $(BIN)LinkedListAPI.o $(BIN)GEDCOMparser.o $(BIN)GEDCOMutilities.o
		
parser: $(BIN)GEDCOMparser.o $(BIN)GEDCOMutilities.o
	ar rcs $(BIN)libparser.a $(BIN)GEDCOMparser.o $(BIN)GEDCOMutilities.o
		
list: $(BIN)LinkedListAPI.o
	ar rcs $(BIN)liblist.a $(BIN)LinkedListAPI.o
	
clean:
	rm -f $(BIN)*
	
$(BIN)LinkedListAPI.o: $(SRC)LinkedListAPI.c
	$(CC) $(CLFAGS) -c $(SRC)LinkedListAPI.c -fpic -Iinclude -o $(BIN)LinkedListAPI.o
	
$(BIN)GEDCOMparser.o: $(SRC)GEDCOMparser.c
	$(CC) $(CLFAGS) -c $(SRC)GEDCOMparser.c -fpic -Iinclude -o $(BIN)GEDCOMparser.o
	
$(BIN)GEDCOMutilities.o: $(SRC)GEDCOMutilities.c
	$(CC) $(CFLAGS) -c $(SRC)GEDCOMutilities.c -fpic -Iinclude -o $(BIN)GEDCOMutilities.o
	
