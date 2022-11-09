/*		Parse HyperText Document Address		HTParse.c
**		================================
*/
#include "../config.h"
#include "HTUtils.h"
#include "HTParse.h"
#include "tcp.h"

#define HEX_ESCAPE '%'

struct struct_parts {
	char *access;
	char *host;
	char *absolute;
	char *relative;
	char *anchor;
};

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

#define SPACE(c) ((c == ' ') || (c == '\t') || (c == '\n')) 

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
	    *p = 0;		/* Zap trailing blanks */
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
    char *after_access;
    char *p;
    char *mp; /* For case in-sensitizing scheme (access) */

    parts->access = NULL;
    parts->host = NULL;
    parts->absolute = NULL;
    parts->relative = NULL;
    parts->anchor = NULL;

    /*
    **  Scan left-to-right for a scheme (access).
    */
    after_access = name;
    for (p = name; *p; p++) {
	if (*p == ':') {
	    *p = '\0';
	    parts->access = name;	/* Access name has been specified */
	    after_access = p + 1;
	    /* Makes HTTP ==> http */
	    if (parts->access && *(parts->access)) {
		mp = parts->access;
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
	    parts->host = p + 2;	/* Host has been specified */
	    *p = '\0';			/* Terminate access */
	    p = strchr(parts->host, '/'); /* Look for end of host name if any */
	    if (p) {
	        *p = '\0';			/* Terminate host */
	        parts->absolute = p + 1;	/* Root has been found */
	    }
	} else {
	    parts->absolute = p + 1;		/* Root found but no host */
	}	    
    } else {
        parts->relative = (*after_access) ?
				after_access : NULL; /* NULL for "" */
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
**	aName		A filename given
**      relatedName     A name relative to which aName is to be parsed
**      wanted          A mask for the bits which are wanted.
**
** On exit,
**	returns		A pointer to a malloc'd string which MUST BE FREED
*/
char *HTParse(char *aName, char *relatedName, int wanted)
{
    char *result;
    char *return_value = 0;
    int len, i, count;
    char *name = 0;
    char *rel = 0;
    char *p;
    char *access;
    struct struct_parts given, related;
    
    if (!aName)
        aName = strdup("\0");
    if (!relatedName)
        relatedName = strdup("\0");
    
    /* Make working copies of input strings to cut up:
    */
    len = strlen(aName) + strlen(relatedName) + 10;
    result = (char *)malloc(len);	/* Lots of space: more than enough */
    
    StrAllocCopy(name, aName);
    StrAllocCopy(rel, relatedName);
    
    scan(name, &given);
    scan(rel, &related); 
    result[0] = 0;		/* Clear string  */

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
    if (wanted & PARSE_ACCESS) {
        if (access) {
	    strcat(result, access);
	    if (wanted & PARSE_PUNCTUATION)
		strcat(result, ":");
	}
    }
    /* If different access methods, inherit nothing. */
    if (given.access && related.access &&
	strcmp(given.access, related.access)) {
	related.host = NULL;
	related.absolute = NULL;
	related.relative = NULL;
	related.anchor = NULL;
    }

    if (wanted & PARSE_HOST) {
        if (given.host || related.host) {
            char *tail = result + strlen(result);
            char *tmp;

	    if (wanted & PARSE_PUNCTUATION)
		strcat(result, "//");
	    strcat(result, given.host ? given.host : related.host);

	    /* Ignore default port numbers, and trailing dots on FQDNs
	       which will only cause identical addresses to look different */
            p = strchr(tail, ':');
            if (p && access) {		/* Port specified */
                if ((!strcmp(access, "http") && !strcmp(p, ":80")) ||
		    (!strcmp(access, "https") && !strcmp(p, ":443")) ||
		    (!strcmp(access, "ftp") && !strcmp(p, ":21")) ||
                    (!strcmp(access, "gopher") &&
		     (!strcmp(p, ":70") || !strcmp(p, ":70+")))) {
                    *p = '\0';	/* It is the default: ignore it */
                } else if (*p && (p[strlen(p) - 1] == '+')) {
                    p[strlen(p) - 1] = '\0';
		}
            }
            if (!p) 
                p = tail + strlen(tail);	/* After hostname */
            p--;				/* End of hostname */
            if (strlen(tail) > 3 && (*p == '.')) {
#ifndef DISABLE_TRACE
                if (www2Trace)
                    fprintf(stderr, "[Parse] tail '%s' p '%s'\n", tail, p);
#endif
                *p = '\0'; /* Chop final . */
                
                /* OK, at this point we know that *(p + 1) exists,
                   else we would not be here.

                   If it's 0, then we're done.

                   If it's not 0, then we move *(p + 2) to *(p + 1),
                   etc.
		 */
                if (*(p + 1) != '\0') {
#ifndef DISABLE_TRACE
                    if (www2Trace)
                        fprintf(stderr,
			        "[Parse] Copying '%s' to '%s', %d bytes\n", 
                                p + 1, p, strlen(p + 1));
#endif
                    memcpy(p, p + 1, strlen(p + 1));
#ifndef DISABLE_TRACE
                    if (www2Trace)
                        fprintf(stderr, "[Parse] Setting '%c' to 0...\n",
                                *(p + strlen(p + 1)));
#endif
                    *(p + strlen(p + 1)) = '\0';
                }
#ifndef DISABLE_TRACE
                if (www2Trace)
                    fprintf(stderr, "[Parse] tail '%s' p '%s'\n", tail, p);
#endif
            }
            tmp = strchr(tail, '@');
            if (!tmp)
                tmp = tail;
            for (; *tmp; tmp++)
                *tmp = TOLOWER(*tmp);
        }
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
	    strcat(result, "/");
	    strcat(result, related.absolute);
	    if (given.relative) {
		p = strchr(result, '?');	/* Search part? */
		if (!p)
		    p = result + strlen(result) - 1;
		for (; *p != '/'; p--)		/* Last / */
		    ;
		p[1] = '\0';			/* Remove filename */
		strcat(result, given.relative);	/* Add given one */
		HTSimplify(result);
	    }
	} else if (given.relative) {
	    strcat(result, given.relative);	/* What we've got */
	} else if (related.relative) {
	    strcat(result, related.relative);
	} else {  /* No inheritance */
	    strcat(result, "/");
	}
    }
		
    if (wanted & PARSE_ANCHOR) {
        if (given.anchor || related.anchor) {
	    if (wanted & PARSE_PUNCTUATION)
		strcat(result, "#");
	    strcat(result, given.anchor ? given.anchor : related.anchor);
	}
    }
    if (rel)
        free(rel);
    if (name)
        free(name);

    /* Now we are almost done but there is still a problem with 	*/
    /* things like ../ syntax at root URL i.e. http://url/../blabla 	*/
    /* not understood because there are not enough necessary sub-dir in */
    /* this path.  The solution is to strip it out and hope it 		*/
    /* works.  See for example http://www.cern.ch where this happens.	*/
    /* J.L. 26-Apr-1999							*/
    StrAllocCopy(rel, result);
    len	= strlen(result);

    /* Purpose of this variable is to restrain the fix to one Level */
    count = 0;
    *rel = 0;
    for (i = 0; (i < len) && (count < 4); i++) {
	if ((result[i] == '.') && (result[i + 1] == '.') &&
	    (result[i + 2] == '/') && (count == 3)) {

	    strncpy(rel, result, i);
	    *(rel + i) = '\0';
	    strcat(rel, result+i+3);
	    /* Copy it back	*/
	    strcpy(result, rel);
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
//		-------------------
//
// A unix-style file is allowed to contain the sequence xxx/../ which may be
// replaced by "" , and the sequence "/./" which may be replaced by "/".
// Simplification helps us recognize duplicate filenames.
//
//	Thus, 	/etc/junk/../fred 	becomes	/etc/fred
//		/etc/junk/./fred	becomes	/etc/junk/fred
//
//      but we should NOT change
//		http://fred.xxx.edu/../..
//
//	or	../../albert.html
*/
void HTSimplify(char *filename)
{
  char *p;
  char *q;

  if (filename[0] && filename[1]) {
      for (p = filename + 2; *p; p++) {
          if (*p == '/') {
              if ((p[1] == '.') && (p[2] == '.') && (p[3] == '/' || !p[3] )) {
                  /* Changed clause below to (q>filename) due to attempted
                     read to q = filename-1 below. */
                  for (q = p - 1; (q>filename) && (*q != '/'); q--)
                      ; /* prev slash */
                  if ((q[0] == '/') && strncmp(q, "/../", 4)
                      && !(q - 1 > filename && q[-1] == '/')) {
                      strcpy(q, p+3);	/* Remove  /xxx/..	*/
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
    char *result = 0;
    WWW_CONST char *p = aName;
    WWW_CONST char *q = relatedName;
    WWW_CONST char *after_access = 0;
    WWW_CONST char *path = 0;
    WWW_CONST char *last_slash = 0;
    int slashes = 0;
    
    for (; *p; p++, q++) {	/* Find extent of match */
    	if (*p != *q)
	    break;
	if (*p == ':')
	    after_access = p+1;
	if (*p == '/') {
	    last_slash = p;
	    slashes++;
	    if (slashes == 3)
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
	result[0] = 0;
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
{    0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,	/* 2x   !"#$%&'()*+,-./	 */
     1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,	/* 3x  0123456789:;<=>?	 */
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 4x  @ABCDEFGHIJKLMNO  */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,	/* 5x  PQRSTUVWXYZ[\]^_	 */
     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 6x  `abcdefghijklmno	 */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };	/* 7x  pqrstuvwxyz{\}~	DEL */

#define HT_HEX(i) (i < 10 ? '0'+i : 'A'+ i - 10)

/* The string returned from here, if any, can be free'd by caller. */
char *HTEscape(char *str)
{
  char *q;
  char *p;		/* Pointers into keywords */
  char *escaped;

  if (!str)
      return NULL;

  escaped = (char *)malloc(strlen(str) * 3 + 1);
  
  for (q = escaped, p = str; *p != '\0'; p++) {
      int c = (int)((unsigned char)(*p));

      if (c >= 32 && c <= 127 && isAcceptable[c - 32]) {
          *q++ = *p;
      } else {
          *q++ = '%';
          *q++ = HT_HEX(c / 16);
          *q++ = HT_HEX(c % 16);
      }
  }
  
  *q = 0;
  
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

PUBLIC char from_hex ARGS1(char, c)
{
    return  (c >= '0') && (c <= '9') ? (c - '0') :
    	    (c >= 'A') && (c <= 'F') ? (c - 'A' + 10) :
    	    (c >= 'a') && (c <= 'f') ? (c - 'a' + 10) :  /* Accept lowercase */
	    0;
}

PUBLIC char *HTUnEscape ARGS1(char *, str)
{
    char *p = str;
    char *q = str;

    while(*p) {
        if (*p == HEX_ESCAPE) {
	    p++;
	    if (*p)
		*q = from_hex(*p++) * 16;
	    if (*p)
		*q = (*q + from_hex(*p++));
	    q++;
        } else if (*p == '+') {
            p++;
            *q++ = ' ';
	} else {
	    *q++ = *p++; 
	}
    }
    
    *q++ = 0;
    return str;
    
}
