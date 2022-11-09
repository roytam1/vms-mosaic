/*			      Mosaic Cookie Support		   HTCookie.c
**			      =====================
**
**	Author: AMK	A.M. Kuchling (amk@magnet.com)	12/25/96
**
**	Incorporated with mods by FM			01/16/97
**
**	Adapted from Lynx for VMS Mosaic by GEC		07/18/99
**
**  Based on:
**	http://www.ics.uci.edu/pub/ietf/http/draft-ietf-http-state-mgmt-05.txt
**
**	Updated for:
**   http://www.ics.uci.edu/pub/ietf/http/draft-ietf-http-state-man-mec-02.txt
**		- FM					1997-07-09
**
**	Updated for:
**   ftp://ds.internic.net/internet-drafts/draft-ietf-http-state-man-mec-03.txt
**		- FM					1997-08-02
**
**  TO DO: (roughly in order of decreasing priority)
      * Persistent cookies are still experimental.  Presently cookies
        lose many pieces of information that distinguish
        version 1 from version 0 cookies.  There is no easy way around
        that with the current cookie file format.  Ports are currently
        not stored persistently at all which is clearly wrong.
      * We currently don't do anything special for unverifiable
        transactions to third-party hosts.
      * We currently don't use effective host names or check for
        Domain=.local.
      * Hex escaping isn't considered at all.  Any semi-colons, commas,
	or spaces actually in cookie names or values (i.e., not serving
	as punctuation for the overall Set-Cookie value) should be hex
	escaped if not quoted, but presumeably the server is expecting
	them to be hex escaped in our Cookie request header as well, so
	in theory we need not unescape them.  We'll see how this works
	out in practice.
      * The truncation heuristic in HTConfirmCookie should probably be
	smarter, smart enough to leave a really short name/value string
	alone.
      * We protect against denial-of-service attacks (see section 6.3.1
	of the draft) by limiting a domain to 50 cookies, limiting the
	total number of cookies to 500, and limiting a processed cookie
	to a maximum of 4096 bytes, but we count on the normal garbage
	collections to bring us back down under the limits, rather than
	actively removing cookies and/or domains based on age or frequency
	of use.
      * If a cookie has the secure flag set, we presently treat only SSL
	connections as secure.	This may need to be expanded for other
	secure communication protocols that become standarized.
*/

/* Copyright (C) 1999, 2000, 2003, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include "../config.h"

#include "HTUtils.h"
#include "HTParse.h"
#include "HTTCP.H"
#include "HTFile.h"
#include "HTTP.h"
#include "HTAlert.h"
#include "HTBTree.h"
#include "HTCookie.h"

#include "../src/mosaic.h"
#include "../libnut/str-tools.h"
#include "../libnut/system.h"

#ifndef DISABLE_TRACE
int cookieTrace;
extern int reportBugs;
#endif

extern char *ident_ver;
extern mo_window *current_win;
extern XtAppContext app_context;       /* From gui.c */
extern char *use_this_url_instead;     /* From htaccess.c */
extern HT_SSL_Host *SSL_ignore_hosts;  /* From http.c */

int HTSetCookies = TRUE;
int HTEatAllCookies = FALSE;
int HTCookieFile = TRUE;

/* Default for new domains, one of the invcheck_type values: */
#define DEFAULT_INVCHECK_TYPE INVCHECK_QUERY

#define FREE(x) if (x) { free(x); x = NULL; }

PRIVATE BOOL ForceSSLCookiesSecure = FALSE;

#define CANCELLED "Cancelled!!!"
#define INVALID_COOKIE_DOMAIN_CONFIRMATION \
 "Accept invalid cookie domain=%s for '%s'?"
#define INVALID_COOKIE_PATH_CONFIRMATION \
 "Accept invalid cookie path=%s as a prefix of '%s'?"
#define ALLOWING_COOKIE "Allowing this cookie."
#define REJECTING_COOKIE "Rejecting this cookie."
#define COOKIE_JAR_IS_EMPTY "The Cookie Jar is empty."
#define COOKIE_JAR_TITLE "Mosaic Cookie Jar"
#define REACHED_COOKIE_JAR_PAGE "VMS Mosaic Cookie Jar"
#define COOKIES_NEVER_ALLOWED "(Cookies never allowed.)\n"
#define COOKIES_ALWAYS_ALLOWED "(Cookies always allowed.)\n"
#define COOKIES_ALLOWED_VIA_PROMPT "(Cookies allowed via prompt.)\n"
#define NO_NAME "(No name.)"
#define NO_VALUE "(No value.)"
#define END_OF_SESSION "(End of session.)"
#define DELETE_COOKIE_CONFIRMATION "Delete this cookie?"
#define COOKIE_EATEN "The cookie has been eaten!"
#define DELETE_EMPTY_DOMAIN_CONFIRMATION "Delete this empty domain?"
#define DOMAIN_EATEN "The domain has been eaten!"
#define DOMAIN_COOKIES_EATEN "All cookies in the domain have been eaten!"
#define ALWAYS_ALLOWING_COOKIES "Always allowing from domain '%s'."
#define NEVER_ALLOWING_COOKIES "Never allowing from domain '%s'."
#define PROMTING_TO_ALLOW_COOKIES "Prompting to allow from domain '%s'."
#define DELETE_ALL_COOKIES_IN_DOMAIN "Delete all cookies in this domain?"
#define ALL_COOKIES_EATEN "All of the cookies in the jar have been eaten!"
#define CANNOT_CONVERT_I_TO_O "Sorry, no known way of converting %s to %s."

#define HTTP_URL_TYPE            1
#define FILE_URL_TYPE            2
#define FTP_URL_TYPE             3
#define WAIS_URL_TYPE            4
#define NEWS_URL_TYPE            5
#define NNTP_URL_TYPE            6
#define TELNET_URL_TYPE          7
#define TN3270_URL_TYPE          8
#define RLOGIN_URL_TYPE          9
#define GOPHER_URL_TYPE         10
#define HTML_GOPHER_URL_TYPE    11
#define TELNET_GOPHER_URL_TYPE  12
#define INDEX_GOPHER_URL_TYPE   13
#define MAILTO_URL_TYPE         14
#define FINGER_URL_TYPE         15
#define CSO_URL_TYPE            16
#define HTTPS_URL_TYPE          17
#define SNEWS_URL_TYPE          18
#define PROSPERO_URL_TYPE       19
#define AFS_URL_TYPE            20
#define DATA_URL_TYPE           21
#define NEWSPOST_URL_TYPE       25
#define NEWSREPLY_URL_TYPE      26
#define COOKIEJAR_URL_TYPE      27
#define UNKNOWN_URL_TYPE        28

/*
**  The first level of the cookie list is a list indexed by the domain
**  string; cookies with the same domain will be placed in the same
**  list.  Thus, finding the cookies that apply to a given URL is a
**  two-level scan; first we check each domain to see if it applies,
**  and if so, then we check the paths of all the cookies on that
**  list.  We keep a running total of cookies as we add or delete them.
*/
PRIVATE HTBTree *domain_btree = NULL;
PRIVATE HTList *cookie_list = NULL;
PRIVATE int total_cookies = 0;

struct _cookie {
    char *lynxID;   /* Lynx cookie identifier */
    char *name;     /* Name of this cookie */
    char *value;    /* Value of this cookie */
    int version;    /* Cookie protocol version (=1) */
    char *comment;  /* Comment to show to user */
    char *commentURL; /* URL for comment to show to user */
    char *domain;   /* Domain for which this cookie is valid */
    int port;	    /* Server port from which this cookie was given (usu. 80) */
    char *PortList; /* List of ports for which cookie can be sent */
    char *path;     /* Path prefix for which this cookie is valid */
    int pathlen;    /* Length of the path */
    int flags;	    /* Various flags */
    time_t expires; /* The time when this cookie expires */
    BOOL quoted;    /* Was a value quoted in the Set-Cookie header? */
};

typedef struct _cookie cookie;

#define COOKIE_FLAG_SECURE 1	   /* If set, cookie requires secure links */
#define COOKIE_FLAG_DISCARD 2	   /* If set, expire at end of session */
#define COOKIE_FLAG_EXPIRES_SET 4  /* If set, an expiry date was set */
#define COOKIE_FLAG_DOMAIN_SET 8   /* If set, an non-default domain was set */
#define COOKIE_FLAG_PATH_SET 16    /* If set, an non-default path was set */
#define COOKIE_FLAG_FROM_FILE 32   /* If set, this cookie was persistent */

static domain_entry sdomain;

struct _HTStream {
    HTStreamClass *isa;
};


/*
 * Skip whitespace
 */
PRIVATE char *SkipBlanks (char *buffer)
{
    while (isspace((unsigned char)*buffer))
        buffer++;
    return buffer;
}


/*
 *  LYstrncpy() terminates strings with a null byte.
 *  Writes a null byte into the n + 1 byte of dst.
 *  Does not pad with null bytes.
 */
PRIVATE char *LYstrncpy (char *dst, char *src, int n)
{
    char *val;
    int len;

    if (!src)
        src = "";
    len = strlen(src);

    /* Don't copy beyond the NULL */
    if (len < n)
	n = len;

    val = strncpy(dst, src, n);
    *(dst + n) = '\0';

    return val;
}


/*
 *  A replacement for 'strsep()'
 */
PRIVATE char *LYstrsep (char **stringp,	WWW_CONST char *delim)
{
    char *tmp, *out;

    if (!stringp || !*stringp)		/* Nothing to do? */
	return NULL;			/* Then don't fall on our faces */

    out = *stringp;			/* Save the start of the string */
    tmp = strpbrk(*stringp, delim);
    if (tmp) {
	*tmp = '\0';			/* Terminate the substring with \0 */
	*stringp = ++tmp;		/* Point at the next substring */
    } else {
	*stringp = NULL;		/* This was the last substring: */
    }					/* Let caller see he's done */
    return out;
}


/*
 * Replaces 'fgets()' calls into a fixed-size buffer with reads into a buffer
 * that is allocated.  When an EOF or error is found, the buffer is freed
 * automatically.
 */
PRIVATE char *LYSafeGets (char **src, FILE *fp)
{
    char buffer[BUFSIZ];
    char *result = NULL;

    if (src)
	result = *src;
    if (result)
	*result = '\0';

    while (fgets(buffer, sizeof(buffer), fp)) {
	if (*buffer)
	    result = StrAllocCat(result, buffer);
	if (strchr(buffer, '\n'))
	    break;
    }
    if (ferror(fp)) {
	FREE(result);
    } else if (feof(fp) && result && !*result) {
	/*
	 *  If the file ends in the middle of a line, return the
	 *  partial line; if another call is made after this, it
	 *  will return NULL. - kw
	 */
	FREE(result);
    }
    if (src)
	*src = result;
    return result;
}


PRIVATE void user_message (WWW_CONST char *message, WWW_CONST char *argument)
{
    char *temp;
    char temp_arg[256];

    if (!message)
	return;
    /*
     *  Make sure we don't overrun any buffers.
     */
    LYstrncpy(temp_arg, argument ? argument : "", 255);
    temp = (char *)malloc(strlen(message) + strlen(temp_arg) + 1);
    if (!temp)
	outofmem(__FILE__, "user_message");
    sprintf(temp, message, temp_arg);

    HTProgress(temp);

    FREE(temp);
    return;
}


/*
 *  This function takes a string in the format
 *	"Mon, 01-Jan-96 13:45:35 GMT" or
 *	"Mon,  1 Jan 1996 13:45:35 GMT"" or
 *	"dd-mm-yyyy"
 *  as an argument, and returns its conversion to clock format
 *  (seconds since 00:00:00 Jan 1 1970), or 0 if the string
 *  doesn't match the expected pattern.  It also returns 0 if
 *  the time is in the past and the "absolute" argument is FALSE.
 *  It is intended for handling 'expires' strings in Version 0
 *  cookies homologously to 'max-age' strings in Version 1 cookies,
 *  for which 0 is the minimum, and greater values are handled as
 *  '[max-age seconds] + time(NULL)'.  If "absolute" is TRUE, we
 *  return the clock format value itself, but if anything goes wrong
 *  when parsing the expected patterns, we still return 0. - FM
 */
PRIVATE time_t LYmktime (char *string, BOOL absolute)
{
    char *s = string;
    time_t clock2;
    int day, month, year, hour, minutes, seconds;
    char *start;
    char temp[8];

    /*
     *	Make sure we have a string to parse. - FM
     */
    if (!(s && *s))
	return(0);
#ifndef DISABLE_TRACE
    if (cookieTrace)
	fprintf(stderr, "LYmktime: Parsing '%s'\n", s);
#endif
    /*
     *	Skip any lead alphabetic "Day, " field and
     *	seek a numeric day field. - FM
     */
    while (*s && !isdigit((unsigned char)*s))
	s++;
    if (!*s)
	return(0);
    /*
     *	Get the numeric day and convert to an integer. - FM
     */
    start = s;
    while (*s && isdigit((unsigned char)*s))
        s++;
    if (!*s || ((s - start) > 2))
	return(0);
    LYstrncpy(temp, start, (int)(s - start));
    day = atoi(temp);
    if (day < 1 || day > 31)
	return(0);
    /*
     *	Get the month string and convert to an integer. - FM
     */
    while (*s && !isalnum((unsigned char)*s))
	s++;
    if (!*s)
	return(0);
    start = s;
    while (*s && isalnum((unsigned char)*s))
	s++;
    if (!*s || ((s - start) < (isdigit((unsigned char)*(s - 1)) ? 2 : 3)) ||
	((s - start) > (isdigit((unsigned char)*(s - 1)) ? 2 : 9)))
	return(0);
    LYstrncpy(temp, start, isdigit((unsigned char)*(s - 1)) ? 2 : 3);
    switch (TOUPPER(temp[0])) {
	case '0':
	case '1':
	    month = atoi(temp);
	    if (month < 1 || month > 12)
		return(0);
	    break;
	case 'A':
	    if (!my_strcasecmp(temp, "Apr")) {
		month = 4;
	    } else if (!my_strcasecmp(temp, "Aug")) {
		month = 8;
	    } else {
		return(0);
	    }
	    break;
	case 'D':
	    if (!my_strcasecmp(temp, "Dec")) {
		month = 12;
	    } else {
		return(0);
	    }
	    break;
	case 'F':
	    if (!my_strcasecmp(temp, "Feb")) {
		month = 2;
	    } else {
		return(0);
	    }
	    break;
	case 'J':
	    if (!my_strcasecmp(temp, "Jan")) {
		month = 1;
	    } else if (!my_strcasecmp(temp, "Jun")) {
		month = 6;
	    } else if (!my_strcasecmp(temp, "Jul")) {
		month = 7;
	    } else {
		return(0);
	    }
	    break;
	case 'M':
	    if (!my_strcasecmp(temp, "Mar")) {
		month = 3;
	    } else if (!my_strcasecmp(temp, "May")) {
		month = 5;
	    } else {
		return(0);
	    }
	    break;
	case 'N':
	    if (!my_strcasecmp(temp, "Nov")) {
		month = 11;
	    } else {
		return(0);
	    }
	    break;
	case 'O':
	    if (!my_strcasecmp(temp, "Oct")) {
		month = 10;
	    } else {
		return(0);
	    }
	    break;
	case 'S':
	    if (!my_strcasecmp(temp, "Sep")) {
		month = 9;
	    } else {
		return(0);
	    }
	    break;
	default:
	    return(0);
    }
    /*
     *	Get the numeric year string and convert to an integer. - FM
     */
    while (*s && !isdigit((unsigned char)*s))
	s++;
    if (!*s)
	return(0);
    start = s;
    while (*s && isdigit((unsigned char)*s))
	s++;
    if ((s - start) == 4) {
	LYstrncpy(temp, start, 4);
    } else if ((s - start) == 2) {
	/*
	 * Assume that received 2-digit dates >= 70 are 19xx; others
	 * are 20xx.  Only matters when dealing with broken software
	 * (HTTP server or web page) which is not Y2K compliant.  The
	 * line is drawn on a best-guess basis; it is impossible for
	 * this to be completely accurate because it depends on what
	 * the broken sender software intends.	(This totally breaks
	 * in 2100 -- setting up the next crisis...) - BL
	 */
	if (atoi(start) >= 70) {
	    LYstrncpy(temp, "19", 2);
	} else {
	    LYstrncpy(temp, "20", 2);
	}
	strncat(temp, start, 2);
	temp[4] = '\0';
    } else {
	return(0);
    }
    year = atoi(temp);
    /*
     *	Get the numeric hour string and convert to an integer. - FM
     */
    while (*s && !isdigit((unsigned char)*s))
	s++;
    if (!*s) {
	hour = minutes = seconds = 0;
    } else {
	start = s;
	while (*s && isdigit((unsigned char)*s))
	    s++;
	if (*s != ':' || (s - start) > 2)
	    return(0);
	LYstrncpy(temp, start, (int)(s - start));
	hour = atoi(temp);
	/*
	 *  Get the numeric minutes string and convert to an integer. - FM
	 */
	while (*s && !isdigit((unsigned char)*s))
	    s++;
	if (!*s)
	    return(0);
	start = s;
	while (*s && isdigit((unsigned char)*s))
	    s++;
	if (*s != ':' || (s - start) > 2)
	    return(0);
	LYstrncpy(temp, start, (int)(s - start));
	minutes = atoi(temp);
	/*
	 *  Get the numeric seconds string and convert to an integer. - FM
	 */
	while (*s && !isdigit((unsigned char)*s))
	    s++;
	if (!*s)
	    return(0);
	start = s;
	while (*s && isdigit((unsigned char)*s))
	    s++;
	if (!*s || ((s - start) > 2))
	    return(0);
	LYstrncpy(temp, start, (int)(s - start));
	seconds = atoi(temp);
    }
    /*
     *	Convert to clock format (seconds since 00:00:00 Jan 1 1970),
     *	but then zero it if it's in the past and "absolute" is not
     *	TRUE.  - FM
     */
    month -= 3;
    if (month < 0) {
	month += 12;
	year--;
    }
    day += (year - 1968) * 1461 / 4;
    day += (((month * 153) + 2) / 5) - 672;
    clock2 = (time_t)((day * 60 * 60 * 24) +
		      (hour * 60 * 60) +
		      (minutes * 60) +
		      seconds);
    if (absolute == FALSE && clock2 <= time(NULL))
	clock2 = (time_t)0;
#ifndef DISABLE_TRACE
    if ((clock2 > 0) && cookieTrace)
        fprintf(stderr, "LYmktime: clock=%ld, ctime=%s",
		(long)clock2, ctime(&clock2));
#endif
    return(clock2);
}

/*
 * Compare a "type:" string, replacing it by the comparison-string if it
 * matches (and return true in that case).
 */
static BOOL compare_type (char *tst, WWW_CONST char *cmp, int len)
{
    if (!my_strncasecmp(tst, cmp, len)) {
	if (strncmp(tst, cmp, len)) {
	    int i;

	    for (i = 0; i < len; i++)
		tst[i] = cmp[i];
	}
	return TRUE;
    }
    return FALSE;
}

/*
**  Must recognize a URL and return the type.
**  If recognized, based on a case-insensitive
**  analysis of the scheme field, ensures that
**  the scheme field has the expected case.
**
**  Returns 0 (not a URL) for a NULL argument or
**  one which lacks a colon.
*/
PRIVATE int is_url (char *filename)
{
    char *cp = filename;

    /*
     *	Don't crash on an empty argument.
     *	Can't be a URL if it lacks a colon.
     */
    if (!cp || !*cp || !strchr(cp, ':'))
	return(0);
    /*
     *	Kill beginning spaces.
     */
    while (isspace((unsigned char)*cp))
        cp++;
    /*
     *	Can't be a URL if it starts with a slash or colon.
     *	So return immediately for this common case,
     *	also to avoid false positives if there was
     *	a colon later in the string. - KW
     */
    if ((*cp == ':') || (*cp == '/'))
	return(0);

    switch (TOLOWER(*cp)) {
	case 'c':
	    if (compare_type(cp, "cookiejar:", 11))
		/*
		 *  Special Internal Mosaic type.
		 */
		return(COOKIEJAR_URL_TYPE);
	    break;
	case 'd':
	    if (compare_type(cp, "data:", 5))
		return(DATA_URL_TYPE);
	    break;
	case 'f':
	    if (compare_type(cp, "file:", 5)) {
	        return(FILE_URL_TYPE);
	    } else if (compare_type(cp, "ftp:", 4)) {
		return(FTP_URL_TYPE);
	    } else if (compare_type(cp, "finger:", 7)) {
		return(FINGER_URL_TYPE);
	    }
	    break;
	case 'm':
	    if (compare_type(cp, "mailto:", 7))
		return(MAILTO_URL_TYPE);
	    break;
	case  'n':
	    if (compare_type(cp, "news:", 5)) {
		return(NEWS_URL_TYPE);
	    } else if (compare_type(cp, "nntp:", 5)) {
		return(NNTP_URL_TYPE);
	    }
	    break;
	case 's':
	    if (compare_type(cp, "snews:", 6))
		return(SNEWS_URL_TYPE);
	    break;
	default:
	    if ((strlen(cp) > 3) && !strstr(cp + 3, "://")) {
		/*
		 *  If it doesn't contain "://", and it's not one of the
		 *  the above, it can't be a URL with a scheme we know.
		 */
		return(0);
 	    } else {
		switch (TOLOWER(*cp)) {
		    case 'h':
			if (compare_type(cp, "http:", 5)) {
			    return(HTTP_URL_TYPE);
			} else if (compare_type(cp, "https:", 6)) {
			    return(HTTPS_URL_TYPE);
			}
			break;
		    case 'g':
 			if (compare_type(cp, "gopher:", 7)) {
			    char *cp1;

			    if (cp1 = strchr(cp + 11, '/')) {
				cp1++;
				if (TOUPPER(*cp1) == 'H' || *cp1 == 'w') {
				    /* If this is a gopher html type */
				    return(HTML_GOPHER_URL_TYPE);
	    			} else if (*cp1 == 'T' || *cp1 == '8') {
				    return(TELNET_GOPHER_URL_TYPE);
	    			} else if (*cp1 == '7') {
				    return(INDEX_GOPHER_URL_TYPE);
				} else {
				    return(GOPHER_URL_TYPE);
				}
			    } else {
				return(GOPHER_URL_TYPE);
			    }
			}
			break;
		    case 'w':
			if (compare_type(cp, "wais:", 5))
			    return(WAIS_URL_TYPE);
			break;
		    case 't':
 			if (compare_type(cp, "telnet:", 7)) {
			    return(TELNET_URL_TYPE);
			} else if (compare_type(cp, "tn3270:", 7)) {
			    return(TN3270_URL_TYPE);
			}
			break;
		    case 'r':
			if (compare_type(cp, "rlogin:", 7))
			    return(RLOGIN_URL_TYPE);
			break;
		    case 'c':
			if (compare_type(cp, "cso:", 4))
			    return(CSO_URL_TYPE);
			break;
		    case 'a':
			if (compare_type(cp, "afs:", 4))
			    return(AFS_URL_TYPE);
			break;
		    case 'p':
			if (compare_type(cp, "prospero:", 9))
			    return(PROSPERO_URL_TYPE);
			break;
		}
	    }
    }
    return(0);
}

/*
**  This function converts any ampersands in allocated
**  strings to "&amp;".  If isTITLE is TRUE, it also
**  converts any angle-brackets to "&lt;" or "&gt;". - FM
*/
PRIVATE void LYEntify (char **str, BOOL isTITLE)
{
    char *p = *str;
    char *q = NULL;
    char *cp;
    int amps = 0;
    int lts = 0;
    int gts = 0;

    if (!p || !*p)
	return;
    /*
     *	Count the ampersands. - FM
     */
    while (*p && (q = strchr(p, '&'))) {
	amps++;
	p = q + 1;
    }
    /*
     *	Count the left-angle-brackets, if needed. - FM
     */
    if (isTITLE == TRUE) {
	p = *str;
	while (*p && (q = strchr(p, '<'))) {
	    lts++;
	    p = q + 1;
	}
    }
    /*
     *	Count the right-angle-brackets, if needed. - FM
     */
    if (isTITLE == TRUE) {
	p = *str;
	while (*p && (q = strchr(p, '>'))) {
	    gts++;
	    p = q + 1;
	}
    }
    /*
     *	Check whether we need to convert anything. - FM
     */
    if (!amps && !lts && !gts)
	return;
    /*
     *	Allocate space and convert. - FM
     */
    q = (char *)malloc(strlen(*str) + (4 * amps) + (3 * lts) + (3 * gts) + 1);
    if ((cp = q) == NULL)
	outofmem(__FILE__, "LYEntify");
    for (p = *str; *p; p++) {
	if (*p == '&') {
	    *q++ = '&';
	    *q++ = 'a';
	    *q++ = 'm';
	    *q++ = 'p';
	    *q++ = ';';
	} else if (isTITLE && *p == '<') {
	    *q++ = '&';
	    *q++ = 'l';
	    *q++ = 't';
	    *q++ = ';';
	} else if (isTITLE && *p == '>') {
	    *q++ = '&';
	    *q++ = 'g';
	    *q++ = 't';
	    *q++ = ';';
	} else {
	    *q++ = *p;
	}
    }
    *q = '\0';
    free(*str);
    *str = cp;
}


PRIVATE void MemAllocCopy (char **dest, WWW_CONST char *start,
			   WWW_CONST char *end)
{
    if (!dest)
	return;

    if (!(start && end) || (end <= start)) {
	HTSACopy(dest, "");
	return;
    }

    if (*dest)
	free(*dest);
    *dest = (char *)malloc((end - start) + 1);
    if (!*dest)
	outofmem(__FILE__, "MemAllocCopy");

    LYstrncpy(*dest, start, end - start);
}


/* Compare routine for domain BTree */
PRIVATE int compare_cookie_domains (domain_entry *domain1,
				    domain_entry *domain2)
{
    char *d1 = domain1->domain;
    char *d2 = domain2->domain;
    int res;

    /* Ignore leading dots for alphabetization */

    if ((*d1 == '.') && (*d2 == '.'))
	return my_strcasecmp(d1, d2);

    if (*d1 == '.') {
	res = my_strcasecmp(++d1, d2);
	if (!res) {
	    return -1;
	} else {
	    return res;
	}
    }
    if (*d2 == '.') {
	res = my_strcasecmp(d1, ++d2);
	if (!res) {
	    return 1;
	} else {
	    return res;
	}
    }
    return my_strcasecmp(d1, d2);
}

	
/*	Confirm a cookie operation.			HTConfirmCookie()
**	---------------------------
**
**  On entry,
**	server			is the server sending the Set-Cookie.
**	domain			is the domain of the cookie.
**	path			is the path of the cookie.
**	name			is the name of the cookie.
**	value			is the value of the cookie.
**
**  On exit,
**	Returns FALSE on cancel,
**		TRUE if the cookie should be set.
*/
PRIVATE BOOL HTConfirmCookie (void *dp,	WWW_CONST char *server,
			      WWW_CONST char *name, WWW_CONST char *value)
{
    char message[256];
    domain_entry *de;
    char ch;
    int namelen, valuelen;
    int width = 720;

    if (!(de = (domain_entry *)dp))
	return FALSE;
    /*
    **	If the user has specified a constant action, don't prompt at all.
    */
    if (de->bv == ACCEPT_ALWAYS || de->bv == FROM_FILE)
	return TRUE;
    if (de->bv == REJECT_ALWAYS)
	return FALSE;
    /*
    **	Figure out how much of the cookie we can show.
    */
    namelen = strlen(name);
    valuelen = strlen(value);
    if ((namelen + valuelen) > 80) {
	float percentage = (float)80 / (float)(namelen + valuelen);

	/*
	**  Argh... there isn't enough space on our single line for
	**  the whole cookie.  Reduce them both by a percentage.
	**  This should be smarter.
	*/
	namelen = (int)(percentage * (float)namelen);
	valuelen = (int)(percentage * (float)valuelen);
    } else if ((namelen + valuelen) < 50) {
	width = 520;
    }
    sprintf(message, "Accept following cookie from %s:\n%.*s=%.*s",
	    server, namelen, name, valuelen, value);
    if (!HTEatAllCookies) {
	XmxSetButtonClueText("Accept this cookie", "Reject this cookie",
			     "Always accept cookies for this domain",
			     "Never accept cookies for this domain", NULL);
	ch = XmxDoFourButtons(current_win->base, app_context,
		           "VMS Mosaic: Cookie Confirmation", message, "Accept",
		           "Reject", "Accept Always", "Never Accept", width);
	XmxSetButtonClueText(NULL, NULL, NULL, NULL, NULL);
    } else {
	ch = 3;
    }
    switch(ch) {
	case 1:
	    /*
	    **  Accept the cookie.
	    */
	    HTProgress(ALLOWING_COOKIE);
	    return TRUE;
	case 3:
	    /*
	    **  Set to accept all cookies for this domain.
	    */
	    de->bv = ACCEPT_ALWAYS;
	    user_message(ALWAYS_ALLOWING_COOKIES, de->domain);
	    return TRUE;
	case 2:
	    /*
	    **  Reject the cookie.
	    */
	    HTProgress(REJECTING_COOKIE);
	    return FALSE;
	case 4:
	    /*
	    **  Set to reject all cookies from this domain.
	    */
	    de->bv = REJECT_ALWAYS;
	    user_message(NEVER_ALLOWING_COOKIES, de->domain);
	default:
	    return FALSE;
    }
}

PRIVATE cookie *newCookie (void)
{
    cookie *p = (cookie *)calloc(1, sizeof(cookie));
    char lynxID[64];

    if (!p)
	outofmem(__FILE__, "newCookie");
    sprintf(lynxID, "%p", p);
    StrAllocCopy(p->lynxID, lynxID);
    p->port = 80;
    return p;
}

PRIVATE void freeCookie (cookie *co)
{
    if (co) {
	free(co->lynxID);
	free(co->name);
	free(co->value);
	free(co->comment);
	free(co->commentURL);
	free(co->domain);
	free(co->path);
	free(co->PortList);
	free(co);
    }
}


/*
**  Compare two hostnames as specified in Section 2 of:
**   http://www.ics.uci.edu/pub/ietf/http/draft-ietf-http-state-man-mec-02.txt
**	- AK & FM
*/
PRIVATE BOOL host_matches (WWW_CONST char *A, WWW_CONST char *B)
{
    /*
     *	The following line will handle both numeric IP addresses and
     *	FQDNs.	Do numeric addresses require special handling?
     */
    if (*B != '.' && !my_strcasecmp(A, B))
	return YES;
    /*
     *	The following will pass a "dotted tail" match to "a.b.c.e"
     *	as described in Section 2 of draft-ietf-http-state-man-mec-10.txt.
     */
    if (*B == '.' && B[1] && B[1] != '.' && *A != '.') {
	int diff = strlen(A) - strlen(B);

	if ((diff > 0) && !my_strcasecmp(A + diff, B))
	    return YES;
    }
    return NO;
}

/*
**  Compare the current port with a port list as specified in Section 4.3 of:
**   http://www.ics.uci.edu/pub/ietf/http/draft-ietf-http-state-man-mec-02.txt
**	- FM
*/
PRIVATE BOOL port_matches (int port, WWW_CONST char *list)
{
    char *number = list;

    if (!(number && isdigit(*number)))
	return(FALSE);

    while (*number) {
	if (atoi(number) == port)
	    return(TRUE);
	while (isdigit(*number))
	    number++;
	while (*number && !isdigit(*number))
	    number++;
    }
    return(FALSE);
}


#define FAILS_COND1 0x01
#define FAILS_COND4 0x02
/*
**  Store a cookie somewhere in the domain list. - AK & FM
*/
PRIVATE void store_cookie (cookie *co, WWW_CONST char *hostname,
			   WWW_CONST char *path)
{
    HTList *hl, *next;
    cookie *c2;
    time_t now = time(NULL);
    char *ptr;
    domain_entry *de;
    BOOL Replacement = FALSE;
    int pos;
    int invprompt_reasons = 0;  /* What is wrong with this cookie - kw */
    static int init = 0;
    static int CDLimit, MaxCookies;

    if (!init) {
	CDLimit = get_pref_int(eCOOKIE_DOMAIN_LIMIT);
	MaxCookies = get_pref_int(eMAX_COOKIES);
	init = 1;
    }
    if (!co)
	return;
    /*
     *  Ensure that the domain list exists.
     */
    if (!domain_btree) {
        domain_btree = HTBTree_new((HTComparer) compare_cookie_domains);
        total_cookies = 0;
    }
    /*
     *  Look through domain_btree to see if the cookie's domain
     *  is already listed.
     */
    sdomain.domain = co->domain;
    de = (domain_entry *)HTBTree_search(domain_btree, &sdomain);
    if (de) {
	cookie_list = de->cookie_list;
    } else {
        cookie_list = NULL;
    }
    /*
     *	Apply sanity checks.
     *
     * Section 4.3.2, condition 1:  The value for the Path attribute is
     * not a prefix of the request-URI.
     *
     * If cookie checking for this domain is set to INVCHECK_LOOSE,
     * then we want to bypass this check.  The user should be queried
     * if set to INVCHECK_QUERY.
     */
    if (strncmp(co->path, path, co->pathlen)) {
	invcheck_type type = de ? de->invcheck_bv : DEFAULT_INVCHECK_TYPE;

	switch (type) {
	    case INVCHECK_LOOSE:
	        break;		/* Continue as if nothing were wrong */

	    case INVCHECK_QUERY:
	        invprompt_reasons |= FAILS_COND1;
	        break;		/* Will prompt later if we get that far */

	    case INVCHECK_STRICT:
#ifndef DISABLE_TRACE
	        if (cookieTrace)
	            fprintf(stderr,
	                "store_cookie: Rejecting because '%s' is not a prefix of '%s'.\n",
		        co->path, path);
#endif
	        freeCookie(co);
	        return;
	}
    }
    /*
     *	The next 4 conditions do NOT apply if the domain is still
     *	the default of request-host.
     */
    if (my_strcasecmp(co->domain, hostname)) {
	/*
	 *  The hostname does not contain a dot.
	 */
	if (!strchr(hostname, '.')) {
#ifndef DISABLE_TRACE
	    if (cookieTrace)
	        fprintf(stderr,
		        "store_cookie: Rejecting because '%s' has no dot.\n",
		        hostname);
#endif
	    freeCookie(co);
	    return;
	}
	/*
	 *  Section 4.3.2, condition 2: The value for the Domain attribute
	 *  contains no embedded dots or does not start with a dot.
	 *  (A dot is embedded if it's neither the first nor last character.)
	 *  Note that we added a lead dot ourselves if a domain attribute
	 *  value otherwise qualified. - FM
	 */
	if (co->domain[0] != '.' || co->domain[1] == '\0') {
#ifndef DISABLE_TRACE
	    if (cookieTrace)
	        fprintf(stderr, "store_cookie: Rejecting domain '%s'.\n",
			co->domain);
#endif
	    freeCookie(co);
	    return;
	}
	ptr = strchr(co->domain + 1, '.');
	if (!ptr || !ptr[1]) {
#ifndef DISABLE_TRACE
	    if (cookieTrace)
	        fprintf(stderr, "store_cookie: Rejecting domain '%s'.\n",
			co->domain);
#endif
	    freeCookie(co);
	    return;
	}
	/*
	 *  Section 4.3.2, condition 3: The value for the request-host does
	 *  not domain-match the Domain attribute.
	 */
	if (!host_matches(hostname, co->domain)) {
#ifndef DISABLE_TRACE
	    if (cookieTrace)
	        fprintf(stderr,
		        "store_cookie: Rejecting domain '%s' for host '%s'.\n",
		        co->domain, hostname);
#endif
	    freeCookie(co);
	    return;
	}
	/*
	 *  Section 4.3.2, condition 4: The request-host is an HDN (not IP
	 *  address) and has the form HD, where D is the value of the Domain
	 *  attribute, and H is a string that contains one or more dots.
	 *
	 *  If cookie checking for this domain is set to INVCHECK_LOOSE,
	 *  then we want to bypass this check.  The user should be queried
	 *  if set to INVCHECK_QUERY.
	 */
	ptr = hostname + strlen(hostname) - strlen(co->domain);
	if (strchr(hostname, '.') < ptr) {
	    invcheck_type type = de ? de->invcheck_bv : DEFAULT_INVCHECK_TYPE;

	    switch (type) {
	        case INVCHECK_LOOSE:
		    break;		/* Continue as if nothing were wrong */

	        case INVCHECK_QUERY:
		    invprompt_reasons |= FAILS_COND4;
		    break;		/* Will prompt later if get that far */

	        case INVCHECK_STRICT:
#ifndef DISABLE_TRACE
		    if (cookieTrace)
		        fprintf(stderr,
			 "store_cookie: Rejecting domain '%s' for host '%s'.\n",
			 co->domain, hostname);
#endif
		    freeCookie(co);
		    return;
	    }
	}
    }
    /*
     *  If we found reasons for issuing an invalid cookie confirmation
     *  prompt, do that now.  Rejection by the user here is the last
     *  chance to completely ignore this cookie; after it passes this
     *  hurdle, it may at least supersede a previous cookie (even if
     *  it finally gets rejected). - kw
     */
    if (invprompt_reasons && get_pref_boolean(eINVALID_COOKIE_PROMPT)) {
  	char *msg;

	if (invprompt_reasons & FAILS_COND4) {
	    msg = (char *)malloc(strlen(INVALID_COOKIE_DOMAIN_CONFIRMATION) +
		                 strlen(co->domain) + strlen(hostname) + 1);
	    sprintf(msg, INVALID_COOKIE_DOMAIN_CONFIRMATION,
		    co->domain, hostname);
	    if (!HTConfirm(msg)) {
#ifndef DISABLE_TRACE
		if (cookieTrace)
		    fprintf(stderr,
			 "store_cookie: Rejecting domain '%s' for host '%s'.\n",
			 co->domain, hostname);
#endif
		freeCookie(co);
		free(msg);
		return;
	    }
	    free(msg);
	}
	if (invprompt_reasons & FAILS_COND1) {
	    msg = (char *)malloc(strlen(INVALID_COOKIE_PATH_CONFIRMATION) +
		                 strlen(co->path) + strlen(path) + 1);
	    sprintf(msg, INVALID_COOKIE_PATH_CONFIRMATION, co->path, path);
	    if (!HTConfirm(msg)) {
#ifndef DISABLE_TRACE
		if (cookieTrace)
		    fprintf(stderr,
			"store_cookie: Rejecting '%s', not a prefix of '%s'.\n",
		        co->path, path);
#endif
		freeCookie(co);
		free(msg);
		return;
	    }
	    free(msg);
	}
    }
    if (!de) {
	/*
	 *  Domain not found; add a new entry for this domain.
	 */
	de = (domain_entry *)calloc(1, sizeof(domain_entry));
	if (!de)
	    outofmem(__FILE__, "store_cookie");
	de->bv = QUERY_USER;
        de->invcheck_bv = DEFAULT_INVCHECK_TYPE;  /* Should this go here? */
	cookie_list = de->cookie_list = HTList_new();
	StrAllocCopy(de->domain, co->domain);

	HTBTree_add(domain_btree, de);
    }
    /*
     *	Loop over the cookie list, deleting expired and matching cookies.
     */
    hl = cookie_list;
    pos = 0;
    while (hl) {
	c2 = (cookie *)hl->object;
	next = hl->next;
	/*
	 *  Check if this cookie has expired.
	 */
	if (c2 && (c2->flags & COOKIE_FLAG_EXPIRES_SET) && c2->expires <= now) {
	    HTList_removeObject(cookie_list, c2);
	    freeCookie(c2);
	    c2 = NULL;
	    total_cookies--;
	/*
	 *  Check if this cookie matches the one we're inserting.
	 */
	} else if (c2 && !my_strcasecmp(co->domain, c2->domain) &&
		   !strcmp(co->path, c2->path) && !strcmp(co->name, c2->name)) {
	    HTList_removeObject(cookie_list, c2);
	    freeCookie(c2);
	    c2 = NULL;
	    total_cookies--;
	    Replacement = TRUE;
	} else if (c2 && (c2->pathlen >= co->pathlen)) {
            /*
             *  This comparison determines the (tentative) position
             *  of the new cookie in the list such that it comes
             *  before existing cookies with a less specific path,
             *  but after existing cookies of equal (or greater)
             *  path length.  Thus it should normally preserve
             *  the order of new cookies with the same path as
             *  they are received, although this is not required.
             *  From RFC 2109 4.3.4:

             If multiple cookies satisfy the criteria above, they are ordered
             in the Cookie header such that those with more specific Path
             attributes precede those with less specific.  Ordering with
	     respect to other attributes (e.g., Domain) is unspecified.

             */
	    pos++;
	}
	hl = next;
    }
    /*
     *	Don't bother to add the cookie if it's already expired.
     */
    if ((co->flags & COOKIE_FLAG_EXPIRES_SET) && co->expires <= now) {
	freeCookie(co);
	co = NULL;
    /*
     *	Don't add the cookie if we're over the domain's limit. - FM
     */
    } else if (HTList_count(cookie_list) > CDLimit) {
#ifndef DISABLE_TRACE
	if (cookieTrace || reportBugs)
	    fprintf(stderr,
		    "Domain's cookie limit exceeded!  Rejecting cookie.\n");
#endif
	freeCookie(co);
	co = NULL;
    /*
     *	Don't add the cookie if we're over the total cookie limit. - FM
     */
    } else if (total_cookies > MaxCookies) {
#ifndef DISABLE_TRACE
	if (cookieTrace || reportBugs)
	    fprintf(stderr,"Total cookie limit exceeded!  Rejecting cookie.\n");
#endif
	freeCookie(co);
	co = NULL;
        /*
         * Don't add the cookie if the value is NULL. - BJP
         */
        /*
         * Presence of value is now needed (indicated normally by '='),
         * but it can now be an empty string.
         * - kw 1999-06-24
         */
    } else if (!co->value) {  /* Should not happen - kw */
#ifndef DISABLE_TRACE
	if (cookieTrace || reportBugs)
	    fprintf(stderr,
        	    "store_cookie: Value is NULL!  Not storing cookie.\n");
#endif
        freeCookie(co);
        co = NULL;
    /*
     *	If it's a replacement for a cookie that had not expired,
     *	and never allow has not been set, add it again without
     *	confirmation. - FM
     */
    } else if ((Replacement == TRUE && de) && de->bv != REJECT_ALWAYS) {
	HTList_insertObjectAt(cookie_list, co, pos);
	total_cookies++;
    /*
     *	Get confirmation if we need it, and add cookie
     *	if confirmed or 'allow' is set to always. - FM
     *
     *  Cookies read from file are accepted without confirmation
     *  prompting.
     */
    } else if ((co->flags & COOKIE_FLAG_FROM_FILE) ||
	       HTConfirmCookie(de, hostname, co->name, co->value)) {
	HTList_insertObjectAt(cookie_list, co, pos);
	total_cookies++;
    } else {
	freeCookie(co);
	co = NULL;
    }
}

/*
**  Scan a domain's cookie_list for any cookies we should
**  include in a Cookie: request header. - AK & FM
*/
PRIVATE char *scan_cookie_sublist (WWW_CONST char *hostname,
				   WWW_CONST char *path,
				   int port,
				   HTList *sublist,
				   char *header,
				   BOOL secure)
{
    HTList *hl = sublist;
    HTList *next;
    cookie *co;
    time_t now = time(NULL);
    int len = 0;

    while (hl) {
	co = (cookie *)hl->object;
	next = hl->next;
#ifndef DISABLE_TRACE
	if (cookieTrace && co) {
	    fprintf(stderr, "Checking cookie %lx %s=%s\n", (long)hl,
		    co->name ? co->name : "(no name)",
		    co->value ? co->value : "(no value)");
	    fprintf(stderr, "%s %s %d %s %s %d%s\n", hostname,
		    co->domain ? co->domain : "(no domain)",
		    host_matches(hostname, co->domain), path, co->path,
		    (co->pathlen > 0) ? strncmp(path, co->path, co->pathlen) :0,
		    (co->flags & COOKIE_FLAG_SECURE) ? " secure" : "");
	}
#endif
	/*
	 *  Check if this cookie has expired, and if so, delete it.
	 */
	if ((co && (co->flags & COOKIE_FLAG_EXPIRES_SET)) &&
	    co->expires <= now) {
	    HTList_removeObject(sublist, co);
	    freeCookie(co);
	    co = NULL;
	    total_cookies--;
	}
	/*
	 *  Check if we have a unexpired match and handle if we do.
	 */
	if ((co && host_matches(hostname, co->domain)) &&
	    (!co->pathlen || !strncmp(path, co->path, co->pathlen))) {
	    /*
	     *	Skip if the secure flag is set and we don't have
	     *	a secure connection.  HTTP.c presently treats only
	     *	SSL connections as secure. - FM
	     */
	    if ((co->flags & COOKIE_FLAG_SECURE) && secure == FALSE) {
		hl = next;
		continue;
	    }
	    /*
	     *	Skip if we have a port list and the
	     *	current port is not listed. - FM
	     */
	    if (co->PortList && !port_matches(port, co->PortList)) {
		hl = next;
		continue;
	    }
	    /*
	     *	Start or append to the request header.
	     */
	    if (!header) {
		if (co->version > 0) {
		    /*
		     *	For Version 1 (or greater) cookies,
		     *	the version number goes before the
		     *	first cookie.
		     */
		    char version[16];

		    sprintf(version, "$Version=\"%d\"; ", co->version);
		    StrAllocCopy(header, version);
		    len += strlen(header);
		}
	    } else {
		/*
		 *  There's already cookie data there, so add
		 *  a separator (always use a semi-colon for
		 *  "backward compatibility"). - FM
		 */
		StrAllocCat(header, "; ");
	    }
	    /*
	     *	Include the cookie name=value pair.
	     */
	    StrAllocCat(header, co->name);
	    StrAllocCat(header, "=");
	    if (co->quoted) {
		StrAllocCat(header, "\"");
		len++;
	    }
	    StrAllocCat(header, co->value);
	    if (co->quoted) {
		StrAllocCat(header, "\"");
		len++;
	    }
	    len += strlen(co->name) + strlen(co->value) + 1;
	    /*
	     *	For Version 1 (or greater) cookies, add
	     *	$PATH, $PORT and/or $DOMAIN attributes for
	     *	the cookie if they were specified via a
	     *	server reply header. - FM
	     */
	    if (co->version > 0) {
		if (co->path && (co->flags & COOKIE_FLAG_PATH_SET)) {
		    /*
		     *	Append the path attribute. - FM
		     */
		    StrAllocCat(header, "; $Path=\"");
		    StrAllocCat(header, co->path);
		    StrAllocCat(header, "\"");
		    len += strlen(co->path) + 10;
		}
		if (co->PortList && isdigit((unsigned char)*co->PortList)) {
		    /*
		     *	Append the port attribute. - FM
		     */
		    StrAllocCat(header, "; $Port=\"");
		    StrAllocCat(header, co->PortList);
		    StrAllocCat(header, "\"");
		    len += strlen(co->PortList) + 10;
		}
		if (co->domain && (co->flags & COOKIE_FLAG_DOMAIN_SET)) {
		    /*
		     *	Append the domain attribute. - FM
		     */
		    StrAllocCat(header, "; $Domain=\"");
		    StrAllocCat(header, co->domain);
		    StrAllocCat(header, "\"");
		    len += strlen(co->domain) + 12;
		}
	    }
	}
	hl = next;
    }
    return(header);
}

/*
**  Process potentially concatenated Set-Cookie2 and/or Set-Cookie
**  headers. - FM
*/
PRIVATE void ProcessSetCookies (WWW_CONST char *SetCookie,
				WWW_CONST char *SetCookie2,
				WWW_CONST char *address,
				char *hostname,
				char *path,
				int  port)
{
    char *p, *attr_start, *attr_end, *value_start, *value_end;
    HTList *CombinedCookies;
    HTList *cl = NULL;
    cookie *cur_cookie = NULL;
    cookie *co = NULL;
    int length = 0;
    int url_type = 0;
    int NumCookies = 0;
    BOOL MaxAgeAttrSet = FALSE;
    BOOL Quoted = FALSE;
    BOOL invalidport = FALSE;
    static int init = 0;
    static int CDLimit, MaxCookies;

    if (!init) {
	CDLimit = get_pref_int(eCOOKIE_DOMAIN_LIMIT);
	MaxCookies = get_pref_int(eMAX_COOKIES);
	init = 1;
    }

    if (!(SetCookie && *SetCookie) && !(SetCookie2 && *SetCookie2))
	/*
	 *  Yuk!  Garbage in, so nothing out. - FM
	 */
	return;
    /*
     *	If we have both Set-Cookie and Set-Cookie2 headers.
     *	process the Set-Cookie2 header.  Otherwise, process
     *	whichever of the two headers we do have.  Note that
     *	if more than one instance of a valued attribute for
     *	the same cookie is encountered, the value for the
     *	first instance is retained.  We only accept up to 50
     *	cookies from the header, and only if a cookie's values
     *	do not exceed the 4096 byte limit on overall size. - FM
     */
    CombinedCookies = HTList_new();

    /*
     *	Process the Set-Cookie2 header, if present and not zero-length,
     *	adding each cookie to the CombinedCookies list. - FM
     */
    p = SetCookie2 ? SetCookie2 : "";
#ifndef DISABLE_TRACE
    if (cookieTrace && SetCookie && *p)
	fprintf(stderr, "ProcessSetCookies: Using Set-Cookie2 header.\n");
#endif
    while ((NumCookies <= CDLimit) && *p) {
	attr_start = attr_end = value_start = value_end = NULL;
	p = SkipBlanks(p);
	/*
	 *  Get the attribute name.
	 */
	attr_start = p;
	while (*p && !isspace((unsigned char)*p) &&
	       *p != '=' && *p != ';' && *p != ',')
	    p++;
	attr_end = p;
	p = SkipBlanks(p);
	/*
	 *  Check for an '=' delimiter, or an 'expires' name followed
	 *  by white, since Netscape's bogus parser doesn't require
	 *  an '=' delimiter, and 'expires' attributes are being
	 *  encountered without them.  These shouldn't be in a
	 *  Set-Cookie2 header, but we'll assume it's an expires
	 *  attribute rather a cookie with that name, since the
	 *  attribute mistake rather than name mistake seems more
	 *  likely to be made by providers. - FM
	 */
	if (*p == '=' || !my_strncasecmp(attr_start, "Expires", 7)) {
	    /*
	     *	Get the value string.
	     */
	    if (*p == '=')
		p++;
	    p = SkipBlanks(p);
	    /*
	     *	Hack alert!  We must handle Netscape-style cookies with
	     *		"Expires=Mon, 01-Jan-96 13:45:35 GMT" or
	     *		"Expires=Mon,  1 Jan 1996 13:45:35 GMT".
	     *	No quotes, but there are spaces.  Argh...
	     *	Anyway, we know it will have at least 3 space separators
	     *	within it, and two dashes or two more spaces, so this code
	     *	looks for a space after the 5th space separator or dash to
	     *	mark the end of the value. - FM
	     */
	    if ((attr_end - attr_start) == 7 &&
		!my_strncasecmp(attr_start, "Expires", 7)) {
		int spaces = 6;

		value_start = p;
		if (isdigit((unsigned char)*p)) {
		    /*
		     *	No alphabetic day field. - FM
		     */
		    spaces--;
		} else {
		    /*
		     *	Skip the alphabetic day field. - FM
		     */
		    while (*p && isalpha((unsigned char)*p))
			p++;
		    while (*p == ',' || isspace((unsigned char)*p))
			p++;
		    spaces--;
		}
		while (*p && *p != ';' && *p != ',' && spaces) {
		    p++;
		    if (isspace((unsigned char)*p)) {
			while (isspace((unsigned char)*(p + 1)))
			    p++;
			spaces--;
		    } else if (*p == '-') {
			spaces--;
		    }
		}
		value_end = p;
	    /*
	     *	Hack Alert!  The port attribute can take a
	     *	comma separated list of numbers as a value,
	     *	and such values should be quoted, but if
	     *	not, make sure we don't treat a number in
	     *	the list as the start of a new cookie. - FM
	     */
	    } else if ((attr_end - attr_start) == 4 &&
		       !my_strncasecmp(attr_start, "port", 4) &&
		       isdigit((unsigned char)*p)) {
		/*
		 *  The value starts as an unquoted number.
		 */
		char *cp, *cp1;

		value_start = p;
		while (1) {
		    while (isdigit((unsigned char)*p))
			p++;
		    value_end = p;
		    p = SkipBlanks(p);
		    if (!*p || (*p == ';'))
			break;
		    if (*p == ',') {
			cp = SkipBlanks(p + 1);
			if (*cp && isdigit((unsigned char)*cp)) {
			    cp1 = cp;
			    while (isdigit((unsigned char)*cp1))
				cp1++;
			    cp1 = SkipBlanks(cp1);
			    if (!*cp1 || *cp1 == ',' || *cp1 == ';') {
				p = cp;
				continue;
			    }
			}
		    }
		    while (*p && *p != ';' && *p != ',')
			p++;
		    value_end = p;
		    /*
		     *	Trim trailing spaces.
		     */
		    if ((value_end > value_start) &&
			isspace((unsigned char)*(value_end - 1))) {
			value_end--;
			while ((value_end > (value_start + 1)) &&
			       isspace((unsigned char)*value_end) &&
			       isspace((unsigned char)*(value_end - 1))) {
			    value_end--;
			}
		    }
		    break;
		}
	    } else if (*p == '"') {
		BOOL escaped = FALSE;

		/*
		 *  It looks like quoted string.
		 */
		value_start = ++p;
		while (*p && (*p != '"' || escaped)) {
		    escaped = (!escaped && *p == '\\');
		    p++;
		}
		if (p != value_start && *p == '"' && !escaped) {
		    value_end = p++;
		    Quoted = TRUE;
		} else {
		    value_start--;
		    value_end = p;
		    if (*p)
			p++;
		    Quoted = FALSE;
		}
	    } else {
		/*
		 *  Otherwise, it's an unquoted string.
		 */
		value_start = p;
		while (*p && *p != ';' && *p != ',')
		    p++;
		value_end = p;
		/*
		 *  Trim trailing spaces.
		 */
		if ((value_end > value_start) &&
		    isspace((unsigned char)*(value_end - 1))) {
		    value_end--;
		    while ((value_end > (value_start + 1)) &&
			   isspace((unsigned char)*value_end) &&
			   isspace((unsigned char)*(value_end - 1))) {
			value_end--;
		    }
		}
	    }
	}
	/*
	 *  Check for a separator character, and skip it.
	 */
	if (*p == ';' || *p == ',')
	    p++;
	/*
	 *  Now, we can handle this attribute/value pair.
	 */
	if (attr_end > attr_start) {
	    int len = attr_end - attr_start;
	    BOOL known_attr = NO;
	    char *value = NULL;

	    if (value_start && (value_end >= value_start)) {
		int value_len = value_end - value_start;

		if (value_len > 4096)
		    value_len = 4096;
		value = (char *)malloc(value_len + 1);
		if (!value)
		    outofmem(__FILE__, "ProcessSetCookies");
		LYstrncpy(value, value_start, value_len);
	    }
	    if (len == 6 && !my_strncasecmp(attr_start, "secure", 6)) {
		if (!value) {
		    known_attr = YES;
		    if (cur_cookie)
			cur_cookie->flags |= COOKIE_FLAG_SECURE;
		} else {
		    /*
		     *	If secure has a value, assume someone
		     *	misused it as cookie name. - FM
		     */
		    known_attr = NO;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "discard", 7)) {
		if (!value) {
		    known_attr = YES;
		    if (cur_cookie)
			cur_cookie->flags |= COOKIE_FLAG_DISCARD;
		} else {
		    /*
		     *	If discard has a value, assume someone
		     *	used it as a cookie name. - FM
		     */
		    known_attr = NO;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "comment", 7)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat comment. - FM
		     */
		    !cur_cookie->comment) {
		    StrAllocCopy(cur_cookie->comment, value);
		    length += strlen(cur_cookie->comment);
		}
	    } else if (len == 10 && !my_strncasecmp(attr_start,
						    "commentURL", 10)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat commentURL. - FM
		     */
		    !cur_cookie->commentURL) {
		    /*
		     *	We should get only absolute URLs as
		     *	values, but will resolve versus the
		     *	request's URL just in case. - FM
		     */
		    cur_cookie->commentURL = HTParse(value, address, PARSE_ALL);
		    /*
		     *	Accept only URLs for http or https servers. - FM
		     */
		    if ((url_type = is_url(cur_cookie->commentURL)) &&
			(url_type == HTTP_URL_TYPE ||
			 url_type == HTTPS_URL_TYPE)) {
			length += strlen(cur_cookie->commentURL);
		    } else {
#ifndef DISABLE_TRACE
			if (cookieTrace)
			    fprintf(stderr,
			       "PSetCookies: Rejecting commentURL value '%s'\n",
			       cur_cookie->commentURL);
#endif
			FREE(cur_cookie->commentURL);
		    }
		}
	    } else if (len == 6 && !my_strncasecmp(attr_start, "domain", 6)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat domain. - FM
		     */
		    !(cur_cookie->flags & COOKIE_FLAG_DOMAIN_SET)) {

		    length -= strlen(cur_cookie->domain);
		    /*
		     *	If the value does not have a lead dot,
		     *	but does have an embedded dot, and is
		     *	not an exact match to the hostname, nor
		     *	is a numeric IP address, add a lead dot.
		     *	Otherwise, use the value as is. - FM
		     *  (domains - case insensitive)
		     */
		    if (*value != '.' && *value && value[1] &&
			my_strcasecmp(value, hostname)) {
			char *ptr = strchr(value, '.');

			if (ptr && ptr[1]) {
			    ptr = value;
			    while (*ptr == '.' || isdigit((unsigned char)*ptr))
				ptr++;
			    if (*ptr) {
#ifndef DISABLE_TRACE
				if (cookieTrace)
 				    fprintf(stderr,
	                               "PSetCookies: Adding lead dot for domain value '%s'\n",
				       value);
#endif
				StrAllocCopy(cur_cookie->domain, ".");
				StrAllocCat(cur_cookie->domain, value);
			    } else {
				StrAllocCopy(cur_cookie->domain, value);
			    }
			} else {
			    StrAllocCopy(cur_cookie->domain, value);
			}
		    } else {
			StrAllocCopy(cur_cookie->domain, value);
		    }
		    length += strlen(cur_cookie->domain);
		    cur_cookie->flags |= COOKIE_FLAG_DOMAIN_SET;
		}
	    } else if (len == 4 && !my_strncasecmp(attr_start, "path", 4)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat path. - FM
		     */
		    !(cur_cookie->flags & COOKIE_FLAG_PATH_SET)) {

		    length -= strlen(cur_cookie->path);
		    StrAllocCopy(cur_cookie->path, value);
		    length += (cur_cookie->pathlen = strlen(cur_cookie->path));
		    cur_cookie->flags |= COOKIE_FLAG_PATH_SET;
		}
	    } else if (len == 4 && !my_strncasecmp(attr_start, "port", 4)) {
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat port. - FM
		     */
		    !cur_cookie->PortList) {
		    char *cp = value;

		    while (*cp && (isdigit((unsigned char)*cp) ||
				   *cp == ',' || *cp == ' '))
			cp++;

		    if (!*cp && !port_matches(port, value)) {
                        invalidport = TRUE;
                        known_attr = YES;
                    } else if (!*cp) {
			StrAllocCopy(cur_cookie->PortList, value);
			length += strlen(cur_cookie->PortList);
			known_attr = YES;
		    } else {
			known_attr = NO;
		    }
		} else if (cur_cookie) {
		    /*
		     *	Don't process a repeat port. - FM
		     */
		    if (!cur_cookie->PortList) {
			char temp[256];

			sprintf(temp, "%d", port);
			StrAllocCopy(cur_cookie->PortList, temp);
			length += strlen(cur_cookie->PortList);
		    }
		    known_attr = YES;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "version", 7)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat version. - FM
		     */
		    cur_cookie->version < 1) {
		    int temp = strtol(value, NULL, 10);

		    if (errno != -ERANGE)
			cur_cookie->version = temp;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "max-age", 7)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat max-age. - FM
		     */
		    !MaxAgeAttrSet) {
		    int temp = strtol(value, NULL, 10);

		    cur_cookie->flags |= COOKIE_FLAG_EXPIRES_SET;
		    if (errno == -ERANGE) {
			cur_cookie->expires = (time_t)0;
		    } else {
			cur_cookie->expires = time(NULL) + temp;
#ifndef DISABLE_TRACE
		        if (cookieTrace)
			    fprintf(stderr, "HTSetCookie: expires %ld, %s",
				    (long) cur_cookie->expires,
				    ctime(&cur_cookie->expires));
#endif
		    }
		    MaxAgeAttrSet = TRUE;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "expires", 7)) {
		/*
		 *  Convert an 'expires' attribute value if we haven't
		 *  received a 'max-age'.  Note that 'expires' should not
		 *  be used in Version 1 cookies, but it might be used for
		 *  "backward compatibility", and, in turn, ill-informed
		 *  people surely would start using it instead of, rather
		 *  than in addition to, 'max-age'. - FM
		 */
		known_attr = YES;
		if ((cur_cookie && !MaxAgeAttrSet) &&
		    !(cur_cookie->flags & COOKIE_FLAG_EXPIRES_SET)) {
		    known_attr = YES;
		    if (value) {
			cur_cookie->flags |= COOKIE_FLAG_EXPIRES_SET;
			cur_cookie->expires = LYmktime(value, FALSE);
#ifndef DISABLE_TRACE
			if ((cur_cookie->expires > 0) && cookieTrace)
			    fprintf(stderr, "HTSetCookie: expires %ld, %s",
				    (long) cur_cookie->expires,
				    ctime(&cur_cookie->expires));
#endif
		    }
		}
	    }
	    /*
	     *	If none of the above comparisons succeeded, and we have
	     *	a value, then we have an unknown pair of the form 'foo=bar',
	     *	which means it's time to create a new cookie.  If we don't
	     *	have a non-zero-length value, assume it's an error or a
	     *	new, unknown attribute which doesn't take a value, and
	     *	ignore it. - FM
	     */
	    if (!known_attr && value && value_end >= value_start) {
		/*
		 *  If we've started a cookie, and it's not too big,
		 *  save it in the CombinedCookies list. - FM
		 */
		if (length <= 4096 && cur_cookie && !invalidport) {
		    /*
		     *	Assume version 1 if not set to that or higher. - FM
		     */
		    if (cur_cookie->version < 1)
			cur_cookie->version = 1;
		    HTList_addObjectAtEnd(CombinedCookies, cur_cookie);
		} else if (cur_cookie) {
#ifndef DISABLE_TRACE
		    if (cookieTrace || reportBugs) {
		        fprintf(stderr,
			        "PSetCookies: Rejecting Set-Cookie2: %s=%s\n",
				cur_cookie->name ? cur_cookie->name :
						   "[no name]",
				cur_cookie->value ? cur_cookie->value :
						    "[no value]");
		        fprintf(stderr, invalidport ?
			     "                     due to excessive length!\n" :
			     "                     due to invalid port!\n");
		    }
#endif
		    if (invalidport)
			NumCookies--;
		    freeCookie(cur_cookie);
		    cur_cookie = NULL;
		}
		/*
		 *  Start a new cookie. - FM
		 */
		cur_cookie = newCookie();
		invalidport = FALSE;
		NumCookies++;
		MemAllocCopy(&cur_cookie->name, attr_start, attr_end);
		length = strlen(cur_cookie->name);
		MemAllocCopy(&cur_cookie->value, value_start, value_end);
		length += strlen(cur_cookie->value);
		StrAllocCopy(cur_cookie->domain, hostname);
		length += strlen(cur_cookie->domain);
		StrAllocCopy(cur_cookie->path, path);
		length += (cur_cookie->pathlen = strlen(cur_cookie->path));
		cur_cookie->port = port;
		MaxAgeAttrSet = FALSE;
		cur_cookie->quoted = TRUE;
	    }
	    FREE(value);
	}
    }
    /*
     *	Add any final SetCookie2 cookie to the CombinedCookie list
     *	if we are within the length limit. - FM
     */
    if ((NumCookies <= CDLimit) &&
	(length <= 4096) && cur_cookie && !invalidport) {
	if (cur_cookie->version < 1)
	    cur_cookie->version = 1;
	HTList_addObjectAtEnd(CombinedCookies, cur_cookie);
    } else if (cur_cookie && !invalidport) {
#ifndef DISABLE_TRACE
	if (cookieTrace || reportBugs) {
	    fprintf(stderr,
		    "PSetCookies: Rejecting Set-Cookie2: %s=%s\n",
		    cur_cookie->name ? cur_cookie->name : "[no name]",
		    cur_cookie->value ? cur_cookie->value : "[no value]");
	    fprintf(stderr, "                     due to excessive %s%s%s\n",
		    length > 4096 ? "length" : "",
		    length > 4096 && NumCookies > CDLimit ? " and " : "",
		    NumCookies > CDLimit ? "number!\n" : "!\n");
	}
#endif
	freeCookie(cur_cookie);
	cur_cookie = NULL;
    } else if (cur_cookie) {                    /* Invalid port */
#ifndef DISABLE_TRACE
	if (cookieTrace || reportBugs) {
	    fprintf(stderr,
        	    "PSetCookies: Rejecting Set-Cookie2: %s=%s\n",
                    cur_cookie->name ? cur_cookie->name : "[no name]",
                    cur_cookie->value ? cur_cookie->value : "[no value]");
            fprintf(stderr, "                     due to invalid port!\n");
	}
#endif
        NumCookies--;
        freeCookie(cur_cookie);
        cur_cookie = NULL;
    }
    /*
     *	Process the Set-Cookie header, if no non-zero-length Set-Cookie2
     *	header was present. - FM
     */
    length = 0;
    NumCookies = 0;
    cur_cookie = NULL;
    p = (SetCookie && !(SetCookie2 && *SetCookie2)) ? SetCookie : "";
#ifndef DISABLE_TRACE
    if (cookieTrace && SetCookie2 && *p)
	fprintf(stderr, "PSetCookies: Using Set-Cookie header.\n");
#endif
    while ((NumCookies <= CDLimit) && *p) {
	attr_start = attr_end = value_start = value_end = NULL;
	p = SkipBlanks(p);
	/*
	 *  Get the attribute name.
	 */
	attr_start = p;
	while (*p && !isspace((unsigned char)*p) &&
	       *p != '=' && *p != ';' && *p != ',')
	    p++;
	attr_end = p;
	p = SkipBlanks(p);
	/*
	 *  Check for an '=' delimiter, or an 'expires' name followed
	 *  by white, since Netscape's bogus parser doesn't require
	 *  an '=' delimiter, and 'expires' attributes are being
	 *  encountered without them. - FM
	 */
	if (*p == '=' || !my_strncasecmp(attr_start, "Expires", 7)) {
	    /*
	     *	Get the value string.
	     */
	    if (*p == '=')
		p++;
	    p = SkipBlanks(p);
	    /*
	     *	Hack alert!  We must handle Netscape-style cookies with
	     *		"Expires=Mon, 01-Jan-96 13:45:35 GMT" or
	     *		"Expires=Mon,  1 Jan 1996 13:45:35 GMT".
	     *	No quotes, but there are spaces.  Argh...
	     *	Anyway, we know it will have at least 3 space separators
	     *	within it, and two dashes or two more spaces, so this code
	     *	looks for a space after the 5th space separator or dash to
	     *	mark the end of the value. - FM
	     */
	    if ((attr_end - attr_start) == 7 &&
		!my_strncasecmp(attr_start, "Expires", 7)) {
		int spaces = 6;

		value_start = p;
		if (isdigit((unsigned char)*p)) {
		    /*
		     *	No alphabetic day field. - FM
		     */
		    spaces--;
		} else {
		    /*
		     *	Skip the alphabetic day field. - FM
		     */
		    while (*p && isalpha((unsigned char)*p))
			p++;
		    while (*p == ',' || isspace((unsigned char)*p))
			p++;
		    spaces--;
		}
		while (*p && *p != ';' && *p != ',' && spaces) {
		    p++;
		    if (isspace((unsigned char)*p)) {
			while (isspace((unsigned char)*(p + 1)))
			    p++;
			spaces--;
		    } else if (*p == '-') {
			spaces--;
		    }
		}
		value_end = p;
	    /*
	     *	Hack Alert!  The port attribute can take a
	     *	comma separated list of numbers as a value,
	     *	and such values should be quoted, but if
	     *	not, make sure we don't treat a number in
	     *	the list as the start of a new cookie. - FM
	     */
	    } else if ((attr_end - attr_start) == 4 &&
		       !my_strncasecmp(attr_start, "port", 4) &&
		       isdigit((unsigned char)*p)) {
		/*
		 *  The value starts as an unquoted number.
		 */
		char *cp, *cp1;

		value_start = p;
		while (1) {
		    while (isdigit((unsigned char)*p))
			p++;
		    value_end = p;
		    p = SkipBlanks(p);
		    if (!*p || *p == ';')
			break;
		    if (*p == ',') {
			cp = SkipBlanks(p + 1);
			if (*cp && isdigit((unsigned char)*cp)) {
			    cp1 = cp;
			    while (isdigit((unsigned char)*cp1))
				cp1++;
			    cp1 = SkipBlanks(cp1);
			    if (!*cp1 || *cp1 == ',' || *cp1 == ';') {
				p = cp;
				continue;
			    }
			}
		    }
		    while (*p && *p != ';' && *p != ',')
			p++;
		    value_end = p;
		    /*
		     *	Trim trailing spaces.
		     */
		    if ((value_end > value_start) &&
			isspace((unsigned char)*(value_end - 1))) {
			value_end--;
			while ((value_end > (value_start + 1)) &&
			       isspace((unsigned char)*value_end) &&
			       isspace((unsigned char)*(value_end - 1))) {
			    value_end--;
			}
		    }
		    break;
		}
	    } else if (*p == '"') {
		BOOL escaped = FALSE;

		/*
		 *  It looks like quoted string.
		 */
		value_start = ++p;
		while (*p  && (*p != '"' || escaped)) {
		    escaped = (!escaped && *p == '\\');
		    p++;
		}
		if (p != value_start && *p == '"' && !escaped) {
		    value_end = p++;
		    Quoted = TRUE;
		} else {
		    value_start--;
		    value_end = p;
		    if (*p)
			p++;
		    Quoted = FALSE;
		}
	    } else {
		/*
		 *  Otherwise, it's an unquoted string.
		 */
		value_start = p;
		while (*p && *p != ';' && *p != ',')
		    p++;
		value_end = p;
		/*
		 *  Trim trailing spaces.
		 */
		if ((value_end > value_start) &&
		    isspace((unsigned char)*(value_end - 1))) {
		    value_end--;
		    while ((value_end > (value_start + 1)) &&
			   isspace((unsigned char)*value_end) &&
			   isspace((unsigned char)*(value_end - 1))) {
			value_end--;
		    }
		}
	    }
	}
	/*
	 *  Check for a separator character, and skip it.
	 */
	if (*p == ';' || *p == ',')
	    p++;
	/*
	 *  Now, we can handle this attribute/value pair.
	 */
	if (attr_end > attr_start) {
	    int len = attr_end - attr_start;
	    BOOL known_attr = NO;
	    char *value = NULL;

	    if (value_start && value_end >= value_start) {
		int value_len = value_end - value_start;

		if (value_len > 4096)
		    value_len = 4096;
		value = (char *)malloc(value_len + 1);
		if (!value)
		    outofmem(__FILE__, "ProcessSetCookies");
		LYstrncpy(value, value_start, value_len);
	    }
	    if (len == 6 && !my_strncasecmp(attr_start, "secure", 6)) {
		if (!value) {
		    known_attr = YES;
		    if (cur_cookie)
			cur_cookie->flags |= COOKIE_FLAG_SECURE;
		} else {
		    /*
		     *	If secure has a value, assume someone
		     *	misused it as cookie name. - FM
		     */
		    known_attr = NO;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "discard", 7)) {
		if (!value) {
		    known_attr = YES;
		    if (cur_cookie)
			cur_cookie->flags |= COOKIE_FLAG_DISCARD;
		} else {
		    /*
		     *	If discard has a value, assume someone
		     *	used it as a cookie name. - FM
		     */
		    known_attr = NO;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "comment", 7)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat comment. - FM
		     */
		    !cur_cookie->comment) {
		    StrAllocCopy(cur_cookie->comment, value);
		    length += strlen(cur_cookie->comment);
		}
	    } else if (len == 10 &&
		       !my_strncasecmp(attr_start, "commentURL", 10)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat commentURL. - FM
		     */
		    !cur_cookie->commentURL) {
		    /*
		     *	We should get only absolute URLs as
		     *	values, but will resolve versus the
		     *	request's URL just in case. - FM
		     */
		    cur_cookie->commentURL = HTParse(value, address, PARSE_ALL);
		    /*
		     *	Accept only URLs for http or https servers. - FM
		     */
		    if ((url_type = is_url(cur_cookie->commentURL)) &&
			(url_type == HTTP_URL_TYPE ||
			 url_type == HTTPS_URL_TYPE)) {
			length += strlen(cur_cookie->commentURL);
		    } else {
#ifndef DISABLE_TRACE
			if (cookieTrace)
			    fprintf(stderr,
			       "PSetCookies: Rejecting commentURL value '%s'\n",
			       cur_cookie->commentURL);
#endif
			FREE(cur_cookie->commentURL);
		    }
		}
	    } else if (len == 6 && !my_strncasecmp(attr_start, "domain", 6)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat domain. - FM
		     */
		    !(cur_cookie->flags & COOKIE_FLAG_DOMAIN_SET)) {
		    length -= strlen(cur_cookie->domain);
		    /*
		     *	If the value does not have a lead dot,
		     *	but does have an embedded dot, and is
		     *	not an exact match to the hostname, nor
		     *	is a numeric IP address, add a lead dot.
		     *	Otherwise, use the value as is. - FM
		     *  (domains - case insensitive).
		     */
		    if (*value != '.' && *value && value[1] &&
			my_strcasecmp(value, hostname)) {
			char *ptr = strchr(value, '.');

			if (ptr && ptr[1]) {
			    ptr = value;
			    while (*ptr == '.' || isdigit((unsigned char)*ptr))
				ptr++;
			    if (*ptr) {
#ifndef DISABLE_TRACE
				if (cookieTrace)
				    fprintf(stderr,
	                               "PSetCookies: Adding lead dot for domain value '%s'\n",
				       value);
#endif
				StrAllocCopy(cur_cookie->domain, ".");
				StrAllocCat(cur_cookie->domain, value);
			    } else {
				StrAllocCopy(cur_cookie->domain, value);
			    }
			} else {
			    StrAllocCopy(cur_cookie->domain, value);
			}
		    } else {
			StrAllocCopy(cur_cookie->domain, value);
		    }
		    length += strlen(cur_cookie->domain);
		    cur_cookie->flags |= COOKIE_FLAG_DOMAIN_SET;
		}
	    } else if (len == 4 && !my_strncasecmp(attr_start, "path", 4)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat path. - FM
		     */
		    !(cur_cookie->flags & COOKIE_FLAG_PATH_SET)) {

		    length -= strlen(cur_cookie->path);
		    StrAllocCopy(cur_cookie->path, value);
		    length += (cur_cookie->pathlen = strlen(cur_cookie->path));
		    cur_cookie->flags |= COOKIE_FLAG_PATH_SET;
		}
	    } else if (len == 4 && !my_strncasecmp(attr_start, "port", 4)) {
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat port. - FM
		     */
		    !cur_cookie->PortList) {
		    char *cp = value;

		    while (*cp && (isdigit((unsigned char)*cp) ||
			           *cp == ',' || *cp == ' '))
			cp++;

		    if (!*cp && port_matches(port, value)) {
			StrAllocCopy(cur_cookie->PortList, value);
			length += strlen(cur_cookie->PortList);
			known_attr = YES;
		    } else {
			known_attr = NO;
		    }
		} else if (cur_cookie) {
		    /*
		     *	Don't process a repeat port. - FM
		     */
		    if (!cur_cookie->PortList) {
			char temp[256];

			sprintf(temp, "%d", port);
			StrAllocCopy(cur_cookie->PortList, temp);
			length += strlen(cur_cookie->PortList);
		    }
		    known_attr = YES;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "version", 7)) {
		known_attr = YES;
		if (cur_cookie && value &&
		    /*
		     *	Don't process a repeat version. - FM
		     */
		    cur_cookie->version < 1) {
		    int temp = strtol(value, NULL, 10);

		    if (errno != -ERANGE)
			cur_cookie->version = temp;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "max-age", 7)) {
		known_attr = YES;
		if (cur_cookie && !MaxAgeAttrSet && value) {
		    int temp = strtol(value, NULL, 10);

		    cur_cookie->flags |= COOKIE_FLAG_EXPIRES_SET;
		    if (errno == -ERANGE) {
			cur_cookie->expires = (time_t)0;
		    } else {
			cur_cookie->expires = time(NULL) + temp;
		    }
		    MaxAgeAttrSet = TRUE;
		}
	    } else if (len == 7 && !my_strncasecmp(attr_start, "expires", 7)) {
		/*
		 *  Convert an 'expires' attribute value if we haven't
		 *  received a 'max-age'.  Note that 'expires' should not
		 *  be used in Version 1 cookies, but it might be used for
		 *  "backward compatibility", and, in turn, ill-informed
		 *  people surely would start using it instead of, rather
		 *  than in addition to, 'max-age'. - FM
		 */
		known_attr = YES;
		if (cur_cookie && !MaxAgeAttrSet &&
		    !(cur_cookie->flags & COOKIE_FLAG_EXPIRES_SET) && value) {
		    cur_cookie->flags |= COOKIE_FLAG_EXPIRES_SET;
		    cur_cookie->expires = LYmktime(value, FALSE);
		}
	    }
	    /*
	     *	If none of the above comparisons succeeded, and we have
	     *	a value, then we have an unknown pair of the form 'foo=bar',
	     *	which means it's time to create a new cookie.  If we don't
	     *	have a non-zero-length value, assume it's an error or a
	     *	new, unknown attribute which doesn't take a value, and
	     *	ignore it. - FM
	     */
	    if (!known_attr && value && value_end >= value_start) {
		/*
		 *  If we've started a cookie, and it's not too big,
		 *  save it in the CombinedCookies list. - FM
		 */
		if (length <= 4096 && cur_cookie) {
		    /*
		     *	If we had a Set-Cookie2 header, make sure
		     *	the version is at least 1, and mark it for
		     *	quoting. - FM
		     */
		    if (SetCookie2) {
			if (cur_cookie->version < 1)
			    cur_cookie->version = 1;
			cur_cookie->quoted = TRUE;
		    }
		    HTList_addObjectAtEnd(CombinedCookies, cur_cookie);
		} else if (cur_cookie) {
#ifndef DISABLE_TRACE
		    if (cookieTrace || reportBugs) {
		        fprintf(stderr,
			        "PSetCookies: Rejecting Set-Cookie: %s=%s\n",
				cur_cookie->name ? cur_cookie->name :
						   "[no name]",
				cur_cookie->value ? cur_cookie->value :
						    "[no value]");
		        fprintf(stderr,
			     "                     due to excessive length!\n");
		    }
#endif
		    freeCookie(cur_cookie);
		    cur_cookie = NULL;
		}
		/*
		 *  Start a new cookie. - FM
		 */
		cur_cookie = newCookie();
		MemAllocCopy(&cur_cookie->name, attr_start, attr_end);
		length = strlen(cur_cookie->name);
		MemAllocCopy(&cur_cookie->value, value_start, value_end);
		length += strlen(cur_cookie->value);
		StrAllocCopy(cur_cookie->domain, hostname);
		length += strlen(cur_cookie->domain);
		StrAllocCopy(cur_cookie->path, path);
		length += (cur_cookie->pathlen = strlen(cur_cookie->path));
		cur_cookie->port = port;
		MaxAgeAttrSet = FALSE;
		cur_cookie->quoted = Quoted;
		Quoted = FALSE;
	    }
	    FREE(value);
	}
    }
    /*
     *	Handle the final Set-Cookie cookie if within length limit. - FM
     */
    if ((NumCookies <= CDLimit) && (length <= 4096) && cur_cookie) {
	if (SetCookie2) {
	    if (cur_cookie->version < 1)
		cur_cookie->version = 1;
	    cur_cookie->quoted = TRUE;
	}
	HTList_addObjectAtEnd(CombinedCookies, cur_cookie);
    } else if (cur_cookie) {
#ifndef DISABLE_TRACE
	if (cookieTrace || reportBugs) {
	    fprintf(stderr,
		    "PSetCookies: Rejecting Set-Cookie: %s=%s\n",
		    cur_cookie->name ? cur_cookie->name : "[no name]",
		    cur_cookie->value ? cur_cookie->value : "[no value]");
	    fprintf(stderr, "                     due to excessive %s%s%s\n",
		    length > 4096 ? "length" : "",
		    length > 4096 && NumCookies > CDLimit ? " and " : "",
		    NumCookies > CDLimit ? "number!\n" : "!\n");
	}
#endif
	freeCookie(cur_cookie);
	cur_cookie = NULL;
    }
    /*
     *	OK, now we can actually store any cookies
     *	in the CombinedCookies list. - FM
     */
    cl = CombinedCookies;
    while (co = (cookie *)HTList_nextObject(cl)) {
#ifndef DISABLE_TRACE
	if (cookieTrace) {
	    fprintf(stderr, "PSetCookie: attr=value pair: '%s=%s'\n",
	            co->name ? co->name : "[no name]",
		    co->value ? co->value : "[no value]");
	    if (co->expires > 0)
		fprintf(stderr, "                    expires: %ld, %s\n",
			(long)co->expires, ctime(&co->expires));
	}
#endif
	if (!my_strncasecmp(address, "https:", 6) &&
	    ForceSSLCookiesSecure == TRUE &&
	    !(co->flags & COOKIE_FLAG_SECURE)) {
	    co->flags |= COOKIE_FLAG_SECURE;
#ifndef DISABLE_TRACE
	    if (cookieTrace)
	        fprintf(stderr,
		        "                    Forced the 'secure' flag on.\n");
#endif
	}
	store_cookie(co, hostname, path);
    }
    HTList_delete(CombinedCookies);
    CombinedCookies = NULL;

    return;
}

/*
**  Entry function for handling Set-Cookie: and/or Set-Cookie2:
**  reply headers.  They may have been concatenated as comma
**  separated lists in HTTP.c or HTMIME.c. - FM
*/
PUBLIC void HTSetCookie (WWW_CONST char *SetCookie,
			 WWW_CONST char *SetCookie2,
			 WWW_CONST char *address)
{
    BOOL BadHeaders = FALSE;
    char *hostname, *path, *ptr;
    int port = 80;

    /*
     *	Get the hostname, port and path of the address, and report
     *	the Set-Cookie and/or Set-Cookie2 header(s) if trace mode is
     *	on, but set the cookie(s) only if HTSetCookies is TRUE. - FM
     */
    if ((hostname = HTParse(address, "", PARSE_HOST)) &&
	(ptr = strchr(hostname, ':')))	{
	/*
	 *  Replace default port number.
	 */
	*ptr++ = '\0';
	port = atoi(ptr);
    } else if (!my_strncasecmp(address, "https:", 6)) {
	port = 443;
    }
    if ((path = HTParse(address, "", PARSE_PATH | PARSE_PUNCTUATION)) &&
	(ptr = strrchr(path, '/'))) {
	if (ptr == path) {
	    *(ptr + 1) = '\0';	/* Leave a single '/' alone */
	} else {
	    *ptr = '\0';
	}
    }
    if (!(SetCookie && *SetCookie) && !(SetCookie2 && *SetCookie2))
	/*
	 *  Yuk, something must have gone wrong in
	 *  HTMIME.c or HTTP.c because both SetCookie
	 *  and SetCookie2 are NULL or zero-length. - FM
	 */
	BadHeaders = TRUE;

#ifndef DISABLE_TRACE
    if (cookieTrace) {
	fprintf(stderr, "HTSetCookie called with host '%s', path '%s',\n",
		hostname ? hostname : "", path ? path : "");
        if (SetCookie)
	    fprintf(stderr, "    and Set-Cookie: '%s'\n",
		    SetCookie ? SetCookie : "");
        if (SetCookie2)
	    fprintf(stderr, "    and Set-Cookie2: '%s'\n",
		    SetCookie2 ? SetCookie2 : "");
        if (HTSetCookies == FALSE || BadHeaders == TRUE)
	    fprintf(stderr,
		    "    Ignoring this Set-Cookie/Set-Cookie2 request.\n");
    }
#endif
    /*
     *	We're done if HTSetCookies is off or we have bad headers. - FM
     */
    if (HTSetCookies == FALSE || BadHeaders == TRUE) {
	FREE(hostname);
	FREE(path);
	return;
    }
    /*
     *	Process the header(s).
     */
    ProcessSetCookies(SetCookie, SetCookie2, address, hostname, path, port);
    FREE(hostname);
    FREE(path);
    return;
}

/*
**  Entry function from creating a Cookie: request header
**  if needed. - AK & FM
*/
PUBLIC char *HTCookie (WWW_CONST char *hostname,
		       WWW_CONST char *path,
		       int port,
		       BOOL secure)
{
    char *header = NULL;
    HTBTElement *ele, *del_ele;
    domain_entry *de;

    /* May not exist yet */
    if (!domain_btree) {
        domain_btree = HTBTree_new((HTComparer) compare_cookie_domains);
        total_cookies = 0;
    }

    ele = HTBTree_next(domain_btree, NULL);

#ifndef DISABLE_TRACE
    if (cookieTrace)
        fprintf(stderr, "HTCookie: Searching for '%s:%d', '%s'.\n",
		hostname ? hostname : "(null)",
		port,
		path ? path : "(null)");
#endif
    /*
     *	Search the cookie_list elements in the domain_btree
     *	for any cookies associated with the //hostname:port/path
     */
    while (ele) {
	de = (domain_entry *)ele->object;
	del_ele = ele;
	ele = HTBTree_next(domain_btree, ele);
	if (de) {
	    if (!HTList_isEmpty(de->cookie_list)) {
		/*
		 *  Scan the domain's cookie_list for
		 *  any cookies we should include in
		 *  our request header.
		 */
		header = scan_cookie_sublist(hostname, path, port,
					     de->cookie_list, header, secure);
	    } else if (de->bv == QUERY_USER &&
		       de->invcheck_bv == DEFAULT_INVCHECK_TYPE) {
		/*
		 *  No cookies in this domain, and no default
		 *  accept/reject choice was set by the user,
		 *  so delete the domain. - FM
		 */
		HTList_delete(de->cookie_list);
		de->cookie_list = NULL;
		HTBTree_ele_delete(domain_btree, del_ele);
		FREE(de->domain);
		FREE(de);
	    }
	}
    }
    if (header)
	return(header);

    return(NULL);
}

/* Persistent cookie support */

PRIVATE char *create_cookie_filename(char *fname)
{
    char *home_ptr, *path;
    char home[256];
#ifndef VMS
    struct passwd *pwdent;
#endif
    
    /*
     * Try the HOME environment variable, then the password file
     */
    if (!(home_ptr = getenv("HOME"))) {
#ifndef VMS
        if (!(pwdent = getpwuid(getuid()))) {
            return(strdup(fname));
        } else {
            strcpy(home, pwdent->pw_dir);
        }
#else
	return(strdup(fname));
#endif
    } else {
        strcpy(home, home_ptr);
    }
    
    path = (char *)malloc(strlen(home) + strlen(fname) + 2);
#ifndef VMS
    sprintf(path, "%s/%s", home, fname);
#else
    sprintf(path, "%s%s", home, fname);
#endif
    
    return(path);
}


PUBLIC void HTLoadCookies (char *cookie_file, char *perm_file)
{
    FILE *cookie_handle, *perm_handle;
    char *buf = NULL;
    char *cookfile, *permfile, *tok_out, *tok_ptr;
    cookie *moo;
    domain_entry *de;
    time_t expires;
    int tok_loop;
    static char domain[257], path[1025], name[257], value[4101];
    static char what[9], secure[9], expires_a[17], type[9];
    static struct {
	char *s;
	size_t n;
    } tok_values[] = {
	{ domain,	sizeof(domain) - 1 },
	{ what,		sizeof(what) - 1 },
	{ path,		sizeof(path) - 1 },
	{ secure,	sizeof(secure) - 1 },
	{ expires_a,	sizeof(expires_a) -1 },
	{ name,		sizeof(name) - 1 },
	{ value,	sizeof(value) - 1 },
	{ NULL, 0 }
    };
    static struct {
	char *s;
	size_t n;
    } perm_values[] = {
	{ what,		sizeof(what) - 1 },
	{ type,		sizeof(type) - 1 },
	{ value,	sizeof(value) - 1 },
	{ domain,	sizeof(domain) - 1 },
	{ NULL, 0 }
    };

    cookfile = create_cookie_filename(cookie_file);
    permfile = create_cookie_filename(perm_file);

#ifndef DISABLE_TRACE
    if (cookieTrace)
	fprintf(stderr, "LoadCookies: reading cookies from %s\n", cookfile);
#endif
    cookie_handle = fopen(cookfile, "r+");
    perm_handle = fopen(permfile, "r+");
    free(cookfile);
    free(permfile);

    if (!cookie_handle || !perm_handle)
	return;

    while (LYSafeGets(&buf, cookie_handle)) {
	unsigned int i = 0;

	if (!*buf || (*buf == '\n') || (*buf == '#'))
	    continue;
	/*
	 * Strip out the newline that fgets() puts at the end of a
	 * cookie.
	 */
	while (buf[i] != '\n' && buf[i])
	    i++;
	if (buf[i] == '\n') {
	    buf[i++] = '\t';	/* Add sep after line if enough space - kw */
	    buf[i] = '\0';
	}
	/*
	 * Tokenise the cookie line into its component parts -
	 * this only works for Netscape style cookie files at the
	 * moment.  It may be worth investigating an alternative
	 * format because the Netscape format isn't all
	 * that useful, or future-proof. - RP
	 *
	 * 'fixed' by using strsep instead of strtok.  No idea
	 * what kind of platform problems this might introduce. - RP
	 */
	/*
	 * This fails when the path is blank
	 *
	 * sscanf(buf, "%s\t%s\t%s\t%s\t%d\t%s\t%[ -~]",
	 *  domain, what, path, secure, &expires, name, value);
	 */
#ifndef DISABLE_TRACE
	if (cookieTrace)
	    fprintf(stderr, "LoadCookies: tokenising %s\n", buf);
#endif
	tok_ptr = buf;
	tok_out = LYstrsep(&tok_ptr, "\t");
	for (tok_loop = 0; tok_out && tok_values[tok_loop].s; tok_loop++) {
#ifndef DISABLE_TRACE
	    if (cookieTrace)
	        fprintf(stderr, "\t%d:%p:%p:[%s]\n",
		        tok_loop, tok_values[tok_loop].s, tok_out, tok_out);
#endif
	    LYstrncpy(tok_values[tok_loop].s, tok_out, tok_values[tok_loop].n);
	    /*
	     * It looks like strtok ignores a leading delimiter,
	     * which makes things a bit more interesting.  Something
	     * like "FALSE\t\tFALSE\t" translates to FALSE,FALSE
	     * instead of FALSE,,FALSE. - RP
	     */
	    tok_out = LYstrsep(&tok_ptr, "\t");
	}

	if (tok_values[tok_loop].s) {
	    /* tok_out in above loop must have been NULL prematurely - kw */
#ifndef DISABLE_TRACE
	    if (cookieTrace)
	        fprintf(stderr, "Not enough tokens, ignoring line!\n");
#endif
	    continue;
	}

	expires = atol(expires_a);
#ifndef DISABLE_TRACE
	if (cookieTrace)
	    fprintf(stderr, "expires:\t%s\n", ctime(&expires));
#endif
	moo = newCookie();
	StrAllocCopy(moo->domain, domain);
	StrAllocCopy(moo->path, path);
	StrAllocCopy(moo->name, name);
	if (value && *value == '"' &&
	    value[1] && value[strlen(value) - 1] == '"' &&
	    value[strlen(value) - 2] != '\\') {

	    value[strlen(value) - 1] = '\0';
	    StrAllocCopy(moo->value, value + 1);
	    moo->quoted = TRUE;
	} else {
	    StrAllocCopy(moo->value, value);
	}
	moo->pathlen = strlen(moo->path);
	/*
	 *  Justification for following flags:
	 *  COOKIE_FLAG_FROM_FILE    So we know were it comes from.
	 *  COOKIE_FLAG_EXPIRES_SET  It must have had an explicit
	 *			     expiration originally, otherwise
	 *			     it would not be in the file.
	 *  COOKIE_FLAG_DOMAIN_SET,  We don't know whether these were
	 *   COOKIE_FLAG_PATH_SET    explicit or implicit, but this
	 *			     only matters for sending version 1
	 *			     cookies; the cookies read from the
	 *			     file are currently treated all like
	 *			     version 0 (we don't set moo->version)
	 *			     so $Domain= and $Path= will normally
	 *			     not be sent to the server.  But if
	 *			     these cookies somehow get mixed with
	 *			     new version 1 cookies we may end up
	 *			     sending version 1 to the server, and
	 *			     in that case we should send $Domain
	 *			     and $Path.  The state-man-mec drafts
	 *			     and RFC 2109 say that $Domain and
	 *			     $Path SHOULD be omitted if they were
	 *			     not given explicitly, but not that
	 *			     they MUST be omitted.
	 *			     See 8.2 Cookie Spoofing in draft -10
	 *			     for a good reason to send them.
	 *			     However, an explicit domain should be
	 *			     now prefixed with a dot (unless it is
	 *			     for a single host), so we check for
	 *			     that.
	 *  COOKIE_FLAG_SECURE	     Should have "FALSE" for normal,
	 *			     otherwise set it.
	 */
	moo->flags |= COOKIE_FLAG_FROM_FILE | COOKIE_FLAG_EXPIRES_SET |
	    	      COOKIE_FLAG_PATH_SET;
	if (domain[0] == '.')
	    moo->flags |= COOKIE_FLAG_DOMAIN_SET;
	if (secure[0] != 'F')
	    moo->flags |= COOKIE_FLAG_SECURE;
	/* @@@ Should we set port to 443 if secure is set? @@@ */
	moo->expires = expires;
	/*
	 * I don't like using this to store the cookies because it's
	 * designed to store cookies that have been received from an
	 * HTTP request, not from a persistent cookie jar.  Hence the
	 * mucking about with the COOKIE_FLAG_FROM_FILE above. - RP
	 */
	store_cookie(moo, domain, path);
    }

#ifndef DISABLE_TRACE
    if (cookieTrace)
	fprintf(stderr, "LoadCookies: reading permissions from %s\n", permfile);
#endif
    /*
     *  Ensure that the domain list exists.
     */
    if (!domain_btree) {
	domain_btree = HTBTree_new((HTComparer) compare_cookie_domains);
	total_cookies = 0;
    }
    while (LYSafeGets(&buf, perm_handle)) {
	unsigned int i = 0;

	if (!*buf || (*buf == '\n') || (*buf == '#'))
	    continue;
	/*
	 * Strip out the newline that fgets() puts at the end
	 */
	while ((buf[i] != '\n') && buf[i])
	    i++;
	if (buf[i] == '\n') {
	    buf[i++] = '\t';	/* Add sep after line if enough space - kw */
	    buf[i] = '\0';
	}
	/*
	 * Tokenise the permission line into its component parts.
	 */
#ifndef DISABLE_TRACE
	if (cookieTrace)
	    fprintf(stderr, "LoadCookies: tokenising %s\n", buf);
#endif
	tok_ptr = buf;
	tok_out = LYstrsep(&tok_ptr, "\t");
	for (tok_loop = 0; tok_out && perm_values[tok_loop].s; tok_loop++) {
#ifndef DISABLE_TRACE
	    if (cookieTrace)
	        fprintf(stderr, "\t%d:%p:%p:[%s]\n",
		        tok_loop, perm_values[tok_loop].s, tok_out, tok_out);
#endif
	    LYstrncpy(perm_values[tok_loop].s, tok_out,perm_values[tok_loop].n);
	    /*
	     * It looks like strtok ignores a leading delimiter,
	     * which makes things a bit more interesting.  Something
	     * like "FALSE\t\tFALSE\t" translates to FALSE,FALSE
	     * instead of FALSE,,FALSE. - RP
	     */
	    tok_out = LYstrsep(&tok_ptr, "\t");
	}
	if (perm_values[tok_loop].s) {
	    /* tok_out in above loop must have been NULL prematurely - kw */
#ifndef DISABLE_TRACE
	    if (cookieTrace)
	        fprintf(stderr, "Not enough tokens, ignoring line!\n");
#endif
	    continue;
	}
	if (my_strcasecmp(what, "host")) {
#ifndef DISABLE_TRACE
	    if (cookieTrace || reportBugs)
	        fprintf(stderr, "Not a host permission: %s\n", buf);
#endif
	    continue;
	}	
	if (my_strcasecmp(type, "cookie")) {
	    if (my_strcasecmp(type, "ssl")) {
#ifndef DISABLE_TRACE
	        if (cookieTrace || reportBugs)
	            fprintf(stderr,
			    "Not a cookie or ssl permission: %s\n", buf);
#endif
	    } else {
	        /* An SSL entry */
	        if (*value == '1') {
	            HT_SSL_Host *shost = calloc(1, sizeof(HT_SSL_Host));

	            StrAllocCopy(shost->host, domain);
		    shost->perm = TRUE;
		    shost->next = SSL_ignore_hosts;
		    SSL_ignore_hosts = shost;
		} else {
#ifndef DISABLE_TRACE
		    if (cookieTrace || reportBugs)
		        fprintf(stderr,
				"Unknown ssl permission: %s\n", value);
#endif
		}
	    }
	    continue;
	}	

	/* Find domain entry */
	sdomain.domain = domain;
	de = (domain_entry *)HTBTree_search(domain_btree, &sdomain);
	if (!de) {
	    /*
	     *	Domain not found; add a new entry for this domain.
	     */
	    de = (domain_entry *)calloc(1, sizeof(domain_entry));
	    if (!de)
		outofmem(__FILE__, "store_cookie");
	    de->bv = QUERY_USER;
            de->invcheck_bv = DEFAULT_INVCHECK_TYPE;  /* Should this go here? */
	    de->cookie_list = HTList_new();
	    StrAllocCopy(de->domain, domain);
	    HTBTree_add(domain_btree, de);
	}
	if (de) {
	    if (*value == '1') {
		de->bv = ACCEPT_ALWAYS;
	    } else if (*value == '2') {
		de->bv = REJECT_ALWAYS;
	    } else {
#ifndef DISABLE_TRACE
		if (cookieTrace || reportBugs)
		    fprintf(stderr, "Unknown cookie permission: %s\n", value);
#endif
	    }
	}
    }
    fclose(cookie_handle);
    fclose(perm_handle);
}

PUBLIC void HTStoreCookies (char *cookie_file, char *perm_file)
{
    HTBTElement *dt;
    HTList *cl;
    HT_SSL_Host *shost;
    domain_entry *de;
    cookie *co;
    FILE *cookie_handle, *perm_handle;
    time_t now = time(NULL);
    char *cookfile = create_cookie_filename(cookie_file);
    char *permfile = create_cookie_filename(perm_file);

#ifndef DISABLE_TRACE
    if (cookieTrace)
        fprintf(stderr, "StoreCookies: save cookies to %s\n", cookfile);
#endif
#ifdef VMS
    if (file_exists(cookfile)) {
        if (get_pref_boolean(eBACKUP_FILES)) {
	    char *tf;
	    char retBuf[BUFSIZ];

	    tf = (char *)malloc(strlen(cookfile) + strlen("_backup") + 5);
	    sprintf(tf, "%s_backup", cookfile);
	    if (my_copy(cookfile, tf, retBuf, BUFSIZ - 1,
		        get_pref_int(eBACKUPFILEVERSIONS)) != SYS_SUCCESS)
	        fprintf(stderr, "%s\n", retBuf);
	    free(tf);
        }
    }
    remove(cookfile);
    cookie_handle = fopen(cookfile, "w", "shr = nil", "rop = WBH", "mbf = 4",
			  "mbc = 32", "deq = 8", "fop = tef");
    if (file_exists(permfile)) {
        if (get_pref_boolean(eBACKUP_FILES)) {
	    char *tf;
	    char retBuf[BUFSIZ];

	    tf = (char *)malloc(strlen(permfile) + strlen("_backup") + 5);
	    sprintf(tf, "%s_backup", permfile);
	    if (my_copy(permfile, tf, retBuf, BUFSIZ - 1,
		        get_pref_int(eBACKUPFILEVERSIONS)) != SYS_SUCCESS)
	        fprintf(stderr, "%s\n", retBuf);
	    free(tf);
        }
    }
    remove(permfile);
    perm_handle = fopen(permfile, "w", "shr = nil", "rop = WBH", "mbf = 4",
			"mbc = 32", "deq = 8", "fop = tef");
#else
    cookie_handle = fopen(cookfile, "w");
    perm_handle = fopen(permfile, "w");
#endif
    free(cookfile);
    free(permfile);
    if (!cookie_handle || !perm_handle)
	return;

    /* Make sure it exists */
    if (!domain_btree) {
        domain_btree = HTBTree_new((HTComparer) compare_cookie_domains);
        total_cookies = 0;
    }
    fprintf(perm_handle, "# Permission File\n");
    fprintf(perm_handle, "# This is a generated file!  Do not edit.\n");

    /* Do SSL hosts first */
    for (shost = SSL_ignore_hosts; shost; shost = shost->next) {
	if (shost->perm == TRUE)
	    fprintf(perm_handle, "host\tssl\t1\t%s\n", shost->host);
    }

    for (dt = HTBTree_next(domain_btree, NULL); dt;
	 dt = HTBTree_next(domain_btree, dt)) {
	if (!(de = (domain_entry *)dt->object))
	    /*
	     *	Fote says the first object is NULL.  Go with that.
	     */
	    continue;

	/* Write domain permissions */
	if (de->bv == ACCEPT_ALWAYS) {
	    fprintf(perm_handle, "host\tcookie\t1\t%s\n", de->domain);
	} else if (de->bv == REJECT_ALWAYS) {
	    fprintf(perm_handle, "host\tcookie\t2\t%s\n", de->domain);
	}
	/*
	 *  Show the domain's cookies. - FM
	 */
	for (cl = de->cookie_list; cl; cl = cl->next) {
	    /*
	     *	First object is always NULL. - FM
	     */
	    if (!(co = (cookie *)cl->object))
		continue;

#ifndef DISABLE_TRACE
	    if (cookieTrace)
                fprintf(stderr, "StoreCookies: %ld cf %ld ", (long)now,
			(long)co->expires);
#endif
            if ((co->flags & COOKIE_FLAG_DISCARD)) {
#ifndef DISABLE_TRACE
		if (cookieTrace)
		    fprintf(stderr, "not stored - DISCARD\n");
#endif
                continue;
            } else if (!(co->flags & COOKIE_FLAG_EXPIRES_SET)) {
#ifndef DISABLE_TRACE
		if (cookieTrace)
		    fprintf(stderr, "not stored - no expiration time\n");
#endif
                continue;
            } else if (co->expires <= now) {
#ifndef DISABLE_TRACE
		if (cookieTrace)
		    fprintf(stderr, "not stored - EXPIRED\n");
#endif
                continue;
            }
	    fprintf(cookie_handle, "%s\t%s\t%s\t%s\t%ld\t%s\t%s%s%s\n",
		    de->domain,
		    (de->domain[0] == '.') ? "TRUE" : "FALSE",
		    co->path,
		    co->flags & COOKIE_FLAG_SECURE ? "TRUE" : "FALSE",
		    (long) co->expires, co->name,
		    co->quoted ? "\"" : "",
		    co->value,
		    co->quoted ? "\"" : "");
#ifndef DISABLE_TRACE
	    if (cookieTrace)
		fprintf(stderr, "STORED\n");
#endif
	}
    }
    fclose(cookie_handle);
    fclose(perm_handle);
}


/*	HandleCookies - F.Macrides (macrides@sci.wfeb.edu)
**	---------------
**
**  Lists all cookies by domain, and allows deletions of
**  individual cookies or entire domains, and changes of
**  'allow' settings.  The list is invoked via the COOKIE_JAR
**  command, and deletions or changes of 'allow'
**  settings are done by activating links in that list.
**  The procedure uses a COOKIEJAR: internal URL scheme.
**
**  Semantics:
**	COOKIEJAR:/			Create and load the Cookie Jar Page.
**	COOKIEJAR://domain		Manipulate the domain.
**	COOKIEJAR://domain/lynxID	Delete cookie with lynxID in domain.
**
**	New functions can be added as extensions to the path, and/or by
**	assigning meanings to ;parameters, a ?searchpart, and/or #fragments.
*/
PRIVATE int HTHandleCookies (WWW_CONST char *arg,
			     HTParentAnchor *anAnchor,
			     HTFormat format_out,
			     HTStream *sink)
{
    HTStream *target;
    char buf[6656];	/* A little more than worst case */
    HTBTElement *dt;
    HTList *cl, *next;
    domain_entry *de;
    cookie *co;
    char *domain = NULL;
    char *lynxID = NULL;
    char *name = NULL;
    char *value = NULL;
    char *path = NULL;
    char *comment = NULL;
    char *Address = NULL;
    char *Title = NULL;
    char *ptr;
    char ch;
    int empty = 0;
    static HTAtom *format_in;
    static int init = 0;

    if (!init) {
	format_in = WWW_HTML;
	init = 1;
    }
    /*
     *  Ensure that the domain list exists.
     */
    if (!domain_btree) {
        domain_btree = HTBTree_new((HTComparer) compare_cookie_domains);
        total_cookies = 0;
    }
    /*
     *	Check whether we have something to do. - FM
     */
    if (!HTBTree_next(domain_btree, NULL)) {
	HTProgress(COOKIE_JAR_IS_EMPTY);
	empty = 1;
    }

    ptr = arg + 10;
    if (*ptr) {
        if (!my_strncasecmp(ptr, "enable", 6)) {
	    HTSetCookies = TRUE;
	    set_pref_boolean(eCOOKIES, TRUE);
        } else if (!my_strncasecmp(ptr, "disable", 7)) {
	    HTSetCookies = FALSE;
	    set_pref_boolean(eCOOKIES, FALSE);
        } else if (!my_strncasecmp(ptr, "save", 4)) {
	    HTProgress("Saving the cookie jar");
	    HTStoreCookies(get_pref_string(eCOOKIE_FILE),
		           get_pref_string(ePERM_FILE));
        } else if (!my_strncasecmp(ptr, "load", 4)) {
	    HTProgress("Loading the cookie jar");
	    HTLoadCookies(get_pref_string(eCOOKIE_FILE),
		          get_pref_string(ePERM_FILE));
        } else if (!my_strncasecmp(ptr, "eatall", 6)) {
	    HTEatAllCookies = TRUE;
	    set_pref_boolean(eACCEPT_ALL_COOKIES, TRUE);
        } else if (!my_strncasecmp(ptr, "noeatall", 8)) {
	    HTEatAllCookies = FALSE;
	    set_pref_boolean(eACCEPT_ALL_COOKIES, FALSE);
        } else if (!my_strncasecmp(ptr, "file", 4)) {
	    HTCookieFile = TRUE;
	    set_pref_boolean(eUSE_COOKIE_FILE, TRUE);
        } else if (!my_strncasecmp(ptr, "nofile", 6)) {
	    HTCookieFile = FALSE;
	    set_pref_boolean(eUSE_COOKIE_FILE, FALSE);
        } else if (!my_strncasecmp(ptr, "refresh_jar", 11)) {
	    /* Just drop on thru. */
        /*
         *  If there's a domain string in the "host" field of the
         *  COOKIEJAR: URL, this is a request to delete something
         *  or change an 'allow' setting. - FM
         */
        } else if (domain = HTParse(arg, "", PARSE_HOST)) {
	    if (!*domain) {
	        FREE(domain);
	    } else {
	        /*
	         *  If there is a path string (not just a slash) in the
	         *  COOKIEJAR: URL, that's a cookie's lynxID and this
	         *  is a request to delete it from the Cookie Jar. - FM
	         */
	        if (lynxID = HTParse(arg, "", PARSE_PATH)) {
		    if (!*lynxID)
		        FREE(lynxID);
	        }
	    }
        }
    }
    if (domain && !empty) {
	/*
	 *  Find the domain in the domain_btree structure. - FM
	 */
	sdomain.domain = domain;
	de = HTBTree_search(domain_btree, &sdomain);
	if (de) {
		/*
		 *  We found the domain.  Check
		 *  whether a lynxID is present. - FM
		 */
		if (lynxID) {
		    /*
		     *	Seek and delete the cookie with this lynxID
		     *	in the domain's cookie list. - FM
		     */
		    for (cl = de->cookie_list; cl; cl = cl->next) {
			if (!(co = (cookie *)cl->object))
			    /*
			     *	First object is always empty. - FM
			     */
			    continue;
			if (!strcmp(lynxID, co->lynxID)) {
			    /*
			     *	We found the cookie.
			     *	Delete it if confirmed. - FM
			     */
			    if (!HTConfirm(DELETE_COOKIE_CONFIRMATION)) {
				FREE(lynxID);
				break;
			    }
			    HTList_removeObject(de->cookie_list, co);
			    freeCookie(co);
			    co = NULL;
			    total_cookies--;
			    if ((de->bv == QUERY_USER &&
				 HTList_isEmpty(de->cookie_list)) &&
				HTConfirm(DELETE_EMPTY_DOMAIN_CONFIRMATION)) {
				/*
				 *  No more cookies in this domain, no
				 *  default accept/reject choice was set
				 *  by the user, and got confirmation on
				 *  deleting the domain, so do it. - FM
				 */
				HTList_delete(de->cookie_list);
				de->cookie_list = NULL;
				HTBTree_delete(domain_btree, de);
				FREE(de->domain);
				FREE(de);
				HTProgress(DOMAIN_EATEN);
			    } else {
				HTProgress(COOKIE_EATEN);
			    }
			    break;
			}
		    }
		} else {
		    char *message;

		    /*
		     *	Prompt whether to delete all of the cookies in
		     *	this domain, or the domain if no cookies in it,
		     *	or to change its 'allow' setting, or to cancel,
		     *	and then act on the user's response. - FM
		     */
		    if (HTList_isEmpty(de->cookie_list)) {
			message =
			  "Delete domain; set allow Always, Prompt or Never; or Cancel?";
			XmxSetButtonClueText("Delete domain",
				     "Always accept cookies for this domain",
				     "Prompt to accept cookies for this domain",
				     "Never accept cookies for this domain",
				     "Dismiss this menu");
		    } else {
			message =
			  "Delete domain's cookies; set allow Always, Prompt or Never; or Cancel?";
			XmxSetButtonClueText("Delete domains's cookies",
				     "Always accept cookies for this domain",
				     "Prompt to accept cookies for this domain",
				     "Never accept cookies for this domain",
				     "Dismiss this menu");
		    }
		    ch = XmxDoFiveButtons(current_win->base, app_context,
				       "VMS Mosaic: Cookie Domain Modification",
				       message, "Delete", "Always", "Prompt",
				       "Never", "Cancel", 520);
		    XmxSetButtonClueText(NULL, NULL, NULL, NULL, NULL);

		    switch(ch) {
			case 2:
			    /*
			     *  Set to accept all cookies
			     *  from this domain. - FM
			     */
			    de->bv = ACCEPT_ALWAYS;
			    user_message(ALWAYS_ALLOWING_COOKIES, de->domain);
			    break;
			case 5:
			    /*
			     *  Cancelled. - FM
			     */
			    HTProgress(CANCELLED);
			    break;
			case 1:
			    if (HTList_isEmpty(de->cookie_list)) {
				/*
				 *  We had an empty domain, so we
				 *  were asked to delete it. - FM
				 */
				HTList_delete(de->cookie_list);
				de->cookie_list = NULL;
				HTBTree_delete(domain_btree, de);
				FREE(de->domain);
				FREE(de);
				HTProgress(DOMAIN_EATEN);
				break;
			    }
 Delete_all_cookies_in_domain:
			    /*
			     *  Delete all cookies in this domain. - FM
			     */
			    cl = de->cookie_list;
			    while (cl) {
				next = cl->next;
				co = cl->object;
				if (co) {
				    HTList_removeObject(de->cookie_list, co);
				    freeCookie(co);
				    co = NULL;
				    total_cookies--;
				}
				cl = next;
			    }
			    HTProgress(DOMAIN_COOKIES_EATEN);
			    /*
			     *  If a default accept/reject
			     *  choice is set, we're done. - FM
			     */
			    if (de->bv != QUERY_USER)
				break;
			    /*
			     *  Check whether to delete
			     *  the empty domain. - FM
			     */
			    if (HTConfirm(DELETE_EMPTY_DOMAIN_CONFIRMATION)) {
				HTList_delete(de->cookie_list);
				de->cookie_list = NULL;
				HTBTree_delete(domain_btree, de);
				FREE(de->domain);
				FREE(de);
				HTProgress(DOMAIN_EATEN);
			    }
			    break;
			case 3:
			    /*
			     *  Set to prompt for cookie acceptence
			     *  from this domain. - FM
			     */
			    de->bv = QUERY_USER;
			    user_message(PROMTING_TO_ALLOW_COOKIES, de->domain);
			    break;
			case 4:
			    /*
			     *  Set to reject all cookies
			     *  from this domain. - FM
			     */
			    de->bv = REJECT_ALWAYS;
			    user_message(NEVER_ALLOWING_COOKIES, de->domain);
			    if (!HTList_isEmpty(de->cookie_list) &&
				HTConfirm(DELETE_ALL_COOKIES_IN_DOMAIN))
				goto Delete_all_cookies_in_domain;
			    break;
			default:;
		    }
		}
	}
	if (!HTBTree_next(domain_btree, NULL))
	    HTProgress(ALL_COOKIES_EATEN);
        FREE(domain);
        FREE(lynxID);
    }
    /*
     *	Display the Cookie Jar Page.  Set up an HTML stream and
     *	return an updated Cookie Jar Page.
     */
    target = HTStreamStack(format_in, format_out, COMPRESSED_NOT,
			   sink, anAnchor);
    if (!target) {
	sprintf(buf, CANNOT_CONVERT_I_TO_O,
		HTAtom_name(format_in), HTAtom_name(format_out));
	HTAlert(buf);
	return(HT_NOT_LOADED);
    }
    /*
     *	Load HTML strings into buf and pass buf
     *	to the target for parsing and rendering. - FM
     */
#define PUTS(buf) (*target->isa->put_block)(target, buf, strlen(buf))

    sprintf(buf, "<HTML>\n<HEAD>\n<TITLE>%s</TITLE>\n</HEAD>\n<BODY>\n",
	    COOKIE_JAR_TITLE);
    PUTS(buf);

    sprintf(buf, "<CENTER><H1>%s</H1></CENTER><HR>\n", REACHED_COOKIE_JAR_PAGE);
    PUTS(buf);
    sprintf(buf, "<TABLE WIDTH=100%%><TR><TD WIDTH=70%%>\n");
    PUTS(buf);
    if (HTSetCookies) {
	sprintf(buf, "Cookies are enabled.&nbsp;&nbsp;\n");
	PUTS(buf);
	sprintf(buf, "<A HREF=\"COOKIEJAR:disable\">Disable</A>?\n<BR>\n");
	PUTS(buf);
    } else {
	sprintf(buf, "Cookies are disabled.&nbsp;&nbsp;\n");
	PUTS(buf);
	sprintf(buf, "<A HREF=\"COOKIEJAR:enable\">Enable</A>?\n<BR>\n");
	PUTS(buf);
    }
    if (HTCookieFile) {
	sprintf(buf, "Cookie file is enabled.&nbsp;&nbsp;\n");
	PUTS(buf);
	sprintf(buf,
		"<A HREF=\"COOKIEJAR:nofile\">Disable</A> it?&nbsp;&nbsp;\n");
	PUTS(buf);
	sprintf(buf, "<A HREF=\"COOKIEJAR:save\">Update</A> it?\n<BR>\n");
	PUTS(buf);
    } else {
	sprintf(buf, "Cookie file is disabled.&nbsp;&nbsp;\n");
	PUTS(buf);
	sprintf(buf, "<A HREF=\"COOKIEJAR:file\">Enable</A> it?\n<BR>\n");
	PUTS(buf);
    }
    if (HTEatAllCookies) {
	sprintf(buf, "Accepting all cookies.&nbsp;&nbsp;\n");
	PUTS(buf);
	sprintf(buf, "<A HREF=\"COOKIEJAR:noeatall\">Do not accept all</A>?\n");
	PUTS(buf);
    } else {
	sprintf(buf, "Not accepting all cookies.&nbsp;&nbsp;\n");
	PUTS(buf);
	sprintf(buf, "<A HREF=\"COOKIEJAR:eatall\">Accept all</A>?\n");
	PUTS(buf);
    }
    sprintf(buf, "</TD><TD>\n<A HREF=\"COOKIEJAR:refresh_jar\">\n");
    PUTS(buf);
    sprintf(buf,
	    "<IMG SRC=\"internal-cookie-image\" HSPACE=5 ALIGN=RIGHT></A>\n");
    PUTS(buf);
    sprintf(buf, "</TD></TR></TABLE>\n<HR>\n");
    PUTS(buf);

    sprintf(buf, "Total cookies:&nbsp;%d<BR>Total cookie domains:&nbsp;%d\n",
	    total_cookies, HTBTree_count(domain_btree));
    PUTS(buf);

    sprintf(buf, "<HR><BR>\n<TABLE><TR><TD VALIGN=TOP><B>Note:&nbsp;</B></TD>");
    PUTS(buf);
    sprintf(buf,
	    "<TD>Activate links to gobble up cookies or entire domains,<BR>\n");
    PUTS(buf);
    sprintf(buf, "%s</TD></TR></TABLE>\n<DL>\n",
	    "or to change a domain's <I>allow</I> setting.\n");
    PUTS(buf);

    empty = 1;
    for (dt = HTBTree_next(domain_btree, NULL); dt;
	 dt = HTBTree_next(domain_btree, dt)) {
	if (!(de = (domain_entry *)dt->object))
	    /*
	     *	First object always is NULL. - FM
	     */
	    continue;

	empty = 0;
	/*
	 *  Show the domain link and 'allow' setting. - FM
	 */
	sprintf(buf, "<SPAN TITLE=\"Change domain settings\">\n");
	PUTS(buf);
	sprintf(buf,
		"<DT>Domain <A HREF=\"COOKIEJAR://%s/\">%s</A>&nbsp;\n",
		de->domain, de->domain);
	PUTS(buf);
	switch (de->bv) {
	    case (ACCEPT_ALWAYS):
		sprintf(buf, COOKIES_ALWAYS_ALLOWED);
		break;
	    case (REJECT_ALWAYS):
		sprintf(buf, COOKIES_NEVER_ALLOWED);
		break;
	    case (QUERY_USER):
		sprintf(buf, COOKIES_ALLOWED_VIA_PROMPT);
		break;
	}
	PUTS(buf);
	sprintf(buf, "<SPAN TITLE=\"Delete cookie?\">\n");
	PUTS(buf);
	/*
	 *  Show the domain's cookies. - FM
	 */
	for (cl = de->cookie_list; cl; cl = cl->next) {
	    if (!(co = (cookie *)cl->object))
		/*
		 *  First object is always NULL. - FM
		 */
		continue;
	    /*
	     *	Show the name=value pair. - FM
	     */
	    if (co->name) {
		StrAllocCopy(name, co->name);
		LYEntify(&name, TRUE);
	    } else {
		StrAllocCopy(name, NO_NAME);
	    }
	    if (co->value) {
		StrAllocCopy(value, co->value);
		LYEntify(&value, TRUE);
	    } else {
		StrAllocCopy(value, NO_VALUE);
	    }
	    sprintf(buf, "<DD><A HREF=\"COOKIEJAR://%s/%s\">%s=%s</A>\n",
	            de->domain, co->lynxID, name, value);
	    FREE(name);
	    FREE(value);
	    PUTS(buf);

            if (co->flags & COOKIE_FLAG_FROM_FILE) {
                sprintf(buf, " &nbsp;(from a previous session)\n");
                PUTS(buf);
            }
	    /*
	     *	Show the path, port, secure and discard setting. - FM
	     */
	    if (co->path) {
		StrAllocCopy(path, co->path);
		LYEntify(&path, TRUE);
	    } else {
		StrAllocCopy(path, "/");
	    }
	    sprintf(buf,
     "<DD>Path=%s\n<DD>Port: %d&nbsp;&nbsp;Secure: %s&nbsp;&nbsp;Discard: %s\n",
		    path, co->port,
		    (co->flags & COOKIE_FLAG_SECURE) ? "Yes" : "No",
		    (co->flags & COOKIE_FLAG_DISCARD) ? "Yes" : "No");
	    FREE(path);
	    PUTS(buf);
	    /*
	     *	Show the list of acceptable ports, if present. - FM
	     */
	    if (co->PortList) {
		sprintf(buf, "<DD>PortList=\"%s\"\n", co->PortList);
		PUTS(buf);
	    }
	    /*
	     *	Show the commentURL, if we have one. - FM
	     */
	    if (co->commentURL) {
		StrAllocCopy(Address, co->commentURL);
		LYEntify(&Address, FALSE);
		StrAllocCopy(Title, co->commentURL);
		LYEntify(&Title, TRUE);
		sprintf(buf, "<DD>CommentURL: <A href=\"%s\">%s</A>\n",
			Address, Title);
		FREE(Address);
		FREE(Title);
		PUTS(buf);
	    }
	    /*
	     *	Show the comment, if we have one. - FM
	     */
	    if (co->comment) {
		StrAllocCopy(comment, co->comment);
		LYEntify(&comment, TRUE);
		sprintf(buf, "<DD>Comment: %s\n", comment);
		FREE(comment);
		PUTS(buf);
	    }
	    /*
	     *	Show the Maximum Gobble Date. - FM
	     */
	    sprintf(buf, "<DD><EM>Maximum Gobble Date:</EM> %s%s",
		    (co->expires > 0 && !(co->flags & COOKIE_FLAG_DISCARD)) ?
		     ctime(&co->expires) : END_OF_SESSION,
		    (co->expires > 0 && !(co->flags & COOKIE_FLAG_DISCARD)) ?
		     "" : "\n");
	    PUTS(buf);
	}
	sprintf(buf, "</DT></SPAN>\n<BR><BR>\n");
	PUTS(buf);
    }
    sprintf(buf, "</DL>\n");
    PUTS(buf);
    if (empty) {
	sprintf(buf,
	        "<BR><BR><H2><CENTER>The Cookie Jar is empty.</CENTER></H2>\n");
	PUTS(buf);
    }
    sprintf(buf, "</BODY>\n</HTML>\n");
    PUTS(buf);
    /*
     *	Free the target to complete loading of the
     *	Cookie Jar Page, and report a successful load. - FM
     */
    (*target->isa->free)(target);

    /* Tell GUI what URL to use */
    use_this_url_instead = strdup("cookiejar:");

    return(HT_LOADED);
}

PUBLIC HTProtocol HTMosaicCookies = { "COOKIEJAR", HTHandleCookies, NULL };
