/*		Parse HyperText Document Address		HTParse.c
**		================================
*/

/* Copyright (C) 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include "HTUtils.h"
#include "HTParse.h"

#define HEX_ESCAPE '%'

struct struct_parts {
    char *access;
    char *host;
    char *absolute;
    char *relative;
    char *search;	/* Normally part of path */
    char *anchor;
};

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

#define SPACE(c) (((c) == ' ') || ((c) == '\t') || ((c) == '\n')) 

static char *strchr_or_end(char *string, int ch)
{
    char *result = strchr(string, ch);

    if (!result)
        result = string + strlen(string);

    return result;
}

/*	Strip white space off a string
**	------------------------------
**
** On exit,
**	Return value points to first non-white character, or to 0 if none.
**	All trailing white space is OVERWRITTEN with zero.
*/
char *HTStrip(char *s)
{
    char *p;

    for (p = s; *p; p++)
        ;		        /* Find end of string */
    for (p--; p >= s; p--) {
    	if (SPACE(*p)) {
	    *p = '\0';		/* Zap trailing blanks */
	} else {
	    break;
	}
    }
    while (SPACE(*s))
	s++;			/* Strip leading blanks */
    return s;
}


/*	Scan a filename for its parts
**	-----------------------------
**
** On entry,
**	name	points to a document name which may be incomplete.
** On exit,
**      absolute or relative may be nonzero (but not both).
**	host, anchor and access may be nonzero if they were specified.
**	Any which are nonzero point to zero terminated strings.
*/
PRIVATE void scan(char *name, struct struct_parts *parts)
{
    char *after_access = name;
    char *p;

    parts->access = NULL;
    parts->host = NULL;
    parts->absolute = NULL;
    parts->relative = NULL;
    parts->search = NULL;	/* Normally not used - kw */
    parts->anchor = NULL;
    /*
    **  Scan left-to-right for a scheme (access).
    */
    for (p = name; *p; p++) {
	if (*p == ':') {
	    *p = '\0';
	    parts->access = name;	/* Access name has been specified */
	    after_access = p + 1;
	    /* Makes HTTP ==> http */
	    if (parts->access && *parts->access) {
 		char *mp = parts->access;

		while (*mp) {
		    *mp = tolower(*mp);
		    mp++;
		}
	    }
	    break;
	}
	if ((*p == '/') || (*p == '#') || (*p == ';') || (*p == '?'))
	    break;
    }
    /*
    **  Scan left-to-right for a fragment (anchor).
    */
    for (p = after_access; *p; p++) {
	if (*p == '#') {
	    parts->anchor = p + 1;
	    *p = '\0';			/* Terminate the rest */
	    break;        		/* Leave things after first # alone */
	}
    }
    /*
    **  Scan left-to-right for a host or absolute path.
    */
    p = after_access;
    if (*p == '/') {
	if (p[1] == '/') {
	    parts->host = p + 2;		/* Host has been specified */
	    *p = '\0';				/* Terminate access */
	    p = strchr(parts->host, '/');  	/* Look for end of host name */
	    if (p) {
	        *p = '\0';			/* Terminate host */
	        parts->absolute = p + 1;	/* Root has been found */
	    } else if (p = strchr(parts->host, '?')) {
		*p = '\0';			/* Terminate host */
		parts->search = p + 1;
	    }
	} else {
	    parts->absolute = p + 1;		/* Root found but no host */
	}	    
    } else {
	/* NULL for "" */
        parts->relative = *after_access ? after_access : NULL;
    }
    /*
    **  Check schemes that commonly have unescaped hashes.
    */
    if (parts->access && parts->anchor) {
        if (!parts->host ||
            !strcmp(parts->access, "nntp") ||
            !strcmp(parts->access, "snews") ||
            !strcmp(parts->access, "news")) {
            /*
             *  Access specified but no host, so the
             *  anchor may not really be one, e.g., news:j462#36487@foo.bar,
             *  or it's an nntp or snews URL, or news URL with a host.
             *  Restore the '#' in the address.
             */
            /* But only if we have found a path component of which this will
             * become part. - kw  */
            if (parts->relative || parts->absolute) {
                *(parts->anchor - 1) = '#';
                parts->anchor = NULL;
            }
        }
    }
}


/*	Parse a Name relative to another name
**	-------------------------------------
**
**	This returns those parts of a name which are given (and requested)
**	substituting bits from the related name where necessary.
**
** On entry,
**	aName		Filename given
**      relatedName     Name relative to which aName is to be parsed
**      wanted          Mask for the bits which are wanted.
**
** On exit,
**	returns		Pointer to a malloc'd string which MUST BE FREED
*/
char *HTParse(char *aName, char *relatedName, int wanted)
{
    int len = 10;
    int i, count;
    char *result, *name, *access, *p;
    char *rel = NULL;
    char *return_value = NULL;
    struct struct_parts given, related;
    
    /*
     * Make working copies of input strings to cut up:
     */
    if (aName) {
        name = strdup(aName);
	len += strlen(aName);
    } else {
        name = strdup("");
    }
    
    scan(name, &given);

    if ((given.access && given.host && given.absolute) ||
	!relatedName || !*relatedName) {
	/*
	 * Inherit nothing!
	 */
	related.access = NULL;
	related.host = NULL;
	related.absolute = NULL;
	related.relative = NULL;
	related.search = NULL;
	related.anchor = NULL;
    } else {
	rel = strdup(relatedName);
	len += strlen(relatedName);
        scan(rel, &related);
    }

    result = (char *)malloc(len);	/* Lots of space: more than enough */
    *result = '\0';
    /*
    **	Handle the scheme (access) field.
    */
    if (given.access && given.host && !given.relative && !given.absolute) {
	if (!strcmp(given.access, "http") ||
	    !strcmp(given.access, "https") ||
	    !strcmp(given.access, "ftp"))
	    /*
	    **	Assume root.
	    */
	    given.absolute = "";
    }
    access = given.access ? given.access : related.access;
    if ((wanted & PARSE_ACCESS) && access) {
	strcat(result, access);
	if (wanted & PARSE_PUNCTUATION)
	    strcat(result, ":");
    }
    /* If different access methods, inherit nothing. */
    if (given.access && related.access &&
	strcmp(given.access, related.access)) {
	related.host = NULL;
	related.absolute = NULL;
	related.relative = NULL;
	related.search = NULL;
	related.anchor = NULL;
    }
    if (wanted & PARSE_HOST) {
        if (given.host || related.host) {
            char *tail = result + strlen(result);

	    if (wanted & PARSE_PUNCTUATION)
		strcat(result, "//");
	    strcat(result, given.host ? given.host : related.host);

	    /* Ignore default port numbers, and trailing dots on FQDNs
	     * which will only cause identical addresses to look different */
            p = strchr(tail, ':');
            if (p && access) {		/* Port specified */
                if ((!strcmp(access, "http") && !strcmp(p, ":80")) ||
		    (!strcmp(access, "https") && !strcmp(p, ":443")) ||
		    (!strcmp(access, "ftp") && !strcmp(p, ":21")) ||
                    (!strcmp(access, "gopher") &&
		     (!strcmp(p, ":70") || !strcmp(p, ":70+")))) {
                    *p = '\0';	  /* It is the default: ignore it */
                } else if (*p && (p[strlen(p) - 1] == '+')) {
                    p[strlen(p) - 1] = '\0';
		}
            }
            if (!p) 
                p = tail + strlen(tail);	/* After hostname */
            p--;				/* End of hostname */
            if (strlen(tail) > 3 && (*p == '.')) {
		char *p1 = p + 1;

#ifndef DISABLE_TRACE
                if (www2Trace)
                    fprintf(stderr, "[Parse] tail '%s' p '%s'\n", tail, p);
#endif
                *p = '\0';  /* Chop final . */
                
                /* OK, at this point we know that *(p + 1) exists,
                 * else we would not be here.
                 * If it's 0, then we're done.
                 * If it's not 0, then we move *(p + 2) to *(p + 1), etc.
		 */
                if (*p1) {
		    int len = strlen(p1);

#ifndef DISABLE_TRACE
                    if (www2Trace)
                        fprintf(stderr,
			        "[Parse] Copying '%s' to '%s', %d bytes\n", 
                                p1, p, len);
#endif
                    memcpy(p, p1, len);
#ifndef DISABLE_TRACE
                    if (www2Trace)
                        fprintf(stderr, "[Parse] Setting '%c' to 0...\n",
                                *(p + len));
#endif
                    *(p + len) = '\0';
                }
#ifndef DISABLE_TRACE
                if (www2Trace)
                    fprintf(stderr, "[Parse] tail '%s' p '%s'\n", tail, p);
#endif
            }
            p = strchr(tail, '@');
            if (!p)
                p = tail;
            for (; *p; p++)
                *p = TOLOWER(*p);
        }
    }	
    /*
     * If host in given or related was ended directly with a '?' (no slash),
     * fake the search part into absolute.  This is the only case search is
     * returned from scan.  A host must have been present.  This restores the
     * '?' at which the host part had been truncated in scan, we have to do
     * this after host part handling is done.  - kw
     */
    if (given.search && !*(given.search - 1)) {
        given.absolute = given.search - 1;
        given.absolute[0] = '?';
    } else if (related.search && !related.absolute && !*(related.search - 1)) {
        related.absolute = related.search - 1;
        related.absolute[0] = '?';
    }

    /* If different hosts, inherit no path. */
    if (given.host && related.host && strcmp(given.host, related.host)) {
	related.absolute = NULL;
	related.relative = NULL;
	related.anchor = NULL;
    }
    /*
    **  Handle the path.
    */
    if (wanted & PARSE_PATH) {
	if (access && !given.absolute && given.relative) {
	    if (!strcmp(access, "nntp") ||
		!strcmp(access, "snews") ||
		(!strcmp(access, "news") &&
		 !strncmp(result, "news://", 7))) {
		/*
		 *  Treat all given nntp or snews paths,
		 *  or given paths for news URLs with a host,
		 *  as absolute.
		 */
		given.absolute = given.relative;
		given.relative = NULL;
	    }
	}
        if (given.absolute) {			/* All is given */
	    if (wanted & PARSE_PUNCTUATION)
		strcat(result, "/");
	    strcat(result, given.absolute);
	} else if (related.absolute) {		/* Adopt path not name */
            char *tail = result + strlen(result);

	    *tail++ = '/';
	    strcpy(tail, related.absolute);
	    if (given.relative) {
		/* RFC 1808 part 4 step 5 (if URL path is empty) */
		if (given.relative[0] == ';') {
		    /* a) if given has params, add/replace that */
		    strcpy(strchr_or_end(tail, ';'), given.relative);
		} else if (given.relative[0] == '?') {
		    /* b) if given has query, add/replace that */
		    strcpy(strchr_or_end(tail, '?'), given.relative);
		} else {
		    /* Otherwise fall through to RFC 1808 part 4 step 6 */
		    p = strchr(tail, '?');	/* Search part? */
		    if (!p)
			p = tail + strlen(tail) - 1;
		    for (; *p != '/'; p--)	/* Find last / */
			;
		    p++;			/* After last / */
		    strcpy(p, given.relative);	/* Put given one after it */
		}
		HTSimplify(result);
	    }
	} else if (given.relative) {
	    strcat(result, given.relative);	/* What we've got */
	} else if (related.relative) {
	    strcat(result, related.relative);
	} else if (strcmp(result, "about:") && strcmp(result, "cookiejar:")) {
	    /* No inheritance */
	    strcat(result, "/");
	}
    }
    /*
     * Handle the fragment (anchor).  Never inherit.
     */
    if ((wanted & PARSE_ANCHOR) && given.anchor && *given.anchor) {
	if (wanted & PARSE_PUNCTUATION)
	    strcat(result, "#");
	strcat(result, given.anchor);
    }
    if (name)
        free(name);

    /* Now we are almost done but there is still a problem with
     * things like ../ syntax at root URL i.e. http://url/../blabla
     * not understood because there are not enough necessary sub-dir
     * in this path.  The solution is to strip it out and hope it
     * works.  See for example http://www.cern.ch where this happens.
     * J.L. 26-Apr-1999
     */
    len = strlen(result);

    /* Purpose of this variable is to restrain the fix to one Level */
    count = 0;
    for (i = 0; (i < len) && (count < 4); i++) {
	if ((result[i] == '.') && (result[i + 1] == '.') &&
	    (result[i + 2] == '/') && (count == 3)) {
	    p = malloc(len + 1);
	    strncpy(p, result, i);
	    *(p + i) = '\0';
	    strcat(p, result + i + 3);
	    /* Copy it back */
	    strcpy(result, p);
	    free(p);
	    count++;
	} else if (result[i] == '/') {
	    count++;
	}
    }
    if (rel)
	free(rel);
    StrAllocCopy(return_value, result);
    free(result);
    return return_value;		/* Exactly the right length */
}


/*	        Simplify a filename
 *		-------------------
 *
 * A unix-style file is allowed to contain the sequence xxx/../ which may be
 * replaced by "", and the sequence "/./" which may be replaced by "/".
 * Simplification helps us recognize duplicate filenames.
 *
 *	Thus, 	/etc/junk/../fred 	becomes	/etc/fred
 *		/etc/junk/./fred	becomes	/etc/junk/fred
 *
 *      but we should NOT change
 *		http://fred.xxx.edu/../..
 *
 *	or	../../albert.html
 */
void HTSimplify(char *filename)
{
  if (*filename && filename[1]) {
      char *p;

      for (p = filename + 2; *p; p++) {
          if (*p == '/') {
              if ((p[1] == '.') && (p[2] == '.') && (p[3] == '/' || !p[3])) {
		  char *q;

                  /* Changed clause below to (q > filename) due to attempted
                   * read to q = filename - 1 below. */
                  for (q = p - 1; (q > filename) && (*q != '/'); q--)
                      ;  /* Previous slash */
                  if ((*q == '/') && strncmp(q, "/../", 4) &&
		      !(q - 1 > filename && q[-1] == '/')) {
                      strcpy(q, p + 3);	 /* Remove /xxx/.. */
                      if (!*filename)
			  strcpy(filename, "/");
                      p = q - 1;	/* Start again with prev slash 	*/
                  } 
              } else if ((p[1] == '.') && (p[2] == '/' || !p[2])) {
                  strcpy(p, p + 2);		/* Remove a slash and a dot */
              }
          }
      }
  }
}
  

/*		Make Relative Name
**		------------------
**
** This function creates and returns a string which gives an expression of
** one address as related to another.  Where there is no relation, an absolute
** address is retured.
**
**  On entry,
**	Both names must be absolute, fully qualified names of nodes
**	(no anchor bits)
**
**  On exit,
**	The return result points to a newly allocated name which, if
**	parsed by HTParse relative to relatedName, will yield aName.
**	The caller is responsible for freeing the resulting name later.
**
*/
char *HTRelative(char *aName, char *relatedName)
{
    char *p = aName;
    char *q = relatedName;
    char *result = NULL;
    char *after_access = NULL;
    char *path = NULL;
    char *last_slash = NULL;
    int slashes = 0;
    
    for (; *p; p++, q++) {	/* Find extent of match */
    	if (*p != *q)
	    break;
	if (*p == ':')
	    after_access = p + 1;
	if (*p == '/') {
	    last_slash = p;
	    if (++slashes == 3)
		path = p;
	}
    }
    /* q, p point to the first non-matching character or zero */
    
    if (!after_access) {			/* Different access */
        StrAllocCopy(result, aName);
    } else if (slashes < 3) {			/* Different nodes */
    	StrAllocCopy(result, after_access);
    } else if (slashes == 3) {			/* Same node, different path */
        StrAllocCopy(result, path);
    } else {					/* Some path in common */
        int levels = 0;

        for (; *q && (*q != '#'); q++) {
	    if (*q == '/')
		levels++;
	}
	result = (char *)malloc(3 * levels + strlen(last_slash) + 1);
	*result = '\0';
	for (; levels; levels--)
	    strcat(result, "../");
	strcat(result, last_slash + 1);
    }
#ifndef DISABLE_TRACE
    if (www2Trace) 
        fprintf(stderr, "HT: `%s' expressed relative to\n    `%s' is\n   `%s'.",
                aName, relatedName, result);
#endif
    return result;
}


static unsigned char isAcceptable[96] =
/*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
{    0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,	 /* 2x   !"#$%&'()*+,-./    */
     1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,	 /* 3x  0123456789:;<=>?    */
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	 /* 4x  @ABCDEFGHIJKLMNO    */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,	 /* 5x  PQRSTUVWXYZ[\]^_    */
     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	 /* 6x  `abcdefghijklmno    */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };	 /* 7x  pqrstuvwxyz{\}~	DEL */

#define HT_HEX(i) (i < 10 ? '0' + i : 'A' + i - 10)

/* The string returned from here, if any, can be free'd by caller. */
char *HTEscape(char *str)
{
  char *q, *p, *escaped;

  if (!str)
      return NULL;

  q = escaped = (char *)malloc(strlen(str) * 3 + 1);
  
  for (p = str; *p; p++) {
      int c = (int)((unsigned char)*p);

      if (c >= 32 && c <= 127 && isAcceptable[c - 32]) {
          *q++ = *p;
      } else {
          *q++ = '%';
          *q++ = HT_HEX(c / 16);
          *q++ = HT_HEX(c % 16);
      }
  }
  *q = '\0';
  
  return escaped;
}


/*		Decode %xx escaped characters			HTUnEscape()
**		-----------------------------
**
**	This function takes a pointer to a string in which some
**	characters may have been encoded in %xy form, where xy is
**	the acsii hex code for character 16x+y.
**	The string is converted in place, as it will never grow.
*/

PUBLIC char from_hex (char c)
{
    return (c >= '0') && (c <= '9') ? (c - '0') :
    	   (c >= 'A') && (c <= 'F') ? (c - 'A' + 10) :
    	   (c >= 'a') && (c <= 'f') ? (c - 'a' + 10) :  /* Accept lowercase */
	   0;
}

PUBLIC char *HTUnEscape (char * str)
{
    char *p = str;
    char *q = str;

    while (*p) {
        if (*p == HEX_ESCAPE) {
	    if (*++p)
		*q = from_hex(*p++) * 16;
	    if (*p)
		*q = *q + from_hex(*p++);
	    q++;
        } else if (*p == '+') {
            p++;
            *q++ = ' ';
	} else {
	    *q++ = *p++; 
	}
    }
    *q = '\0';
    return str;
}
