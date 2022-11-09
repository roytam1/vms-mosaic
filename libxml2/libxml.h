/*
 * libxml.h: internal header only used during the compilation of libxml
 *
 * See COPYRIGHT for the status of this software
 *
 * Author: breese@users.sourceforge.net
 */

#ifndef __XML_LIBXML_H__
#define __XML_LIBXML_H__

#if defined(WIN32) && !defined(__CYGWIN__)
#include "win32config.h"
#elif defined(macintosh)
#include "config-mac.h"
#else
#include "libxml2_config.h"
#include "xmlversion.h"
#endif

#ifndef WITH_TRIO
#include <stdio.h>
#else
/**
 * TRIO_REPLACE_STDIO:
 *
 * This macro is defined if teh trio string formatting functions are to
 * be used instead of the default stdio ones.
 */
#define TRIO_REPLACE_STDIO
#include "trio.h"
#endif

#endif /* ! __XML_LIBXML_H__ */
