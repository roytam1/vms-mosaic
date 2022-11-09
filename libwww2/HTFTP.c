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
**	one minute. Therefore, we have to use a rotating port number.
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
#include "../src/prefs.h" /* Does the required include of X11/Intrinsic.h */
#if defined(__DECC) && defined(MULTINET) && !defined(__alpha)
#undef _POSIX_C_SOURCE
#undef _ANSI_C_SOURCE  /* Gets defined because of _POSIX_C_SOURCE */
#endif /* DEC C, GEC */
#endif /* VMS, moved after tcp.h to avoid conflict, GEC */

#include "HTTCP.h"
#include "HTAnchor.h"
#include "HTFile.h"
#include "HTChunk.h"
#include "HTSort.h"
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
#define IPPORT_FTP	21
#endif

#ifdef __STDC__
#include <stdlib.h>
#endif

#ifndef NIL
#define NIL 0
#endif

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/*		Hypertext object building machinery
*/
#include "HTML.h"

#define PUTC(c) (*targetClass.put_character)(target, c)
#define PUTS(s) (*targetClass.put_string)(target, s)
#define START(e) (*targetClass.start_element)(target, e, 0, 0)
#define END(e) (*targetClass.end_element)(target, e)
#define END_TARGET (*targetClass.end_document)(target)
#define FREE_TARGET (*targetClass.free)(target)
struct _HTStructured {
	WWW_CONST HTStructuredClass *	isa;
	/* ... */
};

struct _HTStream {
      WWW_CONST HTStreamClass*	isa;
      /* ... */
};

static int CLOSE_CONTROL(int s);

extern int broken_crap_hack;

/* 
** Info for cached connection;  right now we only keep one around for a while
*/  
extern XtAppContext app_context;
extern int ftp_timeout_val;
extern int securityType;
static void close_it_up();

char *prompt_for_password(char *msg);

/* SWP -- Yukky Cool Kludge */
int ftpKludge = 0;
extern int loading_length;
BOOL usingNLST = 0;

/* FTP "redial" using ftpRedial resource for number of times */
extern int ftpRedial;
extern int ftpRedialSleep;
extern int ftpFilenameLength;
extern int ftpEllipsisLength;
extern int ftpEllipsisMode;

/* Directory parsing */
int ParseDate(char *szBuffer, char *szFileInfo, char *szMonth, char *szDay, char *szYear, char *szTime);
int ParseFileSizeAndName(char *szBuffer, char *szFileName, char *szSize);

HText *HT;
int fTimerStarted = 0;
XtIntervalId timer;
  static struct ftpcache {
		int control;
		char p1[256];
		char host[256];
		char username[BUFSIZ];
		char password[BUFSIZ];
	} ftpcache = {
		-1,
		"",
		"",
		"",
		""
	};

#ifdef SOCKS
extern struct in_addr SOCKS_ftpsrv;	/* In HTFTP.C */
#endif

/* HTFTPClearCache ()
   Expects: Nothing
   Returns: Nothing
*/
void HTFTPClearCache (void)
{
  ftpcache.password[0] = '\0';
  ftpcache.control = -1;
  ftpcache.p1[0] = 0;
  ftpcache.host[0] = 0;
  ftpcache.username[0] = 0;
}


/*	Module-Wide Variables
**	---------------------
*/
PRIVATE char    response_text[LINE_LENGTH+1]; /* Last response from NewsHost */
PRIVATE int	control = -1;		/* Current connection */
PRIVATE int	data_soc = -1;		/* Socket for data transfer */

PRIVATE int     master_socket = -1;	/* Listening socket = invalid	*/
PRIVATE char	port_command[255];	/* Command for setting the port */


#define DATA_BUFFER_SIZE 2048
PRIVATE char data_buffer[DATA_BUFFER_SIZE];		/* Input data buffer */
PRIVATE char *data_read_pointer;
PRIVATE char *data_write_pointer;
PUBLIC char ftp_type[16];
PUBLIC char ftp_root_directory[100];


/* Convert_VMS_FTP_name  Convert a filename to use VMS directory syntax
** Expects:    *name is a pointer to a string that consists of the FTP
**             filename, including the path.
** Returns     pointer to the new name
*/
PRIVATE char *Convert_VMS_FTP_name ARGS1 ( char *, name )
{
   char *filename = name;
   char *ptr;

   if (!strcmp(filename, "/")) {
      free(filename);
      StrAllocCopy(filename, "[]");
   } else {
      /*
      ** Now we are in trouble.  Convert the UNIX name to a VMS name.  I
      ** guess we can safely assume that we always start in the server's
      ** topdirectory. Therefore a construct like /dir1/dir2.../dirn/name.type
      ** has to be converted to [.dir1.dir2..dirn]name.type.
      **
      ** Following added by PGE.
      ** /dir1/dir2.../dirn converted to [.dir1.dir2..dirn] is handled later.
      ** Also /* becomes * so: /name.type becomes name.type, /dir1 becomes
      ** dir1, and /disk:[rootdir.dir1]name.type becomes
      ** disk:[rootdir.dir1]name.type.
      **
      ** Next /disk:[rootdir.dir1]/dir2/file.type becomes
      ** disk:[rootdir.dir1.dir2]file.type, and /disk:[rootdir.dir1]/dir2/dir3
      ** becomes disk:[rootdir.dir1.dir2]dir3 which is converted to
      ** disk:[rootdir.dir1.dir2.dir3] later.
      */
      char *file_temp = (char *)malloc(strlen(filename) + 4);
      int i1 = 0, i2 = 1, i3 = 2, i4 = 0;

      file_temp[0] = '[';
      file_temp[1] = '.';
      if (filename[0] == '/')
	  i1 = 1;      /* Always true, PGE */
      for (; i1 < strlen(filename); i1++, i3++) {
         if (filename[i1] != '/') {
            /* Handle /disk:[rootdir.dir1]/dir2/file.type, PGE */
            if ((filename[i1] != ']') ||
                (strchr(&(filename[i1+1]), '/') == 0)) {
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
         if (i2 > 1) file_temp[i2] = '.';
         i2 = i3;
      }
      if (i2 == 1) i4 = 2;  /* PGE, remove [. at the start of file_temp */
      file_temp[i3] = ' ';  /* Add a space for later expansion room */
      file_temp[i3+1] = '\0';
      free(filename);
      StrAllocCopy(filename, &(file_temp [i4]));
      free(file_temp);

      /* Remove spaces from end of string. */
      file_temp = filename + strlen(filename) - 1;
      while (isspace(*file_temp)) {
         *file_temp = '\0';
         file_temp--;
      }
   }

   if ((strcmp(filename, "[]")
	&& (ptr = strchr(filename, ']')) != 0)
	&& (ptr != (char*)strlen(filename))
	&& !strchr(ptr, '.')) {
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
   if (strcmp(ptr, ".]") == 0) {
      /*
      ** Handle the /dir1/dir2../dirn/ converted to
      ** [.dir1.dir2..dirn] case by changing
      ** [.dir1.dir2..dirn.] to [.dir1.dir2...dirn]
      */
      *ptr = ']';
      *(ptr + 1) = '\0';
   }

   return filename;
}


/*	Procedure: Read a character from the data connection
**	----------------------------------------------------
*/
PRIVATE int interrupted_in_next_data_char = 0;
PRIVATE char next_data_char NOARGS
{
  int status;
  interrupted_in_next_data_char = 0;
  if (data_read_pointer >= data_write_pointer) {
      status = NETREAD(data_soc, data_buffer, DATA_BUFFER_SIZE);
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
**	See the state machine illustrated in RFC959, p57. This implements
**	one command/reply sequence.  It also interprets lines which are to
**	be continued, which are marked with a "-" immediately after the
**	status code.
**
**	Continuation then goes on until a line with a matching reply code
**	an a space after it.
**
** On entry,
**	con	points to the connection which is established.
**	cmd	points to a command, or is NIL to just get the response.
**
**	The command is terminated with the CRLF pair.
**
** On exit,
**	returns:  The first digit of the reply type,
**		  or negative for communication failure.
*/
#ifdef __STDC__
PRIVATE int response (char * cmd)
#else
PRIVATE int response (cmd)
    char * cmd;
#endif
{
  int result;				/* Three-digit decimal code */
  int status;
  char continuation;
  int multiline_response = 0;

  int messageStarted = 0;

char *ptr;
char bytestr[256], *byteptr;
int bytes;
  
  if (!control || (control == -1)) {
#ifndef DISABLE_TRACE
      if(www2Trace) 
        fprintf(stderr, "FTP: No control connection set up!!\n");
#endif
      return -99;
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
          CLOSE_CONTROL(control);
          control = -1;
          return status;
      }
  }
  
  /* Patch to be generally compatible with RFC 959 servers  -spok@cs.cmu.edu  */
  /* Multiline responses start with a number and a hyphen;
     end with same number and a space.  When it ends, the number must
     be flush left. */
  do 
    {
      char *p = response_text;
      /* If nonzero, it's set to initial code of multiline response */
      for (;;)
        {
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
              CLOSE_CONTROL(control);
              control = -1;
              return HT_INTERRUPTED;
          }
          
          if ((foo == LF) || (p == &response_text[LINE_LENGTH])) {
              *p++ = 0;                 /* Terminate the string */
#ifndef DISABLE_TRACE
              if (www2Trace) 
                fprintf(stderr, "    Rx: %s", response_text);
#endif
	      if (!strncmp(response_text, "150", 3)) {
                  if ((ptr = strrchr(response_text, '(')) && *ptr &&
		      (ptr != strchr(response_text, '('))) {
                      bytes = atoi((ptr + 1));
		  } else {
                      bytes = 0;
                  }
		if (bytes == 0) {
			loading_length = -1;
		} else {
			loading_length = bytes;
		}
	      } else {
		HTMeter(100, NULL);
	      }

              sscanf(response_text, "%d%c", &result, &continuation);

		if ((response_text[0] == '2') || (response_text[0] == '5')) {
				
			if (continuation == '-') {
				char *p;

				if (messageStarted == 0) {
				    HText_appendText(HT, "<PRE>\n");
        			    HTProgress("Receiving directory message");
				    messageStarted = 1;
				}

				p = strchr(response_text, '-');
				p++;
				if (p)
				    HText_appendText(HT, p);
			}
		}

				
              if (continuation == '-' && !multiline_response) {
                  multiline_response = result;
              } else if (multiline_response && continuation == ' ' &&
                         multiline_response == result &&
                         isdigit(response_text[0])) {
                  /* End of response (number must be flush on left) */
                  multiline_response = 0;
              }
              break;
          } /* if end of line */
          
          if (*(p-1) == (char)EOF) {
#ifndef DISABLE_TRACE
              if (www2Trace) 
                fprintf(stderr, "Error on rx: closing socket %d\n",
                        control);
#endif
	      loading_length = -1;
              strcpy(response_text, "000 *** TCP read error on response\n");
              CLOSE_CONTROL(control);
              control = -1;
              return -1;	/* End of file on response */
          }
        } /* Loop over characters */
    } 
  while (multiline_response);

  if (messageStarted)
	  HText_appendText(HT, "</PRE><HR>\n");

  if (result == 421) {
#ifndef DISABLE_TRACE
      if(www2Trace) 
        fprintf(stderr, "FTP: They close so we close socket %d\n",
                control);
#endif
      loading_length = -1;
      CLOSE_CONTROL(control);
      return -1;
  }

  if (result == 550) {
	HTProgress(response_text);
  }

  return result/100;
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

PRIVATE int get_connection ARGS1 (char *,arg)
{
  int status, con;
  
  static char host[BUFSIZ];
  static char username[BUFSIZ];
  static char password[BUFSIZ];
  char dummy[MAXHOSTNAMELEN+32];  

  int redial = 0;

  if (!arg || !*arg) 
    return -1;
  
#ifndef DISABLE_TRACE
  if (www2Trace) 
    fprintf(stderr, "FTP: Looking for %s\n", arg);
#endif

  {
    char *p1 = HTParse(arg, "", PARSE_HOST);
    char *p2 = strrchr(p1, '@');        /* user? */
    char * pw;
    char * un;

    /* Save the actual host */
    {
	char *tmpptr;

	strcpy(host, p1);
	tmpptr = strchr(host, '/');
	if (tmpptr) {
		*tmpptr = '\0';
	}
    }

    if (p2) {
        un = p1;
        *p2 = 0;                            /* terminate */
        p1 = p2 + 1;                        /* point to host */
        pw = strchr(un, ':');
        if (pw) {
            *pw++ = 0;
        }
	if (strcmp(un, username)) { /* New username */
		strcpy(username, un);
		if (pw) {
			strcpy(password, pw);
		} else {
			*password = '\0';
		}
	} else { /* Same username */
		if (!*(ftpcache.host) || (*(ftpcache.host) &&
		    strcmp(host, ftpcache.host))) { /* New host */
			*password = '\0';
		}
	}
    }
    /* No username */
    else { 
	if (strcmp(username, "anonymous")) { /* Last one was not anon */
		*username = '\0';
		*password = '\0';
	}
    }

    /* Copy hostname into dummy URL, since username:password@ 
       might have been part of original */ 
    sprintf(dummy, "ftp://%s", p1);

#ifndef DISABLE_TRACE
    if (www2Trace)
    	fprintf(stderr, "FTP: set dummy to %s\n", dummy);
#endif

    /* Is the cache connection still good? */
    if (ftpcache.control != (-1) && *(ftpcache.host) &&
	!strcmp(ftpcache.host, host) && *username) {
	/* Did we use a username before? */
	if (!*username) {
		return(ftpcache.control);
	}

	/* Is the username and password used the same? */
	if (*username && !strcmp(ftpcache.username,username) &&
	    *password && !strcmp(ftpcache.password,password)) {
		/* For security Icon */
		if (*username && strcmp(username, "anonymous") &&
		    strcmp(username, "ftp")) {
			/* Not anon login...assuming a real login */
			securityType = HTAA_LOGIN;
		} else {
			securityType = HTAA_NONE;
		}
		return(ftpcache.control);
	} else {
		/* Something has changed...reopen connection */
		close(ftpcache.control);
	}
    }

    /* Connection is not good.  Reopen */
    strcpy(ftpcache.p1, p1);
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

    if (!*username) {
      free(p1);
    }
  }

  /* Default the redial values if out of range */
  if (ftpRedial < 0) {
	ftpRedial = 0;
  }
  if (ftpRedialSleep < 1) {
	ftpRedialSleep = 1;
  }

redialFTP:

  con = -1;

#ifndef DISABLE_TRACE
  if (www2Trace)
	fprintf(stderr, "dummy = %s\n", dummy);
#endif

  status = HTDoConnect(dummy, "FTP", IPPORT_FTP, &con);
  
  if (status < 0) {
#ifndef DISABLE_TRACE
      if (www2Trace) {
          if (status == HT_INTERRUPTED)
            fprintf(stderr,
                    "FTP: Interrupted on connect\n");
          else
            fprintf(stderr, 
                    "FTP: Unable to connect to remote host for `%s'.\n",
                    arg);
      }
#endif
      if (status == HT_INTERRUPTED)
        HTProgress ("Connection interrupted.");
      if (con != -1) {
          CLOSE_CONTROL(con);
          con = -1;
      }
/*
      if (username) 
        free(username);
*/
      HTProgress("Unable to connect to remote host.");
      return status;			/* Bad return */
  }
  
#ifndef DISABLE_TRACE
  if (www2Trace) 
    fprintf(stderr, "FTP connected, assigning control socket %d\n", con);
#endif
  control = con;			/* Current control connection */

  ftpcache.control = control;

  /* Initialise buffering for control connection */
  HTInitInput(con);
  

  /* Now we log in; Look up username, prompt for pw. */
  {
    int status = response(NIL);	/* Get greeting */

    if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf(stderr, "FTP: Interrupted at beginning of login.\n");
#endif
	loading_length = -1;
        HTProgress("Connection interrupted.");
        CLOSE_CONTROL(control);
        control = -1;
        return HT_INTERRUPTED;
    }
    if (status == 2) 
      {		/* Send username */
        char *command;
/*
 * Try to figure out what type of server we deal with. Only MultiNet and
 * MadGoat seem to announce themself properly (IMHO). Just take a chance
 * for UCX and TWG.
 */
        ftp_type[0] = '\0';   /* Default to Unix */
        ftp_root_directory [0] = '\0';   /* root directory is unknown */
#ifndef DISABLE_TRACE
        if (www2Trace)
		fprintf(stderr, "FTP: login response is %s.\n", response_text);
#endif

        if (strstr(response_text, "MultiNet")) strcpy(ftp_type, "MultiNet");
        if (strstr(response_text, "MadGoat")) strcpy(ftp_type, "MadGoat");
        if (strstr(response_text, "UCX")) strcpy(ftp_type, "UCX");
        if (strstr(response_text, "Process Software"))
		strcpy(ftp_type, "TCPware");
        if (strstr(response_text, "FTP Server (")) strcpy(ftp_type, "UCX");
        if (strstr(response_text, "CMU")) strcpy(ftp_type, "CMU");
/*
 * Pathway comment:
 * We have to assume... we have gone out of our way to make it so
 * nobody can tell when our server is available on a machine. Ie,
 * configurable greeeting. So if it is not them, it could be us.
 */
        if (strstr(response_text, "Wollongong")) strcpy(ftp_type, "TWG");
        if (strstr(response_text, "FTP Service")) strcpy(ftp_type, "TWG");

        if (*username) {
            command = (char*)malloc(10 + strlen(username) + 2 + 1);
            sprintf(command, "USER %s%c%c", username, CR, LF);
	    if (!*password) {
		char *pw = NULL;

		pw = prompt_for_password("Please Enter Your FTP Password:");
		if (pw && *pw) {
			strcpy(password, pw);
			strcpy(ftpcache.password, password);
			free(pw);
		} else {
			*password = '\0';
			*(ftpcache.password) = '\0';
        		HTProgress("Connection aborted.");
        		CLOSE_CONTROL(control);
        		control = -1;
        		return HT_INTERRUPTED;
		}
	    }
        } else {
            command = (char *)malloc(25);
            sprintf(command, "USER anonymous%c%c", CR, LF);
	    strcpy(username, "anonymous");
	    strcpy(ftpcache.username, username);
        }
        status = response(command);
        free(command);
        if (status == HT_INTERRUPTED)
          {
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf(stderr, "FTP: Interrupted while sending username.\n");
#endif
	    loading_length = -1;
            HTProgress("Connection interrupted.");
            CLOSE_CONTROL(control);
            control = -1;
            return HT_INTERRUPTED;
          }
      }
    if (status == 3) {		/* Send password */
        char * command;
        if (*password) {
            command = (char*)malloc(10+strlen(password)+2+1);
            sprintf(command, "PASS %s%c%c", password, CR, LF);
        } else {
#ifndef VMS
            char * user = getenv("USER");
#else
            char * user = (char *)getenv("USER");
#endif /* VMS, BSN */
            extern char *machine_with_domain;
            char *host = machine_with_domain;
            if (!user) 
              user = "WWWuser";
            /* If not fully qualified, suppress it as ftp.uu.net
               prefers a blank to a bad name */
            if (!strchr(host, '.'))
	      host = "";

            command = (char*)malloc(20 + strlen(host)+2+1);
            sprintf(command,
		    "PASS %s@%s%c%c", user ? user : "WWWuser",
		    host, CR, LF); /*@@*/
	    sprintf(password, "%s@%s", (user ? user : "WWWuser"), host);
	    strcpy(ftpcache.password, password);
        }
        status = response(command);
        free(command);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf(stderr,
                       "FTP: Interrupted while sending password.\n");
#endif
	    loading_length = (-1);
            HTProgress("Connection interrupted.");
            CLOSE_CONTROL(control);
            control = -1;
            return HT_INTERRUPTED;
        }
    }
    
    if (status == 3) {
        char temp[80];
/*
    	if (username) 
		free(username);
*/
        sprintf(temp, "ACCT noaccount%c%c", CR, LF);
        status = response(temp);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf(stderr,
                       "FTP: Interrupted while sending ACCT.\n");
#endif
	    loading_length = (-1);
            HTProgress("Connection interrupted.");
            CLOSE_CONTROL(control);
            control = -1;
            return HT_INTERRUPTED;
        }
    }
    if (status != 2) 
      {
	if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
		if (www2Trace)
			fprintf(stderr,
				 "FTP: Interrupted in redial attempt.\n");
#endif
		loading_length = (-1);
		HTProgress("Connection interrupted.");
		CLOSE_CONTROL(control);
		control = -1;
		return HT_INTERRUPTED;
	}

	if (*username && strcmp(username, "anonymous")) {
		HText_appendText(HT, "<H2>FTP login using username \"");
		HText_appendText(HT, username);
		HText_appendText(HT, "\" failed.</H2><BR>");
		if (*password) {
			HText_appendText(HT, "If you have a login ");
			HText_appendText(HT, "on this machine please check ");
			HText_appendText(HT, "to make sure the password you ");
			HText_appendText(HT, "are specifying is correct.");
		} else {
			HText_appendText(HT, "This is probably because you ");
			HText_appendText(HT, "didn't specify a password ");
			HText_appendText(HT, "along with your username.<BR>");
			HText_appendText(HT, "To do this you have to specify ");
			HText_appendText(HT, "the FTP line like this:<BR>");
			HText_appendText(HT, "<P>");
			HText_appendText(HT, "ftp://username:password@ftp_site/");
			HText_appendText(HT, "<P>");
			HText_appendText(HT, "<strong>OR</strong>");
			HText_appendText(HT, "<P>");
			HText_appendText(HT, "You can now just specify a username ");
			HText_appendText(HT, "and you will be prompted for your ");
			HText_appendText(HT, "password.");
			HText_appendText(HT, "<P>");
			HText_appendText(HT, "e.g. ftp://username@ftp_site/");
		}
	} else {
		char buf[BUFSIZ];

		if (redial < ftpRedial) {
			/* Close down current connection */
			ftpcache.control = -1;
			CLOSE_CONTROL(control);
			control = -1;

			/* Tell them in the progress string */
			sprintf(buf,
				"Login failed. Redial Attempt %d/%d. Sleeping %d seconds.",
				redial, ftpRedial, ftpRedialSleep);
			HTProgress(buf);

			/* If we're tracing, explain it all */
#ifndef DISABLE_TRACE
			if (www2Trace) 
				fprintf(stderr,
					"FTP: Login fail: %s", response_text);
#endif
/*
			sleep(ftpRedialSleep);
*/
/* Commented out until we get a new "sleep" routine...SWP

			if (my_sleep(ftpRedialSleep, 1)) {
#ifndef DISABLE_TRACE
				if (www2Trace)
				    fprintf(stderr,
					"FTP: Interrupted in sleep during redial attempt.\n");
#endif
				loading_length = (-1);
				HTProgress("Connection interrupted.");
				CLOSE_CONTROL(control);
				control = -1;
				return HT_INTERRUPTED;
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

	loading_length = -1;

	ftpcache.control = -1;
        *password = '\0';

        CLOSE_CONTROL(control);
        control = -1;
        return -1;		/* Bad return */
      }
#ifndef DISABLE_TRACE
    if (www2Trace) 
      fprintf(stderr, "FTP: Logged in.\n");
#endif
    /* If MultiNet, then force it to VMS directory style. GEC */
    if (!strcmp(ftp_type, "MultiNet")) {
        char  temp[80];

        sprintf(temp, "CWD []%c%c", CR, LF);
        status = response(temp);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf(stderr,
                      "FTP: Interrupted while sending CD [].\n");
#endif
            HTProgress("Connection interrupted.");
            CLOSE_CONTROL(control);
            control = -1;
            return HT_INTERRUPTED;
        }
    }
/*
 * Try to determine the ftp server type if not already done. TWG replies
 * with the present directory in VMS format
 */
#ifndef DISABLE_TRACE
      if (ftp_type[0] == '\0')
         if (www2Trace)
		fprintf(stderr, "FTP: login response2 is %s.\n", response_text);
#endif

      if (ftp_type[0] == '\0')
        if (strstr(response_text, "[") && strstr (response_text, "]"))
          strcpy(ftp_type, "TWG");
#ifndef DISABLE_TRACE
      if (ftp_type[0] == '\0')
         if (www2Trace)
		fprintf(stderr, "FTP: login response2.5 is %s.\n",
			response_text);
#endif
/*
 * Try a remotehelp site to see if it is UCX
 */
      if (ftp_type[0] == '\0') {
        char temp[80];

        sprintf(temp, "HELP site%c%c", CR, LF);
        status = response(temp);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf(stderr,
                      "FTP: Interrupted while sending remotehelp.\n");
#endif
            HTProgress("Connection interrupted.");
            CLOSE_CONTROL(control);
            control = -1;
            return HT_INTERRUPTED;
        }
#ifndef DISABLE_TRACE
        if (www2Trace)
		fprintf(stderr, "FTP: login response3 is %s.\n", response_text);
#endif
        if (strstr(response_text, "214 Syntax: SITE <sp> Vms | ULtrix | UNix"))
          strcpy(ftp_type, "UCX");
      }

    /* Need to know login directory, sys$login doesn't work on MultiNet. PGE */
    if (ftp_type[0] != '\0') {
        char  temp[80];

        sprintf(temp, "PWD%c%c", CR, LF);
        status = response(temp);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf(stderr,
                       "FTP: Interrupted while sending pwd.\n");
#endif
            HTProgress("Connection interrupted.");
            CLOSE_CONTROL(control);
            control = -1;
            return HT_INTERRUPTED;
        }
        if (strstr(response_text, "257 \"")) {
          sscanf(response_text, "257 \"%s", ftp_root_directory);
          if (ftp_root_directory [0] == '\0')
            strcpy(ftp_root_directory, "sys$login");
          else
            /* Strip closing quote. */
            ftp_root_directory[strlen(ftp_root_directory) - 1] = '\0';
#ifndef DISABLE_TRACE
          if (www2Trace)
		fprintf(stderr, "FTP: root directory is %s.\n",
		ftp_root_directory);
#endif
        } else {
#ifndef DISABLE_TRACE
          if (www2Trace)
		fprintf(stderr, "FTP: pwd response is %s.\n", response_text);
#endif
        }
    }

#ifndef DISABLE_TRACE
    if (www2Trace) 
      if (ftp_type[0] == '\0')
        fprintf(stderr, "FTP: Server type is null (UNIX).\n");
      else
        fprintf(stderr, "FTP: Server type is %s.\n", ftp_type);
#endif

    /* For security Icon */
    if (*username && strcmp(username,"anonymous") && strcmp(username,"ftp")) {
	/* Not anon login...assuming a real login */
	securityType = HTAA_LOGIN;
    } else {
	securityType = HTAA_NONE;
    }
  }
  
  return con;			/* Good return */
} /* Scope of con */


/*	Close Master (listening) socket
**	-------------------------------
**
**
*/
#ifdef __STDC__
PRIVATE void close_master_socket(void)
#else
PRIVATE void close_master_socket()
#endif
{
#ifndef DISABLE_TRACE
  if (www2Trace) 
    fprintf(stderr, "FTP: Closing master socket %d\n", master_socket);
#endif
  NETCLOSE(master_socket);
  master_socket = -1;

  return;
}


/*	Open a master socket for listening on
**	-------------------------------------
**
**	When data is transferred, we open a port, and wait for the server to
**	connect with the data.
**
** On entry,
**	master_socket	Must be negative if not set up already.
** On exit,
**	Returns		socket number if good
**			less than zero if error.
**	master_socket	is socket number if good, else negative.
**	port_number	is valid if good.
*/
#ifdef __STDC__
PRIVATE int get_listen_socket(void)
#else
PRIVATE int get_listen_socket()
#endif
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
  sin->sin_family = AF_INET;	    /* Family = internet, host order  */
  sin->sin_addr.s_addr = INADDR_ANY; /* Any peer address */
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
                         (struct sockaddr *)&soc_address,
			 &address_length);
    if (status < 0) 
      return -1;

#ifndef DISABLE_TRACE
      if (www2Trace) {
	fprintf(stderr, "FTP: This host is %s\n",
		HTInetString(sin));
      }
#endif

    soc_address.sin_port = 0; /* Unspecified: please allocate */
#ifdef SOCKS
    status = Rbind(new_socket,
#else
    status = bind(new_socket,
#endif
              (struct sockaddr*)&soc_address,
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
                           (struct sockaddr*)&soc_address,
                           &address_length);
    if (status < 0) 
      return -1;
  }
  
#ifndef DISABLE_TRACE
  if (www2Trace) {
	fprintf(stderr, "FTP: bound to port %d on %s\n",
		(int)ntohs(sin->sin_port),
		HTInetString(sin));
  }
#endif

  if (master_socket >= 0) 
    close_master_socket ();
  
  master_socket = new_socket;
  
  /* Now we must find out who we are to tell the other guy */
  (void)HTHostName(); 	/* Make address valid - doesn't work*/
  sprintf(port_command, "PORT %d,%d,%d,%d,%d,%d%c%c",
          (int)*((unsigned char *)(&sin->sin_addr)+0),
          (int)*((unsigned char *)(&sin->sin_addr)+1),
          (int)*((unsigned char *)(&sin->sin_addr)+2),
          (int)*((unsigned char *)(&sin->sin_addr)+3),
          (int)*((unsigned char *)(&sin->sin_port)+0),
          (int)*((unsigned char *)(&sin->sin_port)+1),
          CR, LF);
  
  /* Inform TCP that we will accept connections */
#ifdef SOCKS
  if (Rlisten(master_socket, 1) < 0) 
#else
  if (listen(master_socket, 1) < 0) 
#endif
    {
      close_master_socket();
      return -1;
    }

#ifndef DISABLE_TRACE
    if (www2Trace) {
	fprintf(stderr, "FTP: Master socket(), bind() and listen() all OK\n");
    }
#endif

  return master_socket;		/* Good */
} /* get_listen_socket */


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
PRIVATE int read_directory
ARGS4 (
  HTParentAnchor *,		parent,
  WWW_CONST char *,		address,
  HTFormat,			format_out,
  HTStream *,			sink )
{
  HTFormat format;
  HTAtom *pencoding;
  char *filename = HTParse(address, "", PARSE_PATH + PARSE_PUNCTUATION);
  char buffer[BUFSIZ];
  char buf[BUFSIZ];
  char itemtype;
  char itemname[BUFSIZ];
  char itemsize[BUFSIZ];
  char *full_ftp_name, *ptr;
  int count, ret, cmpr, c, rv;
  extern char *HTgeticonname(HTFormat, char *);
  char *ellipsis_string = (char *)calloc(1024, sizeof(char));
  int nTime;
  char szDate[256];
  int nStringLen;
  int nSpaces;
  int nOldSpaces;
  char szFileInfo[32];
  char szMonth[32];
  char szDay[16];
  char szYear[32];
  char szTime[32];
  int lfirst = 0;
  int lcont = 0;

  HTProgress("Reading FTP directory");

  HText_appendText(HT, "<H1>FTP Directory ");
  HText_appendText(HT, filename);
  HText_appendText(HT, "</H1>\n");
  HText_appendText(HT, "<PRE>");
  HText_appendText(HT, "<DL>\n");
  data_read_pointer = data_write_pointer = data_buffer;

  /* If this isnt the root level, spit out a parent directory entry */

  if (strcmp(filename, "/")) {
      HText_appendText(HT, "<DD>");

      HText_appendText(HT, "<A HREF=\"");
      
      strcpy(buffer, filename);
      ptr = strrchr(buffer, '/');
      
      if (ptr)
        *ptr='\0';
      
      if (buffer[0] == '\0') 
        HText_appendText(HT, "/");
      else
        HText_appendText(HT, buffer);
      
      HText_appendText(HT, "\"><IMG SRC=\"");
      HText_appendText(HT, HTgeticonname(NULL, "directory"));
      HText_appendText(HT, "\"> Parent Directory</a>\n");
  }
  
  /* Loop until we hit EOF */
  while(1)
    {
      /* Read in a line of data */
      for (count = 0; count < BUFSIZ; count++) {
          c = next_data_char();
          if (interrupted_in_next_data_char) {
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf(stderr,
		        "FTP: Picked up interrupted_in_next_data_char\n");
#endif
              return HT_INTERRUPTED;
          }
          
          if (c == '\r') {
              c = next_data_char();
              if (interrupted_in_next_data_char) {
#ifndef DISABLE_TRACE
                  if (www2Trace)
                    fprintf(stderr,
		            "FTP: Picked up interrupted_in_next_data_char\n");
#endif
                  return HT_INTERRUPTED;
              }
              
              if (c != '\n') 
                break;
          }
          
          if (c == '\n' || c == (char)EOF) 
            break;
          
          buffer[count] = c;
      }
      
      if (c == (char)EOF) 
        break;
      
      buffer[count] = 0;
      
#ifndef DISABLE_TRACE
      if (www2Trace)
	fprintf(stderr, "FTP: Rx: %s\n", buffer);
#endif
      /* Parse the input buffer, extract the item type, and the item size */
      /* Retain whole string -- we don't use it at the moment, but we will. */
      full_ftp_name = strdup(buffer);
      /* Read but disregard itemsize -- this effectively guarantees we will know
         what we should display and what we shouldn't -- don't ask. */
      if (usingNLST == 1) {
	  ret = sscanf(buffer, "%c%*9s %*d %*s %*s %s", &itemtype, itemsize);
	  if (ret != 2) {
		free(full_ftp_name);
		continue;
	  }
      } else if (usingNLST == 2) { /* Only name */
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
      } else { /* Using LIST command */
        if (ftp_type[0] == '\0') {
	    ret = sscanf(buffer, "%c%*9s %*d %*s %*s %s", &itemtype, itemsize);
	    if (ret != 2) {
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
             ptr = strchr(itemsize, '/');     /* size is "size/allocated" */
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
             if (lcont)
               ret = sscanf(buffer, " %s %s %s", itemsize, date1, date2);
             else
               ret = sscanf(buffer, "%s %s %s %s", itemname, itemsize, date1,
			    date2);
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
      }

      if (usingNLST == 2) { /* Only name */
	strcpy(itemname, buffer);
	nTime = -1;
      } else {
        if (ftp_type[0] == '\0') {
	    /* Due to the various time stamp formats, its "safer" to retrieve the        */
	    /* filename by taking it from the right side of the string, we do that here. */
	    ptr = strrchr(buffer, ' ');
      
	    if (ptr == NULL) 
		continue;
      
	    strcpy(itemname, ptr+1);

	    if (!strcmp(itemname, ".") || !strcmp(itemname, "..")) {
		free(full_ftp_name);
		continue;
	    }

	    nTime = ParseDate(buffer, szFileInfo, szMonth, szDay, szYear,
		 szTime);

	    if (usingNLST == 1) {
		ParseFileSizeAndName(buffer, itemname, itemsize);
	    }

	    if (nTime == 3) {  /* ie a dos or NT server possibly */
		if (!ParseFileSizeAndName(buffer, itemname, itemsize)) {
			itemtype = 'd';
		} else {
			itemtype = '-';
		}
	    }
        } else {
          nTime = -2;
        }
      }

      HText_appendText(HT, "<DD>");
      /* Spit out the anchor refrence, and continue on... */
      
      HText_appendText(HT, "<A HREF=\"");
      /* Assuming it's a relative reference... */
      if (itemname && itemname[0] != '/') {
          HText_appendText(HT, filename);
          if (filename[strlen(filename)-1] != '/') 
            HText_appendText(HT, "/");
      }
      HText_appendText(HT, itemname);
      HText_appendText(HT, "\">");
      
      /* There are 3 "types", directory, link and file.  If its a directory we     */
      /* just spit out the name with a directory icon.  If its a link, we go       */
      /* retrieve the proper name (i.e. the input looks like bob -> ../../../bob   */
      /* so we want to hop past the -> and just grab bob.  The link case falls     */
      /* through to the filetype case.  The filetype shows name and filesize, and  */
      /* then attempts to select the correct icon based on file extension.         */
      switch (itemtype) {
        case 'd': {
	    if (compact_string(itemname, ellipsis_string, ftpFilenameLength,
			       ftpEllipsisMode, ftpEllipsisLength)) {
		strcpy(itemname, ellipsis_string);
	    }
            sprintf(buffer, "%s", itemname);
            HText_appendText(HT, "<IMG SRC=\"");
            HText_appendText(HT, HTgeticonname(NULL, "directory"));
            HText_appendText(HT, "\"> ");
            break;
        }
          
        case 'l': {
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
		strcpy(itemname, ptr+1);

	    if (compact_string(itemname, ellipsis_string, ftpFilenameLength,
		       ftpEllipsisMode, ftpEllipsisLength)) {
		strcpy(itemname, ellipsis_string);
	    }
        }
          
        case '-': {
            
            /* If this is a link type, and the bytes are small, 
               it is probably a directory so lets not show the byte
               count */

		if (compact_string(itemname,ellipsis_string,ftpFilenameLength,ftpEllipsisMode,ftpEllipsisLength)) {
			strcpy(itemname, ellipsis_string);
		}
                sprintf(buffer, "%s", itemname);

	    if (ftp_type[0] != '\0')
		strip_VMS_version(itemname);

            format = HTFileFormat(itemname, &pencoding, WWW_SOURCE, &cmpr);
            
            if (1) {
                HText_appendText(HT, "<IMG SRC=\"");

                /* If this is a link, and we can't figure out what
                   kind of file it is by extension, throw up the unknown
                   icon; however, if it isn't a link and we can't figure
                   out what it is, throw up the text icon...
                   
                   Unless it's compressed. */
                if (itemtype == 'l' && cmpr == COMPRESSED_NOT) {
                    /* If it's unknown, let's call it a menu (since symlinks
                       are most commonly used on FTP servers to point to
                       directories, IMHO... -marc */
                    HText_appendText(HT, HTgeticonname(format, "directory") );
                } else {
                    HText_appendText(HT, HTgeticonname(format, "text")); 
                }
                
                HText_appendText(HT, "\"> ");
            } else {
                HText_appendText(HT, "<IMG SRC=\"");
                HText_appendText(HT, HTgeticonname(format, "application")); 
                HText_appendText(HT, "\"> ");
            }

            break;
          }

          default: {
              HText_appendText(HT, "<IMG SRC=\"");
              HText_appendText(HT, HTgeticonname(NULL, "unknown")); 
              HText_appendText(HT, "\"> ");
              break;
          }
      }
      
	HText_appendText(HT, buffer);
	HText_appendText(HT, "</A>");

	nStringLen = strlen(buffer);
	nSpaces = ftpFilenameLength - nStringLen;
/*
	if (itemtype != 'd') {
*/

		if (nTime == 1) {
			struct tm *ptr;
			time_t t;

			t = time(0);
			ptr = localtime(&t);
			sprintf(szYear, "%d", 1900+ptr->tm_year);
			sprintf(szDate, "%*s%9s %s %s %s %2.2s, %s", nSpaces,
				" ", itemsize, szFileInfo, szTime, szMonth,
				szDay, szYear); 
		}
		else if (nTime == 0) {
			sprintf(szDate, "%*s%9s %s %s %s %2.2s, %s", nSpaces,
				" ", itemsize, szFileInfo, "     ", szMonth,
				szDay, szYear);
		}
		else {
			/* nSpaces += strlen(itemsize); */
			sprintf(szDate, "%*s  %9.9s  %s %s", nSpaces, " ",
				itemsize, szMonth, szTime);
		}
/*
	} else {
		nOldSpaces = nSpaces;
		nSpaces += 22;
		if (nTime == 1) {
			sprintf(szDate, "%*s  %s %s %2.2s", nSpaces, szFileInfo, szTime, szMonth, szDay); 
		}
		else if (nTime == 0) {
			sprintf(szDate, "%*s  %s %s %2.2s, %s", nSpaces, szFileInfo, "00:00", szMonth, szDay, szYear); 
		}
		else {
			sprintf(szDate, "%*s             %s %s", nOldSpaces, " ", szMonth, szTime);  
		}
	}
*/

	if (usingNLST != 2) {
		HText_appendText(HT, szDate);
	}
	HText_appendText(HT, "\n");

	free (full_ftp_name);
    }
  
  HText_appendText(HT, "</DL>\n");
  HText_appendText(HT, "</PRE>\n");
  HText_endAppend(HT);

  /* Some server (wu) don't return a status line at this point */
  broken_crap_hack = 2;
  rv = response(NIL);
  if (broken_crap_hack == 0) {
    return HT_LOADED;
  } else {
    broken_crap_hack = 0;
  }

  if (rv == HT_INTERRUPTED)
    return rv;
  return rv == 2 ? HT_LOADED : -1;
}



/*	Retrieve File from Server
**	-------------------------
**
** On entry,
**	name		WWW address of a file: document, including hostname
** On exit,
**	returns		Socket number for file if good.
**			<0 if bad.
*/

#ifdef SOCKETSHR
jmp_buf accept_env;

void hung_accept()
{
  longjmp(accept_env,1);
}
#endif /* SOCKETSHR, GEC */

PUBLIC int HTFTPLoad
ARGS4 (
  char *,			name,
  HTParentAnchor *,		anchor,
  HTFormat,			format_out,
  HTStream *,			sink
)
{
  BOOL isDirectory = NO;
  int status;
  int retry;			/* How many times tried? */
  HTFormat format;
  int compressed = 0;
#ifdef SOCKETSHR
  int retrya = 0;
  int retryl = 0;
  int stimer = 8;
#endif

#ifdef SOCKETSHR
do {
#endif
  HTProgress("Retrieval in progress");
  if (fTimerStarted) {
	XtRemoveTimeOut(timer);
	fTimerStarted = 0;
  }
  HT = HText_new();
  HText_beginAppend(HT);
  
  for (retry = 0; retry < 2; retry++) {
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf(stderr, "FTP: TRYING in HTFTPLoad, attempt %d\n", retry);
#endif
      status = get_connection(name);
      if (status < 0) {
          CLOSE_CONTROL(control);
          control = -1;
          /* HT_INTERRUPTED will fall through. */
	  loading_length = -1;
          return status;
      }
      
      status = get_listen_socket();
      if (status < 0)
        {
	  HText_appendText(HT, "FTP terminated because login failed");
	  HTProgress("Login failed");
          CLOSE_CONTROL(control);
          control = -1;
          close_master_socket();
	  loading_length = -1;
          /* HT_INTERRUPTED would fall through, if we could interrupt
             somehow in the middle of it, which we currently can't. */
          return status;
        }
      
      /* Inform the server of the port number we will listen on */
      {
        status = response(port_command);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf(stderr, "FTP: Interrupted in response (port_command)\n");
#endif
	    loading_length = (-1);
            HTProgress("Connection interrupted.");
            CLOSE_CONTROL(control);
            control = -1;
            close_master_socket ();
            return HT_INTERRUPTED;
        }
        if (status != 2) {		/* Could have timed out */
            if (status < 0) {
                CLOSE_CONTROL(control);
                control = -1;
                close_master_socket();
                continue;		/* Try again - net error*/
            }

            CLOSE_CONTROL(control);
            control = -1;
            close_master_socket();
	    loading_length = -1;
            return HT_NOT_LOADED;	/* Bad reply */
        }
#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf(stderr, "FTP: Port defined.\n");
#endif
      }
      status = 0;
      break;	/* No more retries */
  } /* For retries */

  if (status < 0) {
      close_master_socket();
      CLOSE_CONTROL(control);
      control = -1;
      loading_length = -1;
      return status;	/* Failed with this code */
  }
  
  /* Ask for the file: */    
  {
    char *filename = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);
    char *fname, *ptr;
    char command[LINE_LENGTH+1];
    HTAtom *encoding;

    if(!filename || !(*filename)) 
      StrAllocCopy(filename, "/");

    if (ftp_type[0] != '\0') {
	/* VMS FTP server, so strip version for testing file format */
	char *temp_filename;

	temp_filename = strdup(filename);
	strip_VMS_version(temp_filename);
	format = HTFileFormat(temp_filename, &encoding, WWW_PLAINTEXT,
		&compressed);
        free(temp_filename);
    } else {
      format = HTFileFormat(filename, &encoding, WWW_PLAINTEXT, &compressed);
    }
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "FTP: filename is %s\n", filename);
#endif
    if (ftp_type[0] != '\0') {
        filename = Convert_VMS_FTP_name(filename);
#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf(stderr, "FTP: VMS filename is %s\n", filename);
#endif
    }

/*
 * The UCX and TWG servers return data differently depending on whether
 * binary or ASCII mode is set.  MultiNet and MadGoat as well as UNIX servers
 * work well with all binary.
 * For UCX and TWG, check the setting in the Options menu, ftp binary
 * mode box.
 * CMU and TCPware are unknown for now so assume they are different.
 *
 * Give UNIX users the choice, because it does make a difference in some
 *   cases.
 */
    if (ftp_type[0]=='\0' ||
	!strcmp(ftp_type, "UCX") || !strcmp(ftp_type, "TWG") ||
        !strcmp(ftp_type, "CMU") || !strcmp(ftp_type, "TCPware")) {
      if (get_pref_boolean(eFTP_BINARY_MODE))
        sprintf(command, "TYPE %s%c%c", "I", CR, LF);
      else
        sprintf(command, "TYPE %s%c%c", "A", CR, LF);
    } else {
      sprintf(command, "TYPE %s%c%c", "I", CR, LF);
    }

    status = response(command);
    if (status != 2) {
        if (status == HT_INTERRUPTED)
          HTProgress("Connection interrupted.");
        close_master_socket();
        CLOSE_CONTROL(control);
        control = -1;
        free(filename);
	loading_length = (-1);
        return (status == HT_INTERRUPTED) ? HT_INTERRUPTED : -1;
    }

    fname = strdup(filename);

    if (ftp_type[0] != '\0') {
        sprintf(command, "CWD %s%c%c", ftp_root_directory, CR, LF);
        status = response(command);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
		fprintf(stderr, "FTP: Interrupted while sending CWD\n");
#endif
            HTProgress("Connection interrupted.");
            CLOSE_CONTROL(control);
            control = -1;
            close_master_socket();
            free(filename);
            loading_length = -1;
            return HT_INTERRUPTED;
        }
    }

    if ((ftp_type[0] == '\0') || (fname[strlen(fname) - 1] != ']')) {
	sprintf(command, "RETR %s%c%c", fname, CR, LF);
	status = response(command);
    } else {
	/* Don't try to retrieve a VMS directory */
	status = 5;
    }
    if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf(stderr, "FTP: Interrupted while sending RETR\n");
#endif
        HTProgress("Connection interrupted.");
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();
        free(filename);
	loading_length = -1;
        return HT_INTERRUPTED;
    }

    if (status != 1) {  /* Failed : try to CWD to it */
        sprintf(command, "CWD %s%c%c", fname, CR, LF);
        status = response(command);
        if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
            if (www2Trace)
              fprintf(stderr, "FTP: Interrupted while sending CWD\n");
#endif
            HTProgress("Connection interrupted.");
            CLOSE_CONTROL(control);
            control = -1;
            close_master_socket();
            free(filename);
	    loading_length = -1;
            return HT_INTERRUPTED;
        }

	/* If we failed CWD and are using VMS server, skip it */
	if ((status == 5) && (ftp_type[0] != '\0')) {
		goto skipDir;
	}

	/* If status is 2, we successfully did a CWD */
        if (status == 2) {
                if (ftp_type[0] == '\0') {
		    /* Successed : let's NLST it */
		    isDirectory = YES;
		    usingNLST = 1;
		    sprintf(command, "NLST %s %c%c", NLST_PARAMS, CR, LF);
		    status = response(command);
		    if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
			if (www2Trace)
				fprintf(stderr,
				       "FTP: Interrupted while sending LIST\n");
#endif
			HTProgress("Connection interrupted.");
			CLOSE_CONTROL(control);
			control = -1;
			close_master_socket();
			free(filename);
			loading_length = (-1);
			return HT_INTERRUPTED;
		    }
                } else {
		    status = 5;
		}
                if (strcmp(ftp_type, "MultiNet") &&
                    strcmp(ftp_type, "MadGoat") &&
                    strcmp(ftp_type, "TCPware") &&
                    strcmp(ftp_type, "UCX")) {

		    if (status == 5) { /* Unrecognized command or failed */
			isDirectory = YES;
			usingNLST = 2;
			sprintf(command, "NLST%c%c", CR, LF);
			status = response(command);
			if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
				if (www2Trace)
				    fprintf(stderr,
				       "FTP: Interrupted while sending NLST\n");
#endif
				HTProgress("Connection interrupted.");
				CLOSE_CONTROL(control);
				control = -1;
				close_master_socket();
				free(filename);
				loading_length = -1;
				return HT_INTERRUPTED;
			}
		    }
                }

		if (status == 5) { /* Unrecognized command or failed */
			isDirectory = YES;
			usingNLST = 0;
			sprintf(command, "LIST%c%c", CR, LF);
			status = response(command);
			if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
				if (www2Trace)
				    fprintf(stderr,
				       "FTP: Interrupted while sending LIST\n");
#endif
				HTProgress("Connection interrupted.");
				CLOSE_CONTROL(control);
				control = -1;
				close_master_socket();
				free(filename);
				loading_length = (-1);
				return HT_INTERRUPTED;
			}
		}
	}
    }

skipDir:

    free(filename);
    free(fname);
    if (status != 1) {
#ifdef SOCKETSHR
#ifndef DISABLE_TRACE
        if (www2Trace)
	  fprintf(stderr, "FTP: LIST directory returned with status %d\n",
		status);
#endif
#endif
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();
#ifdef SOCKETSHR
	if (retryl != 8) {
	    retryl++;
	    sleep(5);
	    fprintf(stderr, "FTP: Directory retry %d.\n", retryl);
            continue;
	}
#ifndef DISABLE_TRACE
        if (www2Trace)
	  fprintf(stderr, "FTP: Retries failed.  Now return.\n");
#endif
        HTProgress("Directory list failed due to SOCKETSHR/NETLIB bug.");
#endif
	loading_length = -1;
        return HT_NOT_LOADED; /* Action not started */
    }
  }
  
  /* Wait for the connection */
  {
    struct sockaddr_in soc_address;

#if !defined(VMS) || (__DECC_VER < 50230003)
    int soc_addrlen = sizeof(soc_address);
#else
    size_t soc_addrlen = sizeof(soc_address);
#endif
#ifdef SOCKETSHR
    if (setjmp(accept_env))
      {
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();
	if (retrya != 8)
	  {
	    retrya++;
	    fprintf(stderr, "FTP: Retry %d.\n", retrya);
            continue;
	  }
#ifndef DISABLE_TRACE
        if (www2Trace)
	  fprintf(stderr, "FTP: Retries failed.  Now return.\n");
#endif
        HTProgress("Connection failed due to SOCKETSHR/NETLIB bug.");
	loading_length = (-1);
        return HT_NOT_LOADED;
      }
    signal(SIGALRM, hung_accept);
    alarm(stimer);
#endif /* SOCKETSHR, GEC */
#ifdef SOCKS
    status = Raccept(master_socket,
#else
    status = accept(master_socket,
#endif
                    (struct sockaddr *)&soc_address,
#ifndef __GNUC__
                    &soc_addrlen);
#else
                    (int)&soc_addrlen);
#endif /* GNU C is picky, GEC */
#ifdef SOCKETSHR
    alarm(0);
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf (stderr, "FTP: Returned from accept\n");
#endif
#endif

    if (status < 0) {
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();
        /* We can't interrupt out of an accept. */
	loading_length = -1;
        return HT_NOT_LOADED;
    }

#ifndef DISABLE_TRACE
    if (www2Trace) {
	fprintf(stderr, "FTP: Accepted new socket %d\n", status);
    }
#endif

    data_soc = status;
  }

  if (isDirectory) 
    {
      int s = read_directory(anchor, name, format_out, sink);

      loading_length = -1;
      close_master_socket();
      NETCLOSE(data_soc);
      data_soc = -1;

#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf(stderr, "FTP: Returning %d after doing read_directory\n", s);
#endif
      /* HT_INTERRUPTED should fall right through. */
      return s;
    } 
  else 
    {
      /* We reproduce ParseSocket below because of socket/child process
         problem. */
      HTStream *stream;
      HTStreamClass targetClass;    
      int rv;
      
      stream = HTStreamStack(format,
                             format_out,
                             compressed,
                             sink, anchor);
      
      if (!stream) {
          char buffer[1024];	/* @@@@@@@@ */
          sprintf(buffer, "Sorry, can't convert from %s to %s.",
                  HTAtom_name(format), HTAtom_name(format_out));
          HTProgress(buffer);
#ifndef DISABLE_TRACE
          if (www2Trace) 
            fprintf(stderr, "FTP: %s\n", buffer);
#endif
	  loading_length = (-1);
          return HT_NOT_LOADED;
      }
      
      targetClass = *(stream->isa);	/* Copy pointers to procedures */
      ftpKludge = 1;
      rv = HTCopy(data_soc, stream, 0, NULL);
      ftpKludge = 0;
      loading_length = -1;
      if (rv == -1) {
          rv = HT_INTERRUPTED;
      } else {
          (*targetClass.end_document)(stream);
          /* Do NOT call *targetClass.free yet -- sockets aren't closed. */
          rv = HT_LOADED;
      }

#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf(stderr,
		"FTP: Got back %d from our local equivalent of ParseSocket\n",
		 rv);
#endif

      /* Reset buffering to control connection -- probably
         no longer necessary, since we don't use a connection
         more than once. */
      HTInitInput(control);

#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf(stderr, "FTP: Closing data socket %d\n", data_soc);
#endif
      NETCLOSE(data_soc);
      data_soc = -1;

      /* Unfortunately, picking up the final reply sometimes causes
         serious problems.  It *probably* isn't safe not to do this,
         as there is the possibility that FTP servers could die if they
         try to send it and we're not listening.

         Testcase for problems (10/30/93): uxc.cso.uiuc.edu,
         AnswerGarden COPYRIGHT in X11R5 contrib clients.
         
         Of course, we may already be triggering hostile actions
         by allowing client-side interrupts as follows... */
      if (rv != HT_INTERRUPTED)
        {
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf(stderr, "FTP: Picking up final reply...\n");
#endif

          status = response(NIL);		/* Pick up final reply */
          if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
              if (www2Trace)
                fprintf(stderr, "FTP: Interrupted picking up final reply.\n");
#endif
              HTProgress("Connection interrupted.");

              CLOSE_CONTROL(control);
              control = -1;
              close_master_socket();

              (*targetClass.handle_interrupt)(stream);

              return HT_INTERRUPTED;
          }
          if (status != 2) {
              CLOSE_CONTROL(control);
              control = -1;
              close_master_socket();
              return HT_NOT_LOADED;
          }
        }
      
      close_master_socket();

      if (rv != HT_INTERRUPTED) {
          /* WAIT until all sockets have been closed. */
#ifndef DISABLE_TRACE
          if (www2Trace)
            fprintf(stderr, "FTP: Calling free method, finally.\n");
#endif
          (*targetClass.free)(stream);
      }

      timer = XtAppAddTimeOut(app_context, ftp_timeout_val * 1000,
		close_it_up, NULL);
      fTimerStarted = 1;

      return rv == HT_INTERRUPTED ? HT_INTERRUPTED : HT_LOADED;
    }
#ifdef SOCKETSHR
  } while (1);
#endif
} /* End HTFTPLoad */


/* HTFTPMkDir  Request that a directory be created on the FTP site.
** Expects:    *name is a pointer to a string that consists of the FTP URL with 
**                   the remote directory name.
** Returns     0 if successful, nonzero on error
*/
PUBLIC int HTFTPMkDir ARGS1 ( char *, name )
{
 char *curpath, *path;
 char command[LINE_LENGTH + 1];
 int status, method = 0;

 HTProgress("FTP mkdir in progress");
 if (fTimerStarted) {
   XtRemoveTimeOut(timer);
   fTimerStarted = 0;
 }

 /* Some routines may try to append text into it, so NULL it */
 HT = NULL;

 /* Open a connection (or get a cached connection) to the FTP site */
 status = get_connection(name);
 if (status < 0) {
   CLOSE_CONTROL(control);
   control = -1;
   return status;
 }

 /* The remote directory name is in the url, so pull it out 
    i.e. ftp://warez.yomama.com/pub/31337&warez_boy
    means to make the directory warez_boy at ftp://warez.yomama.com/pub/31337
 */
 if ((path = strchr(name, '&')) == NULL) { /* No dirname in this URL */
   close_master_socket();
   CLOSE_CONTROL(control);
   control = -1;
   return -1;
 }
 *path = '\0';              /* Make the url normal */
 path++;            /* Move to the dirname */
 /* path is the directory name to create */

 curpath = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION); 
 if (!curpath || !(*curpath)) 
   curpath = strdup("/");
 /* curpath is the remote directory in which to create path */

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "FTPmkdir: path is %s\n", curpath);
#endif
    if (ftp_type[0] != '\0') {
        char *ptr;

        curpath = Convert_VMS_FTP_name(curpath);

        /* [dir1]dir2 is not allowed, convert to [dir1.dir2] */
        ptr = curpath + strlen(curpath) - 1;
        if ((*ptr != ']') && strchr(curpath, ']')) {
           /* Append a ] */
           ptr++;
           *ptr = ']';
           ptr++;
           *ptr = '\0';

           /* Change first ] to a . */
           ptr = strchr(curpath, ']');
           *ptr = '.';
        }

#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf(stderr, "FTPmkdir: VMS path is %s\n", curpath);
#endif
    }

    if (ftp_type[0] != '\0') {
        sprintf(command, "CWD %s%c%c", ftp_root_directory, CR, LF);
        status = response(command);
        if (status != 2) {
          close_master_socket();
          CLOSE_CONTROL(control);
          control = -1;
          if (status == HT_INTERRUPTED)
            HTProgress("Connection interrupted.");
          return (status == HT_INTERRUPTED) ? -2 : -1;
        }
    }

 /* First change to current directory on the server */
 sprintf(command, "CWD %s%c%c", curpath, CR, LF);
 status = response(command);
 if (status != 2) {
   close_master_socket();
   CLOSE_CONTROL(control);
   control = -1;
   if (status == HT_INTERRUPTED) 
     HTProgress("Connection interrupted");
   return (status == HT_INTERRUPTED) ? -2 : -1;
 }

 /* Now create the directory on the server */
 sprintf(command, "MKD %s%c%c", path, CR, LF);
 status = response(command);
 if (status != 2) {
   close_master_socket();
   CLOSE_CONTROL(control);
   control = -1;
   if (status == HT_INTERRUPTED) 
     HTProgress("Connection interrupted");
   return (status == HT_INTERRUPTED) ? -2 : -1;
 }

 /* Clean up */
 close_master_socket();
 timer = XtAppAddTimeOut(app_context, ftp_timeout_val*1000, close_it_up, NULL);
 fTimerStarted = 1;
 HTProgress("Created remote directory.");
 return 0;
} /* End HTFTPMkDir */


/* HTFTRemove  Request that a file (or directory) be removed from the FTP site
** Expects:    *name is a pointer to a string that consists of the FTP URL with the remote filename
**             included.
** Returns     0 if successful, nonzero on error
*/
PUBLIC int HTFTPRemove ARGS1 ( char *, name )
{
 char *fname, *path, *name_ptr;
 char filename[LINE_LENGTH+1];
 char command[LINE_LENGTH+1];
 int status, method = 0, didIt = 0;

 HTProgress("FTP remove in progress");
 if(fTimerStarted) {
   XtRemoveTimeOut(timer);
   fTimerStarted = 0;
 }

 /* Some routines may try to append text into it, so NULL it */
 HT = NULL;

 /* Open a connection (or get a cached connection) to the FTP site */
 status = get_connection(name);
 if (status < 0) {
   CLOSE_CONTROL(control);
   control = -1;
   return status;
 }

 /* Pull out the filename (and path) */
 fname = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);
 if (!fname || !(*fname)) 
   StrAllocCopy(fname, "/");

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "FTPremove: filename is %s\n", fname);
#endif
    if (ftp_type[0] != '\0') {
        fname = Convert_VMS_FTP_name(fname);
#ifndef DISABLE_TRACE
        if (www2Trace)
          fprintf(stderr, "FTPremove: VMS filename is %s\n", fname); /* PGE */
#endif
    }

 /* Pull out just the filename */
 if (ftp_type[0] != '\0')
    name_ptr = strrchr(fname, ']');
 else
    name_ptr = strrchr(fname, '/');
 if (name_ptr) {
    name_ptr++;
    strcpy(filename, name_ptr);
    if (ftp_type[0] != '\0')
       *name_ptr = '\0';
 } else {
    filename[0] = '\0';
 } 
 if (!(*filename)) {   /* No filename in the URL */
   close_master_socket();
   CLOSE_CONTROL(control);
   control = -1;
   return -1;
 }
 
 /* fname is the full path to the file, *filename is just the filename */
 for (method = 0; method < 2; method++ ) {

    if (ftp_type[0] != '\0') {
        sprintf(command, "CWD %s%c%c", ftp_root_directory, CR, LF);
        status = response(command);
        if (status != 2) {
          close_master_socket();
          CLOSE_CONTROL (control);
          control = -1;
          if (status == HT_INTERRUPTED)
            HTProgress("Connection interrupted.");
          return (status == HT_INTERRUPTED) ? -2 : -1;
        }
    }

   switch (method) {

     /* First, attempt to CWD to fname, if successful, fname is a directory. 
      So, CDUP to get to the parent and call RMD on filename  */
   case 0:
     if (ftp_type[0] == '\0') {
       sprintf(command, "CWD %s%c%c", fname, CR, LF);
       status = response(command);
     } else {
       sprintf(command, "CWD %s%c%c", fname, CR, LF);
       status = response(command);
       sprintf(command, "CWD %s%c%c", filename, CR, LF);
       status = response(command);
     }
     if (status != 2) {
       if (status == 5) {  /* Not a directory */
           continue;
       }
       close_master_socket();
       CLOSE_CONTROL(control);
       control = -1;
       if (status == HT_INTERRUPTED) 
           HTProgress ("Connection interrupted.");
       return (status == HT_INTERRUPTED) ? -2 : -1;
     }
     /* Must be a directory, move up and RMD it*/
     if (ftp_type[0] != '\0') {
        sprintf(command, "CDUP%c%c", CR, LF);
     } else {
        *(name_ptr-1) = 0; /* Make fname -> just the path of parent directory */
        sprintf(command, "CWD %s%c%c", fname, CR, LF);
     }
     status = response(command);
     if (status != 2) {
       close_master_socket();
       CLOSE_CONTROL(control);
       control = -1;
       if (status == HT_INTERRUPTED) 
           HTProgress("Connection interrupted.");
       return (status == HT_INTERRUPTED) ? -2 : -1;
     }

     if (ftp_type[0] != '\0')
        strcat(filename, ".dir;0");

     sprintf(command, "RMD %s%c%c", filename, CR, LF);
     status = response(command);
     if (status != 2) {
       close_master_socket();
       CLOSE_CONTROL(control);
       control = -1;
       if (status == HT_INTERRUPTED)
       HTProgress("Connection interrupted.");
       return (status == HT_INTERRUPTED) ? -2 : status;
     }
     didIt = 1;
     break;

     /* If the first attempt failed, CWD to fname and DELE filename */
   case 1:
     if (ftp_type[0] == '\0')
        *(name_ptr-1) = 0;  /* Make fname -> just the path of the file */
     sprintf(command, "CWD %s%c%c", fname, CR, LF);
     status = response(command);
     if (status != 2) {
       close_master_socket();
       CLOSE_CONTROL(control);
       control = -1;
       if (status == HT_INTERRUPTED) {
           HTProgress("Connection interrupted.");
       }
       return (status == HT_INTERRUPTED) ? HT_INTERRUPTED : status;
     }

     if (ftp_type[0] != '\0') {
        if (strchr(filename, ';') == 0)
           strcat(filename, ";*");
     }

     sprintf(command, "DELE %s%c%c", filename, CR, LF);
     status = response(command);
     if (status != 2) {
       close_master_socket();
       CLOSE_CONTROL(control);
       control = -1;
       if (status == HT_INTERRUPTED) {
           HTProgress("Connection interrupted.");
       }
       return (status == HT_INTERRUPTED) ? -2 : status;
     }
     didIt = 1;
     break;
   } /* End of switch (method) */
   if (didIt)
     break;
 } /* End for(method.. */

 /* Clean up */
 close_master_socket();
 timer = XtAppAddTimeOut(app_context, ftp_timeout_val*1000, close_it_up, NULL);
 fTimerStarted = 1;
 return 0;
} /* End HTFTPRemove */


/* HTFTPSend   Send File to the FTP site
** Expects:    *name is a pointer to a string that consists of the FTP URL
**	       with the local filename appended to the URL (delimited by an &.
**	       i.e. ftp://warez.mama.com/pub&/tmp/bubba.tgz
**             would send /tmp/bubba.tgz to warez.mama.com:/pub
** Returns     0 if successful, nonzero on error
*/
#define OUTBUFSIZE 4096  /* Size of the chunk of the file read */
PUBLIC int HTFTPSend ARGS1 (char *, name ) {
 int status;
 FILE *f;
 char *fname, *filename, *path;
 char command[LINE_LENGTH+1], outBuf[OUTBUFSIZE+1];
 long bLeft, bDone, bTotal, chunk;
 extern int twirl_increment;
 int next_twirl = twirl_increment, intr = 0;
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

      /* The local filename is in the url, so pull it out 
            i.e. ftp://warez.yomama.com/pub/31337&/u/warezboy/Mosaic0.1a.tar.gz
            means to send /u/warezboy/Mosaic0.1a.tar.gz to warez.yomama.com/pub/31337
      */
      if ((fname = strchr(name, '&')) == NULL) {
	/* No local filename in this URL */
        close_master_socket();
        CLOSE_CONTROL(control);
        control = -1;
        return -1;
      }

      *fname = '\0';        /* Make the url normal */
      fname++;              /* Move to the filename */

#ifndef VMS   /* PGE, Use VMS file syntax for local file */
      filename = strrchr(fname, '/');
      filename++;
      if (!*filename) {
#else
      filename = strrchr(fname, ']');
      if (!filename || !(*filename++) || !*filename) {   /* No filename */
#endif
        close_master_socket();
        CLOSE_CONTROL(control);
        control = -1;
        return -1;            
      }

      /* *fname is the full path and filename, *filename is filename */ 
      /* Get size information */
#if defined(MULTINET) && defined(__alpha)
      if (decc$stat(fname, &sbuf) < 0) {
#else
      if (stat(fname, &sbuf) < 0) {
#endif /* Alpha DEC C couldn't find it otherwise ?????, GEC */
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();
        return -1;
      }
              
      bTotal = sbuf.st_size;
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf(stderr, "HTFTPSend: Attempting to send %s (%s) (%lu)\n",
		fname, filename, bTotal);
#endif

      status = get_connection(name);
      if (status < 0) {
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();
        return status;
      }

      status = get_listen_socket();
      if (status < 0) {
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();
        return status;
      }
      
      status = response(port_command);
      if (status == HT_INTERRUPTED) {
        HTProgress("Connection interrupted.");
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();
        return -2;
      }

      if (status != 2) {      /* If the port command was not successful */
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();
        if (status < 0) { /* If we were to try again, we would do it here.... */
          return -3;
        }
        return -3;
      }
      
      /* Logged in, set up the port, now let's send the sucka */

        if (get_pref_boolean(eFTP_BINARY_MODE))
          sprintf(command, "TYPE %s%c%c", "I", CR, LF);
        else
          sprintf(command, "TYPE %s%c%c", "A", CR, LF);

      status = response (command);    
      if (status != 2) {
              close_master_socket();
              CLOSE_CONTROL(control);
              control = -1;
              if (status == HT_INTERRUPTED) 
                      HTProgress("Connection interrupted.");
              return (status == HT_INTERRUPTED) ? -2 : -1;        
      }
      
    if (ftp_type[0] != '\0')
      {
        sprintf(command, "CWD %s%c%c", ftp_root_directory, CR, LF);
        status = response(command);
        if (status != 2) {
          close_master_socket();
          CLOSE_CONTROL(control);
          control = -1;
          if (status == HT_INTERRUPTED)
            HTProgress("Connection interrupted.");
          return (status == HT_INTERRUPTED) ? -2 : -1;
        }
      }

      /* Move to correct directory */
      path = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);
      if (!path || !(*path))
              StrAllocCopy(path, "/");
              
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "FTPsend: path is %s\n", path);
#endif
    if (ftp_type[0] != '\0')
      {
        char *ptr;

        path = Convert_VMS_FTP_name(path);

        /* [dir1]dir2 is not allowed, convert to [dir1.dir2] */
        ptr = path + strlen(path) - 1;
        if ((*ptr != ']') && strchr(path, ']')) {
           /* append a ] */
           ptr++;
           *ptr = ']';
           ptr++;
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

      sprintf(command, "CWD %s%c%c", path, CR, LF);
      status = response(command);

      if (status != 2) {
              close_master_socket();
              CLOSE_CONTROL(control);
              control = -1;
              if (status == HT_INTERRUPTED) 
                      HTProgress("Connection interrupted.");
              return (status == HT_INTERRUPTED) ? -2 : -1;        
      }

      if ((f = fopen( fname, "r")) == NULL) {
        CLOSE_CONTROL(control);
        control = -1;
        close_master_socket();         
        return -1;
      }

      if (ftp_type[0] != '\0') {
        char *semicolon = strchr(filename, ';');

        if (semicolon)
          *semicolon = '\0';
      }

      /* Send it */
      sprintf(command, "STOR %s%c%c", filename, CR, LF);
      status = response(command);
      if (status == HT_INTERRUPTED) {
              HTProgress("Connection interrupted.");
              CLOSE_CONTROL(control);
              control = -1;
              close_master_socket();
              return -2;
      }
      
      if (status != 1) { /* Does not seem to understand the STOR command */
              HTProgress("FTP host does not understand STOR command.");
              CLOSE_CONTROL(control);
              control = -1;
              close_master_socket();
              return -3;
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
              CLOSE_CONTROL(control);
              control = -1;
              close_master_socket();
              return -2;
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
        if (chunk > 0) {
          NETWRITE(data_soc, outBuf, chunk);
        }
        if (chunk < OUTBUFSIZE) {
          bDone = bTotal;
          break;
        }
        bDone += chunk;

        HTMeter((bDone * 100) / bTotal, NULL);
      }       
      
      bLeft = bTotal - bDone;

      /* Done, now clean up */
      fclose(f);
      HTMeter(100, NULL);

      close_master_socket();

#ifndef DISABLE_TRACE
      if(www2Trace)
        fprintf(stderr, "HTFTPSend: Closing data socket\n");
#endif
      NETCLOSE(data_soc);
      data_soc = -1;

      status = response (0);
      if (status == HT_INTERRUPTED) {
              HTProgress("Connection interrupted.");
              CLOSE_CONTROL(control);
              control = -1;
              close_master_socket();
              return -2;
      }

      if (bLeft != 0)  {
#ifndef DISABLE_TRACE
        if(www2Trace)
          fprintf(stderr, "HTFTPSend: Error sending file %lu bytes left\n",
		 bLeft);
#endif
        return intr ? -2 : -1;
      }
      
      timer = XtAppAddTimeOut(app_context, ftp_timeout_val * 1000, close_it_up,
		 NULL);
      fTimerStarted = 1;
      
#ifndef DISABLE_TRACE
      if (www2Trace)
        fprintf(stderr, "HTFTPSend: File sent, returning OK\n");
#endif
      return 0;      
} /* End of HTFTPSend */

static CLOSE_CONTROL(s)
int s;
{
	if (s != -1)
	    NETCLOSE(s);
	ftpcache.control = -1;
}

static void
close_it_up()
{
	NETCLOSE(ftpcache.control);
	ftpcache.control = -1;
}

/*
 * This code based off of Rick Vestal's FTP parse code for the NCSA Windows
 * Mosaic client.
 *
 * Modified for X by Scott Powers
 * 9.27.95
 */

int ParseFileSizeAndName(char *szBuffer, char *szFileName, char *szSize) {

char *szPtr, *szName, *szEndPtr, *szLength;
static char *tmpbuf = NULL;

	if (!szBuffer) {
		return(0);
	}

	if (!tmpbuf) {
		tmpbuf = (char *)calloc(BUFSIZ, sizeof(char));
	}

	if (usingNLST == 1) {
		strcpy(tmpbuf, szBuffer);

		/* Filename */
		szPtr = strrchr(tmpbuf, ' ');
		while (szPtr && (*szPtr == ' ')) {
			szPtr--;
		}
		*(szPtr+1) = '\0';
		if (szPtr && *szPtr == '>') { /* Deal with a link */
			if (szPtr) {
				szPtr = strrchr(tmpbuf, ' ');
				while (szPtr && (*szPtr == ' ')) {
					szPtr--;
				}
				*(szPtr+1) = '\0';
			}
			if (szPtr) {
				szPtr = strrchr(tmpbuf, ' ');
				while (szPtr && (*szPtr == ' ')) {
					szPtr--;
				}
				*(szPtr+1) = '\0';
			}
		}

		if (szPtr) {
			/* year/time */
			szPtr = strrchr(tmpbuf, ' ');
			while (szPtr && (*szPtr == ' ')) {
				szPtr--;
			}
		}
		*(szPtr+1) = '\0';

		if (szPtr) {
			/* Date */
			szPtr = strrchr(tmpbuf, ' ');
			while (szPtr && (*szPtr == ' ')) {
				szPtr--;
			}
		}
		*(szPtr+1) = '\0';

		if (szPtr) {
			/* Month */
			szPtr = strrchr(tmpbuf, ' ');
			while (szPtr && (*szPtr == ' ')) {
				szPtr--;
			}
		}
		*(szPtr+1) = '\0';

		if (szPtr) {
			/* Filesize */
			szPtr = strrchr(tmpbuf, ' ');
		}

		/* Beginning of filesize */
		szPtr++;

/*
		szSize = szBuffer+(szPtr-tmpbuf);
*/
		strcpy(szSize, szPtr);
	} else {
		szPtr = strrchr(szBuffer, ' ');
		szName = szPtr + 1;

		if (szPtr) {
			strcpy(szFileName, szName);
		}

		/* go to end of file length */
		while (szPtr && *szPtr == ' ') {
			szPtr--;
		}

		szEndPtr = szPtr+1;
		if (*szPtr != '>') {
			while (szPtr && *szPtr != ' ') {
				szPtr--;
			}
			if (szPtr) {
				szLength = szPtr+1;
				strncpy(szSize, szLength, szEndPtr  - szLength);
				szSize[szEndPtr - szLength] = '\0';
			}
		} else {
			return(0);  /* A directory */
		}
	}

	return(1); /* Not a directory */
}


int ParseDate(char *szBuffer, char *szFileInfo, char *szMonth, char *szDay, char *szYear, char *szTime) {
	
char *szPtr, *szEndPtr;
int nCount;
char *tmpbuf = (char *)calloc(BUFSIZ, sizeof(char));

	if (!szBuffer) {
		free(tmpbuf);
		return(0);
	}

	if ((*szBuffer != 'd') && (*szBuffer != 'l') && (*szBuffer != '-')) {
	  	/* Hopefully this is the dos format */

		szPtr = szBuffer;
		strncpy(szMonth, szBuffer, 8);
		szMonth[8] = '\0';

		szPtr = szPtr + 10;
		if (szPtr) {
			strncpy(szTime, szPtr, 7);
			szTime[7] = '\0';
		}

		szPtr = szPtr + 15;
		if (szPtr) {
			if (*szPtr == 'D') {
				*szDay = 'd';
				szDay[1] = '\0';
			}
			else {
				*szDay = 'f';
				szDay[1] = '\0';
			}
		}

		free(tmpbuf);
		return(3); /* ie the info is this dos way */
	} else {
		szPtr = NULL;
		nCount = 0;

		/* Loop to go to each of the month, day, year, whatever parts */
		while (szPtr || ((nCount == 0) && szBuffer)) {
			switch (nCount) {
				case 0:  /* File info */
					strncpy(szFileInfo, szBuffer, 10);
					szFileInfo[10] = '\0';

					if (usingNLST == 1) {
						strcpy(tmpbuf,szBuffer);
						/*filename*/
						szPtr=strrchr(tmpbuf,' ');
						while (szPtr && (*szPtr == ' ')) {
							szPtr--;
						}
						*(szPtr+1)='\0';
						if (szPtr && *szPtr=='>') { /*deal with a link*/
							if (szPtr) {
								szPtr=strrchr(tmpbuf,' ');
								while (szPtr && (*szPtr == ' ')) {
									szPtr--;
								}
								*(szPtr+1)='\0';
							}
							if (szPtr) {
								szPtr=strrchr(tmpbuf,' ');
								while (szPtr && (*szPtr == ' ')) {
									szPtr--;
								}
								*(szPtr+1)='\0';
							}
						}

						if (szPtr) {
							/*year/time*/
							szPtr=strrchr(tmpbuf,' ');
							while (szPtr && (*szPtr == ' ')) {
								szPtr--;
							}
						}
						*(szPtr+1)='\0';

						if (szPtr) {
							/* Date */
							szPtr=strrchr(tmpbuf,' ');
							while (szPtr && (*szPtr == ' ')) {
								szPtr--;
							}
						}
						*(szPtr+1)='\0';

						if (szPtr) {
							/* Month */
							szPtr=strrchr(tmpbuf,' ');
						}
						/* Beginning of month */
						szPtr++;

						szPtr=szBuffer+(szPtr-tmpbuf);
					} else {
						szPtr = strchr(szBuffer, ' ');
						while (szPtr && (*szPtr == ' ')) {
							szPtr++;
						}

						if (szPtr) {  
							szPtr = strchr(szPtr, ' ');
							while (szPtr && (*szPtr == ' '))  {
								szPtr++;
							}
						}
						if (szPtr) {
							szPtr = strchr(szPtr, ' ');
							while (szPtr && (*szPtr == ' '))  {
								szPtr++;
							}
						}
						if (szPtr) {
							szPtr = strchr(szPtr, ' ');
							while (szPtr && (*szPtr == ' '))  {
								szPtr++;
							}
						}
						if (szPtr) {
							szPtr = strchr(szPtr, ' ');
							while (szPtr && (*szPtr == ' '))  {
								szPtr++;
							}
						}
						/* now we are at the month entry */
					}

					break;

				case 1:
					szEndPtr = strchr(szPtr, ' ');
					if (szEndPtr) {
						strncpy(szMonth, szPtr, szEndPtr - szPtr);
						szMonth[szEndPtr - szPtr] = '\0';
						szPtr = szEndPtr+1;  /* go to next entry (day) */
						while (szPtr && (*szPtr == ' '))  {
								szPtr++;
						}
					} else {
						strcpy(szMonth, " ");
					}
					break;	

				case 2:
					szEndPtr = strchr(szPtr, ' ');
					if (szEndPtr) {
						strncpy(szDay, szPtr, szEndPtr - szPtr);
						szDay[szEndPtr - szPtr] = '\0';
						szPtr = szEndPtr+1;  
						while (szPtr && (*szPtr == ' '))  {
								szPtr++;
						}
					} else {
						strcpy(szDay, " ");
					}
					break;

				case 3:
					szEndPtr = strchr(szPtr, ' ');
					if (szEndPtr) {
						strncpy(szYear, szPtr, szEndPtr - szPtr);
						szYear[szEndPtr - szPtr] = '\0';
						szPtr = szEndPtr+1;  
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
		if (!szPtr) {
			free(tmpbuf);
			return(0);  /* ie the info is month, day, year */
		}

		szPtr -= 2;  /* Beginning of time */

		strncpy(szTime, szPtr, 5);
		szTime[5] = '\0';
	
		free(tmpbuf);
		return(1);  /* ie the info is month, day, time */
	}
}

