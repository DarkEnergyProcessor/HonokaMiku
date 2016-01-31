# HonokaMiku Makefile.
# Compiles HonokaMiku to /bin/<option> folder

NDK_BUILD ?= ndk-build

all: md5.o bindir honokamiku

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

ndk: bindir
	-mkdir -p bin/jni/{arm64-v8a,armeabi{,-v7a},mips{,64},x86{,_64}}{,/stripped}
	$(NDK_BUILD) APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=.
	-cp obj/local/arm64-v8a/HonokaMiku bin/jni/arm64-v8a/
	-cp obj/local/armeabi/HonokaMiku bin/jni/armeabi/
	-cp obj/local/armeabi-v7a/HonokaMiku bin/jni/armeabi-v7a/
	-cp obj/local/mips/HonokaMiku bin/jni/mips/
	-cp obj/local/mips64/HonokaMiku bin/jni/mips64/
	-cp obj/local/x86/HonokaMiku bin/jni/x86/
	-cp obj/local/x86_64/HonokaMiku bin/jni/x86_64/
	rm -R obj
	-cp libs/arm64-v8a/HonokaMiku bin/jni/arm64-v8a/stripped/
	-cp libs/armeabi/HonokaMiku bin/jni/armeabi/stripped/
	-cp libs/armeabi-v7a/HonokaMiku bin/jni/armeabi-v7a/stripped/
	-cp libs/mips/HonokaMiku bin/jni/mips/stripped/
	-cp libs/mips64/HonokaMiku bin/jni/mips64/stripped/
	-cp libs/x86/HonokaMiku bin/jni/x86/stripped/
	-cp libs/x86_64/HonokaMiku bin/jni/x86_64/stripped/
	rm -R libs
	#-find bin/jni/ -type d -empty -delete	#Throws error when using Command Prompt directly to build (without Cygwin)

clean:
	-rm -R bin

# Install needs to be root(sudo su)
install:
	install ./bin/honokamiku/HonokaMiku /usr/local/bin
