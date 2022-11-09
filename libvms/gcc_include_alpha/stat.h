#ifndef __STAT_LOADED
#define __STAT_LOADED	1

/*
 *  <stat.h> - stat/fstat UNIX emulation functions
 *
 *  Copyright (c) 1993 by Digital Equipment Corporation.  All rights reserved.
 *
 *  DEC C for OpenVMS VAX and OpenVMS AXP
 *  DEC C++ for OpenVMS VAX and OpenVMS AXP
 */

#if defined(__DECC) || defined(__DECCXX)
#pragma __nostandard  /* This file uses non-ANSI-Standard features */
#else
#pragma nostandard
#endif

#if defined(__DECC) || defined(__DECCXX)
#pragma __member_alignment __save
#pragma __nomember_alignment
#endif

#if defined(__DECC)
#pragma __message __save
#pragma __message disable (__MISALGNDSTRCT)
#pragma __message disable (__MISALGNDMEM)
#endif

#ifdef __cplusplus
    extern "C" {
#endif

#ifndef _OFF_T
#define _OFF_T
typedef unsigned off_t;
#endif
#ifndef _INO_T
#define _INO_T
typedef unsigned short ino_t;
#endif
#ifndef _DEV_T
#define _DEV_T
typedef char *dev_t;
#endif
struct	stat
{
	dev_t	st_dev;		/* pointer to physical device name */
	ino_t	st_ino[3];	/* 3 words to receive fid */
	unsigned short st_mode; /* file "mode" i.e. prot, dir, reg, etc. */
	int	st_nlink;	/* for compatibility - not really used */
	unsigned st_uid;	/* from ACP - QIO uic field */
	unsigned short st_gid;	/* group number extracted from st_uid */
	dev_t	st_rdev;	/* for compatibility - always zero */
	off_t	st_size;	/* file size in bytes */
	unsigned st_atime;	/* file access time; always same as st_mtime */
	unsigned st_mtime;	/* last modification time */
	unsigned st_ctime;	/* file creation time */
	char	st_fab_rfm;	/* record format */
	char	st_fab_rat;	/* record attributes */
	char	st_fab_fsz;	/* fixed header size */
	unsigned st_fab_mrs;	/* record size */
} __attribute__ ((packed));

typedef struct stat stat_t;

#define S_IFMT	 0170000	 /* type of file */
#define S_IFDIR	 0040000	 /* directory */
#define S_IFCHR	 0020000	 /* character special */
#define S_IFBLK	 0060000	 /* block special */
#define S_IFREG	 0100000	 /* regular */
#define S_IFMPC	 0030000	 /* multiplexed char special */
#define S_IFMPB	 0070000	 /* multiplexed block special */
#define S_ISUID	 0004000	 /* set user id on execution */
#define S_ISGID	 0002000	 /* set group id on execution */
#define S_ISVTX	 0001000	 /* save swapped text even after use */
#define S_IREAD	 0000400	 /* read permission, owner */
#define S_IWRITE 0000200	 /* write permission, owner */
#define S_IEXEC	 0000100	 /* execute/search permission, owner */

#define S_IRWXU		0000700		/* read,write,execute perm: owner */
#define S_IRUSR		0000400		/* read permission: owner */
#define S_IWUSR		0000200		/* write permission: owner */
#define S_IXUSR		0000100		/* execute/search permission: owner */
#define S_IRWXG		0000070		/* read,write,execute perm: group */
#define S_IRGRP		0000040		/* read permission: group */
#define S_IWGRP		0000020		/* write permission: group */
#define S_IXGRP		0000010		/* execute/search permission: group */
#define S_IRWXO		0000007		/* read,write,execute perm: other */
#define S_IROTH		0000004		/* read permission: other */
#define S_IWOTH		0000002		/* write permission: other */
#define S_IXOTH		0000001		/* execute/search permission: other */

/*
 *  Declare the function prototypes.
 *  NOTE:  stat and fstat are defined in UNIXIO.H.  Change both, or none....
 *  NOTE:  umask is defined in STDLIB.H.  Change both, or none....
*/

int stat(char *__file_spec, stat_t *__buffer);
int fstat(int __file_desc, stat_t *__buffer);
unsigned short umask(unsigned short __old_mask);

#ifdef __cplusplus
    }
#endif

#if defined(__DECC) || defined(__DECCXX)
#pragma __member_alignment __restore
#endif

#if defined(__DECC)
#pragma __message __restore
#endif

#if defined(__DECC) || defined(__DECCXX)
#pragma __standard  /* This file uses non-ANSI-Standard features */
#else
#pragma standard
#endif

#endif					/* __STAT_LOADED */
