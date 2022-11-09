/*			Generic Communication Code		HTTCP.c
**			==========================
**
**	This code is in common between client and server sides.
**
**	16 Jan 92  TBL	Fix strtol() undefined on CMU Mach.
**	25 Jun 92  JFG  Added DECNET option through TCP socket emulation.
*/

/* SOCKS mods by:
 * Ying-Da Lee, <ylee@syl.dl.nec.com>
 * NEC Systems Laboratory
 * C&C Software Technology Center
 */
#include "../config.h"
#include "HTUtils.h"
#include "HTParse.h"
#include "HTAlert.h"
#include "HTAccess.h"

#ifdef __STDC__
#include <stdlib.h>
#endif

#if defined(SVR4) && !defined(SCO) && !defined(linux) && !defined(DGUX)
#include <sys/filio.h>
#endif

#if defined(DGUX)
#include <sys/file.h>
#endif

#ifndef DISABLE_TRACE
extern int httpTrace;
extern int www2Trace;
#endif

int broken_crap_hack = 0;

extern int HTCopyOneRead;

/*	Module-Wide variables
*/

PRIVATE char *hostname = NULL;		/* The name of this host */


/*	Encode INET status (as in sys/errno.h)			  inet_status()
**	------------------
**
** On entry,
**	where		gives a description of what caused the error
**	global errno	gives the error number in the unix way.
**
** On return,
**	returns		a negative status in the unix way.
*/
#ifndef errno
extern int errno;
#endif /* errno */

#ifndef VMS
extern char *sys_errlist[];		/* See man perror on cernvax */
extern int sys_nerr;
#endif /* VMS, BSN */

#if defined(VMS) && !defined(MULTINET) && !defined(WIN_TCP) && !defined(SOCKETSHR)
/*
 * A routine to mimick the ioctl function for UCX.
 * Bjorn S. Nilsson, 25-Nov-1993.  Based on an example in the UCX manual.
 * Note.  This code was taken out because it crashed OpenVMS 1.5 systems
 * with UCX 3.0.  I just hope there are none of these left :-(
 */
#include <stdio.h>
#include <iodef.h>

#ifdef TCPWARE
#include "tcpware_include:ucx$inetdef.h"
#else
#define UCX$C_IOCTL 2
#define FIONBIO -2147195266
#endif  /* ucx$inetdef.h is missing in UCX 5.x (DECpaq morons at work) */
#include <errno.h>

#define IOC_OUT (int)0x40000000

extern int vaxc$get_sdc(), sys$qiow();

PUBLIC int ioctl(int d, int request, void *argp)  /* Needed by accept.c, GEC */
{
  int sdc, status;
  unsigned short fun;
  unsigned short iosb[4];
  char *p5, *p6;
  struct comm {
      int command;
      char *addr;
  } ioctl_comm;
  struct it2 {
      unsigned short len;
      unsigned short opt;
      struct comm *addr;
  } ioctl_desc;

  if ((sdc = vaxc$get_sdc(d)) == 0) {
      errno = EBADF;
      return -1;
  }

  ioctl_desc.opt = UCX$C_IOCTL;
  ioctl_desc.len = sizeof(struct comm);
  ioctl_desc.addr = &ioctl_comm;

  if (request & IOC_OUT) {
      fun = IO$_SENSEMODE;
      p5 = 0;
      p6 = (char *)&ioctl_desc;
  } else {
      fun = IO$_SETMODE;
      p5 = (char *)&ioctl_desc;
      p6 = 0;
  }
    
  ioctl_comm.command = request;
  ioctl_comm.addr = argp;

  status = sys$qiow(0, sdc, fun, iosb, 0, 0, 0, 0, 0, 0, p5, p6);
  if (!(status & 01)) {
      errno = status;
      return -1;
  }
  if (!(iosb[0] & 01)) {
      errno = iosb[0];
      return -1;
  }

  return 0;
}
#endif /* VMS, UCX, BSN */

/*	Report Internet Error
**	---------------------
*/
PUBLIC int HTInetStatus(char *where)
{
#ifndef VMS
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr,
	        "TCP: Error %d in `errno' after call to %s() failed.\n\t%s\n",
	        errno,  where,
	        errno < sys_nerr ? sys_errlist[errno] : "Unknown error");
#endif
#else
#ifndef DISABLE_TRACE
    if (www2Trace) {
	fprintf(stderr, "TCP: Unix error number = %ld dec\n", errno);
	fprintf(stderr, "     in call to %s()\n", where);
	fprintf(stderr, "TCP: VMS error         = %lx hex\n", vaxc$errno);
    }
#endif
#ifdef MULTINET
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "TCP: Multinet error    = %lx hex\n", socket_errno);
#endif
#endif /* MULTINET, BSN */
#endif /* VMS, BSN */
    return -1;
}


/*	Parse a cardinal value				       parse_cardinal()
**	----------------------
**
** On entry,
**	*pp	    points to first character to be interpreted, terminated by
**		    non 0:9 character.
**	*pstatus    points to status already valid
**	maxvalue    gives the largest allowable value.
**
** On exit,
**	*pp	    points to first unread character
**	*pstatus    points to status updated iff bad
*/

PUBLIC unsigned int HTCardinal (int *pstatus, char **pp, unsigned int max_value)
{
    int n = 0;

    if ((**pp < '0') || (**pp > '9')) {	    /* Null string is error */
	*pstatus = -3;  /* No number where one expeceted */
	return 0;
    }

    while ((**pp >= '0') && (**pp <= '9'))
	n = n * 10 + *((*pp)++) - '0';

    if (n > max_value) {
	*pstatus = -4;  /* Cardinal outside range */
	return 0;
    }

    return n;
}


/*	Produce a string for an Internet address
**	----------------------------------------
**
** On exit,
**	returns	a pointer to a static string which must be copied if
**		it is to be kept.
*/

PUBLIC char *HTInetString (SockA *sin)
{
    static char string[16];

    sprintf(string, "%d.%d.%d.%d",
	    (int)*((unsigned char *)(&sin->sin_addr) + 0),
	    (int)*((unsigned char *)(&sin->sin_addr) + 1),
	    (int)*((unsigned char *)(&sin->sin_addr) + 2),
	    (int)*((unsigned char *)(&sin->sin_addr) + 3));
    return string;
}


/*	Parse a network node address and port
**	-------------------------------------
**
** On entry,
**	str	points to a string with a node name or number,
**		with optional trailing colon and port number.
**	sin	points to the binary internet or decnet address field.
**
** On exit,
**	*sin	is filled in. If no port is specified in str, that
**		field is left unchanged in *sin.
*/
PUBLIC int HTParseInet (SockA *sin, char *str)
{
  char *port, *tmp;
  char host[256];
  struct hostent *phost;	/* Pointer to host - See netdb.h */
  int numeric_addr;
  static char *cached_host = NULL;
  static char *cached_phost_h_addr = NULL;
  static int cached_phost_h_length = 0;

#ifdef VMS
  if (strlen(str) >= sizeof(host))
      application_user_feedback("HTParseInet host dimension is too small!");
#endif /* VMS, BSN */
  strcpy(host, str);		/* Make a copy we can mutilate */
  
  /* Parse port number if present */    
  if (port = strchr(host, ':')) {
      *port++ = '\0';		/* Chop off port */
      if ((*port >= '0') && (*port <= '9'))
          sin->sin_port = htons(atol(port));
  }
  
  /* Parse host number if present. */  
  numeric_addr = 1;
  for (tmp = host; *tmp; tmp++) {
      /* If there's a non-numeric... */
      if ((*tmp < '0' || *tmp > '9') && *tmp != '.') {
          numeric_addr = 0;
          goto found_non_numeric_or_done;
      }
  }
  
 found_non_numeric_or_done:
  if (numeric_addr) {   /* Numeric node address: */
      sin->sin_addr.s_addr = inet_addr(host);  /* See arpa/inet.h */
  } else {	        /* Alphanumeric node name: */
      if (cached_host && !strcmp(cached_host, host)) {
          memcpy(&sin->sin_addr, cached_phost_h_addr, cached_phost_h_length);
      } else {
          phost = gethostbyname(host);
          if (!phost) {
#ifndef DISABLE_TRACE
              if (www2Trace) 
                  fprintf(stderr, 
                     "HTTPAccess: Can't find internet node name `%s'.\n", host);
#endif
              return -1;  /* Fail? */
          }

          /* Free previously cached strings. */
          if (cached_host) {
              free(cached_host);
	      cached_host = NULL;
	  }
          if (cached_phost_h_addr) {
              free(cached_phost_h_addr);
	      cached_phost_h_addr = NULL;
	  }

          /* Cache new stuff. */
          cached_host = strdup(host);
          cached_phost_h_addr = calloc(phost->h_length + 1, 1);
          memcpy(cached_phost_h_addr, phost->h_addr, phost->h_length);
          cached_phost_h_length = phost->h_length;

          memcpy(&sin->sin_addr, phost->h_addr, phost->h_length);
      }
  }
  
#ifndef DISABLE_TRACE
  if (www2Trace) 
      fprintf(stderr,  
              "TCP: Parsed address as port %d, IP address %d.%d.%d.%d\n",
              (int)ntohs(sin->sin_port),
              (int)*((unsigned char *)(&sin->sin_addr) + 0),
              (int)*((unsigned char *)(&sin->sin_addr) + 1),
              (int)*((unsigned char *)(&sin->sin_addr) + 2),
              (int)*((unsigned char *)(&sin->sin_addr) + 3));
#endif
  
  return 0;	/* OK */
}


/*	Derive the name of the host on which we are
**	-------------------------------------------
**
*/
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64		/* Arbitrary limit */
#endif

PRIVATE void get_host_details(void)
{
    char name[MAXHOSTNAMELEN + 1];	/* The name of this host */
    int namelength = sizeof(name);
    
    if (hostname)
	return;		/* Already done */
    gethostname(name, namelength);	/* Without domain */

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "TCP: Local host name is %s\n", name);
#endif

    StrAllocCopy(hostname, name);
}


PUBLIC char *HTHostName(void)
{
    get_host_details();
    return hostname;
}

#ifdef SOCKS
struct in_addr SOCKS_ftpsrv;
#endif


PUBLIC int HTDoConnect (char *url, char *protocol, int default_port, int *s)
{
  struct sockaddr_in soc_address;
  struct sockaddr_in *sin = &soc_address;
  int status;
  char line[256];

  /* Set up defaults: */
  sin->sin_family = AF_INET;
  sin->sin_port = htons(default_port);
  
  /* Get node name and optional port number: */
  {
    char *p1 = HTParse(url, "", PARSE_HOST);

    sprintf(line, "Looking up %s.", p1);
    HTProgress(line);

    status = HTParseInet(sin, p1);
    if (status) {
        sprintf(line, "Unable to locate remote host %s.", p1);
        HTProgress(line);
        free(p1);
        return HT_NO_DATA;
    }
    sprintf(line, "Making %s connection to %s.", protocol, p1);
    HTProgress(line);
    free(p1);
  }

  /* Now, let's get a socket set up from the server for the data: */      
  *s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

#ifdef SOCKS
  /* SOCKS can't yet deal with non-blocking connect request */
  status = Rconnect(*s, (struct sockaddr *)&soc_address, sizeof(soc_address));
  if ((status == 0) && !strcmp(protocol, "FTP"))
      SOCKS_ftpsrv.s_addr = soc_address.sin_addr.s_addr;

  if (HTCheckActiveIcon(1)) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "*** INTERRUPTED in middle of connect.\n");
#endif
      status = HT_INTERRUPTED;
      errno = EINTR;
  }
  return status;
#else /* SOCKS not defined */

  /*
   * Make the socket non-blocking, so the connect can be canceled.
   * This means that when we issue the connect we should NOT
   * have to wait for the accept on the other end.
   */
  {
    int ret;
    int val = 1;
    
#ifndef MULTINET
    ret = ioctl(*s, FIONBIO, &val);
#else
    ret = socket_ioctl(*s, FIONBIO, &val);
#endif /* VMS, BSN */
    if (ret == -1) {
        sprintf(line, "Could not make connection non-blocking.");
        HTProgress(line);
#ifndef DISABLE_TRACE
        if (www2Trace)
	    fprintf(stderr, "Could not make connection non-blocking.\n");
#endif
    }
  }

  /*
   * Issue the connect.  Since the server can't do an instantaneous accept
   * and we are non-blocking, this will almost certainly return a negative
   * status.
   */
  status = connect(*s, (struct sockaddr *)&soc_address, sizeof(soc_address));

  /*
   * According to the Sun man page for connect:
   *     EINPROGRESS         The socket is non-blocking and the connection
   *			     cannot be completed immediately.  It is
   *			     possible to select(2) for completion by
   *			     selecting the socket for writing.
   * According to the Motorola SVR4 man page for connect:
   *     EAGAIN              The socket is non-blocking and the connection
   *			     cannot be completed immediately.
   *                         It is possible to select for completion
   *                         by selecting the socket for writing.
   *                         However, this is only possible if the
   *                         socket STREAMS module is the topmost
   *                         module on the protocol stack with a
   *                         write service procedure.  This will be
   *                         the normal case.
   */
#ifdef SVR4
  if ((status < 0) && ((errno == EINPROGRESS) || (errno == EAGAIN))) {
#else
#ifndef MULTINET
  if ((status < 0) && (errno == EINPROGRESS)) {
#else
  if ((status < 0) && (socket_errno == EINPROGRESS)) {
#endif /* MULTINET, BSN */
#endif /* SVR4 */
      struct timeval timeout;
      int ret = 0;

      while (ret <= 0) {
          fd_set writefds;
          
          FD_ZERO(&writefds);
          FD_SET((unsigned) *s, &writefds);

	  /* linux (and some other os's, I think) clear timeout... 
	   * Let's reset it every time. */
	  timeout.tv_sec = 0;
	  timeout.tv_usec = 100000;

#if defined(__hpux) || defined(MULTINET) || defined(_DECC_V4_SOURCE) || (defined(__DECC) && !defined(__DECC_VER)) || __DECC_
          ret = select((unsigned) *s + 1, NULL, (int *)&writefds, NULL,
		       &timeout);
#else
          ret = select((unsigned) *s + 1, NULL, &writefds, NULL, &timeout);
#endif
	  /*
	   * Again according to the Sun and Motorola man pages for connect:
           *     EALREADY            The socket is non-blocking and a previous
           *                         connection attempt has not yet been
           *                         completed.
           * Thus if the errno is NOT EALREADY we have a real error, and
	   * should break out here and return that error.
           * Otherwise if it is EALREADY keep on trying to complete the
	   * connection.
	   */
          if ((ret < 0) && (errno != EALREADY)) {
              status = ret;
              break;
          } else if (ret > 0) {
	      /*
	       * Extra check here for connection success, if we try to connect
	       * again, and get EISCONN, it means we have a successful
	       * connection.
	       */
              status = connect(*s, (struct sockaddr *)&soc_address,
                               sizeof(soc_address));
#ifndef MULTINET
#if !defined(VMS) || defined(WIN_TCP) || defined(SOCKETSHR)
              if ((status < 0) && (errno == EISCONN)) {
#else
	      /*
	       * A UCX feature: Instead of returning EISCONN UCX returns
	       * EADDRINUSE.  Test for this status also.
	       */
              if ((status < 0) &&
		  ((errno == EISCONN) || (errno == EADDRINUSE))) {
#endif /* VMS, UCX, BSN */
#else
              if ((status < 0) && (socket_errno == EISCONN)) {
#endif /* MULTINET, BSN */
                  status = 0;
              }
              break;
	  /*
	   * The select says we aren't ready yet.
	   * Try to connect again to make sure.  If we don't get EALREADY
	   * or EISCONN, something has gone wrong.  Break out and report it.
	   * For some reason SVR4 returns EAGAIN here instead of EALREADY,
	   * even though the man page says it should be EALREADY.
	   */
          } else {
              status = connect(*s, (struct sockaddr *)&soc_address,
                               sizeof(soc_address));
#ifdef SVR4
              if ((status < 0) && (errno != EALREADY) && (errno != EAGAIN) &&
		  (errno != EISCONN)) {
#else
#ifndef MULTINET
#if !defined(VMS) || defined(WIN_TCP) || defined(SOCKETSHR)
              if ((status < 0) && (errno != EALREADY) && (errno != EISCONN)) {
#else
	      /*
	       * UCX pre 3 apparently returns errno = 18242 instead of any of
	       * the EALREADY or EISCONN values.
	       */
              if ((status < 0) && (errno != EALREADY) && (errno != EISCONN) &&
                  (errno != 18242)) {
#endif /* UCX, BSN */
#else
              if ((status < 0) && (socket_errno != EALREADY) &&
		  (socket_errno != EISCONN)) {
#endif /* MULTINET, BSN */
#endif /* SVR4 */
                  break;
              }
          }
          if (HTCheckActiveIcon(1)) {
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, "*** INTERRUPTED in middle of connect.\n");
#endif
              status = HT_INTERRUPTED;
              errno = EINTR;
              break;
          }
      }
  }

  /*
   * Make the socket blocking again on good connect
   */
  if (status >= 0) {
      int ret;
      int val = 0;
      
#ifndef MULTINET
      ret = ioctl(*s, FIONBIO, &val);
#else
      ret = socket_ioctl(*s, FIONBIO, &val);
#endif /* VMS, BSN */
      if (ret == -1) {
          sprintf(line, "Could not restore socket to blocking.");
          HTProgress(line);
#ifndef DISABLE_TRACE
          if (www2Trace)
	      fprintf(stderr, "Could not restore socket to blocking.\n");
#endif
      }
  } else {
      /*
       * Else the connect attempt failed or was interrupted,
       * so close up the socket.
       */
#if !defined(VMS)
      close(*s);
#else
      NETCLOSE(*s);
#endif /* VMS */
  }

  return status;
#endif  /* #ifdef SOCKS */
}

/* This is so interruptible reads can be implemented cleanly. */
int HTDoRead (int fildes, void *buf, unsigned nbyte)
{
    fd_set readfds;
    struct timeval timeout;
    int ret;
    int tries = 0;
    int ready = HTCopyOneRead;		/* Select already done? */
#ifdef VMS
    int nb;
#endif /* VMS, BSN */

    while (!ready ) {
	/*
	**  Protect against an infinite loop.  Try for 30 minutes.
	*/
	if (tries++ >= 18,000) {
	    HTProgress("Socket read failed for 18,000 tries.");
	    return HT_INTERRUPTED;
	}
	if (broken_crap_hack && (tries > 40)) {
	    if (broken_crap_hack == 1) {
	        HTProgress("Aborting connection to broken Microsoft server.");
	    } else {
		broken_crap_hack = 0;
	    }
	    return 0;
	}
        FD_ZERO(&readfds);
        FD_SET((unsigned) fildes, &readfds);

	/* linux (and some other os's, I think) clear timeout... 
	 * Let's reset it every time. */
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

#if defined(__hpux) || defined(MULTINET) || defined(_DECC_V4_SOURCE) || (defined(__DECC) && !defined(__DECC_VER)) || __DECC_
        ret = select((unsigned) fildes + 1, (int *)&readfds, NULL, NULL,
		     &timeout);
#else
        ret = select((unsigned) fildes + 1, &readfds, NULL, NULL, &timeout);
#endif
        if (ret < 0)
            return -1;
        if (ret > 0)
            ready = 1;
	if (HTCheckActiveIcon(1))
            return HT_INTERRUPTED;
    }

#ifndef VMS
    ret = read(fildes, buf, nbyte);

#ifndef DISABLE_TRACE
    if (httpTrace) {
	int i;
	unsigned char *outbuf = buf;

	for (i = 0; i < ret; i++) {
	    if (isalnum(outbuf[i]) || isspace(outbuf[i]) ||
		ispunct(outbuf[i])) {
		fprintf(stderr, "%c", outbuf[i]);
	    } else {
		fprintf(stderr, ".");
	    }
	}
    }
#endif

    return ret;

#else   /* VMS, BSN */

#ifndef MULTINET
    nb = read(fildes, buf, nbyte);
#else
    nb = socket_read(fildes, buf, nbyte);
#endif /* MULTINET, BSN */

#if !defined(MULTINET) && !defined(WIN_TCP) && !defined(SOCKETSHR)
    /*
     * A vaxc$errno value of 8428 (SS$_LINKDISCON) indicates end-of-file.
     */
    if ((nb <= 0) && (errno != ECONNRESET) && (vaxc$errno == 8428))
	nb = 0;
#endif /* UCX, BSN */

#ifndef DISABLE_TRACE
    if (httpTrace) {
	int i;
	unsigned char *outbuf = buf;

	if (nb <= 0) {
	    fprintf(stderr, "HTTP: Read errno = %d\n", errno);
#ifdef VMS
	    if (errno == EVMSERR)
		fprintf(stderr, "HTTP: Read vaxc$errno = %d\n", vaxc$errno);
#endif
	}
	for (i = 0; i < nb; i++) {
	    if (isalnum(outbuf[i]) ||
		(isspace(outbuf[i]) && (outbuf[i] < 128)) ||
		(ispunct(outbuf[i]) && (outbuf[i] < 128))) {
		fprintf(stderr, "%c", outbuf[i]);
	    } else {
		fprintf(stderr, ".");
	    }
	}
    }
#endif

    return nb;
#endif  /* VMS, BSN */
}
