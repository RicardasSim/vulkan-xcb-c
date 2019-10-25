/*
 * Learning Vulkan API
 * Vulkan, XCB, C (C99)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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
===================
 main();
===================
*/

int main(int argc, char **argv)
{


    return 0;
}
