#include <linux/printk.h>
#include <linux/string.h>
#include <linux/kernel.h>

#include "enums.h"

void print(const char * format, const char * flag, va_list args)
{
    char message[strlen(format) + 6 + strlen(MODULE_NAME)];
    snprintf(message, sizeof(message), "%s%s: %s\n", flag, MODULE_NAME, format);
    vprintk(message, args);
}

void print_info(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    print(format, KERN_INFO, args);
    va_end(args);
}

void print_alert(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    print(format, KERN_ALERT, args);
    va_end(args);
}

void print_bold(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    print(format, KERN_WARNING, args);
    va_end(args);
}