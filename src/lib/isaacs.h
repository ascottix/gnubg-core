/*
 * -----------------------------------------------------------------------------
 * standard.hi: Standard definitions and types, Bob Jenkins
 * -----------------------------------------------------------------------------
 */

/*
 * Minor modifications for use with GNU Backgammon.
 * Copyright (C) 1999-2013 the AUTHORS
 */

#ifndef ISAACS_H
#define ISAACS_H

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
typedef unsigned int uint32_t;
#endif

typedef uint32_t ub4;           /* unsigned 4-byte quantities */
#define UB4MAXVAL 0xffffffff

typedef int word;               /* fastest type available */

#endif
