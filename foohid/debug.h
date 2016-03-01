#ifndef debug_h
#define debug_h

#include <IOKit/IOLib.h>

#define TAG "foohid: "

#ifdef DEBUG
    #define LogD(fmt, ...) IOLog((TAG fmt "\n"), ##__VA_ARGS__)
#else
    #define LogD(...)
#endif

#endif /* debug_h */
