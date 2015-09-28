//
//  debug.h
//  foohid
//
//  Created by Adriano Di Luzio on 28/09/15.
//  Copyright © 2015 unbit. All rights reserved.
//

#ifndef debug_h
#define debug_h

#include <IOKit/IOLib.h>

#ifdef DEBUG
#   define LogD(fmt, ...) IOLog((fmt), ##__VA_ARGS__)
#else
#   define LogD(...)
#endif

#endif /* debug_h */
