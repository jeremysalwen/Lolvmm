CXXFLAGS+=--std=c++0x `pkg-config --cflags lilv-0` 

all: main

main: main.o
