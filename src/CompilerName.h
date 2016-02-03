/**
* CompilerName.h
* Returns the compiler name
**/

#ifdef __cplusplus
#include <cstdio>
#include <cstring>
#else
#include <stdio.h>
#include <string.h>
#endif

char name[256];
bool already_loaded=false;

const char* CompilerName()
{
	if(already_loaded) return name;
	memset(name,0,256);
#if defined(__clang__) || defined(__ICC) || defined(__INTEL_COMPILER)
	/* Clang/LLVM or Intel Compiler. It support __VERSION__ ----- */
	strcpy(name,__VERSION__);
#elif defined(__GNUC__) || defined(__GNUG__)
	/* GCC. -----------------------------------------------------*/
	strcpy(name,"gcc-" __VERSION__);
#elif defined(__HP_cc) || defined(__HP_aCC)
	/* Hewlett-Packard C/aC++. ---------------------------------- */
#ifdef __cplusplus
	const static int v=__HP_aCC;
#else
	const static int v=__HP_cc;
#endif
	sprintf(name,"HP C/aC++ Version A.%02d.%02d.%02d",v/10000,(v/100)%100,v%100);
#elif defined(__IBMC__) || defined(__IBMCPP__)
	/* IBM XL C/C++. -------------------------------------------- */
	strcpy(name,"IBM XL C/C++ Version " __xlc__);
#elif defined(_MSC_VER)
	/* Microsoft Visual Studio. --------------------------------- */
	sprintf(name,"Visual Studio Version %02d.%02d.%05d.%02d",int(_MSC_VER/100),int(_MSC_VER)%100,int(_MSC_FULL_VER)%100000,_MSC_BUILD);
#elif defined(__PGI)
	/* Portland Group PGCC/PGCPP. ------------------------------- */
	sprintf(name,"PGCC/PGCPP Version %02d.%d.%d",__PGIC__,__PGIC_MINOR,__PGIC_PATCHLEVEL__);
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
	/* Oracle Solaris Studio. ----------------------------------- */
#ifdef __cplusplus
	const static int v=__SUNPRO_CC;
#else
	const static int v=__SUNPRO_C;
#endif
	sprintf(name,"Solaris Studio Version %x.%x.%x",v/0x1000,(v/0x10)%0x100,v%0x10);
#else
	strcpy(name,"Unknown compiler");
#endif
	
	already_loaded=true;
	return name;
}