/*
 * Learning Vulkan API
 * Vulkan, XCB, C (C99)
 */

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <xcb/xcb.h>
#include <ctype.h>
#include <math.h>
#include "linmath.h"

#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>

#include "enum_str_helper.h"

#define GET_GLOBAL_LEVEL_FUN_ADDR(name) \
pfn_##name = (PFN_##name) pfn_vkGetInstanceProcAddr(NULL,#name); \
if (pfn_##name == NULL) \
{ \
    printErrorMsg("cannot get" #name "address\n"); \
    return false; \
}

#define GET_INSTANCE_LEVEL_FUN_ADDR(name) \
pfn_##name = (PFN_##name) pfn_vkGetInstanceProcAddr(g_Instance,#name); \
if (pfn_##name == NULL) \
{ \
    printErrorMsg("cannot get" #name "address\n"); \
    return false; \
}

#define GET_DEVICE_LEVEL_FUN_ADDR(name) \
pfn_##name = (PFN_##name) pfn_vkGetDeviceProcAddr(g_LogicalDevice,#name); \
if (pfn_##name == NULL) \
{ \
    printErrorMsg("cannot get" #name "address\n"); \
    return false; \
}


PFN_vkGetInstanceProcAddr pfn_vkGetInstanceProcAddr = NULL;

PFN_vkGetDeviceProcAddr pfn_vkGetDeviceProcAddr = NULL;


/*
==============================
 printInfoMsg();
==============================
*/

void printInfoMsg(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  printf("INFO: ");
  vprintf(format, args);
  va_end(args);
}

/*
==============================
 printErrorMsg();
==============================
*/

void printErrorMsg(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  printf("ERROR: ");
  vprintf(format, args);
  va_end(args);
}

/*
==============================
 printWarningMsg();
==============================
*/

void printWarningMsg(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  printf("WARNING: ");
  vprintf(format, args);
  va_end(args);
}

/*
==============================
 printHelp();
==============================
*/

static void printHelp(void)
{

    #define LN(s) s "\n"

    printf( LN("usage: program [options]")
            LN("")
            LN("optional arguments:")
            LN("  -d, --devicenum=num   Vulkan device number `num`")
            LN("  -h, --help            display help message and exit"));
}

/*
==============================
 parseOptions();
==============================
*/

bool parseOptions(int argc, char **argv)
{



    return true;
}

/*
==============================
 openLibrary();
==============================
*/

void* openLibrary(const char *libraryName)
{
    void *libraryHandle = dlopen(libraryName, RTLD_NOW);

    if (!libraryHandle)
    {
        printErrorMsg("%s\n", dlerror());
        return NULL;
    }

    dlerror();

    return libraryHandle;
}

/*
==============================
 closeLibrary();
==============================
*/

int closeLibrary(void *libraryHandle)
{
    if (dlclose(libraryHandle))
    {
        printErrorMsg("%s\n", dlerror());
        return 1;
    }

    return 0;
}

/*
==============================
 getPointerFromLib();
==============================
*/

void* getPointerFromLib(void *libraryHandle, const char *fName)
{

    void *p = dlsym(libraryHandle, fName);

    char *error = dlerror();

    if (error != NULL)
    {
        printErrorMsg("%s\n", error);
        return NULL;
    }

    return p;
}

/*
==============================
 shutdownVulkan();
==============================
*/

void shutdownVulkan()
{

}

/*
==============================
 initVulkan();
==============================
*/

void initVulkan()
{

}

/*
===================
 main();
===================
*/

int main(int argc, char **argv)
{


    return 0;
}
