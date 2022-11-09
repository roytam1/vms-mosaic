/*                System dependencies in the W3 library
                                   SYSTEM DEPENDENCIES
                                             
   System-system differences for TCP include files and macros. This
   file includes for each system the files necessary for network and
   file I/O.
   
  AUTHORS
  
  TBL                Tim Berners-Lee, W3 project, CERN, <timbl@info.cern.ch>
  EvA                     Eelco van Asperen <evas@cs.few.eur.nl>
  MA                      Marc Andreessen NCSA
  AT                      Aleksandar Totic <atotic@ncsa.uiuc.edu>
  SCW                     Susan C. Weber <sweber@kyle.eitech.com>
                         
  HISTORY:
  22 Feb 91               Written (TBL) as part of the WWW library.
  16 Jan 92               PC code from EvA
  22 Apr 93               Merged diffs bits from xmosaic release
  29 Apr 93               Windows/NT code from SCW

  Much of the cross-system portability stuff has been intentionally
  REMOVED from this version of the library by Marc A in order to
  discourage attempts to make "easy" ports of Mosaic for X to non-Unix
  platforms.  The library needs to be rewritten from the ground up; in
  the meantime, Unix is *all* we support or intend to support with
  this set of source code.

*/

#ifndef TCP_H
#define TCP_H

/*

Default values

   These values may be reset and altered by system-specific sections
   later on.  There are also a bunch of defaults at the end.
   
 */
/* Default values of those: */
#define NETCLOSE close      /* Routine to close a TCP-IP socket         */
#define NETREAD  HTDoRead   /* Routine to read from a TCP-IP socket     */
#define NETWRITE write      /* Routine to write to a TCP-IP socket      */

/* Unless stated otherwise, */
#define SELECT                  /* Can handle >1 channel.               */
#define GOT_SYSTEM              /* Can call shell with string           */

#ifdef unix
#define GOT_PIPE
#endif

typedef struct sockaddr_in SockA;  /* See netinet/in.h */

#ifndef STDIO_H
#include <stdio.h>
#define STDIO_H
#endif

#ifdef _AIX
#define AIX
#endif
#ifdef AIX
#define unix
#endif

#ifdef _IBMR2
#define USE_DIRENT              /* sys V style directory open */
#endif

/* Solaris. */
#if defined(sun) && defined(__svr4__)
#define USE_DIRENT              /* sys V style directory open */
#endif

#if defined(__alpha)
#define USE_DIRENT
#endif

#ifndef USE_DIRENT
#ifdef SVR4
#define USE_DIRENT
#endif
#endif

#ifndef __STRING_LOADED
#include <string.h>
#endif

/* Use builtin strdup when appropriate. */
#if defined(ultrix) || defined(VMS) || defined(NeXT)
extern char *strdup();
#endif

/*

VAX/VMS

   Under VMS, there are many versions of TCP-IP.  Define one if you do
   not use Digital's UCX product:
   
  UCX                     DEC's "Ultrix connection" (default)
  WIN_TCP                 From Wollongong originally
  MULTINET                From SRI, now from Process Software
  DECNET                  Cern's TCP socket emulation over DECnet
  SOCKETSHR               E. Meyer's socket emulation for NETLIB
                           
   The last three do not interfere with the
   unix i/o library, and so they need special calls to read, write and
   close sockets.  In these cases the socket number is a VMS channel
   number, so we make the @@@ HORRIBLE @@@ assumption that a channel
   number will be greater than 10 but a unix file descriptor less than
   10.  It works.
   
 */
#ifdef vms
#ifdef WIN_TCP
#undef NETREAD
#undef NETWRITE
#undef NETCLOSE
#define NETREAD(s,b,l)  ((s)>10 ? netread((s),(b),(l)) : read((s),(b),(l)))
#define NETWRITE(s,b,l) ((s)>10 ? netwrite((s),(b),(l)) : write((s),(b),(l)))
#define NETCLOSE(s)     ((s)>10 ? netclose(s) : close(s))
#endif /* WIN_TCP */

#ifdef MULTINET
#undef NETCLOSE
#undef NETREAD
#undef NETWRITE
#define NETREAD(s,b,l)  ((s)>10 ? HTDoRead((s),(b),(l)) : read((s),(b),(l)))
#define NETWRITE(s,b,l) ((s)>10 ? socket_write((s),(b),(l)) : \
				write((s),(b),(l)))
#define NETCLOSE(s)     ((s)>10 ? socket_close(s) : close(s))
#endif /* MULTINET */

/*      Certainly this works for UCX and Multinet; not tried for Wollongong
*/
#ifdef MULTINET
#if defined(__DECC) && !defined(__alpha)
#define __TYPES_LOADED 1
#endif /* DECC, fool MultiNet types.h, GEC */ 
#ifdef __DECC
#define _POSIX_C_SOURCE
#endif /* DEC C */
#include "multinet_root:[multinet.include.sys]types.h"
#ifdef __DECC
#undef _POSIX_C_SOURCE
#undef _ANSI_C_SOURCE  /* Gets defined because of _POSIX_C_SOURCE */
#endif /* DEC C */
#include "multinet_root:[multinet.include]errno.h"
#pragma nostandard
#if defined(__DECC) && !defined(__alpha)
#undef __TYPES_LOADED /* MultiNet 3.3D and earlier types.h is broken, GEC */
/* Due to bugs in MultiNet V3.3's ERRNO.H for DEC C on VAX, redefine some
 * things here.
 */
  int *cma$tis_errno_get_addr(void);    /* UNIX style error code */
  int *cma$tis_vmserrno_get_addr(void); /* VMS error code when errno == EVMSE */
#define errno        (*cma$tis_errno_get_addr())
#define vaxc$errno   (*cma$tis_vmserrno_get_addr())
#endif
#pragma standard

#ifdef __TIME_LOADED
/* To avoid multiple defs. VAXC has __TIME_LOADED, Multinet __TIME BSN */
#define __TIME
#else
#if defined(__DECC) && (__DECC_VER > 50230003) && defined (__FD_SET) && !defined(_XOPEN_SOURCE_EXTENDED) && !defined(_TYPES_)
/* If _TYPES_ defined, then is pre-V3.5 MultiNet, GEC */
/* If __FD_SET defined, then ECO DECC052H installed or post V3.5B */ 
#define _XOPEN_SOURCE_EXTENDED
#include <time.h>
#undef _XOPEN_SOURCE_EXTENDED
#undef _XOPEN_SOURCE  /* Gets defined because of _XOPEN_SOURCE_EXTENDED */
#endif /* As if MultiNet V3.5A&B weren't bad enough, then came DECC052H, GEC */
#endif 
#if defined(_TYPES_) && defined (__TIME_T)
#undef __TIME_T
#endif /* For pre-V3.5 MultiNet time.h, GEC */
#define _ANSI_C_SOURCE /* DEC C time.h conflicts with MultiNet V3.5 types.h */
#include "multinet_root:[multinet.include.sys]time.h"
#undef _ANSI_C_SOURCE
#ifdef __TIME_T
#undef __TIME_T
#endif
#define __TIME_T
#else
#ifdef WIN_TCP
#define __STAT
#endif
#ifdef SOCKETSHR
#include "socketshr_files:types.h"
#if defined(__DECC) && !defined(__alpha)
/* The SOCKETSHR types.h does not define enough for DEC C on VAX and
 * there was no way to include both the SOCKETSHR and DEC C types.h, so
 * the following were taken from DEC C's types.h.  Then DEC C V5.2 comes
 * along and makes a total hash of everything.  GEC
 */
#ifndef __DEV_T
# define __DEV_T 1
  typedef char *dev_t;
# define __PID_T 1
  typedef int pid_t;
#if (__DECC_VER <= 50230003)
  typedef unsigned off_t;
  typedef unsigned short ino_t;
  typedef int nlink_t;
  typedef unsigned short mode_t;
#endif
#endif
#endif
#else
#include <types.h>
#endif
#ifdef WIN_TCP
#undef __STAT
#endif
#include <errno.h>
#if defined(SOCKETSHR) && defined(__DECC)
#define __FD_SET 1
#endif /* DEC C V5.2 time.h conflicts with SOCKETSHR types.h, GEC */
#include <time.h>
#endif /* Multinet */

#ifndef __STRING_LOADED
#include <string.h>
#endif

#ifdef SOCKETSHR
#include "socketshr_files:file.h"
#else
#include <file.h>
#endif

#ifndef VMS
#include unixio
#else
#if defined(SOCKETSHR) && defined(__DECC) && (__DECC_VER > 50230003)
#define __UID_T 1
#define __GID_T 1
#endif
#include <unixio.h>
#endif

#define INCLUDES_DONE

#ifdef MULTINET          /* Include from standard Multinet directories */
#include "multinet_root:[multinet.include.sys]socket.h"
#ifdef __TIME_LOADED     /* Defined by sys$library:time.h */
#define __TIME           /* To avoid double definitions in next file */
#endif
#include "multinet_root:[multinet.include.netinet]in.h"
#include "multinet_root:[multinet.include.arpa]inet.h"
#ifdef __DECC
#define _POSIX_C_SOURCE
#endif /* DEC C */
#include "multinet_root:[multinet.include]netdb.h"
#ifdef __DECC
#undef _POSIX_C_SOURCE
#undef _ANSI_C_SOURCE  /* Gets defined because of _POSIX_C_SOURCE */
#endif /* DEC C */
#include "multinet_root:[multinet.include.sys]ioctl.h"

#ifdef __DECC
#define read  decc$read
#define write decc$write
#define close decc$close
#define sleep decc$sleep
#if defined(__VMS_VER) && (__VMS_VER >= 70000000)
#define strdup  decc$strdup
#endif /* VMS V7, VRH */
#endif /* DEC C, BSN */

#else  /* Not Multinet */

#ifdef DECNET
#include "types.h"  /* for socket.h */
#include "socket.h"
#include "dn"
#include "dnetdb"

#else /* UCX or WIN */

#if defined(CADDR_T) && !defined(__CADDR_T)
#define __CADDR_T
#endif /* BSN, include file problem socket.h <-> xlib.h */
#include <socket.h>
#include <in.h>
#include <inet.h>
#include <netdb.h>

#if defined(WIN_TCP)
#include <sys/ioctl.h>
#endif  /* WIN_TCP */

#ifdef SOCKETSHR
#include "socketshr_files:ioctl.h"
#include "socketshr_files:socketshr.h"
#if defined(__DECC) && !defined(__alpha)
#undef alarm
#undef signal
#undef fread
#undef fwrite
#undef fflush
#undef fdopen
#undef fclose
#undef fputs
#undef fputc
#undef fgets
#undef fgetc
#undef fprintf
#endif /* These Socketshr routines fail with DEC C on VAX, GEC */
#endif

#endif  /* Not DECNET */
#endif  /* Of Multinet or other TCP includes */

#define TCP_INCLUDES_DONE

#endif  /* vms */


/*
SCO ODT unix version
 */

#ifdef sco
#include <sys/fcntl.h>
#define USE_DIRENT
#endif

/*
MIPS unix
 */
/* Mips hack (bsd4.3/sysV mixture...) */

#ifdef mips
extern int errno;
#endif


/*
Regular BSD unix versions
   These are a default unix where not already defined specifically.
 */
#ifndef INCLUDES_DONE
#include <sys/types.h>
#include <string.h>

#include <errno.h>          /* independent */
#include <sys/time.h>       /* independent */
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/file.h>       /* For open() etc */
#define INCLUDES_DONE
#endif  /* Normal includes */

/*                      Directory reading stuff - BSD or SYS V
*/
#ifdef unix                   /* if this is to compile on a UNIX machine */
#define GOT_READ_DIR 1        /* if directory reading functions are available */
#ifdef USE_DIRENT             /* sys v version */
#include <dirent.h>
#define direct dirent
#else
#include <sys/dir.h>
#endif
#if defined(sun) && defined(__svr4__)
#include <sys/fcntl.h>
#include <limits.h>
#endif
#endif

/*

Defaults

  INCLUDE FILES FOR TCP
  
 */
#ifndef TCP_INCLUDES_DONE
#include <sys/ioctl.h> /* EJB */
#include <sys/socket.h>
#include <netinet/in.h>
#ifndef __hpux		    /* This may or may not be good -marc */
#include <arpa/inet.h>      /* Must be after netinet/in.h */
#endif
#include <netdb.h>
#endif  /* TCP includes */


/*

  MACROS FOR MANIPULATING MASKS FOR SELECT()
  
 */
#ifdef SELECT
#ifndef FD_SET
typedef unsigned int fd_set;
#define FD_SET(fd, pmask)   (*(pmask)) |=  (1 << (fd))
#define FD_CLR(fd, pmask)   (*(pmask)) &= ~(1 << (fd))
#define FD_ZERO(pmask)      (*(pmask)) = 0
#define FD_ISSET(fd, pmask) (*(pmask) & (1 << (fd)))
#endif
#endif

#ifdef VMS
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64		/* Arbitrary limit */
#endif
#endif /* VMS, BSN */

#endif /* TCP_H */
