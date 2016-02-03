# HonokaMiku Makefile.
# Compiles HonokaMiku to /bin/<option> folder

NDK_BUILD ?= ndk-build

# Check if windows (add --dynamicbase and --nxcompat)
ifeq ($(OS),Windows_NT)
WIN32_ARG := -Wl,--dynamicbase -Wl,--nxcompat
WINSOCK := -lws2_32
else
WIN32_ARG := 
WINSOCK := 
endif

all: honokamiku

honokamiku:
	-mkdir -p bin/honokamiku
	g++ -O3 -o ./bin/honokamiku/HonokaMiku -pie -fPIE $(WIN32_ARG) src/*.cc $(WINSOCK)

static_link:
	-mkdir -p bin/static_link
	g++ -static-libgcc -static-libstdc++ -pie -fPIE -O3 -o ./bin/static_link/HonokaMiku $(WIN32_ARG) src/*.cc $(WINSOCK)

with_resource_static:
	-mkdir -p bin/with_resource_static
	windres -O coff VersionInfo.rc VersionInfo.res
	g++ -static-libgcc -static-libstdc++ -O3 -pie -fPIE -o ./bin/with_resource_static/HonokaMiku $(WIN32_ARG) src/*.cc VersionInfo.res $(WINSOCK)
	-rm VersionInfo.res

ifeq ($(VSINSTALLDIR),)
vscmd:
	@echo Error: Run \"make vscmd\" from Visual Studio command prompt
	@exit 1
else
vscmd:
	-mkdir -p bin/vscmd
	cl -W3 -Zc:wchar_t -Ox -D"_CRT_SECURE_NO_WARNINGS" -D"WIN32" -D"_CONSOLE" -EHsc -MT -wd"4996" -c src\\*.cc
	rc -v -l 0 VersionInfo.rc
	link -OUT:"bin\\vscmd\\HonokaMiku.exe" -MANIFEST -NXCOMPAT -PDB:"bin\\vscmd\\HonokaMiku.pdb" -DEBUG -RELEASE -SUBSYSTEM:CONSOLE *.obj VersionInfo.res ws2_32.lib
	rm *.obj VersionInfo.res
endif

ndk:
	-mkdir -p bin/jni/{arm64-v8a,armeabi{,-v7a},mips{,64},x86{,_64}}{,/stripped}
	$(NDK_BUILD) APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=.
	-for arch in {arm{64-v8a,eabi{,-v7a}},mips{,64},x86{,_64}}; do \
		cp obj/local/$$arch/HonokaMiku bin/jni/$$arch/; \
		cp libs/$$arch/HonokaMiku bin/jni/$$arch/stripped/; \
    done
	rm -R obj
	rm -R libs
	#-find bin/jni/ -type d -empty -delete	#Throws error when using Command Prompt directly to build (without Cygwin)

clean:
	-rm -R bin
	-rm -R obj
	-rm -R libs
	-rm *.o *.obj VersionInfo.res

# Install needs to be root(sudo su)
install:
	install ./bin/honokamiku/HonokaMiku /usr/local/bin

.PHONY: all clean
