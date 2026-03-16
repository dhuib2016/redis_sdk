CXX=g++
CXXFLAGS=-std=c++17 -O2 -pthread

LOCAL_PREFIX=$(HOME)/.local
INCLUDE=-I./include -I$(LOCAL_PREFIX)/include
LDFLAGS=-L$(LOCAL_PREFIX)/lib64 -L$(LOCAL_PREFIX)/lib -Wl,-rpath,$(LOCAL_PREFIX)/lib64 -Wl,-rpath,$(LOCAL_PREFIX)/lib
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
	$(CXX) $(CXXFLAGS) $(INCLUDE) test/main.cpp -L. -lredis_sdk $(LDFLAGS) $(LIBS) -o test_app

clean:
	rm -f src/*.o *.a test_app