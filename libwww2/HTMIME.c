/*			MIME Message Parse			HTMIME.c
**			==================
**
**	This is RFC 1341-specific code.
**	The input stream pushed into this parser is assumed to be
**	stripped on CRs, i.e., lines end with LF, not CR LF.
**	(It is easy to change this except for the body part where
**	conversion can be slow.)
**
** History:
**	   Feb 92	Written Tim Berners-Lee, CERN
**
*/
#include "../config.h"
#include "HTMIME.h"		/* Implemented here */
#include "HTAlert.h"
#include "HTFile.h"
#include "HTCookie.h"
#include "tcp.h"

#if defined(KRB4) || defined(KRB5)
#define HAVE_KERBEROS
#endif

#include "HTAAUtil.h"
extern int securityType;

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/* Export to GUI */
MIMEInfo MIME_http = { NULL, NULL, NULL, NULL };

/*		MIME Object
**		-----------
*/
typedef enum _MIME_state {
  BEGINNING_OF_LINE,
  CONTENT_,
  CONTENT_T,
  CONTENT_TRANSFER_ENCODING,
  CONTENT_TYPE,
  CONTENT_ENCODING,
  CONTENT_LENGTH,
  EXPIRES,
  E,
  EX,
  L,
  LOCATION,
  LAST_MODIFIED,
  EXTENSION,
  R,
  REFRESH,
  SET_COOKIE,
  SET_COOKIE1,
  SET_COOKIE2,
  SKIP_GET_VALUE,		/* Skip space then get value */
  GET_VALUE,		        /* Get value till white space */
  JUNK_LINE,		        /* Ignore the rest of this folded line */
  NEWLINE,		        /* Just found a LF ... maybe continuation */
  CHECK,			/* Check against check_pointer */
  MIME_TRANSPARENT,	        /* Put straight through to target ASAP! */
  MIME_IGNORE		        /* Ignore entire file */
  /* TRANSPARENT and IGNORE are defined as stg else in _WINDOWS */
#ifdef HAVE_KERBEROS
  ,WWW_AUTHENTICATE             /* For kerberos mutual authentication */
#endif
} MIME_state;

#define VALUE_SIZE 8192		/* @@@@@@@ Arbitrary? */

struct _HTStream {
  WWW_CONST HTStreamClass *isa;
  MIME_state		state;		/* Current state */
  MIME_state		if_ok;		/* Got this state if match */
  MIME_state		field;		/* Remember which field */
  MIME_state		fold_state;	/* State on a fold */
  WWW_CONST char       *check_pointer;	/* Checking input */
  char		       *value_pointer;	/* Storing values */
  char 			value[VALUE_SIZE];
  HTParentAnchor       *anchor;		/* Given on creation */
  HTStream	       *sink;		/* Given on creation */
  char		       *set_cookie;     /* Set-Cookie */
  char		       *set_cookie2;    /* Set-Cookie2 */
  HTFormat		encoding;	/* Content-Transfer-Encoding */
  char		       *compression_encoding;
  int                   content_length;
  int                   header_length;  /* For IO accounting -bjs */
  HTFormat		format;		/* Content-Type */
  HTStream 	       *target;		/* While writing out */
  HTStreamClass		targetClass;
  HTAtom	       *targetRep;	/* Converting into? */
  char		       *location;
  char		       *expires;
  char		       *last_modified;
  char		       *refresh;
  char		       *charset;
  int interrupted;
};


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
**
**	This is an FSM parser which is tolerant as it can be of all
**	syntax errors.  It ignores field names it does not understand,
**	and resynchronises on line beginnings.
*/

PRIVATE void HTMIME_put_character (HTStream *me, char c)
{
  char ori_c = c;
#ifdef HAVE_KERBEROS
  static int got_kerb = 0;
  static HTAAScheme kscheme;
  extern int validate_kerberos_server_auth();
#endif

  if (me->state == MIME_TRANSPARENT) {
      (*me->targetClass.put_character)(me->target, c);    /* MUST BE FAST */
      return;
  } else {
      me->header_length++;  /* Update this first */
  }

  /* Strip CRs */
  if (c == '\r')
      return;

  c = tolower(c);

  switch(me->state) {
    case MIME_IGNORE:
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "[MIME] Got MIME_IGNORE; returning...\n");
#endif
      return;
      
    case NEWLINE:
      if (c != '\n' && WHITE(ori_c)) {
          /* Folded line */
          me->state = me->fold_state;	/* Pop state before newline */
          break;
      }
      /* Else falls through */
      
    case BEGINNING_OF_LINE:
      switch(c) {
        case 'c':
          me->check_pointer = "ontent-";
          me->if_ok = CONTENT_;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] C at BOL; check for 'ontent-'\n");
#endif
          break;

	case 'e':
	  me->state = E;
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] E at BOL; check for 'X'\n");
#endif
	  break;

        case 'l':
          me->state = L;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] L at BOL\n");
#endif
          break;

        case 'r':
          me->state = R;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] R at BOL\n");
#endif
          break;

        case 's':
          me->check_pointer = "et-cookie";
          me->if_ok = SET_COOKIE;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] S at BOL; check for 'et-cookie'\n");
#endif
          break;

#ifdef HAVE_KERBEROS
          /*  For kerberos mutual authentication  */
        case 'w':
          me->check_pointer = "ww-authenticate:";
          me->if_ok = WWW_AUTHENTICATE;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] W at BOL; check for 'ww-authenticate'\n");
#endif
          break;
#endif

        case '\n': {			/* Blank line: End of Header! */
            int compressed = COMPRESSED_NOT;

#ifndef DISABLE_TRACE
            if (www2Trace) {
                fprintf(stderr,
                   "[MIME] STREAMSTACK: content type is %s, converting to %s\n",
                   HTAtom_name(me->format), HTAtom_name(me->targetRep));
                fprintf(stderr,
                      "                    Compression encoding '%s'\n",
                      (!me->compression_encoding || !*me->compression_encoding ?
		      "Undefined" : me->compression_encoding));
	    }
#endif
            if (me->compression_encoding) {
                if (!strcmp(me->compression_encoding, "x-compress")) {
                    compressed = COMPRESSED_BIGZ;
                } else if (!strcmp(me->compression_encoding, "x-gzip")) {
                    compressed = COMPRESSED_GNUZIP;
#ifndef DISABLE_TRACE
                } else if (www2Trace) {
                    fprintf(stderr,
			    "[MIME] Unknown compression_encoding '%s'\n",
                            me->compression_encoding);
#endif
                }
            }

#ifndef DISABLE_TRACE
            if (www2Trace)
                fprintf(stderr, "[MIME] compressed == %d\n", compressed);
#endif
	    if (me->set_cookie || me->set_cookie2) {
		HTSetCookie(me->set_cookie, me->set_cookie2,
			    me->anchor->address);
		if (me->set_cookie)
		    free(me->set_cookie);
		if (me->set_cookie2)
		    free(me->set_cookie2);
	    }

            me->target = HTStreamStack(me->format, me->targetRep, compressed,
                                       me->sink, me->anchor);
            if (!me->target) {
#ifndef DISABLE_TRACE
                if (www2Trace) {
                    fprintf(stderr, "[MIME] Can't translate! **\n");
                    fprintf(stderr, "       Defaulting to HTML.\n");
                }
#endif
                /* Default to HTML. */
                me->target = HTStreamStack(HTAtom_for("text/html"),
                                           me->targetRep, compressed,
                                           me->sink, me->anchor);
            }
            if (me->target) {
                me->targetClass = *me->target->isa;
		/* Check for encoding and select state from there @@ */
                /* From now push straight through */
#ifndef DISABLE_TRACE
                if (www2Trace)
                    fprintf (stderr, "[MIME] Entering MIME_TRANSPARENT\n");
#endif
                me->state = MIME_TRANSPARENT; 
		/* Header is now completely read */
            } else {
                /* This is HIGHLY EVIL -- the browser WILL BREAK
                 * if it ever reaches here.  Thus the default to
                 * HTML above, which should always happen... */
#ifndef DISABLE_TRACE
                if (www2Trace) 
                    fprintf(stderr, "[MIME] HIT HIGHLY EVIL!!! ***\n");
#endif
                me->state = MIME_IGNORE;		/* What else to do? */
            }
            break;
        }
	default:
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] No valid field at BOL\n");
#endif
          goto bad_field_name;
          
      }  /* Switch on character */
      break;
      
    case CHECK:				/* Check against string */
      if (c == *(me->check_pointer)++) {
          if (!*me->check_pointer) 
              me->state = me->if_ok;
      } else {		/* Error */
#ifndef DISABLE_TRACE
          if (www2Trace) 
              fprintf(stderr,
                      "[MIME] Bad character `%c' found where `%s' expected\n",
                      ori_c, me->check_pointer - 1);
#endif
          goto bad_field_name;
      }
      break;

    case CONTENT_:
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "[MIME] in case CONTENT_\n");
#endif
      switch(c) {
	case 't':
          me->state = CONTENT_T;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] Found T, state now CONTENT_T\n");
#endif
          break;
          
	case 'e':
          me->check_pointer = "ncoding:";
          me->if_ok = CONTENT_ENCODING;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] Found E, check for 'ncoding:'\n");
#endif
          break;
          
	case 'l':
          me->check_pointer = "ength:";
          me->if_ok = CONTENT_LENGTH;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] Found L, check for 'ength:'\n");
#endif
          break;
          
	default:
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] Found nothing; bleah\n");
#endif
          goto bad_field_name;
      }
      break;
      
    case CONTENT_T:
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "[MIME] in case CONTENT_T\n");
#endif
      switch(c) {
	case 'r':
          me->check_pointer = "ansfer-encoding:";
          me->if_ok = CONTENT_TRANSFER_ENCODING;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] Found R; check for ansfer-encoding:\n");
#endif
          break;
          
	case 'y':
          me->check_pointer = "pe:";
          me->if_ok = CONTENT_TYPE;
          me->state = CHECK;
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] Found Y; check for pe:\n");
#endif
          break;
          
	default:
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[MIME] Found nothing; bleah\n");
#endif
          goto bad_field_name;
      }
      break;

    case L:
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "[MIME] in case L\n");
#endif
      switch(c) {
	case 'a':
	  me->check_pointer = "st-modified:";
	  me->if_ok = LAST_MODIFIED;
	  me->state = CHECK;
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Is LA; check for st-modified:\n");
#endif
	  break;

	case 'o':
	  me->check_pointer = "cation:";
	  me->if_ok = LOCATION;
	  me->state = CHECK;
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Is LO; check for cation:\n");
#endif
	  break;

	default:
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Found nothing; bleah\n");
#endif
	  goto bad_field_name;
      }
      break;

    case R:
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "[MIME] in case R\n");
#endif
      switch(c) {
	case 'e':
	  me->check_pointer = "fresh:";
	  me->if_ok = REFRESH;
	  me->state = CHECK;
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Is RE; check for fresh:\n");
#endif
	  break;

	default:
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Found nothing; bleah\n");
#endif
	      goto bad_field_name;
      }
      break;

    case E:
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "[MIME] in case E\n");
#endif
      switch(c) {
	case 'x':
	  me->state = EX;
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Is EX; check for EXP or EXT:\n");
#endif
	  break;

	default:
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Found nothing; bleah\n");
#endif
	  goto bad_field_name;
      }
      break;

    case EX:
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "[MIME] in case EX\n");
#endif
      switch(c) {
	case 'p':
	  me->check_pointer = "ires";
	  me->if_ok = EXPIRES;
	  me->state = CHECK;
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Is EXP; check for 'ires'\n");
#endif
	  break;

	case 't':
	  me->check_pointer = "ension:";
	  me->if_ok = EXTENSION;
	  me->state = CHECK;
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Is EXT; check for 'ension:'\n");
#endif
	  break;

	default:
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] Was EX; found nothing\n");
#endif
	  goto bad_field_name;
      }
      break;

    case SET_COOKIE:			/* Check for ':' or '2' */
      switch (c) {
	case ':':
	  me->field = SET_COOKIE1;		/* Remember it */
	  me->state = SKIP_GET_VALUE;
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] SET_COOKIE, found :\n");
#endif
	  break;

	case '2':
	  me->check_pointer = ":";
	  me->if_ok = SET_COOKIE2;
	  me->state = CHECK;
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr, "[MIME] SET_COOKIE, found 2, check for ':'\n");
#endif
	  break;

	default:
#ifndef DISABLE_TRACE
	  if (www2Trace)
	      fprintf(stderr,
		      "[MIME] Found `%c' where ':' or '2' expected\n", ori_c);
#endif
	  goto bad_field_name;
      }
      break;

#ifdef HAVE_KERBEROS
    case WWW_AUTHENTICATE:
#endif
    case EXTENSION:
    case CONTENT_TYPE:
    case CONTENT_TRANSFER_ENCODING:
    case CONTENT_ENCODING:
    case CONTENT_LENGTH:
    case LOCATION:
    case EXPIRES:
    case REFRESH:
    case SET_COOKIE1:
    case SET_COOKIE2:
    case LAST_MODIFIED:
      me->field = me->state;		/* Remember it */
      me->state = SKIP_GET_VALUE;
      /* Fall through! (no break!) */
    case SKIP_GET_VALUE:
      if (c == '\n') {
          me->fold_state = me->state;
          me->state = NEWLINE;
          break;
      }
      if (WHITE(ori_c)) 
          break;	/* Skip white space */
      
      me->value_pointer = me->value;
      me->state = GET_VALUE;   
      /* Fall through to store first character */
      
    case GET_VALUE:
      if (WHITE(ori_c) && (c != ' ')) {	/* End of field */
	  char *cp;

          *me->value_pointer = '\0';
	  cp = me->value_pointer - 1;
	  /* Trim trailing white space */
	  while ((cp >= me->value) && WHITE(*cp))
	      *cp-- = '\0';

          switch (me->field) {
            case CONTENT_TYPE:
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, "[MIME] Got content-type '%s'\n", me->value);
#endif
              /* Get up to ';' and lowercase it */
              for (cp = me->value; *cp; cp++) {
                  *cp = TOLOWER(*cp);
		  if (*cp == ';') {
		      *cp++ = '\0';
#ifndef DISABLE_TRACE
                      if (www2Trace)
		          fprintf(stderr, "Removed ';' from '%s'\n", me->value);
#endif
		      break;
		  }
	      }
              me->format = HTAtom_for(me->value);
#ifndef DISABLE_TRACE
              if (www2Trace) {
                  fprintf(stderr, "[MIME] Lowercased to '%s'\n", me->value);
                  fprintf(stderr, "Got content-type atom 0x%08x\n", me->format);
	      }
#endif
	      /* Get charset */
	      while (*cp) {
		 if ((TOLOWER(*cp) == 'c') &&
		     !my_strncasecmp(cp, "charset=", 8)) {
		     if (me->charset)
			 free(me->charset);
		     me->charset = strdup(cp + 8);
#ifndef DISABLE_TRACE
		     if (www2Trace)
			 fprintf(stderr, "[MIME] Got charset '%s'\n",
				 me->charset);
#endif
		     break;
		 }
		 cp++;
	      }
              break;

	    case CONTENT_TRANSFER_ENCODING:
              me->encoding = HTAtom_for(me->value);
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, "[MIME] Got transfer_encoding 0x%08x\n",
                          me->encoding);
#endif
              break;

            case CONTENT_ENCODING:
              me->compression_encoding = strdup(me->value);
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, "[MIME] Got compression encoding '%s'\n", 
                          me->compression_encoding);
#endif
              break;

            case CONTENT_LENGTH:
              me->content_length = atoi(me->value);
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, "[MIME] Got content length '%d'\n", 
                          me->content_length);
#endif
              break;

            case EXPIRES:
	      if (me->expires)
		  free(me->expires);
	      me->expires = strdup(me->value);
#ifndef DISABLE_TRACE
	      if (www2Trace)
		  fprintf(stderr, "[MIME] Got expires '%s'\n", me->value);
#endif
	      break;

            case LAST_MODIFIED:
	      if (me->last_modified)
		  free(me->last_modified);
	      me->last_modified = strdup(me->value);
#ifndef DISABLE_TRACE
	      if (www2Trace)
		  fprintf(stderr, "[MIME] Got last modified '%s'\n", me->value);
#endif
	      break;

            case LOCATION:
	      me->location = me->value;
#ifndef DISABLE_TRACE
	      if (www2Trace)
		  fprintf(stderr, "[MIME] Got location '%s'\n", me->location);
#endif
	      break;

            case REFRESH:
	      if (me->refresh)
		  free(me->refresh);
	      me->refresh = strdup(me->value);
#ifndef DISABLE_TRACE
	      if (www2Trace)
		  fprintf(stderr, "[MIME] Got refresh '%s'\n", me->refresh);
#endif
	      break;

	    case SET_COOKIE1:
#ifndef DISABLE_TRACE
	      if (www2Trace)
		  fprintf(stderr, "[MIME] Set-Cookie: '%s'\n", me->value);
#endif
	      if (me->set_cookie == NULL) {
		  StrAllocCopy(me->set_cookie, me->value);
	      } else {
		  StrAllocCat(me->set_cookie, ", ");
		  StrAllocCat(me->set_cookie, me->value);
	      }
	      break;

	    case SET_COOKIE2:
#ifndef DISABLE_TRACE
	      if (www2Trace)
		  fprintf(stderr, "[MIME] Set-Cookie2: '%s'\n", me->value);
#endif
	      if (me->set_cookie2 == NULL) {
		  StrAllocCopy(me->set_cookie2, me->value);
	      } else {
		  StrAllocCat(me->set_cookie2, ", ");
		  StrAllocCat(me->set_cookie2, me->value);
	      }
	      break;

#ifdef HAVE_KERBEROS
            case WWW_AUTHENTICATE:
                /*
                 * msg from server looks like:
                 * WWW-Authenticate: KerberosV4 [strified ktext]
                 * also allowed: KerberosV5, KerbV4-Encrypted, KerbV5-Encrypted
                 *
                 * This code is ugly: we have to keep this got_kerb static
		 * around because the FSM isn't really designed to have fields
		 * with values that include whitespace.  got_kerb tells us that
		 * we've been in this code before, and that we saw the word
		 * "kerberos"
                 */
#ifndef DISABLE_TRACE
                if (www2Trace)
		    fprintf(stderr, "[MIME] picked up Auth. arg '%s'\n",
                            me->value);
#endif
                if (got_kerb) {
                    validate_kerberos_server_auth(kscheme, me->value);
                    got_kerb = 0;       /* Reset kerb state */
                    me->state = me->field;
		} else if (!my_strncasecmp(me->value, "kerb", 4)) {
                    if (0) {    	/* Just to get things started */
		    }
#ifdef KRB4
                    else if (!my_strncasecmp(me->value, "KerberosV4", 10)) {
                        kscheme = HTAA_KERBEROS_V4;
                        got_kerb = 1;
                        me->state = SKIP_GET_VALUE;
		    }
#endif
#ifdef KRB5
                    else if (!my_strncasecmp(me->value, "KerberosV5", 10)) {
                        kscheme = HTAA_KERBEROS_V5;
                        got_kerb = 1;
                        me->state = SKIP_GET_VALUE;
		    }
#endif
                    else {
                        fprintf(stderr,
			     "Unrecognized field in WWW-Authenticate header\n");
                       	     me->state = me->field;
		    }
		}
                break;
#endif

            case EXTENSION:
#ifndef DISABLE_TRACE
	      if (www2Trace)
		  fprintf(stderr, "[MIME] Got Extension value '%s'\n",
			  me->value);
#endif
	      /* Lowercase it. */
	      {
                  char *tmp;

		  for (tmp = me->value; *tmp; tmp++)
		      *tmp = TOLOWER(*tmp);
	      }
#ifndef DISABLE_TRACE
	      if (www2Trace)
		  fprintf(stderr, "[MIME] Lowercased to '%s'\n", me->value);
#endif
	      switch (*me->value) {
		case 'd':  /* Domain */
		  if (!strcmp(me->value, "domain-restricted")) {
		      securityType = HTAA_DOMAIN;
#ifndef DISABLE_TRACE
		      if (www2Trace)
			  fprintf(stderr,
			         "[MIME] Domain restrict extension found.\n");
#endif
		      break;
		  }
		  /* Fall through */
		default:  /* Unknown */
#ifndef DISABLE_TRACE
		  if (www2Trace)
		      fprintf(stderr, "[MIME] Unknown extension header: '%s'\n",
			      me->value);
#endif
		  me->state = me->field;
		  break;
	      }
	      break;

	    default:		/* Should never get here */
              break;
	  }
      } else {
          if (me->value_pointer < (me->value + VALUE_SIZE - 1)) {
              *me->value_pointer++ = ori_c;
              break;
          } else {
              goto value_too_long;
	  }
      }
      /* Fall through */
      
    case JUNK_LINE:
      if (c == '\n') {
          me->state = NEWLINE;
          me->fold_state = me->state;
      }
      break;
      
  }  /* Switch on state */
  
  return;
  
 value_too_long:
#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "[MIME] *** Syntax error. (string too long)\n");
#endif
  
 bad_field_name:				/* Ignore it */
  me->state = JUNK_LINE;
  return;
}


/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/
PRIVATE void HTMIME_put_string (HTStream *me, WWW_CONST char *s)
{
#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "[HTMIME_put_string] Putting '%s'\n", s);
#endif

  if (me->state == MIME_TRANSPARENT) {		/* Optimisation */
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "via transparent put_string\n");
#endif
      (*me->targetClass.put_string)(me->target, s);

  } else if (me->state != MIME_IGNORE) {
      WWW_CONST char *p;

#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "via char-by-char put_character\n");
#endif
      for (p = s; *p; )
          HTMIME_put_character(me, *p++);

#ifndef DISABLE_TRACE
  } else if (www2Trace) {
      fprintf(stderr, "DOING NOTHING!\n");
#endif
  }
  return;
}


/*	Buffer write.  Buffers can (and should!) be big.
**	------------
*/
PRIVATE void HTMIME_write (HTStream *me, WWW_CONST char *s, int l)
{
#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "[HTMIME_write] Putting %d bytes\n", l);
#endif

  if (me->state == MIME_TRANSPARENT) {		/* Optimisation */
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "via transparent put_block\n");
#endif
      (*me->targetClass.put_block)(me->target, s, l);

  } else if (me->state != MIME_IGNORE) {
      WWW_CONST char *p;

#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "via char-by-char put_character\n");
#endif
      for (p = s; p < (s + l); )
          HTMIME_put_character(me, *p++);

#ifndef DISABLE_TRACE
  } else if (www2Trace) {
      fprintf(stderr, "DOING NOTHING!\n");
#endif
  }
  return;
}


/*	Free an HTML object
**	-------------------
**
*/
PRIVATE void HTMIME_free (HTStream *me)
{
  if (!me->target && !me->interrupted) {
#ifndef DISABLE_TRACE
      if (www2Trace) {
          fprintf(stderr, "[HTMIME_free] No target!\n");
          fprintf(stderr, "  me 0x%08x, me->target 0x%08x\n", me, me->target);
      }
#endif
      me->format = HTAtom_for("text/html");
      me->target = HTStreamStack(me->format, me->targetRep, 0,
                                 me->sink, me->anchor);
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "  me->target->isa 0x%08x\n", me->target->isa);
#endif
      me->targetClass = *me->target->isa;

      /* VAX C compiler cannot deal with the orginal line at all, GEC */
      (*me->targetClass.put_string) (me->target, 
      "<H1>ERROR IN HTTP/1.0 RESPONSE</H1> The remote server returned a HTTP/1.0 response that Mosaic's MIME parser could not understand.  Please contact the server maintainer.<P> Sorry for the inconvenience,<P> <ADDRESS>The Management</ADDRESS>");
      /* BSN put an extra line feed here for VAXC's limited record length */

      securityType = HTAA_UNKNOWN;
  } 
  if (me->target) 
      (*me->targetClass.free)(me->target);
      
  if (me->expires) {
      if (MIME_http.expires)
          free(MIME_http.expires);
      MIME_http.expires = me->expires;
  }
  if (me->last_modified) {
      if (MIME_http.last_modified)
          free(MIME_http.last_modified);
      MIME_http.last_modified = me->last_modified;
  }
  if (me->refresh) {
      if (MIME_http.refresh)
          free(MIME_http.refresh);
      MIME_http.refresh = me->refresh;
  }
  if (me->charset) {
      if (MIME_http.charset)
          free(MIME_http.charset);
      MIME_http.charset = me->charset;
  }

  free(me);

  return;
}

/*	End writing
*/

PRIVATE void HTMIME_end_document (HTStream *me)
{
  if (me->target) 
      (*me->targetClass.end_document)(me->target);
}

PRIVATE void HTMIME_handle_interrupt (HTStream *me)
{
  me->interrupted = 1;

  /* Propagate interrupt message down, then free down */
  if (me->target) {
      (*me->targetClass.handle_interrupt)(me->target);
      (*me->targetClass.free)(me->target);
      me->target = NULL;
  }
  return;
}


/*	Structured Object Class
**	-----------------------
*/
PUBLIC WWW_CONST HTStreamClass HTMIME = {		
  "MIMEParser",
  HTMIME_free,
  HTMIME_end_document,
  HTMIME_put_character, 	HTMIME_put_string,
  HTMIME_write,
  HTMIME_handle_interrupt
}; 


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStream *HTMIMEConvert (HTPresentation *pres,
				HTParentAnchor *anchor,
				HTStream *sink,
    				HTFormat format_in,
        			int compressed)
{
    HTStream *me;
    static HTAtom *www_plaintext;
    static int init = 0;

    if (!init) {
	www_plaintext = WWW_PLAINTEXT;
	init = 1;
    }
    me = calloc(1, sizeof(*me));
    if (!me)
        outofmem(__FILE__, "HTMIMEConvert");
#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "[HTMIMEConvert] HELLO!\n");
#endif

    me->isa = &HTMIME;       
    me->sink = sink;
    me->anchor = anchor;
    me->state = BEGINNING_OF_LINE;
    me->format = www_plaintext;
    me->targetRep = pres->rep_out;
    me->content_length = -1;

    /** calloc zeros
    me->target = NULL;
    me->set_cookie = NULL;
    me->set_cookie2 = NULL;
    me->location = NULL;
    me->interrupted = 0;
    me->encoding = 0;
    me->compression_encoding = NULL;
    me->header_length = 0;      * To allow differentiation between
			        * content and header for read length
    me->expires = NULL;
    me->last_modified = NULL;
    me->refresh = NULL;
    me->charset = NULL;
    **/
    return me;
}

int HTMIME_get_header_length(HTStream *me)
{
    if (me->isa != &HTMIME)
	return 0;  /* In case we screw up */
    return me->header_length;
}

int HTMIME_get_content_length(HTStream *me)
{
    if (me->isa != &HTMIME)
	return -1;  /* In case we screw up */
    return me->content_length;
}

Boolean HTMIME_header_done(HTStream *me)
{
    if (me->state == MIME_TRANSPARENT)
	return True;
    return False;
}
