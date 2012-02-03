TARGET = bdftocbf.bin
SRC = bdftocbf.cpp

TARGET:
	g++ -Wall $(SRC) -o $(TARGET)

