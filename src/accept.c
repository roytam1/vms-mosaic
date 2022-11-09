/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright (C) 1993, Board of Trustees of the University of Illinois      *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/

/* Copyright (C) 2005, 2006 - The VMS Mosaic Project */

#include "../config.h"
#ifdef CCI

#ifdef linux
#define SCREWY_BLOCKING
#endif

#include <stdio.h>
#include <stdlib.h>

#ifndef MULTINET

#ifdef SOCKETSHR
#include "socketshr_files:types.h"
#else
#include <sys/types.h>
#endif

#if defined(VMS) && defined(CADDR_T)
#ifdef __CADDR_T
#undef __CADDR_T
#endif
#define __CADDR_T
#endif /* VMS, include file problem, BSN */

#ifdef VMS
#if defined(SOCKETSHR) && defined(__DECC)
#define __FD_SET 1
#endif /* DEC C V5.2 socket.h conflicts with SOCKETSHR types.h, GEC */
#include <socket.h>
#include <in.h>
#include <time.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif

#include <netdb.h>

#if defined(VMS) && !defined(WIN_TCP)

#ifndef SOCKETSHR
int  ioctl (int d, int request, char *argp);
#ifndef UCX_COMPAT
#include "sys$library:ucx$inetdef.h"
#else
#ifdef TCPWARE
#include "tcpware_include:ucx$inetdef.h"
#else
#define FIONBIO -2147195266
#endif
#endif /* ucx$inetdef.h will be missing for UCX compatibility builds, GEC */
#endif
#include <tcp.h>
#ifdef SOCKETSHR
#include "socketshr_files:socketshr.h"
#if defined(__DECC) && !defined(__alpha)
#undef fprintf
#endif /* Avoid possible problems, GEC */
#endif

#else

#include <sys/ioctl.h>
#endif /* VMS, use ioctl stuff in [.libwww2]httcp.c for UCX, GEC */

#else /* MultiNet */

#if defined(__TIME_T) && !defined(__TYPES_LOADED) && !defined(__TYPES)
#define __TYPES_LOADED
#endif /* Different defs in OpenVMS and MultiNet include files, BSN */
#ifdef __DECC
#define _POSIX_C_SOURCE
#endif /* DEC C, GEC */
#include "multinet_root:[multinet.include.sys]types.h"
#include "multinet_root:[multinet.include.sys]socket.h"
#include "multinet_root:[multinet.include.netinet]in.h"
#include "multinet_root:[multinet.include]netdb.h"
#ifdef __DECC
#undef _POSIX_C_SOURCE
#endif /* DEC C, GEC */
#if defined(__DECC) && !defined(_XOPEN_SOURCE_EXTENDED) && defined(__FD_SET)
#define _XOPEN_SOURCE_EXTENDED
#include "multinet_root:[multinet.include.sys]time.h"
#undef _XOPEN_SOURCE_EXTENDED
#else
#include "multinet_root:[multinet.include.sys]time.h"
#endif /* As if MultiNet V3.5A&B weren't bad enough, then came DECC052H, GEC */
#include "multinet_root:[multinet.include.sys]ioctl.h"
#endif /* MULTINET, BSN, GEC */

#ifdef SCREWY_BLOCKING
#include <sys/fcntl.h>
#endif

/* For memset */
#ifndef VMS
#include <memory.h>
#else
#include <string.h>
#endif /* VMS, GEC */

#ifdef MOTOROLA
#include <sys/filio.h>
#endif

#ifdef DGUX
#include <sys/file.h>
#endif  

#ifdef SVR4
#ifndef SCO
#ifndef DGUX
#include <sys/filio.h>
#endif
#endif
#endif

#include "cci.h"
#include "accept.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

/* Return -1 on error */
ListenAddress NetServerInitSocket(int portNumber)
{
	ListenAddress socketFD;
	struct sockaddr_in serverAddress;
	struct protoent *protocolEntry;

	protocolEntry = getprotobyname("tcp");
	if (protocolEntry) {
		socketFD = socket(AF_INET, SOCK_STREAM, protocolEntry->p_proto);
	} else {
		socketFD = socket(AF_INET, SOCK_STREAM, 0);
	}
	
	if (socketFD < 0) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Can't create socket.\n");
#endif
		return(-1);
	}

	memset((char *) &serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(portNumber);

	if (bind(socketFD, (struct sockaddr *) &serverAddress, 
		 sizeof(serverAddress)) < 0) {
#ifndef DISABLE_TRACE
			if (srcTrace)
				fprintf(stderr, "Can't bind to address.\n");
#endif
		return(-1);
	}

#ifdef SCREWY_BLOCKING
        /* Set socket to non-blocking for linux */
        fcntl(socketFD, FNDELAY, 0);
#endif
        
	if (listen(socketFD, 5) == -1) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Can't listen.\n");
#endif
		return(-1);
	}

#ifndef SCREWY_BLOCKING
	/* Set socket to non-blocking */
#ifndef MULTINET
	ioctl(socketFD, FIONBIO, 0);
#else
	socket_ioctl(socketFD, FIONBIO, 0);
#endif
#endif
	return(socketFD);
}


/* Accept a connection off of a base socket */
/* Do not block! */
/* Return NULL if no connection else return PortDescriptor*  */
PortDescriptor *NetServerAccept(ListenAddress socketFD)
{
	int newSocketFD;
	struct sockaddr_in clientAddress;
#if !defined(VMS) || (__DECC_VER < 50230003)
	int clientAddressLength = sizeof(clientAddress);
#else
	size_t clientAddressLength = sizeof(clientAddress);
#endif
	PortDescriptor *c;

	/* It's assumed that the socketFD has already been set to non block */
	newSocketFD = accept(socketFD, (struct sockaddr *) &clientAddress,
#ifndef __GNUC__
			     &clientAddressLength);
#else
			     (int)&clientAddressLength);
#endif /* GNU C, GEC */
	if (newSocketFD < 0)
		return(NULL);

	/* We have connection */
	if (!(c = (PortDescriptor *)malloc(sizeof(PortDescriptor))))
		return(0);
	c->socketFD = newSocketFD;
	c->numInBuffer = 0;

	return(c);
}


/* Read input from port, return number of bytes read */
int NetRead(PortDescriptor *c, char *buffer, int bufferSize)
{
	int length;

#ifndef MULTINET
	length = read(c->socketFD, buffer, bufferSize);
#else
	length = socket_read(c->socketFD, buffer, bufferSize);
#endif
	return(length);
}


/* Send buffer, return number of bytes sent */
int NetServerWrite(PortDescriptor *c, char *buffer, int bufferSize)
{
	int length;

#ifndef MULTINET
	length = write(c->socketFD, buffer, bufferSize);
#else
	length = socket_write(c->socketFD, buffer, bufferSize);
#endif
	return(length);
}


/* Close the connection */
void NetCloseConnection(PortDescriptor *c)
{
#ifndef MULTINET
	close(c->socketFD);
#else
	socket_close(c->socketFD);
#endif
	return;
}

void NetCloseAcceptPort(int s)
{
#ifndef MULTINET
	close(s);
#else
	socket_close(s);
#endif
	return;
}


/* Do a non block check on socket for input and return 1 for yes, 0 for no */
int NetIsThereInput(PortDescriptor *p)
{
	static struct timeval timeout = { 0L , 0L };
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(p->socketFD, &readfds);
#if defined(__hpux) || defined(MULTINET) || defined(_DECC_V4_SOURCE) || (defined(__DECC) && !defined(__DECC_VER)) || __DECC_
	if (0 < select(32, (int *)&readfds, 0, 0, &timeout)) {
#else
	if (0 < select(32, &readfds, 0, 0, &timeout)) {
#endif		
		return(1);
	} else {
		return(0);
	}
}

/* Do a non block check on socket for input and return 1 for yes, 0 for no */
int NetIsThereAConnection(int socketFD)
{
	static struct timeval timeout = { 0L , 0L };
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(socketFD, &readfds);
#if defined(__hpux) || defined(MULTINET) || defined(_DECC_V4_SOURCE) || (defined(__DECC) && !defined(__DECC_VER)) || __DECC_
	if (0 < select(32, (int *)&readfds, 0, 0, &timeout)) {
#else
	if (0 < select(32, &readfds, 0, 0, &timeout)) {
#endif		
		return(1);
	} else {
		return(0);
	}
}

/* Extract socket file descriptor from the Port structure */
int NetGetSocketDescriptor(PortDescriptor *s)
{
        return(s->socketFD);
}

#else
int acceptdummy;  /* Shut the freaking stupid compiler up */
#endif /* CCI */
