# HonokaMiku Makefile.
# Compiles HonokaMiku to /bin/<option> folder

NDK_BUILD ?= ndk-build

CXXFLAGS?=
LDFLAGS?=
NDK_BUILD ?= ndk-build

# Append dash
ifdef PREFIX
xPREFIX = $(PREFIX)-
endif

# Check if we are compiling for Windows
ifeq ($(OS),Windows_NT)
# However, if PREFIX is set, it's possible that we are cross-compiling, so don't set it if prefix is set
ifndef PREFIX
RC_CMD := windres -O coff VersionInfo.rc VersionInfo.res
RC_FILE := VersionInfo.res
else
ifneq (,$(findstring mingw32,$(PREFIX)))
RC_CMD := $(xPREFIX)windres -O coff VersionInfo.rc VersionInfo.res
RC_FILE := VersionInfo.res
else
RC_CMD :=
RC_FILE :=
endif
endif
else
ifneq (,$(findstring mingw32,$(PREFIX)))
RC_CMD := $(xPREFIX)windres -O coff VersionInfo.rc VersionInfo.res
RC_FILE := VersionInfo.res
# MinGW32 Cross compiler doesn't automatically append .exe
EXTENSION_APPEND := .exe
else
RC_CMD :=
RC_FILE :=
endif
endif

# Debug flags
RELEASE_GCC_CMD := -O3
RELEASE_MSV_CMD := -Ox -MT
DEBUG_GCC_CMD :=
DEBUG_MSV_CMD :=
NDK_DEBUG :=

# Files
GCC_FILES=JP_Decrypter.o V2_Decrypter.o HonokaMiku.o
MSVC_FILES=JP_Decrypter.obj V2_Decrypter.obj HonokaMiku.obj VersionInfo.res

# Rules
all: honokamiku

debug:
	@echo Debug build.
	$(eval RELEASE_GCC_CMD = -O0)
	$(eval RELEASE_MSV_CMD = -Od -D"_DEBUG" -MTd)
	$(eval DEBUG_GCC_CMD = -g -D_DEBUG)
	$(eval DEBUG_MSV_CMD = -PDB:"bin\\vscmd\\HonokaMiku.pdb" -DEBUG)
	$(eval NDK_DEBUG = NDK_DEBUG=1)

honokamiku: $(GCC_FILES)
	-mkdir -p bin/honokamiku
	$(RC_CMD)
	$(xPREFIX)g++ $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) -o bin/honokamiku/HonokaMiku$(EXTENSION_APPEND) $(CXXFLAGS) $(LDFLAGS) $(GCC_FILES) $(RC_FILE)
	-rm $(GCC_FILES) $(RC_FILE)

ndk:
	-mkdir -p bin/jni/{arm64-v8a,armeabi{,-v7a},mips{,64},x86{,_64}}{,/stripped}
	$(NDK_BUILD) APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=. $(NDK_DEBUG)
	-for arch in {arm{64-v8a,eabi{,-v7a}},mips{,64},x86{,_64}}; do \
		cp obj/local/$$arch/HonokaMiku bin/jni/$$arch/; \
		cp libs/$$arch/HonokaMiku bin/jni/$$arch/stripped/; \
	done
	rm -R obj
	rm -R libs

ifeq ($(VSINSTALLDIR),)
vscmd:
	@echo "Run from Visual Studio command prompt!"
	@false
else
vscmd: $(MSVC_FILES)
	-mkdir -p bin/vscmd
	link -OUT:"bin\\vscmd\\HonokaMiku.exe" -NXCOMPAT $(DEBUG_MSV_CMD) -RELEASE -SUBSYSTEM:CONSOLE $(LDFLAGS) $(MSVC_FILES)
	-rm $(MSVC_FILES)
endif

clean:
	-@rm $(GCC_FILES) $(MSVC_FILES) $(RC_FILE)
	-@rm -R obj
	-@rm -R libs

# Object files
# .o for GCC
# .obj for MSVC

JP_Decrypter.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) src/JP_Decrypter.cc

JP_Decrypter.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) src/JP_Decrypter.cc

V2_Decrypter.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) src/V2_Decrypter.cc

V2_Decrypter.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) src/V2_Decrypter.cc

HonokaMiku.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) src/HonokaMiku.cc

HonokaMiku.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) src/HonokaMiku.cc

VersionInfo.res:
	rc -v -l 0 VersionInfo.rc

.PHONY: all honokamiku debug ndk vscmd clean
