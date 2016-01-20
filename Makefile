# HonokaMiku Makefile.
# Compiles HonokaMiku to /bin/<option> folder

NDK_BUILD ?= ndk-build

all: md5.o bindir honokamiku

everything: md5.o honokamiku static_link with_resource_static

md5.o:
	gcc -c -o cpp_src/md5.o cpp_src/md5.c

bindir:
	-mkdir bin

honokamiku: md5.o bindir
	-mkdir bin/honokamiku
	g++ -O3 -o ./bin/honokamiku/HonokaMiku cpp_src/*.cc cpp_src/md5.o
	-rm cpp_src/md5.o

static_link: md5.o bindir
	-mkdir bin/static_link
	g++ -static-libgcc -static-libstdc++ -O3 -o ./bin/static_link/HonokaMiku cpp_src/*.cc cpp_src/md5.o
	-rm cpp_src/md5.o

with_resource_static: md5.o bindir
	-mkdir bin/with_resource_static
	windres -O coff VersionInfo.rc VersionInfo.res
	g++ -static-libgcc -static-libstdc++ -O3 -o ./bin/with_resource_static/HonokaMiku cpp_src/*.cc cpp_src/md5.o VersionInfo.res
	-rm cpp_src/md5.o VersionInfo.res

vscmd: bindir
	-mkdir bin/vscmd
	cl -W3 -Zc:wchar_t -Ox -D"_CRT_SECURE_NO_WARNINGS" -D"WIN32" -D"_CONSOLE" -EHsc -MT -c cpp_src\\*.c*
	rc -v -l 0 VersionInfo.rc
	link -OUT:"bin\\vscmd\\HonokaMiku.exe" -MANIFEST -NXCOMPAT -PDB:"bin\\vscmd\\HonokaMiku.pdb" -DEBUG -RELEASE -SUBSYSTEM:CONSOLE *.obj VersionInfo.res
	rm *.obj VersionInfo.res

ndk_arm: bindir
	-mkdir bin/jni
	-mkdir bin/jni/armeabi
	-mkdir bin/jni/armeabi-v7a
	-mkdir bin/jni/armeabi/stripped
	-mkdir bin/jni/armeabi-v7a/stripped
	-mkdir jni
	cp cpp_src/* jni/
	mv *.mk jni/
	$(NDK_BUILD)
	cp obj/local/armeabi/HonokaMiku bin/jni/armeabi/
	cp obj/local/armeabi-v7a/HonokaMiku bin/jni/armeabi-v7a/
	rm -R obj
	cp libs/armeabi/HonokaMiku bin/jni/armeabi/stripped/
	cp libs/armeabi-v7a/HonokaMiku bin/jni/armeabi-v7a/stripped/
	mv jni/*.mk .
	rm -R libs
	rm -R jni

# Install needs to be root(sudo su)
install:
	install ./bin/honokamiku/HonokaMiku /usr/local/bin
