# HonokaMiku Makefile.
# Compiles HonokaMiku to /bin/<option> folder

all: md5.o adddir honokamiku

everything: md5.o honokamiku static_link with_resource_static vscmd

md5.o:
	gcc -c -o cpp_src/md5.o cpp_src/md5.c

adddir:
	! mkdir bin\\honokamiku
	! mkdir bin\\static_link
	! mkdir bin\\with_resource_static
	! mkdir bin\\vscmd

honokamiku: adddir
	g++ -std=c++11 -o ./bin/honokamiku/HonokaMiku cpp_src/*.cc cpp_src/md5.o

static_link: adddir
	g++ -std=c++11 -static-libgcc -static-libstdc++ -o ./bin/static_link/HonokaMiku cpp_src/*.cc cpp_src/md5.o

with_resource_static: adddir
	windres -O coff VersionInfo.rc VersionInfo.res
	g++ -std=c++11 -static-libgcc -static-libstdc++ -o ./bin/with_resource_static/HonokaMiku cpp_src/*.cc cpp_src/md5.o VersionInfo.res

vscmd: adddir
ifeq ($(VSINSTALLDIR),)
$(error Make sure you run from Visual Studio command prompt)
endif
	cl -W3 -Zc:wchar_t -Ox -D"_CRT_SECURE_NO_WARNINGS" -D"WIN32" -D"_CONSOLE" -EHsc -MT -c cpp_src\\HonokaMiku.cc cpp_src\\EN_Decrypter.cc cpp_src\\JP_Decrypter.cc cpp_src\\md5.c
	rc -v -l 0 VersionInfo.rc
	link -OUT:"bin\\vscmd\\HonokaMiku.exe" -MANIFEST -NXCOMPAT -PDB:HonokaMiku.pdb -DEBUG -RELEASE -SUBSYSTEM:CONSOLE HonokaMiku.obj EN_Decrypter.obj JP_Decrypter.obj md5.obj VersionInfo.res
	rm *.obj VersionInfo.res

# Install needs to be root(sudo su)
install:
	install ./bin/honokamiku/HonokaMiku /usr/local/bin
