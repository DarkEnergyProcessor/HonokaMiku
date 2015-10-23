# HonokaMiku Makefile.
# Needs more testing.
# Compiles HonokaMiku to /bin/ folder

COMPILER_FLAGS=-std=c++11

all: md5.o honokamiku

md5.o:
	gcc -c -o cpp_src/md5.o cpp_src/md5.c

honokamiku:
	g++ $(COMPILER_FLAGS) -o ./bin/HonokaMiku cpp_src/*.cc cpp_src/md5.o

static_link: 
	g++ $(COMPILER_FLAGS) -static-libgcc -static-libstdc++ -o ./bin/HonokaMiku cpp_src/*.cc cpp_src/md5.o
