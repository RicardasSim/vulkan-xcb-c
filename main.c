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

int g_Width = INIT_WIDTH;
int g_Height = INIT_HEIGHT;

PFN_vkGetInstanceProcAddr pfn_vkGetInstanceProcAddr = NULL;

PFN_vkGetDeviceProcAddr pfn_vkGetDeviceProcAddr = NULL;

bool g_Quit = false;
bool g_Ready = false;

int g_MousePosX = 0;
int g_MousePosY = 0;
int g_MousePosOldX = 0;
int g_MousePosOldY = 0;
bool g_MouseButton1 = false;
bool g_MouseButton2 = false;
bool g_MouseButton3 = false;

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

    g_Ready = true;

    printInfoMsg("Ready !\n");

  	while (!g_Quit)
	{

        event = xcb_poll_for_event(connection);
        xcb_resize_request_event_t* resizeRequestEvent;
        xcb_button_press_event_t* buttonPressEvent;
        xcb_motion_notify_event_t* motionNotify;

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
    }

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
