APP_PLATFORM := android-9
APP_STL := gnustl_static	# I'm having trouble when using stlport_static. Complains about multiple-definition of std::exception
ifeq ($(NDK_COMPILE_ABI),)
APP_ABI := all
else
APP_ABI := $(NDK_COMPILE_ABI)
endif
