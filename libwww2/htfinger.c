/*			FINGER ACCESS				HTFinger.c
**			=============
** Authors:
**  ARB  Andrew Brooks
**
** History:
**	21 Apr 94   First version (ARB, from HTNews.c by TBL)
**	12 Mar 96   Made the URL and command buffering secure from
**		     stack modifications, beautified the HTLoadFinger()
**		     and response() functions, and added support for the
**		     following URL formats for sending a "", "/w",
**		     "username[@host]", or "/w username[@host]" command
**		     to the server:
**			finger://host
**			finger://host/
**			finger://host/%2fw
**			finger://host/%2fw%20username[@host]
**			finger://host/w/username[@host]
**			finger://host/username[@host]
**			finger://host/username[@host]/w
**			finger://username@host
**			finger://username@host/
**			finger://username@host/w
**	15 Mar 96   Added support for port 79 gtype 0 gopher URLs
**		     relayed from HTLoadGopher. - FM
*/

#include "../config.h"

#include "HTFinger.h"

#include "HTParse.h"
#include "HTAlert.h"
#include "HTML.h"
#include "HTFormat.h"
#include "HTTCP.h"
#include "HTString.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

#define FINGER_PORT 79		/* See rfc742 */
#define BIG 1024		/* Bug */

#define FREE(x) if (x) {free(x); x = NULL;}

#define PUTC(c) (*targetClass.put_character)(target, c)
#define PUTS(s) (*targetClass.put_string)(target, s)
#define START(e) (*targetClass.start_element)(target, e, 0, 0)
#define END(e) (*targetClass.end_element)(target, e)
#define FREE_TARGET (*targetClass.free)(target)
#define NEXT_CHAR HTGetCharacter() 


/*	Module-wide variables
*/
PRIVATE int s;					/* Socket for FingerHost */

struct _HTStructured {
	WWW_CONST HTStructuredClass *isa;	/* For gopher streams */
	/* ... */
};

PRIVATE HTStructured *target;			/* The output sink */
PRIVATE HTStructuredClass targetClass;		/* Copy of fn addresses */

/*	Initialisation for this module
**	------------------------------
*/
PRIVATE BOOL initialized = NO;
PRIVATE BOOL initialize (void)
{
    s = -1;		/* Disconnected */
    return YES;
}


/*	Start anchor element
**	--------------------
*/
PRIVATE void start_anchor (WWW_CONST char *href)
{
    BOOL present[HTML_A_ATTRIBUTES];
    WWW_CONST char *value[HTML_A_ATTRIBUTES];
    int i;
    
    for (i = 0; i < HTML_A_ATTRIBUTES; i++)
	present[i] = (i == HTML_A_HREF);

    ((WWW_CONST char **)value)[HTML_A_HREF] = href;
    (*targetClass.start_element)(target, HTML_A, present,
    				 (WWW_CONST char **)value);
}

/*	Send Finger Command line to remote host & Check Response
**	--------------------------------------------------------
**
** On entry,
**	command	points to the command to be sent, including CRLF, or is null
**		pointer if no command to be sent.
** On exit,
**	Negative status indicates transmission error, socket closed.
**	Positive status is a Finger status.
*/
PRIVATE int response (WWW_CONST char *command,
		      char *sitename,
		      HTParentAnchor *anAnchor,
		      HTFormat format_out,
		      HTStream *sink)
{
    int length = strlen(command);
    int status, ch, i;
    char line[BIG];
    char *l;
    char *cmd = NULL;
    char *p = line;
    char *href = NULL;
    extern int interrupted_in_htgetcharacter;

    if (!length)
        return(-1);

    /* Set up buffering.
    */
    HTInitInput(s);

    /* Send the command.
    */
#ifndef DISABLE_TRACE
    if (www2Trace) 
        fprintf(stderr, "HTFinger command to be sent: %s", command);
#endif
    status = NETWRITE(s, (char *)command, length);
    if (status < 0) {
#ifndef DISABLE_TRACE
        if (www2Trace)
	    fprintf(stderr,
                    "HTFinger: Unable to send command. Disconnecting.\n");
#endif
        NETCLOSE(s);
        s = -1;
        return status;
    }
  
    /* Make a hypertext object with an anchor list.
    */
    target = HTML_new(anAnchor, format_out, sink);
    targetClass = *target->isa;	 /* Copy routine entry points */

    /* Create the results report.
    */
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "HTFinger: Reading finger information\n");
#endif
    START(HTML_HTML);
    START(HTML_HEAD);
    START(HTML_TITLE);
    PUTS("Finger server on ");
    PUTS(sitename);
    END(HTML_TITLE);
    END(HTML_HEAD);
    START(HTML_BODY);
    START(HTML_H1);
    PUTS("Finger server on ");
    START(HTML_EM);
    PUTS(sitename);
    END(HTML_EM);
    PUTS(": ");
    if (command) {
        StrAllocCopy(cmd, command);
    } else {
        StrAllocCopy(cmd, "");
    }
    for (i = strlen(cmd) - 1; i >= 0; i--) {
        if (cmd[i] == LF || cmd[i] == CR) {
	    cmd[i] = '\0';
	} else {
	    break;
	}
    }
    PUTS(cmd);
    free(cmd);
    END(HTML_H1);
    START(HTML_PRE);

    while ((ch = NEXT_CHAR) != (char)EOF) {
	if (interrupted_in_htgetcharacter) {
#ifndef DISABLE_TRACE
	    if (www2Trace)
	        fprintf(stderr,
		      "HTFinger: Interrupted in HTGetCharacter, apparently.\n");
#endif
	    HTProgress("Connection interrupted.");
	    goto end_html;
        }
	if (ch != LF) {
	    *p = ch;		/* Put character in line */
	    if (p < &line[BIG - 1])
	        p++;
	} else {
	    *p = '\0';		/* Terminate line */
	    /*
	     * OK we now have a line.
	     * Load it as 'l' and parse it.
	     */
	    p = l = line;
	    while (*l) {
		if (strncmp(l, "news:", 5) &&
		    strncmp(l, "snews://", 8) &&
		    strncmp(l, "nntp://", 7) &&
		    strncmp(l, "snewspost:", 10) &&
		    strncmp(l, "snewsreply:", 11) &&
		    strncmp(l, "newspost:", 9) &&
		    strncmp(l, "newsreply:", 10) &&
		    strncmp(l, "ftp://", 6) &&
		    strncmp(l, "file:/", 6) &&
		    strncmp(l, "finger://", 9) &&
		    strncmp(l, "http://", 7) &&
		    strncmp(l, "https://", 8) &&
		    strncmp(l, "wais://", 7) &&
		    strncmp(l, "mailto:", 7) &&
		    strncmp(l, "cso://", 6) &&
		    strncmp(l, "gopher://", 9)) {
		    PUTC(*l++);
		} else {
		    StrAllocCopy(href, l);
		    start_anchor(strtok(href, " \r\n\t,>)\""));
		    while (*l && !strchr(" \r\n\t,>)\"", *l))
		        PUTC(*l++);
		    END(HTML_A);
		    FREE(href);
		}
	    }
	    PUTC('\n');
	}
    }
    NETCLOSE(s);
    s = -1;

 end_html:
    END(HTML_PRE);
    END(HTML_BODY);
    END(HTML_HTML);
    FREE_TARGET;
    return(0);
}


/*		Load by name					HTLoadFinger
**		============
*/
PUBLIC int HTLoadFinger (WWW_CONST char *arg,
			 HTParentAnchor *anAnchor,
			 HTFormat format_out,
			 HTStream *stream)
{
    char *username, *sitename, *colon;	/* Fields extracted from URL */
    char *slash, *at_sign;		/* Fields extracted from URL */
    char *command, *str;		/* Buffers */
    int port;				/* Port number from URL */
    int status;				/* tcp return */
    WWW_CONST char *p1 = arg;
    BOOL IsGopherURL = FALSE;
  
#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "HTFinger: Looking for %s\n", arg ? arg : "NULL");
#endif
  
    if (!(arg && *arg)) {
        HTAlert("Could not load data.");
	return HT_NOT_LOADED;			/* Ignore if no name */
    }
    if (!initialized) 
        initialized = initialize();
    if (!initialized) {
        HTAlert("Could not set up finger connection.");
	return HT_NOT_LOADED;	/* Fail */
    }
    
    /*  Set up the host and command fields.
    */        
    if (!my_strncasecmp(arg, "finger://", 9)) {
        p1 = arg + 9;  /* Skip "finger://" prefix */
    } else if (!my_strncasecmp(arg, "gopher://", 9)) {
        p1 = arg + 9;  /* Skip "gopher://" prefix */
	IsGopherURL = TRUE;
    }
    sitename = (char *)p1;

    if (slash = strchr(sitename, '/')) {
        *slash++ = '\0';
	HTUnEscape(slash);
	if (IsGopherURL) {
	    if (*slash != '0') {
	        HTAlert("Could not load data.");
		return HT_NOT_LOADED;	/* Fail */
	    }
	    *slash++ = '\0';
	}
    }
    if (at_sign = strchr(sitename, '@')) {
        if (IsGopherURL) {
            HTAlert("Could not load data.");
	    return HT_NOT_LOADED;	/* Fail */
	}
        *at_sign++ = '\0';
        username = sitename;
	sitename = at_sign;
	HTUnEscape(username);
    } else if (slash) {
        username = slash;
    } else {
        username = "";
    }
    if (!*sitename) {
        HTAlert("Could not load data (no sitename in finger URL)");
	return HT_NOT_LOADED;		/* Ignore if no name */
    }
    if (colon = strchr(sitename, ':')) {
        *colon++ = '\0';
	port = atoi(colon);
	if (port != 79) {
	    HTAlert("Invalid port number - will only use port 79!");
	    return HT_NOT_LOADED;	/* Ignore if wrong port */
	}
    }

    /* Load the string for making a connection */
    if (!(str = (char *)malloc(strlen(sitename) + 10)))
        outofmem(__FILE__, "HTLoadFinger");
    sprintf(str, "lose://%s/", sitename);
    
    /* Load the command for the finger server.
    */
    if (!(command = (char *)malloc(strlen(username) + 10)))
        outofmem(__FILE__, "HTLoadFinger");
    if (at_sign && slash) {
        if (*slash == 'w' || *slash == 'W') {
	    sprintf(command, "/w %s\r\n", username);
	} else {
	    sprintf(command, "%s\r\n", username);
	}
    } else if (at_sign) {
	sprintf(command, "%s\r\n", username);
    } else if (*username == '/') {
        if (slash = strchr(username + 1, '/'))
	    *slash = ' ';
	sprintf(command, "%s\r\n", username);
    } else if ((*username == 'w' || *username == 'W') &&
    	       *(username + 1) == '/') {
	if (*(username + 2)) {
	    *(username + 1) = ' ';
	} else {
	    *(username + 1) = '\0';
	}
	sprintf(command, "/%s\r\n", username);
    } else if ((*username == 'w' || *username == 'W') && !*(username + 1)) {
	sprintf(command, "/%s\r\n", username);
    } else if (slash = strchr(username, '/')) {
	*slash++ = '\0';
	if (*slash == 'w' || *slash == 'W') {
	    sprintf(command, "/w %s\r\n", username);
	} else {
	    sprintf(command, "%s\r\n", username);
	}
    } else {
	sprintf(command, "%s\r\n", username);
    }
  
    /* Now, let's get a stream setup up from the FingerHost:
    ** CONNECTING to finger host
    */
#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "HTFinger: doing HTDoConnect on '%s'\n", str);
#endif
    status = HTDoConnect(str, "finger", FINGER_PORT, &s);
#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "HTFinger: Done DoConnect; status %d\n", status);
#endif

    if (status == HT_INTERRUPTED) {
        /* Interrupt cleanly */
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr,
	    	    "HTFinger: Interrupted on connect; recovering cleanly.\n");
#endif
	HTProgress("Connection interrupted.");
	free(str);
	free(command);
	return HT_NOT_LOADED;
    }
    if (status < 0) {
        NETCLOSE(s);
	s = -1;
#ifndef DISABLE_TRACE
	if (www2Trace) 
	    fprintf(stderr, "HTFinger: Unable to connect to finger host.\n");
#endif
        HTAlert("Could not access finger host.");
	free(str);
	free(command);
	return HT_NOT_LOADED;	/* Fail */
    }
#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "HTFinger: Connected to finger host '%s'.\n", str);
#endif
    free(str);

    /* Send the command, and process response if successful.
    */
    if (response(command, sitename, anAnchor, format_out, stream)) {
        HTAlert("No response from finger server.");
	free(command);
	return HT_NOT_LOADED;
    }
    free(command);
    return HT_LOADED;
}

PUBLIC HTProtocol HTFinger = { "finger", HTLoadFinger, NULL };
