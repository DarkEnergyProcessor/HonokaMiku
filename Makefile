# HonokaMiku Makefile.
# Compiles HonokaMiku to /bin/<option> folder

NDK_BUILD ?= ndk-build

# Check if windows (add --dynamicbase and --nxcompat)
ifeq ($(OS),Windows_NT)
WIN32_ARG := -Wl,--dynamicbase -Wl,--nxcompat
RC_C := windres -O coff VersionInfo.rc VersionInfo.res
RC_D := rm VersionInfo.res
RC_F := VersionInfo.res
else
WIN32_ARG := 
RC_C :=
RC_D :=
RC_F :=
endif
DEBUG_GENERATE =
PDB_GENERATE =
NDK_DEBUG =

all: honokamiku

honokamiku:
	-mkdir -p bin/honokamiku
	$(RC_C)
	g++ -O3 -o ./bin/honokamiku/HonokaMiku -pie -fPIE $(DEBUG_GENERATE) $(WIN32_ARG) src/*.cc $(RC_F)
	$(RC_D)

static_link:
	-mkdir -p bin/static_link
	$(RC_C)
	g++ -static-libgcc -static-libstdc++ -pie -fPIE $(DEBUG_GENERATE) -O3 -o ./bin/static_link/HonokaMiku $(WIN32_ARG) src/*.cc $(RC_F)
	$(RC_D)

ifeq ($(VSINSTALLDIR),)
vscmd:
	@echo Error: Run \"make vscmd\" from Visual Studio command prompt
	@exit 1
else
vscmd:
	-mkdir -p bin/vscmd
	cl -W3 -Zc:wchar_t -Ox -D"WIN32" -D_CONSOLE -EHsc -MT -wd"4996" -c src\\*.cc
	rc -v -l 0 VersionInfo.rc
	link -OUT:"bin\\vscmd\\HonokaMiku.exe" -MANIFEST -NXCOMPAT $(PDB_GENERATE) -RELEASE -SUBSYSTEM:CONSOLE *.obj VersionInfo.res
	rm *.obj VersionInfo.res
endif

debug:
	$(eval PDB_GENERATE = -PDB:"bin\\vscmd\\HonokaMiku.pdb" -DEBUG)
	$(eval DEBUG_GENERATE = -g)
	$(eval NDK_DEBUG = NDK_DEBUG=1)

ndk:
	-mkdir -p bin/jni/{arm64-v8a,armeabi{,-v7a},mips{,64},x86{,_64}}{,/stripped}
	$(NDK_BUILD) APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=. $(NDK_DEBUG)
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

.PHONY: all honokamiku static_link vscmd debug ndk clean
