CXX=g++
CXXFLAGS=-std=c++17 -O2 -pthread

INCLUDE=-I./include
LIBS=-lredis++ -lhiredis

SRC=$(wildcard src/*.cpp)
OBJ=$(SRC:.cpp=.o)

TARGET=libredis_sdk.a

all: $(TARGET)

$(TARGET): $(OBJ)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

test: $(TARGET)
	$(CXX) $(CXXFLAGS) test/main.cpp -Iinclude -L. -lredis_sdk $(LIBS) -o test_app
	$(CXX) $(CXXFLAGS) test/eventtest.cpp -Iinclude -L. -lredis_sdk $(LIBS) -o test_event

clean:
	rm -f src/*.o *.a test_app