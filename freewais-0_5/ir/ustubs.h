/* WIDE AREA INFORMATION SERVER SOFTWARE:
   No guarantees or restrictions.  See the readme file for the full standard
   disclaimer.	
  
   4.14.90	Harry Morris, morris@think.com
   7.24.90  	added include of sys/types
*/

/* Copyright (c) CNIDR (see ../COPYRIGHT) */


/*----------------------------------------------------------------------*/
/* definitions that non-ansi (aka sun) C doesn't provide */

#ifndef USTUBS_H
#define USTUBS_H

#include "cdialect.h"

#ifndef  ANSI_LIKE

#include <sys/types.h>

#ifdef M_XENIX
#include <string.h>
#endif /* ndef M_XENIX */

#ifndef VMS
#ifndef size_t
#ifndef M_XENIX
#define	size_t	unsigned long
#endif /* ndf M_XENIX */
#endif /* ndef size_t */
#endif /* VMS, BSN */

#ifndef ANSI_LIKE
#ifndef M_XENIX
#define time_t long
#endif /* ndef M_XENIX */
#endif /* ndef ANSI_LIKE */

/* #if defined(K_AND_R) && !defined(__STDC__) */  /* this might be too general, but it is needed on vaxen */
#if defined(K_AND_R)  /* this might be too general, but it is needed on vaxen */
#define void char
#endif /* ndef K_AND_R */

#ifdef __cplusplus
/* declare these as C style functions */
extern "C"
	{
#endif /* def __cplusplus */

char *strstr _AP((char *src, char *sub));

#ifdef SYSV
char *getwd _AP((char *pathname));
#ifndef __hpux
#define rename(f1,f2) {link((f1),(f2)); unlink((f1)); }
#endif
#endif /* defu SYSV */

#if !(defined(NeXT) || defined(Mach))
#ifndef M_XENIX
#ifndef cstar
#ifndef __osf__
char* malloc _AP((size_t size));
char* calloc _AP((size_t nelem,size_t elsize));
void free _AP((char* ptr));
char* realloc _AP((char* ptr,size_t size));
#endif /* ndef __osf__ */
#ifndef mips
#ifndef __AIX32
#ifndef _AIX32
#ifndef hpux
#ifndef vax
#ifndef __osf__
char* memcpy _AP((char* s1,char* s2,size_t c));
void* memmove _AP((void* s1,void* s2,size_t n));
#endif /* ndef __osf__ */
#endif /* ndef vax */
#endif /* ndef hpux */
#endif /* ndef _AIX32 */
#endif /* ndef __AIX32 */
#endif /* ndef mips */
char *strcat _AP((char *s1, char *s2));
#endif /* ndef cstar */
#endif /* ndef M_XENIX */
#endif /* not NeXT or Mach */

long atol _AP((char *s));

#ifdef __cplusplus
	}
#endif /* def __cplusplus */

#else /* def ANSI_LIKE */

#ifdef __GNUC__ /* we are ansi like, are we gcc? */

#ifdef __cplusplus
/* declare these as C style functions */
extern "C"
	{
#endif /* def __cplusplus */
/*void* memmove _AP((void* s1,void* s2,size_t n));*/
#ifdef __cplusplus
	}
#endif /* def __cplusplus */

#endif /* ifdef __GNUC__ */

#endif /* else ndef ANSI_LIKE */

/*----------------------------------------------------------------------*/

#endif /* ndef USTUBS_H */

