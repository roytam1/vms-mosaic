#ifndef __SOCKET_LOADED
#define __SOCKET_LOADED	1

/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	socket.h	6.6 (Berkeley) 6/8/85
 */

#ifndef __SOCKET_TYPEDEFS
#if !defined(__CADDR_T) && !defined(CADDR_T)
typedef char * caddr_t;
#define CADDR_T
#define __CADDR_T
#endif /* Do it only if needed and indicate we did it, GEC */

typedef unsigned short u_short;

typedef unsigned long u_long;

typedef unsigned char u_char;

#define __SOCKET_TYPEDEFS
#endif

/*
 * Definitions related to sockets: types, address families, options.
 */

/*
 * Types
 */
#define	SOCK_STREAM	1		/* stream socket */
#define	SOCK_DGRAM	2		/* datagram socket */
#define	SOCK_RAW	3		/* raw-protocol interface */
#define	SOCK_RDM	4		/* reliably-delivered message */
#define	SOCK_SEQPACKET	5		/* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x01		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x02		/* socket has had listen() */
#define	SO_REUSEADDR	0x04		/* allow local address reuse */
#define	SO_KEEPALIVE	0x08		/* keep connections alive */
#define	SO_DONTROUTE	0x10		/* just use interface addresses */
#define	SO_BROADCAST	0x20		/* permit sending of broadcast msgs */
#define	SO_USELOOPBACK	0x40		/* bypass hardware when possible */
#define	SO_LINGER	0x80		/* linger on close if data present */
#define SO_OOBINLINE	0x100		/* leave received OOB data in line */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF	0x1001		/* send buffer size */
#define SO_RCVBUF	0x1002		/* receive buffer size */
#define SO_SNDLOWAT	0x1003		/* send low-water mark */
#define SO_RCVLOWAT	0x1004		/* receive low-water mark */
#define SO_SNDTIMEO	0x1005		/* send timeout */
#define SO_RCVTIMEO	0x1006		/* receive timeout */
#define SO_ERROR	0x1007		/* get error status and clear */
#define SO_TYPE		0x1008		/* get socket type */

/*
 * Structure used for manipulating linger option.
 */
struct	linger {
	int	l_onoff;		/* option on/off */
	int	l_linger;		/* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define	SOL_SOCKET	0xffff		/* options for socket level */

/*
 * Address families.
 */
#define	AF_UNSPEC	0		/* unspecified */
#define	AF_UNIX		1		/* local to host (pipes, portals) */
#define	AF_INET		2		/* internetwork: UDP, TCP, etc. */
#define	AF_IMPLINK	3		/* arpanet imp addresses */
#define	AF_PUP		4		/* pup protocols: e.g. BSP */
#define	AF_CHAOS	5		/* mit CHAOS protocols */
#define	AF_NS		6		/* XEROX NS protocols */
#define	AF_NBS		7		/* nbs protocols */
#define	AF_ECMA		8		/* european computer manufacturers */
#define	AF_DATAKIT	9		/* datakit protocols */
#define	AF_CCITT	10		/* CCITT protocols, X.25 etc */
#define	AF_SNA		11		/* IBM SNA */
#define AF_DECnet	12		/* DECnet */
#define AF_DLI		13		/* Direct data link interface */
#define AF_LAT		14		/* LAT */
#define	AF_HYLINK	15		/* NSC Hyperchannel */
#define AF_APPLETALK	16		/* Apple talk */
#define AF_BSC		17		/* BISYNC 2780/3780 */
#define AF_DSS		18		/* Distributed system services */

#define	AF_MAX		19

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
	u_short	sa_family;		/* address family */
	char	sa_data[14];		/* up to 14 bytes of direct address */
};

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
	u_short	sp_family;		/* address family */
	u_short	sp_protocol;		/* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define	PF_UNSPEC	AF_UNSPEC
#define	PF_UNIX		AF_UNIX
#define	PF_INET		AF_INET
#define	PF_IMPLINK	AF_IMPLINK
#define	PF_PUP		AF_PUP
#define	PF_CHAOS	AF_CHAOS
#define	PF_NS		AF_NS
#define	PF_NBS		AF_NBS
#define	PF_ECMA		AF_ECMA
#define	PF_DATAKIT	AF_DATAKIT
#define	PF_CCITT	AF_CCITT
#define	PF_SNA		AF_SNA
#define PF_DECnet	AF_DECnet
#define PF_DLI		AF_DLI
#define PF_LAT		AF_LAT
#define	PF_HYLINK	AF_HYLINK
#define PF_APPLETALK	AF_APPLETALK
#define PF_BSC		AF_BSC
#define PF_DSS		AF_DSS

#define	PF_MAX		AF_MAX

/*
 * Maximum queue length specifiable by listen.
 */
#define	SOMAXCONN	5

/*
 * I/O buffer element.
 */
struct iovec {
	caddr_t	iov_base;
	int	iov_len;
};

/*
 * Message header for recvmsg and sendmsg calls.
 */
struct msghdr {
	caddr_t	msg_name;		/* optional address */
	int	msg_namelen;		/* size of address */
	struct	iovec *msg_iov;		/* scatter/gather array */
	int	msg_iovlen;		/* # elements in msg_iov */
	caddr_t	msg_accrights;		/* access rights sent/received */
	int	msg_accrightslen;
};

#define	MSG_OOB		0x1		/* process out-of-band data */
#define	MSG_PEEK	0x2		/* peek at incoming message */
#define	MSG_DONTROUTE	0x4		/* send without using routing tables */

#define	MSG_MAXIOVLEN	16

/*  From Ultrix's time.h */
struct timeval {
    long tv_sec;
    long tv_usec;
};

int socket( int af, int mess_type, int prot_type);
int accept( int sd, struct sockaddr *S_addr, int addrlen);
int bind( int sd, struct sockaddr *s_name, int namelen);
int listen( int sd, int backlog);
int connect( int sd, struct sockaddr *name, int namelen);
int send( int sd, char * msg, int length, int flags);
int recvmsg( int sd, struct msghdr *msg, int flags);
int sendmsg( int sd, struct msghdr * msg, int flags);
int sendto( int sd, char * msg, int length, int flags, struct sockaddr *to, int tolen);
int recv( int sd, char * buf, int length, int flags);
int recvfrom( int sd, char * buf, int length, int flags, struct sockaddr *from, int *fromlen);
int shutdown( int sd, int mode);
int select( int nfds, int *readfds, int *writefds, int *exceptfds, struct timeval *timeout);
int gethostname( char *name, int namelen);
int gethostaddr( char *addr);
int getpeername( int sd, struct sockaddr *name, int *namelen);
int getsockname( int sd, struct sockaddr *name, int *namelen);
int getsockopt( int sd, int level, int optname, char *optval, int *optlen);
int setsockopt( int sd, int level, int optname, char *optval, int optlen);
int vaxc$get_sdc( int descrip_no);

#endif					/* __SOCKET_LOADED */
