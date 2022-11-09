/*			File Transfer Protocol (FTP) Client
**			for a WorldWideWeb browser
**			===================================
**
**	A cache of one control connection is kept
**
** Note: Port allocation
**
**	It is essential that the port is allocated by the system, rather
**	than chosen in rotation by us (POLL_PORTS), or the following
**	problem occurs.
**
**	It seems that an attempt by the server to connect to a port which has
**	been used recently by a listen on the same socket, or by another
**	socket this or another process causes a hangup of (almost exactly)
**	one minute.  Therefore, we have to use a rotating port number.
**	The problem remains that if the application is run twice in quick
**	succession, it will hang for what remains of a minute.
**
** Authors
**	TBL	Tim Berners-lee <timbl@info.cern.ch>
**	DD	Denis DeLaRoca 310 825-4580 <CSP1DWD@mvs.oac.ucla.edu>
** History:
**	 2 May 91	Written TBL, as a part of the WorldWideWeb project.
**	15 Jan 92	Bug fix: close() was used for NETCLOSE for control soc
**	10 Feb 92	Retry if cached connection times out or breaks
**	 8 Dec 92	Bug fix 921208 TBL after DD
**	17 Dec 92	Anon FTP password now just WWWuser@ suggested by DD
**			fails on princeton.edu!
**      30 Jun 95       Re-added cached connection so it works; Added support
**			to display informational messages that the FTP site
**			shows.
**
*/

/* SOCKS mods by:
 * Ying-Da Lee, <ylee@syl.dl.nec.com>
 * NEC Systems Laboratory
 * C&C Software Technology Center
 */
#include "../config.h"
#ifndef VMS
#include <X11/Intrinsic.h>
#endif /* Includes types.h which is a bad thing, so moved below, GEC */
#include <string.h>
#include <stdlib.h>
#ifndef VMS
#include <time.h>
#endif /* time.h loaded by tcp.h, etc., GEC */
#include "HTFTP.h"	/* Implemented here */
#define LINE_LENGTH 1024

#include "HTParse.h"

#ifdef VMS
#ifndef __DECC   /* PGE, needed to compile under VaxC */
#ifndef __CADDR_T
#define __CADDR_T 1   /* DECwindows xresource.h wants __CADDR_T, PGE */
#endif
#ifndef CADDR_T
#define CADDR_T 1     /* DECwindows Motif 1.1 xresource.h wants CADDR_T, GEC */
#endif
#else  /* Needed for DEC C types.h, GEC */
#if defined(MULTINET) && !defined(__alpha)
#define _POSIX_C_SOURCE  /* Work around the inclusion of types.h, GEC */
#endif
#endif
#include "../src/prefs.h"  /* Does the required include of X11/Intrinsic.h */
#if defined(__DECC) && defined(MULTINET) && !defined(__alpha)
#undef _POSIX_C_SOURCE
#undef _ANSI_C_SOURCE  /* Gets defined because of _POSIX_C_SOURCE */
#endif /* DEC C, GEC */
#endif /* VMS, moved after tcp.h to avoid conflict, GEC */

#include "HTTCP.h"
#include "HTAnchor.h"
#include "HTFile.h"
#include "HTChunk.h"
#include "HTIcon.h"
#include "HText.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64           /* Arbitrary limit */
#endif

/*For HTAA_LOGIN */
#include "HTAAUtil.h"

#ifdef SOCKETSHR
#include <signal.h>
#include <setjmp.h>
#endif /* SOCKETSHR, GEC */

#ifdef __GNUC__
#include <stat.h>
#endif /* GNU C, probably should do in tcp.h, GEC */

#ifndef IPPORT_FTP
#define IPPORT_FTP 21
#endif

#ifdef __STDC__
#include <stdlib.h>
#endif

#ifndef DISABLE_TRACE
extern int reportBugs;
extern int www2Trace;
#endif

/*		Hypertext object building machinery
*/
#include "HTML.h"

struct _HTStream {
        WWW_CONST HTStreamClass *isa;
        /* ... */
};

extern int broken_crap_hack;

typedef enum {
	GENERIC_SERVER,
	UNIX_SERVER,
	UNIX_L8_SERVER,
	VMS_SERVER,
	WINDOWS_NT_SERVER,
	WINDOWS_2K_SERVER,
	FILEZILLA_SERVER
} eServerType;

static eServerType server_type = GENERIC_SERVER;

/* 
** Info for cached connection;  right now we only keep one around for a while
*/  
extern XtAppContext app_context;
extern int ftp_timeout_val;
extern int securityType;

extern char *prompt_for_password(char *msg);

static BOOL usingNLST = 0;

/* FTP "redial" using ftpRedial resource for number of times */
extern int ftpRedial;
extern int ftpRedialSleep;
extern int ftpFilenameLength;
extern int ftpEllipsisLength;
extern int ftpEllipsisMode;

/* Passive mode support */
BOOL ftp_passive = FALSE;

/* Directory parsing */
static int ParseDate(char *szBuffer, char *szFileInfo, char *szMonth,
		     char *szDay, char *szYear, char *szTime);
static int ParseFileSizeAndName(char *szBuffer, char *szFileName, char *szSize);

static HText *HT;
static int fTimerStarted = 0;
static XtIntervalId timer;

static struct ftpcache {
	int control;
	int ftp_bug;		/* Cache FileZilla bug enable */
	char host[256];
	char username[BUFSIZ];
	char password[BUFSIZ];
} ftpcache = {
	-1,
	0,
	"",
	"",
	""
};

#ifdef SOCKS
extern struct in_addr SOCKS_ftpsrv;	/* In HTTCP.C */
#endif

/*	Module-Wide Variables
**	---------------------
*/
PRIVATE char  response_text[LINE_LENGTH + 1];  /* Last response */
PRIVATE int   control = -1;		/* Current connection */
PRIVATE int   data_soc = -1;		/* Socket for data transfer */

PRIVATE int   master_socket = -1;	/* Listening socket = invalid */
PRIVATE char  port_command[255];	/* Command for setting the port */

#define DATA_BUFFER_SIZE 2048
PRIVATE char data_buffer[DATA_BUFFER_SIZE];	/* Input data buffer */
PRIVATE char *data_read_pointer;
PRIVATE char *data_write_pointer;
PRIVATE char ftp_root_directory[100];
PUBLIC char ftp_type[16];
PRIVATE int loading_length;
PUBLIC int HTftp_loading_length = 0;

/*
 * ProFTPD 1.2.5rc1 is known to have a broken implementation of RETR.  If asked
 * to retrieve a directory, it gets confused and fails subsequent commands such
 * as CWD and LIST.  Lynx fix - TD 2004/1/1.
 */
/* FileZilla also has a similar problem */
static int ProFTPD_bug = FALSE;
static int login_nodisplay = FALSE;


/* HTFTPClearCache ()
**  Expects: Nothing
**  Returns: Nothing
*/
void HTFTPClearCache(void)
{
    ftpcache.password[0] = '\0';
    ftpcache.control = -1;
    ftpcache.ftp_bug = 0;
    ftpcache.host[0] = '\0';
    ftpcache.username[0] = '\0';
}


/*	Close Master (listening) socket
**	-------------------------------
*/
static void close_master_socket()
{
#ifndef DISABLE_TRACE
    if (www2Trace) 
        fprintf(stderr, "FTP: Closing master socket %d\n", master_socket);
#endif
    if (master_socket != -1)
        NETCLOSE(master_socket);
    master_socket = -1;
}


/* Close control and master sockets if open */
static void close_control()
{
    if (control != -1) {
	NETCLOSE(control);
	control = -1;
    }
    ftpcache.control = -1;
    ftpcache.ftp_bug = 0;

    if (master_socket != -1)
        close_master_socket();
}


/* Timer routine to close cached sockets after some period */
static void close_it_up()
{
    NETCLOSE(ftpcache.control);
    ftpcache.control = -1;
    ftpcache.ftp_bug = 0;
    if (master_socket != -1)
        close_master_socket();
    fTimerStarted = 0;
}


/* Convert_VMS_FTP_name  Convert a filename to use VMS directory syntax
** Expects:    *name is a pointer to a string that consists of the FTP
**             filename, including the path.
** Returns     pointer to the new name
*/
PRIVATE char *Convert_VMS_FTP_name (char *name)
{
   char *filename = name;
   char *ptr;

   if (!strcmp(filename, "/")) {
      StrAllocCopy(filename, "[]");
   } else {
      /*
      ** Now we are in trouble.  Convert the UNIX name to a VMS name.  I
      ** guess we can safely assume that we always start in the server's
      ** topdirectory.  Therefore a construct like /dir1/dir2.../dirn/name.type
      ** has to be converted to [.dir1.dir2..dirn]name.type.
      **
      ** Following added by PGE.
      ** /dir1/dir2.../dirn converted to [.dir1.dir2..dirn] is handled later.
      ** Also /x becomes x so: /name.type becomes name.type, /dir1 becomes
      ** dir1, and /disk:[rootdir.dir1]name.type becomes
      ** disk:[rootdir.dir1]name.type.
      **
      ** Next /disk:[rootdir.dir1]/dir2/file.type becomes
      ** disk:[rootdir.dir1.dir2]file.type, and /disk:[rootdir.dir1]/dir2/dir3
      ** becomes disk:[rootdir.dir1.dir2]dir3 which is converted to
      ** disk:[rootdir.dir1.dir2.dir3] later.
      */
      char *file_temp = (char *)malloc(strlen(filename) + 4);
      int i1 = 0;
      int i2 = 1;
      int i3 = 2;
      int i4 = 0;

      *file_temp = '[';
      file_temp[1] = '.';
      if (*filename == '/')
	 i1 = 1;      /* Always true, PGE */
      for (; i1 < strlen(filename); i1++, i3++) {
         if (filename[i1] != '/') {
            /* Handle /disk:[rootdir.dir1]/dir2/file.type, PGE */
            if ((filename[i1] != ']') || !strchr(&filename[i1 + 1], '/')) {
               /* Normal, copy the character. */
               file_temp[i3] = filename[i1];
            } else {
               /*
	       ** Handle /disk:[rootdir.dir1]/dir2/file.type by skipping the ]
	       ** and setting i4 to remove the [. at the start of file_temp.
	       ** PGE
               */
               i3--;
               i4 = 2;
            }
            continue;
         }
         file_temp[i3] = ']';
         if (i2 > 1)
	    file_temp[i2] = '.';
         i2 = i3;
      }
      if (i2 == 1)
	 i4 = 2;	    /* PGE, remove [. at the start of file_temp */
      file_temp[i3] = ' ';  /* Add a space for later expansion room */
      file_temp[i3 + 1] = '\0';
      StrAllocCopy(filename, &file_temp[i4]);
      free(file_temp);

      /* Remove spaces from end of string. */
      file_temp = filename + strlen(filename) - 1;
      while (isspace(*file_temp))
         *file_temp-- = '\0';
   }
   if (strcmp(filename, "[]") && (ptr = strchr(filename, ']')) &&
       (ptr != (char *)strlen(filename)) && !strchr(ptr, '.')) {
      /*
      ** Handle the /dir1/dir2../dirn converted to
      ** [.dir1.dir2..dirn] case by changing
      ** [.dir1.dir2..]dirn to [.dir1.dir2...dirn]
      */

      /* Add a ] to the end */
      strcat(filename, "]");

      /* Replace first ] with a . */
      *ptr = '.';
      /* fname is [dir1.dir2...dirn] */
   }
   ptr = filename + strlen(filename) - 2;
   if (!strcmp(ptr, ".]")) {
      /*
      ** Handle the /dir1/dir2../dirn/ converted to
      ** [.dir1.dir2..dirn] case by changing
      ** [.dir1.dir2..dirn.] to [.dir1.dir2...dirn]
      */
      *ptr++ = ']';
      *ptr = '\0';
   }
   return filename;
}

/*
 * Extract the name, size, and date from an EPLF line.  - 08-06-96 DJB
 */
static void parse_eplf_line(char *line, char *type, char *name,
			    char *size, char *datetime)
{
    char *cp = line;
    char ct[26];
    time_t secs;
    static time_t base;		/* time() value on this OS in 1970 */
    static int flagbase = 0;

    if (!flagbase) {
	struct tm t;

	t.tm_year = 70;
	t.tm_mon = 0;
	t.tm_mday = 0;
	t.tm_hour = 0;
	t.tm_min = 0;
	t.tm_sec = 0;
	t.tm_isdst = -1;
	base = mktime(&t);	/* could return -1 */
	flagbase = 1;
    }
    *name = '\0';
    *type = '-';
    *size = '\0';
    *datetime = '\0';

    while (*cp) {
      switch (*cp) {
	case '\t':
	    strcpy(name, cp + 1);
	    return;
	case 's': {
	    char *start = cp + 1;
	    int len = 0;

	    while (*++cp && (*cp != ',') && (len < (BUFSIZ - 1)))
		len++;
	    strncpy(size, start, len);
	    *(size + len) = '\0';
	    break;
	}
	case 'm':
	    secs = 0;
	    while (*++cp && (*cp != ','))
		secs = (secs * 10) + (*cp - '0');
	    secs += base;	/* Assumes that time_t is #seconds */
	    strcpy(ct, ctime(&secs));
	    ct[24] = '\0';
	    strcpy(datetime, ct);
	    break;
	case '/':
	    *type = 'd';
	    /* Fallthru */
	default:
	    while (*cp) {
		if (*cp++ == ',')
		    break;
	    }
	    break;
      }
    }
}


/*	Procedure: Read a character from the data connection
**	----------------------------------------------------
*/
PRIVATE int interrupted_in_next_data_char = 0;

PRIVATE char next_data_char (void)
{
  interrupted_in_next_data_char = 0;

  if (data_read_pointer >= data_write_pointer) {
      int status = NETREAD(data_soc, data_buffer, DATA_BUFFER_SIZE);

      if (status == HT_INTERRUPTED)
          interrupted_in_next_data_char = 1;
      if (status <= 0) 
          return (char)-1;
      data_write_pointer = data_buffer + status;
      data_read_pointer = data_buffer;
  }
  return *data_read_pointer++;
}


/*	Execute Command and get Response
**	--------------------------------
**
**	See the state machine illustrated in RFC959, p57.  This implements
**	one command/reply sequence.  It also interprets lines which are to
**	be continued, which are marked with a "-" immediately after the
**	status code.
**
**	Continuation then goes on until a line with a matching reply code
**	an a space after it.
**
** On entry,
**	con	points to the connection which is established.
**	cmd	points to a command, or is NULL to just get the response.
**
**	The command is terminated with the CRLF pair.
**
** On exit,
**	returns:  The first digit of the reply type,
**		  or negative for communication failure.
*/
static int response(char *cmd)
{
  int result;				/* Three-digit decimal code */
  int status, bytes;
  int multiline_response = 0;
  int messageStarted = 0;
  char continuation;
  char *ptr;
  
  if (!control || (control == -1)) {
#ifndef DISABLE_TRACE
      if (www2Trace) 
          fprintf(stderr, "FTP: No control connection set up!!\n");
#endif
      return HT_INTERNAL;
  }
  
  if (cmd) {
#ifndef DISABLE_TRACE
      if (www2Trace) 
          fprintf(stderr, "  Tx: %s", cmd);
#endif
      
      status = NETWRITE(control, cmd, (int)strlen(cmd));
      if (status < 0) {
#ifndef DISABLE_TRACE
          if (www2Trace)
	      fprintf(stderr, 
                      "FTP: Error %d sending command: closing socket %d\n",
                      status, control);
#endif
          close_control();
          return status;
      }
  }
  
  /* Patch to be generally compatible with RFC 959 servers  -spok@cs.cmu.edu  */
  /* Multiline responses start with a number and a hyphen;
   * end with same number and a space.  When it ends, the number must
   * be flush left. */
  do {
      char *p = response_text;
      /* If nonzero, it's set to initial code of multiline response */

      for (;;) {
          int foo;
          /* This is set to 0 at the start of HTGetCharacter. */
          extern int interrupted_in_htgetcharacter;
          
          foo = (*p++ = HTGetCharacter());
          if (interrupted_in_htgetcharacter) {
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr,
			  "FTP: Interrupted in HTGetCharacter, apparently.\n");
#endif
              close_control();
              return HT_INTERRUPTED;
          }
          if ((foo == LF) || (p == &response_text[LINE_LENGTH])) {
              *p++ = '\0';              /* Terminate the string */
#ifndef DISABLE_TRACE
              if (www2Trace) 
                  fprintf(stderr, "    Rx: %s", response_text);
#endif
	      if (!strncmp(response_text, "150", 3)) {
                  if ((ptr = strrchr(response_text, '(')) && *ptr &&
		      (ptr != strchr(response_text, '('))) {
                      bytes = atoi(ptr + 1);
		  } else {
                      bytes = 0;
                  }
		  if (bytes > 0)
		      loading_length = bytes;
	      } else {
		  HTMeter(100, NULL);
	      }
	      result = 0;
              sscanf(response_text, "%d%c", &result, &continuation);

	      /* Display login text */
	      if (((result == 230) || !strncmp(response_text, "220-", 4)) &&
		  !login_nodisplay) {
		  if (!messageStarted) {
		      HText_appendText(HT, "<PRE>\n");
        	      HTProgress("Receiving message");
		      messageStarted = 1;
		  }
		  HText_appendText(HT, response_text + 4);
	      } else if (messageStarted && multiline_response &&
			 strncmp(response_text, "220 ", 4)) {
		  HText_appendText(HT, response_text);
	      }

              if (continuation == '-' && !multiline_response) {
                  multiline_response = result;
              } else if (multiline_response && continuation == ' ' &&
                         multiline_response == result &&
                         isdigit(response_text[0])) {
                  /* End of response (number must be flush on left) */
                  multiline_response = 0;
              }
	      if (result == 220 && strstr(response_text, "ProFTPD 1.2.5"))
		  ProFTPD_bug = TRUE;

              break;
          }  /* If end of line */
          
          if (*(p - 1) == (char)EOF) {
#ifndef DISABLE_TRACE
              if (www2Trace) 
                  fprintf(stderr, "Error on rx: closing socket %d\n", control);
#endif
              strcpy(response_text, "000 *** TCP read error on response\n");
              close_control();
              return -1;	/* End of file on response */
          }
      }  /* Loop over characters */
  } while (multiline_response);

  if (messageStarted)
      HText_appendText(HT, "</PRE><HR>\n");

  if (result == 421) {
#ifndef DISABLE_TRACE
      if (www2Trace) 
          fprintf(stderr, "FTP: They close so we close socket %d\n", control);
#endif
      close_control();
      return -1;
  }
  if (result == 550)
      HTProgress(response_text);

  return (result / 100);
}


/* This function turns MSDOS-like directory output off for
 * Windows NT servers.
 */
static void set_unix_dirstyle(eServerType *ServerType)
{
    /* This is a toggle.  It seems we have to toggle in order to see
     * the current state (after toggling), so we may end up toggling
     * twice.  - kw
     */
    int status = response("SITE DIRSTYLE\r\n");

    if (status != 2) {
	*ServerType = GENERIC_SERVER;
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr, "FTP: DIRSTYLE failed, treat as Generic server.\n");
#endif
	return;
    } else {
	char *cp = strstr(response_text + 4, "MSDOS");

	/* Expecting one of:
	 * 200 MSDOS-like directory output is off
	 * 200 MSDOS-like directory output is on
	 * The following code doesn't look for the full exact string -
	 * who knows how the wording may change in some future version.
	 * If the first response isn't recognized, we toggle again
	 * anyway, under the assumption that it's more likely that
	 * the MSDOS setting was "off" originally. - kw
	 */
	if (cp && strstr(cp, " off")) {
	    return;		/* Already off now. */
	} else {
	    response("SITE DIRSTYLE\r\n");
	}
    }
}


/*	Get a valid connection to the host
**	----------------------------------
**
** On entry,
**	arg	points to the name of the host in a hypertext address
** On exit,
**	returns	<0 if error
**		socket number if success
**
**	This routine takes care of managing timed-out connections, and
**	limiting the number of connections in use at any one time.
**
**	It ensures that all connections are logged in if they exist.
**	It ensures they have the port number transferred.
*/

PRIVATE int get_connection (char *arg)
{
  char host[BUFSIZ];
  char username[BUFSIZ];
  char password[BUFSIZ];
  char dummy[MAXHOSTNAMELEN + 32];
  int status, con;
  int redial = 0;
  int pass_failed = 0;

  ProFTPD_bug = FALSE;

  if (!arg || !*arg) 
      return -1;
  
#ifndef DISABLE_TRACE
  if (www2Trace) 
      fprintf(stderr, "FTP: Looking for %s\n", arg);
#endif
  {
    char *p1 = HTParse(arg, "", PARSE_HOST);
    char *p2 = strrchr(p1, '@');        /* User? */
    char *pw, *un, *tmpptr;

    /* Save the actual host */
    if (strlen(p1) < 256) {		/* Sanity check */
	strcpy(host, p1);
    } else {
	free(p1);
	close_control();
	return -1;
    }
    tmpptr = strchr(host, '/');
    if (tmpptr)
	*tmpptr = '\0';

    if (p2) {
        un = p1;
        *p2 = '\0';                         /* Terminate */
        p1 = p2 + 1;                        /* Point to host */
        pw = strchr(un, ':');
        if (pw) {
            *pw++ = '\0';
	    if (strlen(pw) < BUFSIZ) {
		strcpy(password, pw);
	    } else {
		*password = '\0';
	    }
        } else {
	    *password = '\0';
	}
	if (strlen(un) < BUFSIZ) {
	    strcpy(username, un);
	} else {
	    *username = '\0';
	}
    } else {
	*username = '\0';
	*password = '\0';
    }
    /* Copy hostname into dummy URL, since username:password@ 
     * might have been part of original */
    if (strlen(p1) < (MAXHOSTNAMELEN + 24)) {	  /* Sanity check */
	sprintf(dummy, "ftp://%s", p1);
    } else {
	free(p1);
	close_control();
	return -1;
    }
#ifndef DISABLE_TRACE
    if (www2Trace)
    	fprintf(stderr, "FTP: set dummy to %s\n", dummy);
#endif

    /* Is the cache connection still good? */
    if (ftpcache.control != -1) {
	if (*ftpcache.host && !strcmp(ftpcache.host, host)) {
	    /* Did URL have a username? */
	    if (!*username) {
#ifndef DISABLE_TRACE
	        if (www2Trace)
		    fprintf(stderr,
			    "FTP: Cached connection using cached username\n");
#endif
	        if (ftpcache.username[0] &&
		    strcmp(ftpcache.username, "anonymous") &&
		    strcmp(ftpcache.username, "ftp")) {
		    /* Not anon login...assuming a real login */
	            securityType = HTAA_LOGIN;
	        } else {
		    securityType = HTAA_NONE;
	        }
	        free(p1);
		ProFTPD_bug = ftpcache.ftp_bug;
	        return(ftpcache.control);
	    }

	    /* Username but no password? */
	    if (!*password && ftpcache.password[0] &&
	        ftpcache.username[0] && !strcmp(ftpcache.username, username)) {
#ifndef DISABLE_TRACE
	        if (www2Trace)
		    fprintf(stderr,
			    "FTP: Cached connection using cached password\n");
#endif
	        if (strcmp(ftpcache.username, "anonymous") &&
		    strcmp(ftpcache.username, "ftp")) {
		    /* Not anon login...assuming a real login */
	            securityType = HTAA_LOGIN;
	        } else {
		    securityType = HTAA_NONE;
	        }
	        free(p1);
		ProFTPD_bug = ftpcache.ftp_bug;
	        return(ftpcache.control);
	    }

	    /* Is the username and password used the same? */
	    if (ftpcache.username[0] && !strcmp(ftpcache.username, username) &&
	        *password && ftpcache.password[0] &&
	        !strcmp(ftpcache.password, password)) {
	        /* For security Icon */
	        if (strcmp(username, "anonymous") && strcmp(username, "ftp")) {
		    /* Not anon login...assuming a real login */
		    securityType = HTAA_LOGIN;
	        } else {
		    securityType = HTAA_NONE;
	        }
#ifndef DISABLE_TRACE
	        if (www2Trace)
		    fprintf(stderr, "FTP: Cached connection with username %s\n",
			    username);
#endif
	        free(p1);
		ProFTPD_bug = ftpcache.ftp_bug;
	        return(ftpcache.control);
	    }
	}
	/* Cached connection is not useable.  Close it. */
	close_control();
    }

    strcpy(ftpcache.host, host);
    if (*username) {
	strcpy(ftpcache.username, username);
    } else {
	ftpcache.username[0] = '\0';
    }
    if (*password) {
	strcpy(ftpcache.password, password);
    } else {
	ftpcache.password[0] = '\0';
    }
    free(p1);
  }

  /* Default the redial values if out of range */
  if (ftpRedial < 0)
      ftpRedial = 0;
  if (ftpRedialSleep < 1)
      ftpRedialSleep = 1;

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "FTP: New connection with username %s\n", username);
#endif

 redialFTP:

  con = -1;
  status = HTDoConnect(dummy, "FTP", IPPORT_FTP, &con);
  
  if (status < 0) {
#ifndef DISABLE_TRACE
      if (www2Trace) {
          if (status == HT_INTERRUPTED) {
              fprintf(stderr, "FTP: Interrupted on connect\n");
          } else {
              fprintf(stderr, "FTP: Unable to connect to host for `%s'\n", arg);
	  }
      }
#endif
      if (status == HT_INTERRUPTED) {
          HTProgress("Connection interrupted.");
      } else {
          HTProgress("Unable to connect to remote host.");
	  status = HT_NO_ACCESS;
      }
      if (con != -1)
	  NETCLOSE(con);
      return status;			/* Failure return */
  }

#ifndef DISABLE_TRACE
  if (www2Trace) 
      fprintf(stderr, "FTP: Connected, control socket = %d\n", con);
#endif

  /* Current control connection */
  ftpcache.control = control = con;

  /* Initialize buffering for control connection */
  HTInitInput(con);
  
  /* Now we log in; Look up username, prompt for pw. */
  {
    int status = response(NULL);  /* Get greeting */

    if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
        if (www2Trace)
            fprintf(stderr, "FTP: Interrupted at start of login.\n");
#endif
	goto Interrupted;
    }
    ftp_type[0] = '\0';		/* Default to Unix */
    server_type = GENERIC_SERVER;

    if (status == 2) {		/* Send username */
        char *command;

        /*
         * Try to figure out what type of server we deal with.  Only MultiNet
         * and MadGoat seem to announce themself properly (IMHO).  Just take
	 * a chance for UCX and TWG.
         */
        ftp_root_directory[0] = '\0';   /* root directory is unknown */
#ifndef DISABLE_TRACE
        if (www2Trace)
	    fprintf(stderr, "FTP: login response is %s\n", response_text);
#endif
        if (strstr(response_text, "MultiNet")) {
	    strcpy(ftp_type, "MultiNet");
        } else if (strstr(response_text, "MadGoat") ||
		   strstr(response_text, "HGFTP")) {
	    strcpy(ftp_type, "MadGoat");
        } else if (strstr(response_text, "UCX") ||
		   strstr(response_text, "FTP Server (V")) {
	    strcpy(ftp_type, "UCX");
        } else if (strstr(response_text, "Process Software")) {
	    strcpy(ftp_type, "TCPware");
        } else if (strstr(response_text, "CMU")) {
	    strcpy(ftp_type, "CMU");
	/*
	 * Pathway comment:
	 * We have to assume... we have gone out of our way to make it so
	 * nobody can tell when our server is available on a machine.  Ie.,
	 * configurable greeting.  So if it is not them, it could be us.
	 */
        } else if (strstr(response_text, "Wollongong")) {
	    strcpy(ftp_type, "TWG");
	}
	if (ftp_type[0])
		server_type = VMS_SERVER;

        if (*username) {
	    if (!*password) {
		char *pw;

		pw = prompt_for_password("Please Enter Your FTP Password:");
		if (pw && *pw) {
		    strcpy(password, pw);
		    strcpy(ftpcache.password, password);
		    free(pw);
		} else {
		    *password = '\0';
		    *ftpcache.password = '\0';
        	    HTProgress("Connection aborted.");
        	    close_control();
		    if (pw)
		        free(pw);
        	    return HT_INTERRUPTED;
		}
	    }
            command = (char *)malloc(10 + strlen(username) + 2 + 1);
            sprintf(command, "USER %s\r\n", username);
        } else {
            command = strdup("USER anonymous\r\n");
        }
        status = response(command);
        free(command);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
                fprintf(stderr, "FTP: Interrupted while sending username.\n");
#endif
	    goto Interrupted;
        }
    }
    if (status == 3) {		/* Send password */
        char *command;

        if (*password) {
            command = (char *)malloc(10 + strlen(password) + 2 + 1);
            sprintf(command, "PASS %s\r\n", password);
        } else {
            char *user = (char *)getenv("USER");
            extern char *machine_with_domain;
            char *host = machine_with_domain;

	    if (!user)
		user = "WWWuser";
            /* If not fully qualified, suppress it as ftp.uu.net
             * prefers a blank to a bad name */
            if (!strchr(host, '.'))
	        host = "";
            command = (char *)malloc(strlen(user) + strlen(host) + 8 + 1);
            sprintf(command, "PASS %s@%s\r\n", user, host);
        }
        status = response(command);
        free(command);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
                fprintf(stderr, "FTP: Interrupted while sending password.\n");
#endif
	    goto Interrupted;
        }
	if ((status == 5) && !strncmp(response_text, "530", 3))
	    pass_failed = 1;
    }
    if (status == 3) {
        status = response("ACCT noaccount\r\n");
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
                fprintf(stderr, "FTP: Interrupted while sending ACCT.\n");
#endif
	    goto Interrupted;
        }
    }
    if (status != 2) {
	if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
	    if (www2Trace)
		fprintf(stderr, "FTP: Interrupted in redial attempt.\n");
#endif
	    goto Interrupted;
	}
	if (*username && strcmp(username, "anonymous")) {
	    HText_appendText(HT, "<H2>FTP login using username \"");
	    HText_appendText(HT, username);
	    HText_appendText(HT, "\" failed.</H2><BR>");
	    if (*password) {
		HText_appendText(HT,
		    "If you have a login on this machine please check ");
		HText_appendText(HT,
		    "to make sure the password you are specifying is correct.");
	    } else {
		HText_appendText(HT,
		    "This is probably because you didn't specify a password ");
		HText_appendText(HT,
		    "along with your username.<BR>To do this you have to ");
		HText_appendText(HT, "specify the FTP line like this:<BR>");
		HText_appendText(HT,
		    "<P>ftp://username:password@ftp_site/<P><strong>OR");
		HText_appendText(HT,
		    "</strong><P>You can now just specify a username ");
		HText_appendText(HT, "and you will be prompted for your ");
		HText_appendText(HT,
		    "password.<P>e.g. ftp://username@ftp_site/");
	    }
	} else if (pass_failed &&
		   (!*username || !strcmp(username, "anonymous"))) {
	    /* No point in retrying if password failure */
	    HText_appendText(HT, "<H2>Anonymous FTP login failed.</H2>");
	} else {
	    char buf[BUFSIZ];

	    if (redial < ftpRedial) {
		/* Close down current connection */
		close_control();

		/* Tell them in the progress string */
		sprintf(buf,
		     "Login failed. Redial attempt %d/%d. Sleeping %d seconds.",
		      redial, ftpRedial, ftpRedialSleep);
		HTProgress(buf);
#ifndef DISABLE_TRACE
		if (www2Trace) 
		    fprintf(stderr, "FTP: Redial login fail: %s",
			    response_text);
#endif

/* Commented out until we get a new "sleep" routine...SWP

		if (my_sleep(ftpRedialSleep, 1)) {
#ifndef DISABLE_TRACE
		    if (www2Trace)
			fprintf(stderr,
			  "FTP: Interrupted in sleep during redial attempt.\n");
#endif
		    goto Interrupted;
		}
*/
		/* Index redial and try again */
		redial++;
		goto redialFTP;
	    }

	    /* Printout message and stop retrying */
	    sprintf(buf,
	      "<H2>Anonymous FTP login failed.<br><br>There were %d redial attempts made.</h2>",
	      redial);
	    HText_appendText(HT, buf);
	}
        HTProgress("Login failed");
#ifndef DISABLE_TRACE
        if (www2Trace) 
            fprintf(stderr, "FTP: Login fail: %s", response_text);
#endif
	HText_appendText(HT,
		         "\n\n<hr><p>Reason for Failure:<br><br><plaintext>\n");
	HText_appendText(HT, response_text);
        close_control();
        return HT_FORBIDDEN;		/* Bad return */
    }
#ifndef DISABLE_TRACE
    if (www2Trace) 
        fprintf(stderr, "FTP: Logged in.\n");
#endif
    /* If MultiNet, then force it to VMS directory style. GEC */
    if (!strcmp(ftp_type, "MultiNet") || !strcmp(ftp_type, "MadGoat")) {
        status = response("CWD []\r\n");
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
                fprintf(stderr, "FTP: Interrupted while sending CD [].\n");
#endif
	    goto Interrupted;
        }
    }
    /*
     * Try to determine the ftp server type if not already done.
     * TWG replies with the present directory in VMS format.
     */
#ifndef DISABLE_TRACE
    if (www2Trace && (server_type != VMS_SERVER))
	fprintf(stderr, "FTP: login response2 is %s\n", response_text);
#endif
    if ((server_type != VMS_SERVER) && strstr(response_text, "[") &&
	strstr(response_text, "]")) {
        strcpy(ftp_type, "TWG");
	server_type = VMS_SERVER;
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr, "FTP: Set to TWG server\n");
#endif
    }
    /*
     * Try a remotehelp site to see if it is UCX
     */
    if (server_type != VMS_SERVER) {
        status = response("HELP site\r\n");
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
                fprintf(stderr, "FTP: Interrupted while sending HELP site.\n");
#endif
	    goto Interrupted;
        }
        if (strstr(response_text,
		   "214 Syntax: SITE <sp> Vms | ULtrix | UNix")) {
            strcpy(ftp_type, "UCX");
	    server_type = VMS_SERVER;
	}
    }

    /* Need to know login directory, sys$login doesn't work on MultiNet. PGE */
    if (server_type == VMS_SERVER) {
        status = response("PWD\r\n");
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
                fprintf(stderr, "FTP: Interrupted while sending PWD.\n");
#endif
	    goto Interrupted;
        }
        if (strstr(response_text, "257 \"")) {
            sscanf(response_text, "257 \"%99s", ftp_root_directory);
            if (!ftp_root_directory[0]) {
                strcpy(ftp_root_directory, "sys$login");
            } else {
                /* Strip closing quote. */
                ftp_root_directory[strlen(ftp_root_directory) - 1] = '\0';
	    }
#ifndef DISABLE_TRACE
            if (www2Trace)
		fprintf(stderr, "FTP: root directory is %s.\n",
		        ftp_root_directory);
#endif
        }
    } else if (response("SYST\r\n") == 2) {
	if (!strncmp(response_text + 4, "Windows_NT", 10)) {
	    server_type = WINDOWS_NT_SERVER;
	    set_unix_dirstyle(&server_type);
	} else if (!strncmp(response_text + 4, "Windows2000", 11)) {
	    server_type = WINDOWS_2K_SERVER;
	    set_unix_dirstyle(&server_type);
	} else if (strstr(response_text + 4, "FileZilla")) {
	    server_type = FILEZILLA_SERVER;
	    ftpcache.ftp_bug = ProFTPD_bug = TRUE;
	} else if (strstr(response_text + 4, "UNIX Type: L8")) {
	    /* Common UNIX server where LIST command should be okay */
	    server_type = UNIX_L8_SERVER;
	} else if (strstr(response_text + 4, "UNIX ")) {
	    server_type = UNIX_SERVER;
	}
    }
#ifndef DISABLE_TRACE
    if (www2Trace) {
	switch (server_type) {
            case VMS_SERVER:
        	fprintf(stderr, "FTP: Server type is %s.\n", ftp_type);
		break;
            case WINDOWS_NT_SERVER:
        	fprintf(stderr, "FTP: Server type is Windows NT.\n");
		break;
            case WINDOWS_2K_SERVER:
        	fprintf(stderr, "FTP: Server type is Windows 2K.\n");
		break;
            case FILEZILLA_SERVER:
        	fprintf(stderr, "FTP: Server type is FileZilla.\n");
		break;
            case UNIX_L8_SERVER:
        	fprintf(stderr, "FTP: Server type is UNIX L8.\n");
		break;
            case UNIX_SERVER:
        	fprintf(stderr, "FTP: Server type is UNIX.\n");
		break;
	    default:
        	fprintf(stderr, "FTP: Server type is Generic.\n");
        }
    }
#endif

    /* For security Icon */
    if (*username && strcmp(username, "anonymous") && strcmp(username, "ftp")) {
	/* Not anon login... assuming a real login */
	securityType = HTAA_LOGIN;
    } else {
	securityType = HTAA_NONE;
    }
  }
  return con;			/* Good return */

 Interrupted:
  HTProgress("Connection interrupted.");
  close_control();
  return HT_INTERRUPTED;
}


/*	Open a master socket for listening on
**	-------------------------------------
**
**	When data is transferred, we open a port and wait for the server
**	to connect with the data.
**
** On entry,
**	master_socket	Must be negative if not set up already.
** On exit,
**	Returns		socket number if good
**			less than zero if error.
**	master_socket	is socket number if good, else negative.
**	port_number	is valid if good.
*/
static int get_listen_socket()
{
  struct sockaddr_in soc_address;	/* Binary network address */
  struct sockaddr_in *sin = &soc_address;
  int new_socket;			/* Will be master_socket */

  /* Create internet socket */
  new_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (new_socket < 0)
      return -1;
  
#ifndef DISABLE_TRACE
  if (www2Trace) 
      fprintf(stderr, "FTP: Opened master socket number %d\n", new_socket);
#endif
    
  /* Search for a free port. */
  sin->sin_family = AF_INET;	       /* Family = internet, host order  */
  sin->sin_addr.s_addr = INADDR_ANY;   /* Any peer address */
  {
    int status;
#if !defined(VMS) || (__DECC_VER < 50230003)
    int address_length = sizeof(soc_address);
#else
    size_t address_length = sizeof(soc_address);
#endif

#ifdef SOCKS
    status = Rgetsockname(control,
#else
    status = getsockname(control,
#endif
                         (struct sockaddr *)&soc_address, &address_length);
    if (status < 0) 
        return -1;

#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "FTP: This host is %s\n", HTInetString(sin));
#endif

    soc_address.sin_port = 0;  /* Unspecified: please allocate */
#ifdef SOCKS
    status = Rbind(new_socket,
#else
    status = bind(new_socket,
#endif
                  (struct sockaddr *)&soc_address,
                /* Cast to generic sockaddr */
#ifdef SOCKS
                  sizeof(soc_address), SOCKS_ftpsrv.s_addr);
#else
                  sizeof(soc_address));
#endif
    if (status < 0) 
        return -1;
      
    address_length = sizeof(soc_address);
#ifdef SOCKS
    status = Rgetsockname(new_socket,
#else
    status = getsockname(new_socket,
#endif
                          (struct sockaddr *)&soc_address, &address_length);
    if (status < 0) 
        return -1;
  }
  
#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "FTP: bound to port %d on %s\n",
	      (int)ntohs(sin->sin_port), HTInetString(sin));
#endif

  if (master_socket >= 0)
      close_master_socket();
  master_socket = new_socket;
  
  /* Now we must find out who we are to tell the other guy */
  (void)HTHostName(); 	/* Make address valid - doesn't work */
  sprintf(port_command, "PORT %d,%d,%d,%d,%d,%d\r\n",
          (int)*((unsigned char *)(&sin->sin_addr)),
          (int)*((unsigned char *)(&sin->sin_addr) + 1),
          (int)*((unsigned char *)(&sin->sin_addr) + 2),
          (int)*((unsigned char *)(&sin->sin_addr) + 3),
          (int)*((unsigned char *)(&sin->sin_port)),
          (int)*((unsigned char *)(&sin->sin_port) + 1));
  
  /* Inform TCP that we will accept connections */
#ifdef SOCKS
  if (Rlisten(master_socket, 1) < 0) {
#else
  if (listen(master_socket, 1) < 0) {
#endif
      close_master_socket();
      return -1;
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "FTP: Master socket(), bind() and listen() all OK\n");
#endif

  return master_socket;		/* Good */
}


/* Set up a connection including a listen socket */
static int setup_connection(char *name)
{
  int retry, status;

  for (retry = 0; retry < 2; retry++) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr,
		  "FTP: setup_connection: retry attempt %d\n", retry);
#endif
      status = get_connection(name);
      if (status < 0)
          /* HT_INTERRUPTED falls through */
          goto error_exit;

 try_passive:
      if (!ftp_passive) {
          status = get_listen_socket();
          if (status < 0) {
	      HText_appendText(HT, "FTP terminated because login failed");
	      HTProgress("Login failed");
              /* HT_INTERRUPTED would fall through, if we could interrupt
               * somehow in the middle of it, which we currently can't. */
              goto error_exit;
          }
      
          /* Inform the server of the port number we will listen on */
          status = response(port_command);
          if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
              if (www2Trace)
                 fprintf(stderr,
			 "FTP: Interrupted in response (port_command)\n");
#endif
	      HTProgress("Connection interrupted.");
	      goto error_exit;
          }
	  if ((status == 5) && !strncmp(response_text, "502", 3)) {
	      /* 502 response means port command not permitted,
	       * so passive is only option */
	      ftp_passive = TRUE;
	      goto try_passive;
	   }
          if (status != 2) {		/* Could have timed out */
              if (status < 0) {
                  close_control();
                  continue;		/* Try again - net error */
              }
              status = HT_FAILED; 	/* Bad reply */
	      goto error_exit;
          }
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "FTP: Port defined.\n");
#endif
      } else {		/* Tell server to be passive */
	  char *command = NULL;
	  char *p;
	  int h0, h1, h2, h3, p0, p1;
	  int passive_port;   /* Port server specified for data */

	  data_soc = status;
          status = response("PASV\r\n");
          if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
              if (www2Trace)
                 fprintf(stderr,
			 "FTP: Interrupted in response (PASV)\n");
#endif
	      HTProgress("Connection interrupted.");
	      goto error_exit;
          }
          if (status != 2) {
              if (status < 0)
                  continue;   /* Retry or Bad return */
              status = HT_FAILED;       /* Bad reply */
	      goto error_exit;
          }
          for (p = response_text; *p && *p != ','; p++)
              ;
          while (--p > response_text && '0' <= *p && *p <= '9')
              ;
          status = sscanf(p + 1, "%d,%d,%d,%d,%d,%d",
                          &h0, &h1, &h2, &h3, &p0, &p1);
          if (status < 4) {
#ifndef DISABLE_TRACE
	      if (www2Trace)
                  fprintf(stderr, "FTP: PASV reply has no inet address!\n");
#endif
              status = HT_FAILED;
              goto error_exit;
          }
          passive_port = (p0 << 8) + p1;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "FTP: Server listening on port %d.\n",
		      passive_port);
#endif
	  /* Open connection for data */
	  command = malloc(128);
	  sprintf(command, "ftp://%d.%d.%d.%d:%d/",
		  h0, h1, h2, h3, passive_port);
	  status = HTDoConnect(command, "FTP data", passive_port, &data_soc);
	  free(command);
	  if (status < 0) {
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, "FTP: Connect to passive port failed.\n");
#endif
	      NETCLOSE(data_soc);
	      data_soc = -1;
	      goto error_exit;
	  }
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "FTP: data connected on socket %d.\n", data_soc);
#endif
      }
      status = HT_OK;  /* Success */
      break;
  }  /* For retries */

  return status;

 error_exit:
  close_control();
  return status;
}


/*	Read a directory into an hypertext object from the data socket
**	--------------------------------------------------------------
**
** On entry,
**	anchor		Parent anchor to link the this node to
**	address		Address of the directory
** On exit,
**	returns		HT_LOADED if OK
**			<0 if error.
**
** Author: Charles Henrich (henrich@crh.cl.msu.edu)  October 2, 1993
**
** Completely re-wrote this chunk of code to present FTP directory information
** in a much more useful manner.  Also included the use of icons. -Crh
*/
PRIVATE int read_directory (HTParentAnchor *parent,
			    WWW_CONST char *address,
			    HTFormat format_out,
			    int	do_header)
{
  HTFormat format;
  HTAtom *pencoding;
  char *filename = HTParse(address, "", PARSE_PATH + PARSE_PUNCTUATION);
  char *full_ftp_name, *ptr;
  char itemtype;
  char buffer[BUFSIZ], buf[BUFSIZ];
  char itemname[BUFSIZ], itemsize[BUFSIZ];
  char ellipsis_string[1024];
  char szDate[256], szTime[32], szFileInfo[32];
  char szMonth[32], szDay[16], szYear[32];
  int count, ret, cmpr, c, rv, nTime;
  int lfirst = 0;
  int lcont = 0;
  int first_line = 1;
  int dcount = 0;
  static HTAtom *www_source;
  static int init = 0;

  if (!init) {
      www_source = WWW_SOURCE;
      init = 1;
  }

  data_read_pointer = data_write_pointer = data_buffer;

  if (do_header) {
      HTProgress("Reading FTP directory");

      HText_appendText(HT, "<H1>FTP Directory ");
      HText_appendText(HT, filename);
      HText_appendText(HT, "</H1>\n<PRE><DL>\n");

      /* If this is not the root level, spit out a parent directory entry */
      if (strcmp(filename, "/")) {
          HText_appendText(HT, "<DD><A HREF=\"");
     
          strcpy(buffer, filename);
          ptr = strrchr(buffer, '/');
          if (ptr)
              *ptr = '\0';

          if (!buffer[0]) {
              HText_appendText(HT, "/");
          } else {
              HText_appendText(HT, buffer);
          }
          HText_appendText(HT, "\"><IMG SRC=\"");
          HText_appendText(HT, HTgeticonname(NULL, "directory"));
          HText_appendText(HT, "\"> Parent Directory</a>\n");
      }
  }

  /* Loop until we hit EOF */
  while (1) {
      /* Read in a line of data */
      for (count = 0; count < BUFSIZ; count++) {
          c = next_data_char();
          if (interrupted_in_next_data_char)
	      goto read_interrupted;
          if (c == '\r') {
              c = next_data_char();
              if (interrupted_in_next_data_char)
		  goto read_interrupted;
              if (c != '\n') 
                  break;
          }
          if (c == '\n') { 
	      /* Handle VMS server long line wrapping */
	      if (count && (server_type == VMS_SERVER)) {
	          if ((data_read_pointer < data_write_pointer) &&
		      (*data_read_pointer == ' ')) {
		      data_read_pointer++;
	          } else if (data_read_pointer >= data_write_pointer) {
		      int status;

		      status = NETREAD(data_soc, data_buffer, DATA_BUFFER_SIZE);
		      if (status == HT_INTERRUPTED)
		          goto read_interrupted;
		      if (status <= 0) {
		          c = (char)-1;
		          break;
		      }
		      data_write_pointer = data_buffer + status;
		      data_read_pointer = data_buffer;
		      if (*data_read_pointer == ' ') {
		          data_read_pointer++;
		      } else { 
		         break;
		      }
	          } else {
		      break;
		  }
	      } else {
	          break;
	      }
          } else if (c == (char)EOF) {
              break;
          }
          buffer[count] = c;
      }
      if (c == (char)EOF)
          break;

      buffer[count] = '\0';
      
#ifndef DISABLE_TRACE
      if (www2Trace)
	  fprintf(stderr, "FTP: Rx: %s\n", buffer);
#endif
      /* Parse the input buffer, extract the item type, and the item size */
      /* Retain whole string -- we don't use it at the moment, but we will. */
      full_ftp_name = strdup(buffer);

      /* Read but disregard itemsize -- this effectively guarantees we will know
       * what we should display and what we shouldn't -- don't ask. */
      if (usingNLST == 1) {
	  ret = sscanf(buffer, "%c%*9s %*d %*s %*s %s", &itemtype, itemsize);
	  if (ret != 2) {
	      if (first_line && (!strncmp(buffer, ".", 1) ||
				 (server_type == FILEZILLA_SERVER))) {
		  /* Appears to be a name only list */
		  usingNLST = 2;
	      } else {
	          free(full_ftp_name);
	          continue;
	      }
	  }
	  first_line = 0;
      }
      if (usingNLST == 2) {	/* Only name */
	  if (!strcmp(buffer, ".") || !strcmp(buffer, "..")) {
	      free(full_ftp_name);
	      continue;
	  }
	  ptr = strrchr(buffer, '.');
	  itemtype = '-';
	  if (ptr && *ptr) {
	      if (!my_strncasecmp(ptr, ".dir", 4)) {
		  *ptr = '\0';
		  itemtype = 'd';
	      }
	  }
      } else if (!usingNLST) {	/* Using LIST command */
        if (server_type != VMS_SERVER) {
	  ret = sscanf(buffer, "%c%*9s %*d %*s %*s %s", &itemtype, itemsize);
	  if ((ret != 2) && (itemtype != '+')) {
	      /* Neither normal list or EPLF */
	      free(full_ftp_name);
	      continue;
	  }
	} else {
          /* MultiNet, MadGoat, and UCX servers LIST support. */
          char date1[16], date2[16];

          /* UCX is somewhat different, handle here. PGE */
          if (!strcmp(ftp_type, "UCX")) {
             ret = sscanf(buffer, "%s %s %s", itemname, itemsize, date1);

             /* Skip lines which don't parse right. */
             if ((ret != 3) || !strchr(itemname, '.') ||
                 !strchr(itemsize, '/') || !strchr(date1, '-')) {
                 free(full_ftp_name);
                 continue;
             }

             /* Keep date a constant 11 characters. */
             if (strlen(date1) != 11) {
                 strcpy(szMonth, " ");
                 strcat(szMonth, date1);
             } else {
                 strcpy(szMonth, date1);
             }
             szTime[0] = '\0';               /* UCX gives no time. */
             ptr = strchr(itemsize, '/');    /* size is "size/allocated" */
             if (ptr)
		 *ptr = '\0';

             /* Mark type of file for later processing. */
             if (strstr(itemname, ".DIR") || strstr(itemname, ".dir")) {
                 itemtype = 'd';
                 ptr = strchr(itemname, '.');
                 *ptr = '\0';
             } else {
                 itemtype = '-';
	     }
          } else {
             /* MultiNet and MadGoat servers LIST support. */
             if (lcont) {
                 ret = sscanf(buffer, " %s %s %s", itemsize, date1, date2);
             } else {
                 ret = sscanf(buffer, "%s %s %s %s", itemname, itemsize, date1,
			      date2);
	     }
	     /* Check for empty directory */
	     if (!lcont && (ret >= 1) && !strncmp(itemname, "*.*;", 4)) {
                 free(full_ftp_name);
                 continue;
             }
             if ((ret == 1) && !lfirst) {
                 lfirst = 1;
                 free(full_ftp_name);
                 continue;
             }
             if ((ret == 1) && lfirst && strchr(itemname, '.')) {
                 lcont = 1;
                 free(full_ftp_name);
                 continue;
             }
             if (((ret != 4) && !lcont) || !strchr(itemname, '.')) {
                 free(full_ftp_name);
                 continue;
             }
             if ((ret != 3) && lcont) {
                 free(full_ftp_name);
                 continue;
             }
             lcont = 0;
             if (strlen(date1) != 11) {
                 strcpy(szMonth, " ");
                 strcat(szMonth, date1);
             } else {
                 strcpy(szMonth, date1);
             }
             strcpy(szTime, date2);
             if (strstr(itemname, ".DIR") || strstr(itemname, ".dir")) {
                 itemtype = 'd';
                 ptr = strchr(itemname, '.');
                 *ptr = '\0';
             } else {
                 itemtype = '-';
	     }
          }
	}
      }

      if (!buffer || !*buffer) {
	  continue;
      } else {
	  dcount++;
      }
      if (usingNLST == 2) {  /* Only name */
	  strcpy(itemname, buffer);
	  nTime = -1;
      } else if (server_type == VMS_SERVER) {
          nTime = -2;
      } else if (itemtype != '+') {
	  /* Due to the various time stamp formats, it's "safer" to retrieve
	   * the filename by taking it from the right side of the string,
           * so we do that here. */
	  if (!(ptr = strrchr(buffer, ' ')))
	      continue;
	  strcpy(itemname, ptr + 1);

	  if (!strcmp(itemname, ".") || !strcmp(itemname, "..")) {
	      free(full_ftp_name);
	      continue;
	  }
	  nTime = ParseDate(buffer, szFileInfo, szMonth, szDay, szYear, szTime);
	  if (usingNLST == 1)
	      ParseFileSizeAndName(buffer, itemname, itemsize);

	  if (nTime == 3) {  /* A dos or NT server possibly */
	      if (!ParseFileSizeAndName(buffer, itemname, itemsize)) {
		  itemtype = 'd';
	      } else {
		  itemtype = '-';
	      }
	  }
      } else {
	  nTime = 4;
	  parse_eplf_line(buffer, &itemtype, itemname, itemsize, szTime);
      }

      /* Spit out the anchor reference, and continue on... */
      HText_appendText(HT, "<DD><A HREF=\"");
      /* Assuming it's a relative reference... */
      if (itemname && (*itemname != '/')) {
          HText_appendText(HT, filename);
          if (filename[strlen(filename) - 1] != '/') 
              HText_appendText(HT, "/");
      }
      HText_appendText(HT, itemname);
      HText_appendText(HT, "\">");
      
      /* There are 3 "types", directory, link and file.  If it's a directory we
       * just spit out the name with a directory icon.  If it's a link, we go
       * retrieve the proper name (i.e., input looks like bob -> ../../../bob)
       * so we want to hop past the -> and just grab bob.  The link case falls
       * through to the filetype case.  The filetype shows name and filesize,
       * and then attempts to select the correct icon based on file extension.
       */
      switch (itemtype) {
        case 'd':
	    if ((usingNLST != 2) &&
		(compact_string(itemname, ellipsis_string, ftpFilenameLength,
			        ftpEllipsisMode, ftpEllipsisLength) != 2)) {
                sprintf(buffer, "%s", ellipsis_string);
	    } else {
                sprintf(buffer, "%s", itemname);
	    }
            HText_appendText(HT, "<IMG SRC=\"");
            HText_appendText(HT, HTgeticonname(NULL, "directory"));
            HText_appendText(HT, "\"> ");
            break;
          
        case 'l':
            ptr = strrchr(buffer, ' ');
            if (ptr) {
                *ptr = '\0';
                ptr = strrchr(buffer, ' ');
            }
            if (ptr) {
                *ptr = '\0';
                ptr = strrchr(buffer, ' ');
            }
            if (ptr)
		strcpy(itemname, ptr + 1);

        case '-':
            /* If this is a link type, and the bytes are small, 
             * it is probably a directory so lets not show the byte count
	     */
	    if ((usingNLST != 2) &&
	        (compact_string(itemname, ellipsis_string, ftpFilenameLength,
			        ftpEllipsisMode, ftpEllipsisLength) != 2))
		strcpy(itemname, ellipsis_string);

            sprintf(buffer, "%s", itemname);

	    if (server_type == VMS_SERVER)
		strip_VMS_version(itemname);

            format = HTFileFormat(itemname, &pencoding, www_source, &cmpr);
            
            HText_appendText(HT, "<IMG SRC=\"");

            /* If this is a link, and we can't figure out what
             * kind of file it is by extension, throw up the unknown
             * icon; however, if it isn't a link and we can't figure
             * out what it is, throw up the text icon...
             * 
             * Unless it's compressed.
	     */
            if (itemtype == 'l' && cmpr == COMPRESSED_NOT) {
                /* If it's unknown, let's call it a menu since symlinks
                 * are most commonly used on FTP servers to point to
                 * directories, IMHO... -marc */
                HText_appendText(HT, HTgeticonname(format, "directory"));
            } else {
                HText_appendText(HT, HTgeticonname(format, "text")); 
            }
            HText_appendText(HT, "\"> ");
            break;

          default:
              HText_appendText(HT, "<IMG SRC=\"");
              HText_appendText(HT, HTgeticonname(NULL, "unknown"));
              HText_appendText(HT, "\"> ");
	}

	HText_appendText(HT, buffer);
	HText_appendText(HT, "</A>");

	if (usingNLST != 2) {
	    int nStringLen = strlen(buffer);
	    int nSpaces = ftpFilenameLength - nStringLen;

	    if (nTime == 1) {
		struct tm *ptr;
		time_t t = time(0);
		int day = atoi(szDay);

		ptr = localtime(&t);
		if ((!my_strncasecmp("Jan", szMonth, 3) &&
		     ((ptr->tm_mon > 0) ||
		      ((ptr->tm_mon == 0) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Feb", szMonth, 3) &&
		     ((ptr->tm_mon > 1) ||
		      ((ptr->tm_mon == 1) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Mar", szMonth, 3) &&
		     ((ptr->tm_mon > 2) ||
		      ((ptr->tm_mon == 2) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Apr", szMonth, 3) &&
		     ((ptr->tm_mon > 3) ||
		      ((ptr->tm_mon == 3) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("May", szMonth, 3) &&
		     ((ptr->tm_mon > 4) ||
		      ((ptr->tm_mon == 4) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Jun", szMonth, 3) &&
		     ((ptr->tm_mon > 5) ||
		      ((ptr->tm_mon == 5) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Jul", szMonth, 3) &&
		     ((ptr->tm_mon > 6) ||
		      ((ptr->tm_mon == 6) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Aug", szMonth, 3) &&
		     ((ptr->tm_mon > 7) ||
		      ((ptr->tm_mon == 7) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Sep", szMonth, 3) &&
		     ((ptr->tm_mon > 8) ||
		      ((ptr->tm_mon == 8) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Oct", szMonth, 3) &&
		     ((ptr->tm_mon > 9) ||
		      ((ptr->tm_mon == 9) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Nov", szMonth, 3) &&
		     ((ptr->tm_mon > 10) ||
		      ((ptr->tm_mon == 10) && (ptr->tm_mday >= day)))) ||
		    (!my_strncasecmp("Dec", szMonth, 3) &&
		     (ptr->tm_mday >= day))) {
		    sprintf(szYear, "%d", 1900 + ptr->tm_year);
		} else {
		    sprintf(szYear, "%d", 1899 + ptr->tm_year);
		}
		sprintf(szDate, "%*s%9s %s %s %s %2.2s, %s", nSpaces, " ",
			itemsize, szFileInfo, szTime, szMonth, szDay, szYear);
	    } else if (nTime == 0) {
		sprintf(szDate, "%*s%9s %s %s %s %2.2s, %s", nSpaces, " ",
			itemsize, szFileInfo, "     ", szMonth,	szDay, szYear);
	    } else if (nTime == 4) {
		sprintf(szDate, "%*s  %9.9s  %s", nSpaces, " ",
			itemsize, szTime);
	    } else {
		sprintf(szDate, "%*s  %9.9s  %s %s", nSpaces, " ",
			itemsize, szMonth, szTime);
	    }
	    HText_appendText(HT, szDate);
	}
	HText_appendText(HT, "\n");
	free(full_ftp_name);
  }
  free(filename);

  if ((usingNLST == 1) && !dcount) {
      /* It probably didn't like the -Lla and only sent an EOF, or
       * it sent only filenames */
#ifndef DISABLE_TRACE
      if (www2Trace)
	  fprintf(stderr, "FTP: NLST -Lla probably failed\n");
#endif
      broken_crap_hack = 2;
      rv = response(NULL);
      broken_crap_hack = 0;
      return HT_BAD_EOF;
  }
  HText_appendText(HT, "</DL>\n</PRE>\n");
  HText_endAppend(HT);

  /* Some servers (wu) don't return a status line at this point */
  broken_crap_hack = 2;
  rv = response(NULL);
  if (!broken_crap_hack) {
      return HT_LOADED;
  } else {
      broken_crap_hack = 0;
  }

  if (rv == HT_INTERRUPTED)
      return rv;
  return rv == 2 ? HT_LOADED : HT_FAILED;

 read_interrupted:

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "FTP: read dir interrupted\n");
#endif
      free(filename);
      return HT_INTERRUPTED;
}


#ifdef SOCKETSHR
static jmp_buf accept_env;

static void hung_accept()
{
  longjmp(accept_env, 1);
}
#endif

/*	Retrieve File from Server
**	-------------------------
**
** On entry,
**	name		WWW address of a file: document, including hostname
** On exit,
**	returns		Socket number for file if good.
**			<0 if bad.
*/
PUBLIC int HTFTPLoad (char *name,
		      HTParentAnchor *anchor,
		      HTFormat format_out,
		      HTStream *sink)
{
  BOOL isDirectory = NO;
  HTFormat format;
  char *filename;
  int status;
  int compressed = 0;
  int no_Lla = 0;
#ifdef SOCKETSHR
  int retrya = 0;
  int retryl = 0;
  int stimer = 8;
#endif
  static HTAtom *www_plaintext;
  static int init = 0;

  if (!init) {
      www_plaintext = WWW_PLAINTEXT;
      init = 1;
  }
  loading_length = -1;

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "HTFTPLoad(%s) %s connection\n", name,
	      ftp_passive ? "passive" : "normal");
#endif

#ifdef SOCKETSHR
do {
#endif
  filename = NULL;		/* For interrupted_exit: */

  HTProgress("Retrieval in progress");
  if (fTimerStarted) {
      XtRemoveTimeOut(timer);
      fTimerStarted = 0;
  }
  if (!ftp_passive || (ftp_passive && (ftpcache.control == -1))) {
      /* Also start over if passive and no cached connection because
       * we have to redo all the login stuff */
      HT = HText_new();
      HText_beginAppend(HT);
  }

 retry_dir:
  /* Get connection including listen port */
  status = setup_connection(name);

  /* Reset because could be TRUE on retry */
  login_nodisplay = FALSE;

  if (status < 0)
      return status;	/* Failed with this code */
  
  /* Ask for the file: */    
  {
    char command[LINE_LENGTH + 1];
    HTAtom *encoding;

    filename = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);

    if (!filename || !*filename) 
        StrAllocCopy(filename, "/");

    if (server_type == VMS_SERVER) {
	/* VMS FTP server, so strip version for testing file format */
	char *temp_filename = strdup(filename);

	strip_VMS_version(temp_filename);
	format = HTFileFormat(temp_filename, &encoding, www_plaintext,
		              &compressed);
        free(temp_filename);
    } else {
        format = HTFileFormat(filename, &encoding, www_plaintext, &compressed);
    }
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "FTP: filename is %s\n", filename);
#endif
    if (server_type == VMS_SERVER) {
        filename = Convert_VMS_FTP_name(filename);
#ifndef DISABLE_TRACE
        if (www2Trace)
            fprintf(stderr, "FTP: VMS filename is %s\n", filename);
#endif
    }
    /*
     * The UCX and TWG servers return data differently depending on whether
     * binary or ASCII mode is set.  MultiNet and MadGoat as well as UNIX
     * servers work well with all binary.
     * For UCX and TWG, check the setting in the Options menu, ftp binary
     * mode box.
     * CMU and TCPware are unknown for now so assume they are different.
     *
     * Give UNIX users the choice, because it does make a difference in
     * some cases.
     */
    if ((server_type != VMS_SERVER) ||
	!strcmp(ftp_type, "UCX") || !strcmp(ftp_type, "TWG") ||
        !strcmp(ftp_type, "CMU") || !strcmp(ftp_type, "TCPware")) {
        if (get_pref_boolean(eFTP_BINARY_MODE)) {
            sprintf(command, "TYPE %s\r\n", "I");
        } else {
            sprintf(command, "TYPE %s\r\n", "A");
        }
    } else {
        sprintf(command, "TYPE %s\r\n", "I");
    }

    status = response(command);
    if (status != 2) {
        if (status == HT_INTERRUPTED)
            goto interrupted_exit;
        close_control();
        free(filename);
        return HT_FAILED;
    }

    if (server_type == VMS_SERVER) {
        sprintf(command, "CWD %s\r\n", ftp_root_directory);
        status = response(command);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
		fprintf(stderr, "FTP: Interrupted while sending CWD\n");
#endif
	    goto interrupted_exit;
        }
    }

    if ((server_type != VMS_SERVER) ||
	(filename[strlen(filename) - 1] != ']')) {
	sprintf(command, "RETR %s\r\n", filename);
	status = response(command);
	if ((status >= 5) &&
	    (ProFTPD_bug || !strncmp(response_text, "503", 3))) {
	    int pstatus;

#ifndef DISABLE_TRACE
            if (www2Trace || reportBugs)
	        fprintf(stderr,
		        "FTP: broken ProFTPD, FileZilla, etc. server\n");
#endif
	    if (server_type != FILEZILLA_SERVER) {
	        close_control();

	        /* Do not redisplay login text */
	        login_nodisplay = TRUE;

	        pstatus = setup_connection(name);
	        login_nodisplay = FALSE;

	        if (pstatus < 0) {
#ifndef DISABLE_TRACE
                    if (www2Trace)
		        fprintf(stderr,
			        "FTP: failed reconnect to ProFTPD server.\n");
#endif
		    free(filename);
		    return HT_FAILED;
	        }
	        ftpcache.control = -1;	/* Must always restart when ProFTPD */
	    } else if (!ftp_passive) {
		/* FileZilla or response 503 should just need new PORT */
		pstatus = response(port_command);
		if (pstatus != 2) {
#ifndef DISABLE_TRACE
                    if (www2Trace || reportBugs)
		        fprintf(stderr,
			        "FTP: failed 2nd PORT to FileZilla server.\n");
#endif
		    free(filename);
		    close_control();
		    return HT_FAILED;
		}
	    }
	}
    } else {
	/* Don't try to retrieve a VMS directory */
	status = 5;
    }
    if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
        if (www2Trace)
            fprintf(stderr, "FTP: Interrupted while sending RETR\n");
#endif
	goto interrupted_exit;
    }

    if (status != 1) {  /* Failed : try to CWD to it */
        sprintf(command, "CWD %s\r\n", filename);
        status = response(command);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
                fprintf(stderr, "FTP: Interrupted while sending CWD\n");
#endif
	    goto interrupted_exit;
        }

	/* If we failed CWD and are using VMS server, skip it */
	if ((status == 5) && (server_type == VMS_SERVER))
	    goto dir_done;

	/* If status is 2, we successfully did a CWD */
        if (status == 2) {
	    if (server_type == UNIX_L8_SERVER) {
		isDirectory = YES;
		usingNLST = 0;
		status = response("LIST\r\n");
		if (status == 1)
		    goto dir_done;
		if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
		   if (www2Trace)
		       fprintf(stderr,
			      "FTP: Interrupt while sending LIST to UNIX L8\n");
#endif
		    goto interrupted_exit;
	        }
	    }
            if ((server_type != VMS_SERVER) && !no_Lla) {
		/* Let's NLST it */
		isDirectory = YES;
		usingNLST = 1;
		sprintf(command, "NLST %s \r\n", NLST_PARAMS);
		status = response(command);
		if (status == 1)
		    goto dir_done;
		if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
		    if (www2Trace)
			fprintf(stderr,
				"FTP: Interrupt while sending NLST -Lla\n");
#endif
		    goto interrupted_exit;
		}
	    }
            if (strcmp(ftp_type, "MultiNet") &&	strcmp(ftp_type, "MadGoat") &&
		strcmp(ftp_type, "TCPware") && strcmp(ftp_type, "UCX")) {
		isDirectory = YES;
		usingNLST = 2;
		status = response("NLST\r\n");
		if (status == 1)
		    goto dir_done;
		if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
		    if (www2Trace)
			fprintf(stderr,
				"FTP: Interrupted while sending NLST\n");
#endif
		    goto interrupted_exit;
		}
            }
	    /* Don't retry LIST on UNIX type L8 servers */
	    if (server_type != UNIX_L8_SERVER) {
		isDirectory = YES;
		usingNLST = 0;
		status = response("LIST\r\n");
		if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
		    if (www2Trace)
		        fprintf(stderr, "FTP: Interrupt while sending LIST\n");
#endif
		    goto interrupted_exit;
		}
	    }
	    if (status == 5) {
#ifdef SOCKETSHR
		if (retryl)
		    goto dir_done;
#endif
		if (ftp_passive)
		    /* Both active and passive failed */
		    HText_appendText(HT,
				     "<H2>Failed to get file list.</H2><BR>");
	    }
	}
    }

 dir_done:

    free(filename);
    filename = NULL;

    if (status != 1) {
#ifndef DISABLE_TRACE
        if (www2Trace)
	    fprintf(stderr, "FTP: LIST directory returned with status %d\n",
		    status);
#endif
	/* Keep open for retry in passive mode */
	if (ftp_passive)
            close_control();

#ifdef SOCKETSHR
	if (retryl++ != 8) {
	    sleep(5);
	    fprintf(stderr, "FTP: Directory retry %d.\n", retryl);
            close_control();
            continue;
	}
#ifndef DISABLE_TRACE
        if (www2Trace)
	    fprintf(stderr, "FTP: Retries failed.  Now return.\n");
#endif
        HTProgress("Directory list failed due to SOCKETSHR/NETLIB bug.");
#endif
        return HT_NOT_LOADED;  /* Action not started */
    }
  }
  
  if (!ftp_passive) {
    /* Wait for the connection */
    struct sockaddr_in soc_address;

#if !defined(VMS) || (__DECC_VER < 50230003)
    int soc_addrlen = sizeof(soc_address);
#else
    size_t soc_addrlen = sizeof(soc_address);
#endif
#ifdef SOCKETSHR
    if (setjmp(accept_env)) {
        close_control();
	if (retrya++ != 8) {
	    fprintf(stderr, "FTP: Retry %d.\n", retrya);
            continue;
	}
#ifndef DISABLE_TRACE
        if (www2Trace)
	    fprintf(stderr, "FTP: Retries failed.  Now return.\n");
#endif
        HTProgress("Connection failed due to SOCKETSHR/NETLIB bug.");
        return HT_NOT_LOADED;
    }
    signal(SIGALRM, hung_accept);
    alarm(stimer);
#endif /* SOCKETSHR, GEC */

#ifdef SOCKS
    status = Raccept(master_socket, (struct sockaddr *)&soc_address,
#else
    status = accept(master_socket, (struct sockaddr *)&soc_address,
#endif
#ifndef __GNUC__
                    &soc_addrlen);
#else
                    (int)&soc_addrlen);
#endif /* GNU C is picky, GEC */

#ifdef SOCKETSHR
    alarm(0);
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "FTP: Returned from accept\n");
#endif
#endif

    if (status < 0) {
        close_control();
        /* We can't interrupt out of an accept. */
        return HT_NOT_LOADED;
    }
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "FTP: Accepted new socket %d\n", status);
#endif
    data_soc = status;
  }

  if (isDirectory) {
      int s = read_directory(anchor, name, format_out, !no_Lla);

      NETCLOSE(data_soc);
      data_soc = -1;

      /* Check for NLST -Lla failure */
      if ((s == HT_BAD_EOF) && !no_Lla) {
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "FTP: Bad response.  Retrying read_directory\n");
#endif
	  no_Lla = 1;
	  login_nodisplay = TRUE;
          goto retry_dir;
      }

#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "FTP: Returning %d after doing read_directory\n", s);
#endif
      /* Eventually close up the sockets if not reused */
      timer = XtAppAddTimeOut(app_context, ftp_timeout_val * 1000,
		              (XtTimerCallbackProc)close_it_up, NULL);
      fTimerStarted = 1;

      /* HT_INTERRUPTED falls through. */
      return s;
  } else {
      /* We reproduce ParseSocket below because of socket/child process
       * problem. */
      HTStream *stream;
      HTStreamClass targetClass;    
      int rv;
      
      stream = HTStreamStack(format, format_out, compressed, sink, anchor);
      if (!stream) {
          char buffer[1024];

          sprintf(buffer, "Sorry, can't convert from %s to %s.",
                  HTAtom_name(format), HTAtom_name(format_out));
          HTProgress(buffer);
#ifndef DISABLE_TRACE
          if (www2Trace) 
              fprintf(stderr, "FTP: %s\n", buffer);
#endif
	  NETCLOSE(data_soc);
	  data_soc = -1;
	  close_control();
          return HT_NOT_LOADED;
      }
      
      targetClass = *stream->isa;	/* Copy pointers to procedures */

      /* Loading length for HTSaveAndExecute */
      HTftp_loading_length = loading_length;
      rv = HTCopy(data_soc, stream, 0, NULL, loading_length);
      HTftp_loading_length = 0;

#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "FTP: Got %d from HTCopy\n", rv);
#endif
      if (rv == -1) {
          rv = HT_INTERRUPTED;
      } else {
          (*targetClass.end_document)(stream);
          /* Do NOT call *targetClass.free yet -- sockets aren't closed. */
          rv = HT_LOADED;
      }
      /* Reset buffering to control connection -- probably no longer
       * necessary, since we don't use a connection more than once. */
      HTInitInput(control);

#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "FTP: Closing data socket %d\n", data_soc);
#endif
      NETCLOSE(data_soc);
      data_soc = -1;

      /* Unfortunately, picking up the final reply sometimes causes
       * serious problems.  It *probably* isn't safe not to do this,
       * as there is the possibility that FTP servers could die if they
       * try to send it and we're not listening.
       *
       * Testcase for problems (10/30/93): uxc.cso.uiuc.edu,
       * AnswerGarden COPYRIGHT in X11R5 contrib clients.
       *
       * Of course, we may already be triggering hostile actions
       * by allowing client-side interrupts as follows...
       */
      if (rv != HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "FTP: Picking up final reply...\n");
#endif
	  broken_crap_hack = 2;			/* Time it out quick */

          status = response(NULL);		/* Pick up final reply */

	  if (!broken_crap_hack) {
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, "FTP: Final reply timeout.\n");
#endif
	      /* Sockets closed at this point, I think */
	      goto finished;
	  } else {
	      broken_crap_hack = 0;
	  }

          if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, "FTP: Interrupted in final reply.\n");
#endif
              HTProgress("Connection interrupted.");
              close_control();
	      (*targetClass.handle_interrupt)(stream);
	      
	      return HT_INTERRUPTED;
          }
          if (status != 2) {
              close_control();
              (*targetClass.free)(stream);
              return HT_NOT_LOADED;
          }

 finished:
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "FTP: Calling free method, finally.\n");
#endif
	  /* Wait until all sockets have been closed. */
          (*targetClass.free)(stream);
      } else {
          close_control();
          (*targetClass.free)(stream);
	  return rv;
      }

      /* Eventually close up the sockets if not reused */
      timer = XtAppAddTimeOut(app_context, ftp_timeout_val * 1000,
		              (XtTimerCallbackProc)close_it_up, NULL);
      fTimerStarted = 1;

      return rv;
  }
#ifdef SOCKETSHR
 } while (1);     /* End of SOCKETSHR do while loop */
#endif

 interrupted_exit:
  HTProgress("Connection interrupted.");
  close_control();
  if (filename)
      free(filename);
  return HT_INTERRUPTED;
}  /* End HTFTPLoad */


/* HTFTPMkDir  Request that a directory be created on the FTP site.
** Expects:    *name is a pointer to a string that consists of the FTP URL with 
**                   the remote directory name.
** Returns     0 if successful, nonzero on error
*/
PUBLIC int HTFTPMkDir (char *name)
{
  char *curpath, *path;
  char command[LINE_LENGTH + 1];
  int status;

  HTProgress("FTP mkdir in progress");
  if (fTimerStarted) {
      XtRemoveTimeOut(timer);
      fTimerStarted = 0;
  }

  /* Some routines may try to append text into it, so NULL it */
  HT = NULL;

  /* Open a connection (or get a cached connection) to the FTP site */
  status = get_connection(name);
  if (status < 0)
      return status;

  /* The remote directory name is in the URL, so pull it out 
   * i.e., ftp://warez.yomama.com/pub/31337&warez_boy
   * means to make the directory warez_boy at ftp://warez.yomama.com/pub/31337
   */
  if (!(path = strchr(name, '&'))) {  /* No dirname in this URL */
      close_control();
      return HT_FAILED;
  }
  *path++ = '\0';         /* Make the URL normal and move to the dirname */
  /* path is the directory name to create */

  curpath = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION); 
  if (!curpath || !*curpath)
      curpath = strdup("/");

  /* curpath is the remote directory in which to create path */
#ifndef DISABLE_TRACE
  if (www2Trace)
       fprintf(stderr, "FTPmkdir: path is %s\n", curpath);
#endif
  if (server_type == VMS_SERVER) {
      char *ptr;

      curpath = Convert_VMS_FTP_name(curpath);

      /* [dir1]dir2 is not allowed, convert to [dir1.dir2] */
      ptr = curpath + strlen(curpath) - 1;
      if ((*ptr != ']') && strchr(curpath, ']')) {
          /* Append a ] */
          ptr++;
          *ptr++ = ']';
          *ptr = '\0';

          /* Change first ] to a . */
          ptr = strchr(curpath, ']');
          *ptr = '.';
      }
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "FTPmkdir: VMS path is %s\n", curpath);
#endif
      sprintf(command, "CWD %s\r\n", ftp_root_directory);
      status = response(command);
      if (status != 2)
	  goto error_exit;
  }

  /* First change to current directory on the server */
  sprintf(command, "CWD %s\r\n", curpath);
  status = response(command);
  if (status != 2)
      goto error_exit;

  /* Now create the directory on the server */
  sprintf(command, "MKD %s\r\n", path);
  status = response(command);
  if (status != 2)
      goto error_exit;

  /* Clean up */
  timer = XtAppAddTimeOut(app_context, ftp_timeout_val * 1000,
			  (XtTimerCallbackProc)close_it_up, NULL);
  fTimerStarted = 1;
  HTProgress("Created remote directory.");
  return HT_OK;

 error_exit:

  close_control();
  if (status == HT_INTERRUPTED) {
      HTProgress("Connection interrupted.");
      return status;
  } else {
      return HT_FAILED;
  }
}  /* End HTFTPMkDir */


/* HTFTRemove  Request that a file (or directory) be removed from the FTP site
** Expects:    *name is a pointer to a string that consists of the FTP URL
**	       with the remote filename included.
** Returns     0 if successful, nonzero on error
*/
PUBLIC int HTFTPRemove (char *name)
{
  char *fname, *name_ptr;
  char filename[LINE_LENGTH + 1];
  char command[LINE_LENGTH + 1];
  int status, method;
  int didIt = 0;

  HTProgress("FTP remove in progress");
  if (fTimerStarted) {
     XtRemoveTimeOut(timer);
     fTimerStarted = 0;
  }

  /* Some routines may try to append text into it, so NULL it */
  HT = NULL;

  /* Open a connection (or get a cached connection) to the FTP site */
  status = get_connection(name);
  if (status < 0)
     return status;

  /* Pull out the filename (and path) */
  fname = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);
  if (!fname || !*fname) 
     StrAllocCopy(fname, "/");

#ifndef DISABLE_TRACE
  if (www2Trace)
     fprintf(stderr, "FTPremove: filename is %s\n", fname);
#endif
  if (server_type == VMS_SERVER) {
     fname = Convert_VMS_FTP_name(fname);
#ifndef DISABLE_TRACE
     if (www2Trace)
         fprintf(stderr, "FTPremove: VMS filename is %s\n", fname);  /* PGE */
#endif
  }

  /* Pull out just the filename */
  if (server_type == VMS_SERVER) {
     name_ptr = strrchr(fname, ']');
  } else {
     name_ptr = strrchr(fname, '/');
  }
  if (name_ptr) {
     strcpy(filename, ++name_ptr);
     if (server_type == VMS_SERVER)
         *name_ptr = '\0';
  } else {
     *filename = '\0';
  } 
  if (!*filename) {   /* No filename in the URL */
     close_control();
     return HT_FAILED;
  }
 
  /* fname is the full path to the file, *filename is just the filename */
  for (method = 0; method < 2; method++) {
     if (server_type == VMS_SERVER) {
         sprintf(command, "CWD %s\r\n", ftp_root_directory);
         status = response(command);
         if (status != 2)
	     goto error_exit;
     }
     switch (method) {
       /* First, attempt to CWD to fname, if successful, fname is a directory. 
        * So, CDUP to get to the parent and call RMD on filename  */
       case 0:
          sprintf(command, "CWD %s\r\n", fname);
          status = response(command);
          if (server_type == VMS_SERVER) {
      	      sprintf(command, "CWD %s\r\n", filename);
              status = response(command);
          }
          if (status != 2) {
              if (status == 5)  /* Not a directory */
                  continue;
              goto error_exit;
          }
          /* Must be a directory, move up and RMD it*/
          if (server_type == VMS_SERVER) {
              status = response("CDUP\r\n");
          } else {
              *(name_ptr - 1) = '\0';  /* Make fname -> path of parent dir */
              sprintf(command, "CWD %s\r\n", fname);
              status = response(command);
          }
          if (status != 2)
	      goto error_exit;

          if (server_type == VMS_SERVER)
              strcat(filename, ".dir;0");

          sprintf(command, "RMD %s\r\n", filename);
          status = response(command);
          if (status != 2)
	      goto error_exit;
          didIt = 1;
          break;

       /* If the first attempt failed, CWD to fname and DELE filename */
       case 1:
          if (server_type != VMS_SERVER)
              *(name_ptr - 1) = '\0';  /* Make fname -> just the path of file */
          sprintf(command, "CWD %s\r\n", fname);
          status = response(command);
          if (status != 2)
	      goto error_exit;

          if ((server_type == VMS_SERVER) && !strchr(filename, ';'))
              strcat(filename, ";*");

          sprintf(command, "DELE %s\r\n", filename);
          status = response(command);
          if (status != 2)
	      goto error_exit;
          didIt = 1;
          break;
     }  /* End of switch (method) */
     if (didIt)
         break;
  }

  /* Clean up */
  timer = XtAppAddTimeOut(app_context, ftp_timeout_val * 1000,
			  (XtTimerCallbackProc)close_it_up, NULL);
  fTimerStarted = 1;
  return HT_OK;

 error_exit:

  close_control();
  if (status == HT_INTERRUPTED) {
     HTProgress("Connection interrupted.");
     return status;
  } else {
     return HT_FAILED;
  }
}  /* End HTFTPRemove */


/* HTFTPSend   Send File to the FTP site
** Expects:    *name is a pointer to a string that consists of the FTP URL
**	       with the local filename appended to the URL (delimited by an &)
**	       e.g. ftp://warez.mama.com/pub&/tmp/bubba.tgz
**             would send /tmp/bubba.tgz to warez.mama.com:/pub
** Returns     0 if successful, nonzero on error
*/
#define OUTBUFSIZE 4096  /* Size of the chunk of the file read */
PUBLIC int HTFTPSend (char *name)
{
    FILE *f;
    char *fname, *filename, *path;
    char command[LINE_LENGTH + 1];
    char outBuf[OUTBUFSIZE + 1];
    unsigned int bLeft, bDone, bTotal, chunk;
    extern int twirl_increment;
    int next_twirl = twirl_increment;
    int intr = 0;
    int status;
    struct sockaddr_in soc_address;
#if !defined(VMS) || (__DECC_VER < 50230003)
    int soc_addrlen = sizeof(soc_address);
#else
    size_t soc_addrlen = sizeof(soc_address);
#endif

#ifdef VMS
#if (stat != decc$stat) || !defined(MULTINET)
    struct stat sbuf;
#else
#undef stat
    struct stat sbuf;
#define stat decc$stat
#endif /* VMS MultiNet work around, GEC, VRH */
#else
    struct stat sbuf;
#endif
 
    HTProgress("FTP send in progress.");

    if (fTimerStarted) {
        XtRemoveTimeOut(timer);
        fTimerStarted = 0;
    }
      
    /* Some routines may try to append text into it, so NULL it */
    HT = NULL;

    /* The local filename is in the URL, so pull it out
     * i.e., ftp://warez.yomama.com/pub/31337&/u/warezboy/Mosaic0.1a.tar.gz
     * means to send /u/warezboy/Mosaic0.1a.tar.gz to warez.yomama.com/pub/31337
     */
    if (!(fname = strchr(name, '&'))) {
	/* No local filename in this URL */
        close_control();
        return HT_NOT_SENT;
    }

    *fname++ = '\0';        /* Make the URL normal and move to filename */

#ifndef VMS   /* PGE, Use VMS file syntax for local file */
    filename = strrchr(fname, '/');
    if (!*++filename) {
#else
    filename = strrchr(fname, ']');
    if (!filename || !(*filename++) || !*filename) {   /* No filename */
#endif
        close_control();
        return HT_NOT_SENT;
    }

    /* *fname is the full path and filename, *filename is filename */ 
    /* Get size information */
#if defined(MULTINET) && defined(__alpha)
    if (decc$stat(fname, &sbuf) < 0) {
#else
    if (stat(fname, &sbuf) < 0) {
#endif /* Alpha DEC C couldn't find it otherwise ?????, GEC */
        close_control();
        return HT_NOT_SENT;
    }
              
    bTotal = sbuf.st_size;
#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "HTFTPSend: Attempting to send %s (%s) (%lu)\n",
		fname, filename, bTotal);
#endif

    status = setup_connection(name);
    if (status < 0)
	goto error_exit;
      
    /* Logged in, set up the port, now let's send the sucka */

    if (get_pref_boolean(eFTP_BINARY_MODE)) {
    	status = response("TYPE I\r\n");
    } else {
    	status = response("TYPE A\r\n");
    }
    if (status != 2)
	goto error_exit;

    if (server_type == VMS_SERVER) {
        sprintf(command, "CWD %s\r\n", ftp_root_directory);
        status = response(command);
        if (status != 2)
	    goto error_exit;
    }

    /* Move to correct directory */
    path = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);
    if (!path || !*path)
        StrAllocCopy(path, "/");
              
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "FTPsend: path is %s\n", path);
#endif
    if (server_type == VMS_SERVER) {
        char *ptr;

        path = Convert_VMS_FTP_name(path);

        /* [dir1]dir2 is not allowed, convert to [dir1.dir2] */
        ptr = path + strlen(path) - 1;
        if ((*ptr != ']') && strchr(path, ']')) {
            /* append a ] */
            ptr++;
            *ptr++ = ']';
            *ptr = '\0';

            /* Change first ] to a . */
            ptr = strchr(path, ']');
            *ptr = '.';
        }
#ifndef DISABLE_TRACE
        if (www2Trace)
            fprintf(stderr, "FTPsend: VMS path is %s\n", path);
#endif
    }
    sprintf(command, "CWD %s\r\n", path);
    status = response(command);
    if (status != 2)
	goto error_exit;

    if (!(f = fopen(fname, "r"))) {
        close_control();
        return HT_NOT_SENT;
    }
    if (server_type == VMS_SERVER) {
        char *semicolon = strchr(filename, ';');

        if (semicolon)
            *semicolon = '\0';
    }

    /* Send it */
    sprintf(command, "STOR %s\r\n", filename);
    status = response(command);
    if (status != 1) {  /* Does not seem to understand the STOR command */
        if (status != HT_INTERRUPTED)
            HTProgress("FTP host does not understand STOR command.");
	goto error_exit;
    }
      
    /* Ready to send the data now, server is primed and ready... here we go */
      
#ifdef SOCKS
    status = Raccept(master_socket, (struct sockaddr *)&soc_address,
		     &soc_addrlen);
#else
#ifndef __GNUC__
    status = accept(master_socket, (struct sockaddr *)&soc_address,
		    &soc_addrlen);
#else
    status = accept(master_socket, (struct sockaddr *)&soc_address,
		    (int)&soc_addrlen);
#endif /* GNU C is picky, GEC */
#endif
    if (status < 0) {
        close_control();
        return HT_NOT_SENT;
    }

    data_soc = status;
    /* Server has contacted us... send them data */
    /* Send the data! */

    HTMeter(0, NULL);
    bDone = 0;
    bLeft = bTotal;
    for (;;) {
        if (bDone > next_twirl) {
            intr = HTCheckActiveIcon(1);
            next_twirl += twirl_increment;
        } else {
            intr = HTCheckActiveIcon(0);
        }     
        if (intr) {
            HTProgress("Data transfer interrupted");
            HTMeter(100, NULL);
            break;
        }
        /*
	** This is how Unix should do it, except with bLeft removed from the
	** Send function.  Their way, VMS pukes when sending text files.
	** fread converts each line from (two byte count, data) to (data, LF)
	** so each line is one byte shorter.  This means (bTotal < numer of
	** bytes to send) and the test below for bLeft != 0 fails.
        */
        chunk = fread(outBuf, 1, OUTBUFSIZE, f);
        if (chunk > 0)
            NETWRITE(data_soc, outBuf, chunk);
        if (chunk < OUTBUFSIZE) {
            bDone = bTotal;
            break;
        }
        bDone += chunk;

	if (bDone > 10000000) {
	    /* Avoid Integer overflow */
            HTMeter(bDone / (bTotal / 100), NULL);
	} else {
            HTMeter((bDone * 100) / bTotal, NULL);
	}
    }       
    bLeft = bTotal - bDone;

    /* Done, now clean up */
    fclose(f);
    HTMeter(100, NULL);

#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "HTFTPSend: Closing data socket\n");
#endif
    NETCLOSE(data_soc);
    data_soc = -1;

    status = response(NULL);
    if (status == HT_INTERRUPTED)
	goto error_exit;

    if (bLeft)  {
#ifndef DISABLE_TRACE
        if (www2Trace)
            fprintf(stderr, "HTFTPSend: Error sending file %lu bytes left\n",
		    bLeft);
#endif
        return intr ? HT_INTERRUPTED : HT_NOT_SENT;
    }
    timer = XtAppAddTimeOut(app_context, ftp_timeout_val * 1000,
			    (XtTimerCallbackProc)close_it_up, NULL);
    fTimerStarted = 1;
      
#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "HTFTPSend: File sent, returning OK\n");
#endif
    return HT_OK;      

 error_exit:

   close_control();
   if (status == HT_INTERRUPTED) {
       HTProgress("Connection interrupted.");
       return status;
   } else {
       return HT_NOT_SENT;
   }
}  /* End of HTFTPSend */


/*
 * This code based off of Rick Vestal's FTP parse code for the NCSA Windows
 * Mosaic client.
 *
 * Modified for X by Scott Powers
 * 9.27.95
 */
static char *tmpbuf = NULL;

static int ParseFileSizeAndName(char *szBuffer, char *szFileName, char *szSize)
{
	char *szPtr, *szName, *szEndPtr, *szLength;

	if (!szBuffer)
		return(0);
	if (!tmpbuf)
		tmpbuf = (char *)calloc(BUFSIZ, sizeof(char));
	if (usingNLST == 1) {
		strcpy(tmpbuf, szBuffer);

		/* Filename */
		szPtr = strrchr(tmpbuf, ' ');
		while (szPtr && (*szPtr == ' '))
			szPtr--;
		if (szPtr)
			*(szPtr + 1) = '\0';
		if (szPtr && *szPtr == '>') {  /* Deal with a link */
			if (szPtr) {
				szPtr = strrchr(tmpbuf, ' ');
				while (szPtr && (*szPtr == ' '))
					szPtr--;
			}
			if (szPtr) {
				*(szPtr + 1) = '\0';
				szPtr = strrchr(tmpbuf, ' ');
				while (szPtr && (*szPtr == ' '))
					szPtr--;
				if (szPtr)
					*(szPtr + 1) = '\0';
			}
		}
		if (szPtr) {
			/* year/time */
			szPtr = strrchr(tmpbuf, ' ');
			while (szPtr && (*szPtr == ' '))
				szPtr--;
		}
		if (szPtr) {
			/* Date */
			*(szPtr + 1) = '\0';
			szPtr = strrchr(tmpbuf, ' ');
			while (szPtr && (*szPtr == ' '))
				szPtr--;
		}
		if (szPtr) {
			/* Month */
			*(szPtr + 1) = '\0';
			szPtr = strrchr(tmpbuf, ' ');
			while (szPtr && (*szPtr == ' '))
				szPtr--;
		}
		if (szPtr) {
			/* Filesize */
			*(szPtr + 1) = '\0';
			szPtr = strrchr(tmpbuf, ' ');
		}
		/* Beginning of filesize */
		if (szPtr) {
			strcpy(szSize, ++szPtr);
		} else {
			szSize = strdup("0");
		}
	} else {
		szPtr = strrchr(szBuffer, ' ');
		szName = szPtr + 1;
		if (szPtr)
			strcpy(szFileName, szName);

		/* Go to end of file length */
		while (szPtr && *szPtr == ' ')
			szPtr--;
		szEndPtr = szPtr + 1;
		if (*szPtr != '>') {
			while (szPtr && *szPtr != ' ')
				szPtr--;
			if (szPtr) {
				szLength = szPtr + 1;
				strncpy(szSize, szLength, szEndPtr  - szLength);
				szSize[szEndPtr - szLength] = '\0';
			}
		} else {
			return(0);  /* A directory */
		}
	}
	return(1);  /* Not a directory */
}


static int ParseDate(char *szBuffer, char *szFileInfo, char *szMonth,
		     char *szDay, char *szYear, char *szTime)
{
	char *szPtr, *szEndPtr;
	int nCount;

	if (!szBuffer)
		return(0);

	if (!tmpbuf)
		tmpbuf = (char *)calloc(BUFSIZ, sizeof(char));

	if ((*szBuffer != 'd') && (*szBuffer != 'l') && (*szBuffer != '-')) {
	  	/* Hopefully this is the dos format */
		szPtr = szBuffer;
		strncpy(szMonth, szBuffer, 8);
		szMonth[8] = '\0';

		szPtr += 10;
		if (szPtr) {
			strncpy(szTime, szPtr, 7);
			szTime[7] = '\0';
		}
		szPtr += 15;
		if (szPtr) {
			if (*szPtr == 'D') {
				*szDay = 'd';
				szDay[1] = '\0';
			} else {
				*szDay = 'f';
				szDay[1] = '\0';
			}
		}
		return(3);  /* i.e., the info is dos way */
	} else {
		szPtr = NULL;
		nCount = 0;

		/* Loop to go to each of the month, day, year, whatever parts */
		while (szPtr || (!nCount && szBuffer)) {
			switch (nCount) {
				case 0:  /* File info */
					strncpy(szFileInfo, szBuffer, 10);
					szFileInfo[10] = '\0';

					if (usingNLST == 1) {
					    strcpy(tmpbuf, szBuffer);
					    /* filename */
					    szPtr = strrchr(tmpbuf, ' ');
					    while (szPtr && (*szPtr == ' '))
						szPtr--;
					    if (szPtr)
					        *(szPtr + 1) = '\0';
					    if (szPtr && *szPtr == '>') {
						/* Deal with a link */
						if (szPtr) {
						    szPtr = strrchr(tmpbuf,' ');
						    while (szPtr &&
							   (*szPtr == ' '))
							szPtr--;
						}
						if (szPtr) {
						    *(szPtr + 1) = '\0';
						    szPtr = strrchr(tmpbuf,' ');
						    while (szPtr &&
							   (*szPtr == ' '))
							szPtr--;
						    if (szPtr)
						        *(szPtr + 1) = '\0';
						}
					    }
					    if (szPtr) {
						/* year/time */
						szPtr = strrchr(tmpbuf, ' ');
						while (szPtr && (*szPtr == ' '))
						    szPtr--;
					    }
					    if (szPtr) {
					        *(szPtr + 1) = '\0';
						/* Date */
						szPtr = strrchr(tmpbuf, ' ');
						while (szPtr && (*szPtr == ' '))
						    szPtr--;
					    }
					    if (szPtr)
					        *(szPtr + 1) = '\0';
						/* Month */
						szPtr = strrchr(tmpbuf, ' ');
					    /* Beginning of month */
					    szPtr++;

					    szPtr = szBuffer + (szPtr - tmpbuf);
					} else {
					    szPtr = strchr(szBuffer, ' ');
					    while (szPtr && (*szPtr == ' '))
						szPtr++;

					    if (szPtr) {  
						szPtr = strchr(szPtr, ' ');
						while (szPtr && (*szPtr == ' '))
						    szPtr++;
					    }
					    if (szPtr) {
						szPtr = strchr(szPtr, ' ');
						while (szPtr && (*szPtr == ' '))
						    szPtr++;
					    }
					    if (szPtr) {
						szPtr = strchr(szPtr, ' ');
						while (szPtr && (*szPtr == ' '))
						    szPtr++;
					    }
					    if (szPtr) {
						szPtr = strchr(szPtr, ' ');
						while (szPtr && (*szPtr == ' '))
						    szPtr++;
					    }
					    /* Now at the month entry */
					}
					break;

				case 1:
					szEndPtr = strchr(szPtr, ' ');
					if (szEndPtr) {
					    strncpy(szMonth, szPtr,
						    szEndPtr - szPtr);
					    szMonth[szEndPtr - szPtr] = '\0';
					    /* Goto next entry (day) */
					    szPtr = szEndPtr + 1;
					    while (szPtr && (*szPtr == ' '))
						szPtr++;
					} else {
					    strcpy(szMonth, " ");
					}
					break;	

				case 2:
					szEndPtr = strchr(szPtr, ' ');
					if (szEndPtr) {
					    strncpy(szDay, szPtr,
						    szEndPtr - szPtr);
					    szDay[szEndPtr - szPtr] = '\0';
					    szPtr = szEndPtr + 1;  
					    while (szPtr && (*szPtr == ' '))
						szPtr++;
					} else {
					    strcpy(szDay, " ");
					}
					break;

				case 3:
					szEndPtr = strchr(szPtr, ' ');
					if (szEndPtr) {
					    strncpy(szYear, szPtr,
						    szEndPtr - szPtr);
					    szYear[szEndPtr - szPtr] = '\0';
					    szPtr = szEndPtr + 1;  
					} else if (szEndPtr) {
					    strcpy(szYear, " ");
					}
					break;

				case 4:
					szPtr = NULL;
			}
			nCount++;
		}
		szPtr = strchr(szYear, ':');
		if (!szPtr)
			return(0);  /* i.e., the info is month, day, year */
		szPtr -= 2;  /* Beginning of time */

		strncpy(szTime, szPtr, 5);
		szTime[5] = '\0';
	
		return(1);  /* i.e., the info is month, day, time */
	}
}
