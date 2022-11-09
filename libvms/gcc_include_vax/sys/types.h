/* <sys/types.h>
 *
 *	This is the UNIX-compatible sys/types definition.
 *	Fixed to support X Mosaic GNU C builds - GEC
 */
#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ifndef _TYPES_
#define _TYPES_		/* old indicator */
#define __TYPES__

#ifndef _TIME_T
#include <types.h>	/* this gets us time_t */
#endif
/*
 * Basic system types
 */
#ifndef __SOCKET_TYPEDEFS
typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned long	u_long;
#define __SOCKET_TYPEDEFS
#endif /* Avoid conflict with socket.h, GEC */
typedef unsigned int	u_int;

#if !defined(_USHORT_T) && !defined(_USHORT_T_)
typedef unsigned short	ushort;		/* sys III compat */
#endif
#define _USHORT_T
#define _USHORT_T_

#ifndef _QUAD_T
#define _QUAD_T
typedef struct	_quad { long val[2]; } quad;
#endif
typedef long	daddr_t;
#ifndef caddr_t
#if !defined(_CADDR_T) && !defined(_CADDR_T_) && !defined(CADDR_T) && !defined(__CADDR_T)
typedef char   *caddr_t;
#endif
#define _CADDR_T
#define _CADDR_T_
#define CADDR_T
#define __CADDR_T  /* Motif 1.2 needs, GEC */
#endif

#if !defined(_INO_T) && !defined(_INO_T_)
#ifdef FIXED_INO_T
 /* use a definition that makes sense for VMS ODS-2; <stat.h> must cooperate */
typedef struct { unsigned : 16, : 16, : 16; } ino_t;
#else
 /* use a definition that's compatible with VAXC's broken one; array[3] used */
typedef u_short ino_t;
#endif
#endif
#define _INO_T
#define _INO_T_
typedef long	swblk_t;

#if !defined(_SIZE_T) && !defined(_SIZE_T_)
typedef unsigned int size_t;
#endif
#define _SIZE_T
#define _SIZE_T_
#if !defined(_DEV_T) && !defined(_DEV_T_)
typedef char	*dev_t;
#endif
#define _DEV_T
#define _DEV_T_
#if !defined(_OFF_T) && !defined(_OFF_T_)
typedef long off_t;
#endif
#define _OFF_T
#define _OFF_T_

/*
	uid_t and gid_t ought to be unsigned short, because they
	represent 16 bit quantities, but getuid() and getgid() are
	declared to returned unsigned int.
 */

/* user ID -- actually, member portion of UIC */
#if !defined(_UID_T) && !defined(_UID_T_)
typedef u_int uid_t;
#endif
#define _UID_T
#define _UID_T_
/* group ID -- group portion of UIC */
#if !defined(_GID_T) && !defined(_GID_T_)
typedef u_int gid_t;
#endif
#define _GID_T
#define _GID_T_
/* process ID */
#if !defined(_PID_T) && !defined(_PID_T_) && !defined(__PID_T)
typedef int pid_t;
#endif
#define _PID_T
#define _PID_T_
#define __PID_T

#define NBBY	8		/* number of bits in a byte */

#ifdef _Xt_Intrinsic_h		/* (see <X11/Intrinsic.h>) */
#define MAX(a,b) (((a)<(b))?(a):(b))
#define MIN(a,b) (((a)>(b))?(a):(b))
#endif

#endif	/*_TYPES_*/

#endif	/*_SYS_TYPES_H*/
