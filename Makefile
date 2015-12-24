# HonokaMiku Makefile.
# Compiles HonokaMiku to /bin/<option> folder

all: md5.o bindir honokamiku

everything: md5.o honokamiku static_link with_resource_static

md5.o:
	gcc -c -o cpp_src/md5.o cpp_src/md5.c

bindir:
	-mkdir bin

honokamiku: md5.o bindir
	-mkdir bin/honokamiku
	g++ -std=c++11 -O3 -o ./bin/honokamiku/HonokaMiku cpp_src/*.cc cpp_src/md5.o
	-rm cpp_src/md5.o

static_link: md5.o bindir
	-mkdir bin/static_link
	g++ -std=c++11 -static-libgcc -static-libstdc++ -O3 -o ./bin/static_link/HonokaMiku cpp_src/*.cc cpp_src/md5.o
	-rm cpp_src/md5.o

with_resource_static: md5.o bindir
	-mkdir bin/with_resource_static
	windres -O coff VersionInfo.rc VersionInfo.res
	g++ -std=c++11 -static-libgcc -static-libstdc++ -O3 -o ./bin/with_resource_static/HonokaMiku cpp_src/*.cc cpp_src/md5.o VersionInfo.res
	-rm cpp_src/md5.o VersionInfo.res

vscmd: bindir
	-mkdir bin/vscmd
	cl -W3 -Zc:wchar_t -Ox -D"_CRT_SECURE_NO_WARNINGS" -D"WIN32" -D"_CONSOLE" -EHsc -MT -c cpp_src\\HonokaMiku.cc cpp_src\\EN_Decrypter.cc cpp_src\\JP_Decrypter.cc cpp_src\\TW_Decrypter.cc cpp_src\\md5.c
	rc -v -l 0 VersionInfo.rc
	link -OUT:"bin\\vscmd\\HonokaMiku.exe" -MANIFEST -NXCOMPAT -PDB:"bin\\vscmd\\HonokaMiku.pdb" -DEBUG -RELEASE -SUBSYSTEM:CONSOLE HonokaMiku.obj EN_Decrypter.obj JP_Decrypter.obj md5.obj TW_Decrypter.obj VersionInfo.res
	rm *.obj VersionInfo.res

# Install needs to be root(sudo su)
install:
	install ./bin/honokamiku/HonokaMiku /usr/local/bin
