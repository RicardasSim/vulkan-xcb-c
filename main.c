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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define INIT_WIDTH 800
#define INIT_HEIGHT 600

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
 getFncAddress();
==============================
*/

int getFncAddress(void *libraryHandle)
{

    *(void **)(&pfn_vkGetInstanceProcAddr) = getPointerFromLib(libraryHandle, "vkGetInstanceProcAddr");

    if (pfn_vkGetInstanceProcAddr == NULL)
    {
        printErrorMsg("cannot get vkGetInstanceProcAddr address\n");
        return false;
    }

    return true;
}

/*
==============================
 debugMessengerCallback();
==============================
*/

#ifdef DEBUG

static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData)
{

    /* Silence compiler warning for unused parameter pUserData */
    struct sUserData *p_sUserData = (struct sUserData *)pUserData;
    p_sUserData->dummy = 0;

    printf("\nDEBUG: ");

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) printf("%s","VK VERBOSE, ");
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) printf("%s","VK INFO, ");
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) printf("%s","VK WARNING, ");
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) printf("%s","VK ERROR, ");


    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) printf("%s","GENERAL ");
    else
    {
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) printf("%s","VALIDATION ");

        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        {
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) printf("%s","| ");
            printf("%s","PERFORMANCE ");
        }
    }

    printf("\nMessage ID: %d, Message Name: %s, Message: %s\n", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);

    if (pCallbackData->objectCount > 0)
    {
        printf("count of items in the pObjects array: %d\n", pCallbackData->objectCount);

        for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
        {
            printf("Object[%d] - Type: %s, Handle: %p, Name: %s\n",
                        i,
                        str_VkObjectType(pCallbackData->pObjects[i].objectType),
                        (void*)(pCallbackData->pObjects[i].objectHandle),
                        pCallbackData->pObjects[i].pObjectName ? pCallbackData->pObjects[i].pObjectName : "nullptr"
                        );
        }
    }

    if (pCallbackData->queueLabelCount > 0)
    {
        printf("count of items in the pQueueLabels array: %d\n",pCallbackData->queueLabelCount);

        for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
        {
            printf("Queue Label[%d] - Name: %s Color: {%f, %f, %f, %f}\n", i, pCallbackData->pQueueLabels[i].pLabelName, pCallbackData->pQueueLabels[i].color[0],
                pCallbackData->pQueueLabels[i].color[1], pCallbackData->pQueueLabels[i].color[2], pCallbackData->pQueueLabels[i].color[3]);
        }
    }

    if (pCallbackData->cmdBufLabelCount > 0)
    {
        printf("count of items in the pCmdBufLabels array: %d\n",pCallbackData->cmdBufLabelCount);

        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
        {
            printf("Buf Label[%d] - Name: %s Color: {%f, %f, %f, %f}\n", i, pCallbackData->pCmdBufLabels[i].pLabelName, pCallbackData->pCmdBufLabels[i].color[0],
                pCallbackData->pCmdBufLabels[i].color[1], pCallbackData->pCmdBufLabels[i].color[2], pCallbackData->pCmdBufLabels[i].color[3]);
        }
    }

    printf("\n");

    return VK_FALSE;
}

#endif

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

    if(!parseOptions(argc, argv))
    {
        return -1;
    }

    printInfoMsg("Starting a program.\n");





    printInfoMsg("Bye bye !\n");

    return 0;
}
