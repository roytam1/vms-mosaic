/* WIDE AREA INFORMATION SERVER SOFTWARE:
   No guarentees or restrictions.  See the readme file for the full standard
   disclaimer.

   5.29.90	Harry Morris, morris@think.com
*/

/* Copyright (c) CNIDR (see ../COPYRIGHT) */


/* there are three kinds of C that we have encountered so far:

	 	1. ANSI_LIKE 	- full prototypes and stdargs and ansi includes
	 	2. PROTO_ANSI 	- full prototypes, varargs and misc includes
	 	3. K_AND_R	- k&r prototypes, varargs and misc includes
	 	
  This file is a mess because some compilers do not handle elif.  sorry.

  defined symbols for other machines:
    hp:         hpux
    Next:       NeXT 
*/

#ifndef C_DIALECT
#define C_DIALECT

/*----------------------------------------------------------------------*/

#ifdef STDC
#define ANSI_LIKE
#endif /* def STDC */

#ifdef __STDC__
#ifndef M_XENIX /* for some reason XENIX defines __STDC__ */

#ifndef __HIGHC__
#define ANSI_LIKE
#endif /* ndef __HIGHC__ */
#endif /* ndef M_XENIX */
#endif /* __STDC__ */

#ifdef A_UX
#define ANSI_LIKE
#endif /* A_UX */

#ifdef THINK_C
#define ANSI_LIKE
#endif /* THINK_C */

#ifdef MPW			
#define ANSI_LIKE
#endif /* MPW */

/* #ifdef VAX_C			/* vms */
#if defined(VAXC) || defined(__DECC) /* vms BSN, GEC */
#define ANSI_LIKE
#endif /* VAX_C */

#ifdef MSC			/* XXX not sure if this is correct */
#define ANSI_LIKE
#endif /* MSC */

#ifndef ANSI_LIKE

#ifdef SABER			/* c interpreter/debugger */
#define PROTO_ANSI
#endif /* SABER */

#ifdef cstar			/* parallel C on the CM */
#define PROTO_ANSI
#endif /* cstar */

#ifndef PROTO_ANSI

#ifdef SUN_C			/* XXX not sure if this is correct */
#define K_AND_R
#endif /* SUN_C */

#ifdef __GNUC__			/* gcc in traditional mode */
#define K_AND_R			/* gcc in ansi mode defines __STDC__ */
#endif /* __GNUC__ */

#ifdef M_XENIX
#define K_AND_R
#define NOTCPIP /* doesn't support TCP */
#define SYSV /* is based on system V */
#endif /* M_XENIX */

/* otherwise */
#define K_AND_R			/* default to the stone age */

#endif /* ndef PROTO_ANSI */
#endif /* ndef ANSI_LIKE */

/* set up the function prototyping convention */

#ifdef ANSI_LIKE
#define _AP(args) args
#endif /* def ANSI_LIKE */

#ifdef PROTO_ANSI
#define _AP(args) args
#endif /* def PROTO_ANSI */

#ifdef K_AND_R
#define _AP(args) ()
#endif /* def K_AND_R */

/* if you are not k_and_r, then load in ustubs always */
#ifdef K_AND_R
#include "ustubs.h"
#endif /* def K_AND_R */

/* Comment this back in to figure out what the compiler thinks we are */
/*
#ifdef ANSI_LIKE
WeareAnsiLike
#endif
#ifdef PROTO_ANSI
WeareProtoAnsi
#endif
#ifdef K_AND_R
WeareKandR
#endif
Crash-and-Burn
*/
/* End of chunk to comment back in */

#endif /* ndef C_DIALECT */

/*----------------------------------------------------------------------*/


