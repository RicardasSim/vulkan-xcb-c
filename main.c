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

#define SWAP_CHAIN_IMAGE_COUNT 2

int g_Width = INIT_WIDTH;
int g_Height = INIT_HEIGHT;

PFN_vkGetInstanceProcAddr pfn_vkGetInstanceProcAddr = NULL;

PFN_vkEnumerateInstanceLayerProperties pfn_vkEnumerateInstanceLayerProperties = NULL;
PFN_vkEnumerateInstanceExtensionProperties pfn_vkEnumerateInstanceExtensionProperties = NULL;
PFN_vkCreateInstance pfn_vkCreateInstance = NULL;

PFN_vkDestroyInstance pfn_vkDestroyInstance = NULL;
#ifdef DEBUG
PFN_vkCreateDebugUtilsMessengerEXT pfn_vkCreateDebugUtilsMessengerEXT = NULL;
PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT = NULL;
#endif
PFN_vkCreateXcbSurfaceKHR pfn_vkCreateXcbSurfaceKHR = NULL;
PFN_vkDestroySurfaceKHR pfn_vkDestroySurfaceKHR = NULL;
PFN_vkEnumeratePhysicalDevices pfn_vkEnumeratePhysicalDevices = NULL;
PFN_vkGetPhysicalDeviceProperties pfn_vkGetPhysicalDeviceProperties = NULL;
PFN_vkEnumerateDeviceLayerProperties pfn_vkEnumerateDeviceLayerProperties = NULL;
PFN_vkEnumerateDeviceExtensionProperties pfn_vkEnumerateDeviceExtensionProperties = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties pfn_vkGetPhysicalDeviceQueueFamilyProperties = NULL;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR pfn_vkGetPhysicalDeviceSurfaceSupportKHR = NULL;
PFN_vkCreateDevice pfn_vkCreateDevice = NULL;
PFN_vkGetDeviceProcAddr pfn_vkGetDeviceProcAddr = NULL;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR pfn_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR pfn_vkGetPhysicalDeviceSurfaceFormatsKHR = NULL;

PFN_vkDestroyDevice pfn_vkDestroyDevice = NULL;
PFN_vkGetDeviceQueue pfn_vkGetDeviceQueue = NULL;
PFN_vkCreateSemaphore pfn_vkCreateSemaphore = NULL;
PFN_vkDestroySemaphore pfn_vkDestroySemaphore = NULL;
PFN_vkCreateFence pfn_vkCreateFence = NULL;
PFN_vkResetFences pfn_vkResetFences = NULL;
PFN_vkWaitForFences pfn_vkWaitForFences = NULL;
PFN_vkDestroyFence pfn_vkDestroyFence = NULL;

#ifdef DEBUG
struct sUserData{
    int dummy;
};

struct sUserData sUsrDt;
#endif

bool g_Quit = false;
bool g_Ready = false;

int g_MousePosX = 0;
int g_MousePosY = 0;
int g_MousePosOldX = 0;
int g_MousePosOldY = 0;
bool g_MouseButton1 = false;
bool g_MouseButton2 = false;
bool g_MouseButton3 = false;

uint32_t g_RequestedDeviceNum = 0;

#ifdef DEBUG
const char *g_InstanceLayers[] = {"VK_LAYER_KHRONOS_validation"};
#else
const char *g_InstanceLayers[] = {0};
#endif

char **g_InstanceLayersArray = NULL;
uint32_t g_InstanceLayersArrayCount = 0;

#ifdef DEBUG
const char *g_InstanceExtensions[] = { "VK_KHR_surface" , "VK_KHR_xcb_surface" , "VK_EXT_debug_utils"};
#else
const char *g_InstanceExtensions[] = { "VK_KHR_surface" , "VK_KHR_xcb_surface" };
#endif

char **g_InstanceExtensionArray = NULL;
uint32_t g_InstanceExtensionArrayCount = 0;

VkInstance g_Instance = VK_NULL_HANDLE;

#ifdef DEBUG
VkDebugUtilsMessengerEXT g_DebugMessenger = NULL;
#endif

VkSurfaceKHR g_Surface = NULL;

uint32_t g_PhysicalDeviceCount = 0;
VkPhysicalDevice* g_PhysicalDevices = NULL;
VkPhysicalDevice g_SelectedPhysicalDevice = VK_NULL_HANDLE;

#ifdef DEBUG
const char *g_DeviceLayers[] = { "VK_LAYER_KHRONOS_validation" };
#else
const char *g_DeviceLayers[] = {0};
#endif

char** g_DeviceLayersArray = NULL;
uint32_t g_DeviceLayersArrayCount = 0;

#ifdef DEBUG
const char *g_DeviceExtensions[] = {"VK_KHR_swapchain"};
#else
const char *g_DeviceExtensions[] = {"VK_KHR_swapchain"};
#endif

char** g_DeviceExtArray = NULL;
uint32_t g_DeviceExtArrayCount = 0;

int32_t g_GraphicsQueueFamilyIndex = -1;
int32_t g_PresentQueueFamilyIndex = -1;

VkDevice g_LogicalDevice = NULL;

VkQueue g_GraphicsQueue = VK_NULL_HANDLE;
VkQueue g_PresentQueue = VK_NULL_HANDLE;

VkSemaphore g_semaphoreImageAvailableArr[SWAP_CHAIN_IMAGE_COUNT] = {NULL};
VkSemaphore g_semaphoreRenderFinishedArr[SWAP_CHAIN_IMAGE_COUNT] = {NULL};

VkFence fenceArr[SWAP_CHAIN_IMAGE_COUNT] = {NULL};

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
            LN("  -d, --devicenum=num   Vulkan device number `num`, first 1")
            LN("  -h, --help            display help message and exit"));
}

/*
===============================
 isNumberPositiveAndNotNull();
===============================
*/

bool isNumberPositiveAndNotNull(char *str, int *num)
{

    int numTmp = 0;

    if (str[0]=='-') return false;

    for (int i=0; str[i] != 0; ++i)
    {
        if (!isdigit(str[i])) return false;
    }

    numTmp=atoi(str);

    if (!numTmp) return false;

    *num = numTmp;

    return true;
}

/*
==============================
 parseOptions();
==============================
*/

bool parseOptions(int argc, char **argv)
{

    if (argc >1)
    {

        struct optparse options;

        static const struct optparse_long longopts[] = {
            {"help",        'h',    OPTPARSE_NONE},
            {"devicenum",   'd',    OPTPARSE_REQUIRED},
            { 0, 0, 0 },
        };

        optparse_init(&options, argv);

        char *arg;
        int opt, longindex;

        while ((opt = optparse_long(&options, longopts, &longindex)) != -1)
        {
            switch (opt)
            {

                case 'd':

                    printInfoMsg("device number, optarg -> %s\n", options.optarg);

                    int argNumber = 0;

                    if (isNumberPositiveAndNotNull(options.optarg, &argNumber))
                    {
                        printf("is valid, number %d\n",argNumber);
                        g_RequestedDeviceNum = argNumber;
                    }
                    else
                    {
                        printErrorMsg("requested physical device parameter not valid\n");
                        return false;
                    }
                    break;

                case 'h':
                    printHelp();
                    return false;

                case '?':

                    printInfoMsg("%s: %s\n", argv[0], options.errmsg);
                    //printInfoMsg("Unknown option!\n\n");
                    return false;

            }

            printInfoMsg("%c (%d, %d) = '%s'\n", opt, options.optind, longindex, options.optarg);

        }
        printInfoMsg("optind = %d\n", options.optind);

        while ((arg = optparse_arg(&options)))
            printInfoMsg("argument: %s\n", arg);
    }

    return true;
}

/*
==============================
 openLibrary();
==============================
*/

void *openLibrary(const char *libraryName)
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

void *getPointerFromLib(void *libraryHandle, const char *fName)
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
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData)
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

    printf("\nMessage ID: %d, Message Name: %s, Message: %s\n", pCallbackData->messageIdNumber,
                                        pCallbackData->pMessageIdName, pCallbackData->pMessage);

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
            printf("Queue Label[%d] - Name: %s Color: {%f, %f, %f, %f}\n", i,
                                    pCallbackData->pQueueLabels[i].pLabelName,
                                    pCallbackData->pQueueLabels[i].color[0],
                                    pCallbackData->pQueueLabels[i].color[1],
                                    pCallbackData->pQueueLabels[i].color[2],
                                    pCallbackData->pQueueLabels[i].color[3]);
        }
    }

    if (pCallbackData->cmdBufLabelCount > 0)
    {
        printf("count of items in the pCmdBufLabels array: %d\n",pCallbackData->cmdBufLabelCount);

        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
        {
            printf("Buf Label[%d] - Name: %s Color: {%f, %f, %f, %f}\n", i,
                                    pCallbackData->pCmdBufLabels[i].pLabelName,
                                    pCallbackData->pCmdBufLabels[i].color[0],
                                    pCallbackData->pCmdBufLabels[i].color[1],
                                    pCallbackData->pCmdBufLabels[i].color[2],
                                    pCallbackData->pCmdBufLabels[i].color[3]);
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
    if(pfn_vkDestroyFence)
    {
        for ( int32_t i = 0; i < SWAP_CHAIN_IMAGE_COUNT; ++i )
        {
            if(fenceArr[i])
            {
                pfn_vkDestroyFence( g_LogicalDevice, fenceArr[i], NULL );
                printInfoMsg("vkDestroyFence() [%d]\n", i);
            }
        }
    }

    if(pfn_vkDestroySemaphore)
    {
        for(uint32_t i = 0; i < SWAP_CHAIN_IMAGE_COUNT; ++i)
        {
            if (g_semaphoreImageAvailableArr[i])
            {
                pfn_vkDestroySemaphore(g_LogicalDevice, g_semaphoreImageAvailableArr[i], NULL);
                printInfoMsg("vkDestroySemaphore() [%d] (image available)\n", i);
            }
        }

        for(uint32_t i = 0; i < SWAP_CHAIN_IMAGE_COUNT; ++i)
        {
            if (g_semaphoreRenderFinishedArr[i])
            {
                pfn_vkDestroySemaphore(g_LogicalDevice, g_semaphoreRenderFinishedArr[i], NULL);
                printInfoMsg("vkDestroySemaphore() [%d] (render finished)\n", i);
            }
        }
    }

    if (g_LogicalDevice && pfn_vkDestroyDevice)
    {
        pfn_vkDestroyDevice(g_LogicalDevice, NULL);
        printInfoMsg("destroy a logical device, vkDestroyDevice()\n");
    }

    if (g_DeviceExtArrayCount)
    {
        for (uint32_t i = 0; i < g_DeviceExtArrayCount; ++i)
        {
            printInfoMsg("free device extensions array [%d]\n",i);
            free(g_DeviceExtArray[i]);
        }
    }

    g_DeviceExtArrayCount = 0;

    if (g_DeviceExtArray)
    {
        free(g_DeviceExtArray);
        printInfoMsg("free g_DeviceExtArray\n");
    }

    if (g_DeviceLayersArrayCount)
    {
        for ( uint32_t i = 0; i < g_DeviceLayersArrayCount; ++i)
        {
            printInfoMsg("free device layers array [%d]\n",i);
            free(g_DeviceLayersArray[i]);
        }
    }

    g_DeviceLayersArrayCount = 0;

    if (g_DeviceLayersArray)
    {
        free(g_DeviceLayersArray);
        printInfoMsg("free g_DeviceLayersArray\n");
    }

    if (g_PhysicalDevices)
    {
        free(g_PhysicalDevices);
        printInfoMsg("free g_PhysicalDevices\n");
    }

    if (g_Surface && pfn_vkDestroySurfaceKHR)
    {
        pfn_vkDestroySurfaceKHR(g_Instance, g_Surface, NULL);
        printInfoMsg("vkDestroySurfaceKHR()\n");
    }

#ifdef DEBUG
    if (g_DebugMessenger && pfn_vkDestroyDebugUtilsMessengerEXT)
    {
        pfn_vkDestroyDebugUtilsMessengerEXT(g_Instance, g_DebugMessenger,NULL);
        printInfoMsg("DestroyDebugUtilsMessenger()\n");
    }
#endif

    if (g_Instance && pfn_vkDestroyInstance)
    {
        pfn_vkDestroyInstance(g_Instance, NULL);
        printInfoMsg("vkDestroyInstance()\n");
    }

    if (g_InstanceExtensionArrayCount)
    {
        for ( uint32_t i = 0; i < g_InstanceExtensionArrayCount; ++i)
        {
            free(g_InstanceExtensionArray[i]);
            printInfoMsg("free instance extensions array [%d]\n",i);
        }
    }

    if (g_InstanceExtensionArray)
    {
        free(g_InstanceExtensionArray);
        printInfoMsg("free g_InstanceExtensionArray\n");
    }

    if (g_InstanceLayersArrayCount)
    {
        for ( uint32_t i = 0; i < g_InstanceLayersArrayCount; ++i)
        {
            free(g_InstanceLayersArray[i]);
            printInfoMsg("free instance layers array [%d]\n",i);
        }
    }

    if (g_InstanceLayersArray)
    {
        free(g_InstanceLayersArray);
        printInfoMsg("free g_InstanceLayersArray\n");
    }
}

/*
==============================
 maxValU();
==============================
*/

uint32_t maxValU(uint32_t a, uint32_t b)
{
    if (a > b) return a;
    return b;
}

/*
==============================
 isAvailable();
==============================
*/

bool isAvailable(char **array, uint32_t count, const char *name)
{

    if (count<1) return false;

    for (uint32_t i = 0; i < count; ++i)
    {
        if (strcmp(array[i],name)==0)
        {
            return true;
        }
    }

    return false;
}

/*
==============================
 initVulkan();
==============================
*/

bool initVulkan(xcb_window_t wnd, xcb_connection_t *conn)
{

    //get global level fnc address

    GET_GLOBAL_LEVEL_FUN_ADDR(vkEnumerateInstanceLayerProperties);
    GET_GLOBAL_LEVEL_FUN_ADDR(vkEnumerateInstanceExtensionProperties);
    GET_GLOBAL_LEVEL_FUN_ADDR(vkCreateInstance);

    //enumerate instance layers
    {
        uint32_t layerCount = 0;

        pfn_vkEnumerateInstanceLayerProperties(&layerCount,NULL);

        printInfoMsg("number of instance layer properties available: %d\n",layerCount);

        printInfoMsg("available instance layers: ");

        if (layerCount>0)
        {

            printf("\n");

            VkLayerProperties *layerProperties = NULL;

            layerProperties = (VkLayerProperties*) malloc(layerCount * sizeof(VkLayerProperties));

            if (layerProperties==NULL)
            {
                printErrorMsg("unable to allocate memory (1)\n");
                return false;
            }

            pfn_vkEnumerateInstanceLayerProperties(&layerCount,layerProperties);

            for ( uint32_t i = 0; i < layerCount; ++i)
            {
                printf("\t%s | %s\n", layerProperties[i].layerName, layerProperties[i].description);
            }

            uint32_t numberOfEllementsInst = sizeof g_InstanceLayers / sizeof g_InstanceLayers[0];

            if(numberOfEllementsInst == 1 && g_InstanceLayers[0] == NULL) numberOfEllementsInst = 0;

            if (numberOfEllementsInst)
            {
                for (uint32_t i = 0 ; i < layerCount; ++i)
                {
                    for (uint32_t a = 0; a < numberOfEllementsInst; ++a)
                    {
                        if (!strcmp(g_InstanceLayers[a], layerProperties[i].layerName))
                        {
                            char **pInstanceLayersArrayTmp =
                                (char**) realloc(g_InstanceLayersArray,
                                sizeof g_InstanceLayers[0] * (g_InstanceLayersArrayCount+1));

                            if (!pInstanceLayersArrayTmp)
                            {
                                printErrorMsg("unable to reallocate memory (1)\n");
                                free(layerProperties);
                                return false;
                            }

                            g_InstanceLayersArray = pInstanceLayersArrayTmp;

                            g_InstanceLayersArray[g_InstanceLayersArrayCount] =
                                (char*) malloc(strlen(layerProperties[i].layerName) + 1);

                            if (g_InstanceLayersArray[g_InstanceLayersArrayCount] == NULL)
                            {
                                printErrorMsg("unable to allocate memory (2)[%d]\n",g_InstanceLayersArrayCount);
                                free(layerProperties);
                                return false;
                            }

                            strcpy(g_InstanceLayersArray[g_InstanceLayersArrayCount], layerProperties[i].layerName);

                            g_InstanceLayersArrayCount++;
                        }

                    }
                }
            }

            free(layerProperties);
        }
        else printf("none.\n");
    }

#ifdef DEBUG
    if (!isAvailable(g_InstanceLayersArray, g_InstanceLayersArrayCount, "VK_LAYER_KHRONOS_validation"))
    {
        printErrorMsg("requested layer not available: VK_LAYER_KHRONOS_validation\n");
		return false;
    }
    else printInfoMsg("VK_LAYER_KHRONOS_validation available OK.\n");
#endif

    printInfoMsg("count of instance layers to be used: %d\n",g_InstanceLayersArrayCount);

    if (g_InstanceLayersArrayCount>0)
    {
        printInfoMsg("list of instance layers to be used:\n");
        for (uint32_t i = 0; i < g_InstanceLayersArrayCount; ++i)
        {
            printf("\t%s\n",g_InstanceLayersArray[i]);
        }
    }

    //enumerate instance extensions
    {

        uint32_t extensionCount = 0;

        pfn_vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

        printInfoMsg("instance extension count: %d\n", extensionCount);

        printInfoMsg("available instance extensions: ");

        if (extensionCount>0)
        {

            printf("\n");

            VkExtensionProperties *extensionProperties =
                (VkExtensionProperties*) malloc(extensionCount * sizeof(VkExtensionProperties));

            if (extensionProperties==NULL)
            {
                printErrorMsg("unable to allocate memory (3)\n");
                return false;
            }

            pfn_vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties);

            for (uint32_t i = 0 ; i < extensionCount; ++i)
            {
                printf("\t%s\n",extensionProperties[i].extensionName);
            }

            uint32_t numberOfEllementsExt = sizeof g_InstanceExtensions / sizeof g_InstanceExtensions[0];

            if(numberOfEllementsExt == 1 && g_InstanceExtensions[0] == NULL) numberOfEllementsExt = 0;

            if (numberOfEllementsExt)
            {
                for (uint32_t i = 0 ; i < extensionCount; ++i)
                {
                    for (uint32_t a = 0; a < numberOfEllementsExt; ++a)
                    {
                        if (!strcmp(g_InstanceExtensions[a], extensionProperties[i].extensionName))
                        {
                            char **pInstanceExtensionArrayTmp = (char**) realloc(g_InstanceExtensionArray,
                                    sizeof g_InstanceExtensionArray[0] * (g_InstanceExtensionArrayCount+1));

                            if (!pInstanceExtensionArrayTmp)
                            {
                                printErrorMsg("unable to reallocate memory (2)\n");
                                free(extensionProperties);
                                return false;
                            }

                            g_InstanceExtensionArray = pInstanceExtensionArrayTmp;

                            g_InstanceExtensionArray[g_InstanceExtensionArrayCount] =
                                (char*) malloc(strlen(extensionProperties[i].extensionName) + 1);

                            if (g_InstanceExtensionArray[g_InstanceExtensionArrayCount] == NULL)
                            {
                                printErrorMsg("unable to allocate memory (4)[%d]\n",g_InstanceExtensionArrayCount);
                                free(extensionProperties);
                                return false;
                            }

                            strcpy(g_InstanceExtensionArray[g_InstanceExtensionArrayCount],
                                extensionProperties[i].extensionName);

                            g_InstanceExtensionArrayCount++;
                        }
                    }
                }
            }

            free(extensionProperties);

        }
        else printf("none.\n");

    }

    printInfoMsg("count of instance extensions to be used: %d\n",g_InstanceExtensionArrayCount);

    if (g_InstanceExtensionArrayCount>0)
    {
        printInfoMsg("list of instance extensions to be used:\n");
        for (uint32_t i = 0; i < g_InstanceExtensionArrayCount; ++i)
        {
            printf("\t%s\n",g_InstanceExtensionArray[i]);
        }
    }

#ifdef DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugMsgrCreateInfo = {0};

    debugMsgrCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMsgrCreateInfo.flags = 0;
    //| VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    debugMsgrCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMsgrCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMsgrCreateInfo.pfnUserCallback = debugMessengerCallback;
    debugMsgrCreateInfo.pUserData = &sUsrDt;
#endif

    //create instance
    {
        VkApplicationInfo applicationInfo = {0};

        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = NULL;
        applicationInfo.pApplicationName = "Linux Vulkan XCB C Sandbox";
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
        applicationInfo.pEngineName = "Linux Vulkan XCB C Sandbox";
        applicationInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
        applicationInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo = {0};

        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef DEBUG
        instanceCreateInfo.pNext = &debugMsgrCreateInfo;
#else
        instanceCreateInfo.pNext = NULL;
#endif
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledLayerCount = g_InstanceLayersArrayCount;
        if (g_InstanceLayersArrayCount)
            instanceCreateInfo.ppEnabledLayerNames = (const char* const*) g_InstanceLayersArray;
        else instanceCreateInfo.ppEnabledLayerNames = NULL;
        instanceCreateInfo.enabledExtensionCount = g_InstanceExtensionArrayCount;
        if (g_InstanceExtensionArrayCount)
            instanceCreateInfo.ppEnabledExtensionNames = (const char* const*) g_InstanceExtensionArray;
        else instanceCreateInfo.ppEnabledExtensionNames = NULL;

       	VkResult result = pfn_vkCreateInstance(&instanceCreateInfo, NULL, &g_Instance);

        //TODO: free g_InstanceExtensionArray here

        if (result != VK_SUCCESS)
        {
            if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
                printErrorMsg("failed to create Vulkan instance, can't find a compatible Vulkan driver.\n");
            else printErrorMsg("failed to create Vulkan instance\n");

            return false;
        }

        printInfoMsg("create vulkan instance OK.\n");

    }

    //get inst level fnc address

    GET_INSTANCE_LEVEL_FUN_ADDR(vkDestroyInstance);
#ifdef DEBUG
    GET_INSTANCE_LEVEL_FUN_ADDR(vkCreateDebugUtilsMessengerEXT);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkDestroyDebugUtilsMessengerEXT);
#endif
    GET_INSTANCE_LEVEL_FUN_ADDR(vkCreateXcbSurfaceKHR);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkDestroySurfaceKHR);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkEnumeratePhysicalDevices);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkGetPhysicalDeviceProperties);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkEnumerateDeviceLayerProperties);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkEnumerateDeviceExtensionProperties);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkGetPhysicalDeviceQueueFamilyProperties);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkGetPhysicalDeviceSurfaceSupportKHR);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkCreateDevice);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkGetDeviceProcAddr);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_INSTANCE_LEVEL_FUN_ADDR(vkGetPhysicalDeviceSurfaceFormatsKHR);

#ifdef DEBUG
    {
        VkResult result = pfn_vkCreateDebugUtilsMessengerEXT(g_Instance, &debugMsgrCreateInfo, NULL, &g_DebugMessenger);

        if (result != VK_SUCCESS)
        {
            printErrorMsg("CreateDebugUtilsMessengerEXT()\n");
            return false;
        }

        printInfoMsg("CreateDebugUtilsMessengerEXT() OK.\n");

    }
#endif

    //create surface
    {
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {0};

        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.connection = conn;
        surfaceCreateInfo.window = wnd;

        VkResult result = pfn_vkCreateXcbSurfaceKHR(g_Instance, &surfaceCreateInfo, NULL, &g_Surface);

        if (result != VK_SUCCESS)
        {
            printErrorMsg("vkCreateXcbSurfaceKHR().");
            return false;
        }
    }

    printInfoMsg("create surface OK.\n");

    //enumerate physical devices (1)
    {
        VkResult result = pfn_vkEnumeratePhysicalDevices(g_Instance, &g_PhysicalDeviceCount, NULL);

        if (result != VK_SUCCESS)
        {
            printErrorMsg("failed to query the number of physical devices present.\n");
            return false;
        }

        // there has to be at least one device present
        if (g_PhysicalDeviceCount == 0)
        {
            printErrorMsg("couldn't detect any device present with Vulkan support.\n");
            return false;
        }

        printInfoMsg("number of Vulkan physical devices found: %d\n", g_PhysicalDeviceCount);

    }

    g_PhysicalDevices = (VkPhysicalDevice*) malloc(g_PhysicalDeviceCount * sizeof(VkPhysicalDevice));

    if (g_PhysicalDevices == NULL) {
        printErrorMsg("unable to allocate memory (5)\n");
        return false;
    }

    //enumerate physical devices (2)
    {
        VkResult result = pfn_vkEnumeratePhysicalDevices(g_Instance, &g_PhysicalDeviceCount, g_PhysicalDevices);

        if (result != VK_SUCCESS)
        {
            printErrorMsg("faied to enumerate physical devices present.\n");
            return false;
        }
    }

    for (uint32_t i = 0; i < g_PhysicalDeviceCount; ++i)
    {

        VkPhysicalDeviceProperties deviceProperties ={0};
        pfn_vkGetPhysicalDeviceProperties(g_PhysicalDevices[i], &deviceProperties);
        printInfoMsg("device number (%d):\n", i+1);

        printf("\tdevice name: %s\n", deviceProperties.deviceName);

        printf("\t%s\n", str_VkPhysicalDeviceType(deviceProperties.deviceType));

        printf("\tAPI version: %d.%d.%d\n", (deviceProperties.apiVersion >> 22),
                                            (deviceProperties.apiVersion >> 12) & 0x3FF,
                                            (deviceProperties.apiVersion & 0xFFF));
    }


    if(g_RequestedDeviceNum > g_PhysicalDeviceCount)
    {
        printErrorMsg("there is no physical device with such number.\n");
        printf("\tyou have requested physical device number: %d\n", g_RequestedDeviceNum);
        if(g_PhysicalDeviceCount == 1)
        {
            printf("\tbut there is only %d physical device found\n", g_PhysicalDeviceCount);
            printf("\trequested physical device number on this mashine can be only 1\n");
        }
        else
        {
            printf("\trequested physical device number on this mashine should be within range 1-%d\n",
                g_PhysicalDeviceCount);
            printf("\tbut there is only %d physical devices found\n", g_PhysicalDeviceCount);
        }


        return false;
    }

    if (g_RequestedDeviceNum > 0)
    {
        printInfoMsg("requested physical device number: %d\n", g_RequestedDeviceNum);
    }
    else
        printInfoMsg("by default using first available physical device.\n");

    if (g_RequestedDeviceNum > 0)
        g_SelectedPhysicalDevice = g_PhysicalDevices[g_RequestedDeviceNum-1];
    else
        g_SelectedPhysicalDevice = g_PhysicalDevices[0];

    //enumerate device layers
    {

        uint32_t layerCount = 0;

        pfn_vkEnumerateDeviceLayerProperties(g_SelectedPhysicalDevice, &layerCount, NULL);

        printInfoMsg("available device layers count %d\n",layerCount);

        printInfoMsg("available device layers: ");

        if (layerCount>0)
        {
            printf("\n");

            VkLayerProperties* layerProperties = NULL;

            layerProperties = (VkLayerProperties*) malloc(layerCount * sizeof(VkLayerProperties));

            if (layerProperties == NULL)
            {
                printErrorMsg("unable to allocate memory (7)\n");
                return false;
            }

            pfn_vkEnumerateDeviceLayerProperties(g_SelectedPhysicalDevice, &layerCount, layerProperties);

            for (uint32_t i=0 ; i < layerCount; ++i)
            {
                printf("\t%s | %s\n", layerProperties[i].layerName, layerProperties[i].description);
            }

            uint32_t numberOfEllements = sizeof g_DeviceLayers / sizeof g_DeviceLayers[0];

            if(numberOfEllements == 1 && g_DeviceLayers[0] == NULL) numberOfEllements = 0;

            if (numberOfEllements)
            {

                for (uint32_t i = 0 ; i < layerCount; ++i)
                {
                    for (uint32_t a = 0; a < numberOfEllements; ++a)
                    {
                        if (!strcmp(g_DeviceLayers[a], layerProperties[i].layerName))
                        {

                            char** pDeviceLayersArrayTmp = (char**) realloc(g_DeviceLayersArray, sizeof g_DeviceLayers[0] * (g_DeviceLayersArrayCount+1));

                            if (!pDeviceLayersArrayTmp)
                            {
                                printErrorMsg("unable to reallocate memory (3)\n");
                                free(layerProperties);
                                return false;
                            }

                            g_DeviceLayersArray = pDeviceLayersArrayTmp;


                            g_DeviceLayersArray[g_DeviceLayersArrayCount] = (char*) malloc(strlen(layerProperties[i].layerName) + 1);


                            if (g_DeviceLayersArray[g_DeviceLayersArrayCount] == NULL)
                            {
                                printErrorMsg("unable to allocate memory (8)[%d]\n", g_DeviceLayersArrayCount);
                                free(layerProperties);
                                return false;
                            }

                            strcpy(g_DeviceLayersArray[g_DeviceLayersArrayCount],layerProperties[i].layerName);

                            g_DeviceLayersArrayCount++;

                        }
                    }
                }
            }

            free(layerProperties);
        }
        else printf("none.\n");
	}

    printInfoMsg("count of device layers to be used: %d\n",g_DeviceLayersArrayCount);

    if (g_DeviceLayersArrayCount>0)
    {
        printInfoMsg("list of device layers to be used:\n");
        for (uint32_t i = 0; i < g_DeviceLayersArrayCount; ++i)
        {
            printf("\t%s\n",g_DeviceLayersArray[i]);
        }
    }

	//enumerate device extensions
    {

        uint32_t extensionCount = 0;

        pfn_vkEnumerateDeviceExtensionProperties(g_SelectedPhysicalDevice, NULL, &extensionCount, NULL);

        printInfoMsg("available device extensions count: %d\n", extensionCount);

        printInfoMsg("available device extensions: ");

        if (extensionCount>0)
        {
            printf("\n");

            VkExtensionProperties* extensionProperties =
            (VkExtensionProperties*) malloc(extensionCount * sizeof(VkExtensionProperties));

            if (extensionProperties==NULL)
            {
                printErrorMsg("unable to allocate memory (9)\n");
                return false;
            }

            pfn_vkEnumerateDeviceExtensionProperties(g_SelectedPhysicalDevice,
                                                        NULL,
                                                        &extensionCount,
                                                        extensionProperties);

            for (uint32_t i=0 ; i < extensionCount; ++i)
            {
                printf("\t%s\n", extensionProperties[i].extensionName);
            }

            uint32_t numberOfEllements = 0;

            numberOfEllements = sizeof g_DeviceExtensions / sizeof g_DeviceExtensions[0];

            if(numberOfEllements == 1 && g_DeviceExtensions[0] == NULL) numberOfEllements = 0;

            if (numberOfEllements)
            {

                for (uint32_t a = 0; a < numberOfEllements; ++a)
                {
                    for (uint32_t i = 0 ; i < extensionCount; ++i)
                    {
                        if (!strcmp(g_DeviceExtensions[a], extensionProperties[i].extensionName))
                        {

                            char** pDeviceExtensionsArrayTmp = (char**) realloc(g_DeviceExtArray,
                                    sizeof g_DeviceExtensions[0] * (g_DeviceExtArrayCount+1));

                            if (!pDeviceExtensionsArrayTmp)
                            {
                                printErrorMsg("unable to reallocate memory (4)\n");
                                free(extensionProperties);
                                return false;
                            }

                            g_DeviceExtArray = pDeviceExtensionsArrayTmp;

                            g_DeviceExtArray[g_DeviceExtArrayCount] =
                                (char*) malloc(strlen(extensionProperties[i].extensionName) + 1);


                            if (g_DeviceExtArray[g_DeviceExtArrayCount] == NULL)
                            {
                                printErrorMsg("unable to allocate memory (10)[%d]\n",g_DeviceExtArrayCount);
                                free(extensionProperties);
                                return false;
                            }

                            strcpy(g_DeviceExtArray[g_DeviceExtArrayCount],
                                extensionProperties[i].extensionName);

                            g_DeviceExtArrayCount++;

                        }
                    }
                }


            }
           free(extensionProperties);
        }
        else printf("none.\n");

	}

    printInfoMsg("count of device extensions to be used: %d\n",g_DeviceExtArrayCount);

    if (g_DeviceExtArrayCount>0)
    {
        printInfoMsg("list of device extensions to be used:\n");
        for (uint32_t i = 0; i < g_DeviceExtArrayCount; ++i)
        {
            printf("\t%s\n",g_DeviceExtArray[i]);
        }
    }

    //queue families
    {
        uint32_t queueFamilyCount = 0;

        pfn_vkGetPhysicalDeviceQueueFamilyProperties(g_SelectedPhysicalDevice, &queueFamilyCount, NULL);

        printInfoMsg("queue family count %d\n",queueFamilyCount);

        if (queueFamilyCount<1)
        {
            printErrorMsg("queue family count is not greater than zero, exitting.\n");
            return false;
        }

        VkQueueFamilyProperties* familyProperties =
            (VkQueueFamilyProperties*) malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));

        if (familyProperties == NULL)
        {
            printErrorMsg("unable to allocate memory (11)\n");
            return false;
        }

        pfn_vkGetPhysicalDeviceQueueFamilyProperties(g_SelectedPhysicalDevice,	&queueFamilyCount, familyProperties);

        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            printInfoMsg("count of queues in queue family [%d]: %d\n", i, familyProperties[i].queueCount);

            printInfoMsg("Supported operations on this queue family [%d]:\n",i);

            if (familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                printf("\tGraphics\n");

            if (familyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
                printf("\tCompute\n");

            if (familyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
                printf("\tTransfer\n");

            if (familyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                printf("\tSparse Binding\n");

            if (familyProperties[i].queueFlags & VK_QUEUE_PROTECTED_BIT)
                printf("\tProtected\n");

        }

        /*
         find the first matching index for g_GraphicsQueueFamilyIndex and g_PresentQueueFamilyIndex
         if there is none, find first for g_GraphicsQueueFamilyIndex and first for g_PresentQueueFamilyIndex
        */

        for (uint32_t i = 0; i<queueFamilyCount; ++i)
        {
            VkBool32 presentationSupported;

            VkResult result = pfn_vkGetPhysicalDeviceSurfaceSupportKHR( g_SelectedPhysicalDevice, i, g_Surface, &presentationSupported);

            if (result != VK_SUCCESS)
            {
                free(familyProperties);
                printErrorMsg("vkGetPhysicalDeviceSurfaceSupportKHR().\n");
                return false;
            }

            if (familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentationSupported)
            {
                g_GraphicsQueueFamilyIndex = i;
                g_PresentQueueFamilyIndex = i;
                break;
            }

            if( g_GraphicsQueueFamilyIndex == -1 )
            {
                if( familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) g_GraphicsQueueFamilyIndex = i;
            }

            if( g_PresentQueueFamilyIndex == -1 )
            {
                if( presentationSupported ) g_PresentQueueFamilyIndex = i;
            }
        }

        free(familyProperties);
    }

    if (g_GraphicsQueueFamilyIndex == -1)
    {
        printErrorMsg("no Queue Family with VkQueue Graphics Transfer Bit.\n");
        return false;
    }

    if (g_PresentQueueFamilyIndex == -1)
    {
        printErrorMsg("no Queue Family with Present Queue.\n");
        return false;
    }

    printInfoMsg("Graphics Bit on Queue Family [%d]\n", g_GraphicsQueueFamilyIndex);
    printInfoMsg("Present Queue on Queue Family [%d]\n", g_PresentQueueFamilyIndex);

    //create logical device
    {
        int32_t queueInfoCount;

        if (g_GraphicsQueueFamilyIndex != g_PresentQueueFamilyIndex) queueInfoCount = 2;
        else queueInfoCount = 1;

        VkDeviceQueueCreateInfo queueCreateInfo[queueInfoCount];

        memset( queueCreateInfo, 0, sizeof queueCreateInfo);

        float queuePriorities[queueInfoCount];

        queuePriorities[0] = 1.0f;

        queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[0].pNext = NULL;
        queueCreateInfo[0].queueFamilyIndex = g_GraphicsQueueFamilyIndex;
        queueCreateInfo[0].queueCount = 1;
        queueCreateInfo[0].pQueuePriorities = queuePriorities;

        if (queueInfoCount==2)
        {
            queuePriorities[1] = 1.0f;

            queueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo[1].pNext = NULL;
            queueCreateInfo[1].queueFamilyIndex = g_PresentQueueFamilyIndex;
            queueCreateInfo[1].queueCount = 1;
            queueCreateInfo[1].pQueuePriorities = queuePriorities;
        }

        VkDeviceCreateInfo deviceCreateInfo = {0};

        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = NULL;
        deviceCreateInfo.queueCreateInfoCount = queueInfoCount;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
        deviceCreateInfo.enabledLayerCount = g_DeviceLayersArrayCount;
        if (g_DeviceExtArrayCount) deviceCreateInfo.ppEnabledLayerNames =
            (const char* const*) g_DeviceLayersArray;
        else deviceCreateInfo.ppEnabledLayerNames = NULL;
        deviceCreateInfo.enabledExtensionCount = g_DeviceExtArrayCount;
        if (g_DeviceExtArrayCount) deviceCreateInfo.ppEnabledExtensionNames =
            (const char* const*) g_DeviceExtArray;
        else deviceCreateInfo.ppEnabledExtensionNames = NULL;
        deviceCreateInfo.pEnabledFeatures = NULL;

        VkResult result = pfn_vkCreateDevice( g_SelectedPhysicalDevice,
            &deviceCreateInfo, NULL, &g_LogicalDevice);

        if (result != VK_SUCCESS)
        {
            printErrorMsg("failed to create logical device.\n");
            return false;
        }
    }

    printInfoMsg("Create Logical Device. OK.\n");

    //get device level fnc address
    GET_DEVICE_LEVEL_FUN_ADDR(vkDestroyDevice);
    GET_DEVICE_LEVEL_FUN_ADDR(vkGetDeviceQueue);
    GET_DEVICE_LEVEL_FUN_ADDR(vkCreateSemaphore);
    GET_DEVICE_LEVEL_FUN_ADDR(vkDestroySemaphore);
    GET_DEVICE_LEVEL_FUN_ADDR(vkCreateFence);
    GET_DEVICE_LEVEL_FUN_ADDR(vkResetFences);
    GET_DEVICE_LEVEL_FUN_ADDR(vkWaitForFences);
    GET_DEVICE_LEVEL_FUN_ADDR(vkDestroyFence);

    //get device queues
    pfn_vkGetDeviceQueue(g_LogicalDevice, g_GraphicsQueueFamilyIndex, 0, &g_GraphicsQueue);

    if (g_GraphicsQueueFamilyIndex != g_PresentQueueFamilyIndex)
    {
        pfn_vkGetDeviceQueue(g_LogicalDevice, g_PresentQueueFamilyIndex, 0, &g_PresentQueue);
    }
    else
    {
        g_PresentQueue = g_GraphicsQueue;
    }

    //create semaphores
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {0};

        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for(uint32_t i = 0; i < SWAP_CHAIN_IMAGE_COUNT; ++i)
        {
            VkResult result = pfn_vkCreateSemaphore (g_LogicalDevice, &semaphoreCreateInfo, NULL, &g_semaphoreImageAvailableArr[i]);

            if (result != VK_SUCCESS)
            {
                printErrorMsg("cannot create semaphore (image available) [%d].\n", i);
                return false;
            }

            result = pfn_vkCreateSemaphore (g_LogicalDevice, &semaphoreCreateInfo, NULL, &g_semaphoreRenderFinishedArr[i]);

            if (result != VK_SUCCESS)
            {
                printErrorMsg("cannot create semaphore (render finished) [%d].\n", i);
                return false;
            }
        }
    }

    printInfoMsg("create semaphores: OK.\n");

    //create fences
    {
        for ( int32_t i = 0; i < SWAP_CHAIN_IMAGE_COUNT; ++i )
        {
            VkFenceCreateInfo fenceCreateInfo = {0};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            VkResult result = pfn_vkCreateFence( g_LogicalDevice, &fenceCreateInfo, NULL, &fenceArr[i]);

            if (result != VK_SUCCESS)
            {
                printErrorMsg("cannot create fence. [%d].\n", i);
                return false;
            }
    }
    }

    printInfoMsg("create fences: OK.\n");

    //get surface capabilities

    VkSurfaceCapabilitiesKHR surfaceCapabilities = {0};

    {
        VkResult result = pfn_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_SelectedPhysicalDevice, g_Surface, &surfaceCapabilities );

        if (result != VK_SUCCESS)
        {
            printErrorMsg("vkGetPhysicalDeviceSurfaceCapabilitiesKHR().\n");
            return false;
        }

        printInfoMsg("surfaceCapabilities.minImageCount %d\n",surfaceCapabilities.minImageCount);
        printInfoMsg("surfaceCapabilities.maxImageCount %d\n",surfaceCapabilities.maxImageCount);
        printInfoMsg("surfaceCapabilities.currentExtent.width %d\n",surfaceCapabilities.currentExtent.width);
        printInfoMsg("surfaceCapabilities.currentExtent.height %d\n",surfaceCapabilities.currentExtent.height);
        printInfoMsg("surfaceCapabilities.minImageExtent.width %d\n",surfaceCapabilities.minImageExtent.width);
        printInfoMsg("surfaceCapabilities.minImageExtent.height %d\n",surfaceCapabilities.minImageExtent.height);
        printInfoMsg("surfaceCapabilities.maxImageExtent.width %d\n",surfaceCapabilities.maxImageExtent.width);
        printInfoMsg("surfaceCapabilities.maxImageExtent.height %d\n",surfaceCapabilities.maxImageExtent.height);
        printInfoMsg("surfaceCapabilities.maxImageArrayLayers %d\n",surfaceCapabilities.maxImageArrayLayers);
        printInfoMsg("surfaceCapabilities.supportedTransforms %08x\n",surfaceCapabilities.supportedTransforms);
        printInfoMsg("surfaceCapabilities.currentTransform %08x\n",surfaceCapabilities.currentTransform);
        printInfoMsg("surfaceCapabilities.supportedCompositeAlpha %08x\n",surfaceCapabilities.supportedCompositeAlpha);
        printInfoMsg("surfaceCapabilities.supportedUsageFlags %08x\n",surfaceCapabilities.supportedUsageFlags);
    }

    return true;
}

/*
==============================
 updateData();
==============================
*/

void updateData()
{

}

/*
==============================
 renderVulkan();
==============================
*/

void renderVulkan()
{

}

/*
===================
 main();
===================
*/

int main(int argc, char **argv)
{

    void *libHandle = NULL;
    char *envVar;
    xcb_connection_t *connection = NULL;
    int screenNum = 0;
    xcb_screen_iterator_t screenIter;
    xcb_screen_t *screen = NULL;
    xcb_window_t window = 0;
    uint32_t mask;
    uint32_t values[2];
    xcb_generic_error_t *error;
    xcb_void_cookie_t cookieWindow;
    xcb_void_cookie_t cookieMap;
    xcb_intern_atom_reply_t *atomReply = NULL;
    char *title = "Linux Vulkan XCB C Sandbox";
    xcb_generic_event_t *event;
    xcb_key_press_event_t *keyPressEvent;

    if(!parseOptions(argc, argv))
    {
        return -1;
    }

    printInfoMsg("Starting a program.\n");

    libHandle = openLibrary("libvulkan.so");

    if (!libHandle)
    {
        return -1;
    }

    printInfoMsg("shared library libvulkan.so openned OK.\n");

    if (!getFncAddress(libHandle))
    {
        printErrorMsg("failed to load functions pointers.\n");

        if (closeLibrary(libHandle))
        {
            printErrorMsg("close libvulkan.so.\n");
        }

        return -1;
    }

    envVar = getenv("VK_LAYER_PATH");

    if (envVar == NULL)
    {
        printWarningMsg("environment variable VK_LAYER_PATH is not set.\n");
    }
    else
    {
        printInfoMsg("VK_LAYER_PATH: %s\n",envVar);
    }

    connection = xcb_connect(NULL, &screenNum);

    if (connection == NULL)
    {
        printErrorMsg("can't connect to an X server.\n");

        if (closeLibrary(libHandle))
        {
            printErrorMsg("close libvulkan.so.\n");
        }

        return 1;
    }

    printInfoMsg("connect to an X server OK.\n");

    screenIter = xcb_setup_roots_iterator(xcb_get_setup(connection));

    while (screenNum-- > 0)
    {
        xcb_screen_next( &screenIter );
    }

    screen = screenIter.data;

    if (!screen)
    {
        printErrorMsg("can't get the current screen.\n");

        xcb_disconnect(connection);

        if (closeLibrary(libHandle))
        {
            printErrorMsg("close libvulkan.so.\n");
        }

        return 1;
    }

    printInfoMsg("get current screen OK.\n");

    window = xcb_generate_id(connection);

    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    values[0] = screen->black_pixel;

    values[1] = XCB_EVENT_MASK_EXPOSURE |
                    XCB_EVENT_MASK_RESIZE_REDIRECT |
                    XCB_EVENT_MASK_KEY_PRESS |
                    XCB_EVENT_MASK_KEY_RELEASE |
                    XCB_EVENT_MASK_POINTER_MOTION |
                    XCB_EVENT_MASK_BUTTON_PRESS |
                    XCB_EVENT_MASK_BUTTON_RELEASE;

    cookieWindow = xcb_create_window_checked(connection,
                                XCB_COPY_FROM_PARENT,
                                window,
                                screen->root,
                                0, 0, g_Width, g_Height,
                                0,
                                XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                screen->root_visual,
                                mask,
                                values);

    error = xcb_request_check (connection, cookieWindow);

    if (error)
    {
        printErrorMsg("can't create window : %d\n", error->error_code);

        xcb_disconnect(connection);

        if (closeLibrary(libHandle))
        {
            printErrorMsg("close libvulkan.so.\n");
        }

        return 1;
    }

    printInfoMsg("create window OK.\n");

    /* Magic code that will send notification when window is destroyed */
    xcb_intern_atom_cookie_t cookie1 = xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie1, 0);
    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(connection, 0, 16,"WM_DELETE_WINDOW");
    atomReply = xcb_intern_atom_reply(connection, cookie2, 0);
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, (*reply).atom, 4, 32, 1, &(*atomReply).atom);
    free(reply);

    /* set title of the window */
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen (title), title);

    cookieMap = xcb_map_window_checked(connection, window);

    error = xcb_request_check(connection, cookieMap);

    if (error)
    {
        printErrorMsg("can't map window : %d\n", error->error_code);

        free(atomReply);

        xcb_destroy_window(connection, window);

        xcb_disconnect(connection);

        if (closeLibrary(libHandle))
        {
            printErrorMsg("close libvulkan.so.\n");
        }

        return 1;
    }

    printInfoMsg("map window OK.\n");

    xcb_flush (connection);

    if (!initVulkan(window,connection))
    {
        printErrorMsg("initVulkan().\n");

        shutdownVulkan();

        free(atomReply);

        xcb_destroy_window(connection, window);

        xcb_disconnect(connection);

        if (closeLibrary(libHandle))
        {
            printErrorMsg("close libvulkan.so.\n");
        }

        return 1;
    }

    g_Ready = true;

    printInfoMsg("Ready !\n");

    while (!g_Quit)
    {

        event = xcb_poll_for_event(connection);
        xcb_resize_request_event_t *resizeRequestEvent;
        xcb_button_press_event_t *buttonPressEvent;
        xcb_motion_notify_event_t *motionNotify;

        if (event)
        {
            switch (event->response_type & ~0x80)
            {

                case XCB_EXPOSE:

                    xcb_flush(connection);

                    break;

                case XCB_CLIENT_MESSAGE:

                    if ((*(xcb_client_message_event_t*)event).data.data32[0] == (*atomReply).atom)
                    {
                        g_Quit = true;
                    }

                    break;

                case XCB_RESIZE_REQUEST:

                    resizeRequestEvent = (xcb_resize_request_event_t*) event;

                    if (resizeRequestEvent->width != g_Width || resizeRequestEvent->height != g_Height)
                    {
                        g_Ready = false;

                        printInfoMsg("resize is not yet supported !\n");
                    }

                    break;

                case XCB_KEY_PRESS:

                    keyPressEvent = (xcb_key_press_event_t*)event;

                    switch (keyPressEvent->detail)
                    {
                        case 0x9:	//Esc

                            g_Quit = true;

                            break;

                        case 0x18:	//Q

                            if (keyPressEvent->state & XCB_MOD_MASK_CONTROL) g_Quit = true;	//Ctrl+Q

                            break;

                        default:
                            break;
                    }

                    break;


                case XCB_MOTION_NOTIFY:

                    motionNotify = (xcb_motion_notify_event_t*)event;

                    g_MousePosX = motionNotify->event_x;
                    g_MousePosY = motionNotify->event_y;

                    break;

                case XCB_BUTTON_PRESS:

                    buttonPressEvent = (xcb_button_press_event_t*)event;

                    switch (buttonPressEvent->detail)
                    {
                        case XCB_BUTTON_INDEX_1:

                            g_MouseButton1 = true;

                            g_MousePosX = buttonPressEvent->event_x;
                            g_MousePosY = buttonPressEvent->event_y;

                            break;

                        case XCB_BUTTON_INDEX_2:

                            g_MouseButton2 = true;

                            break;

                        case XCB_BUTTON_INDEX_3:

                            g_MouseButton3 = true;

                            break;

                        default:
                            break;

                    }

                    break;

                case XCB_BUTTON_RELEASE:

                    buttonPressEvent = (xcb_button_press_event_t*)event;

                    switch (buttonPressEvent->detail)
                    {
                        case XCB_BUTTON_INDEX_1:

                            g_MouseButton1 = false;

                            break;

                        case XCB_BUTTON_INDEX_2:

                            g_MouseButton2 = false;

                            break;

                        case XCB_BUTTON_INDEX_3:

                            g_MouseButton3 = false;

                            break;

                        default:
                            break;

                    }

                    break;

                default:
                    break;

            }

            free (event);

        }

        if (g_Ready)
        {
            updateData();
            renderVulkan();
        }

    }

    shutdownVulkan();

    free(atomReply);
    xcb_destroy_window(connection, window);
    xcb_disconnect(connection);

    if (closeLibrary(libHandle))
    {
        printErrorMsg("close libvulkan.so.\n");
        return -1;
    }

    printInfoMsg("Bye bye !\n");

    return 0;
}
