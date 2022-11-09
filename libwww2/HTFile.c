/*			File Access				HTFile.c
**			===========
**
**	This is unix-specific code in general, with some VMS bits.
**	These are routines for file access used by browsers.
**
** History:
**	   Feb 91	Written Tim Berners-Lee CERN/CN
**	   Apr 91	vms-vms access included using DECnet syntax
**	26 Jun 92 (JFG) When running over DECnet, suppressed FTP.
**			Fixed access bug for relative names on VMS.
**
** Bugs:
**	FTP: Cannot access VMS files from a unix machine.
**      How can we know that the target machine runs VMS?
*/

/*
 * VMS note: The handling of VMS-like filenames below is in my
 * opinion rather messy.  A full rewrite is called for. /BSN
 */
#include "../config.h"
#include "HTFile.h"		/* Implemented here */

#define INFINITY 512		/* File name length @@ FIXME */
#define MULTI_SUFFIX ".multi"   /* Extension for scanning formats */

#include <stdio.h>
#ifndef VMS
#include <sys/param.h>
#endif /* VMS, BSN */
#include "HText.h"

#include "HTParse.h"
#include "HTTCP.h"
#include "HTFTP.h"
#include "HTAnchor.h"
#include "HTAtom.h"
#include "HTWriter.h"
#include "HTFWriter.h"
#include "HTInit.h"
#include "HTIcon.h"
#include "HTSort.h"

#ifdef __GNUC__
#include <stat.h>
#endif /* GNU C, probably should do in tcp.h */

#include "HTVMSUtils.h"
#include "../libnut/system.h"

/* Define parameter that everyone should already have */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

typedef struct _HTSuffix {
	char   *suffix;
	HTAtom *rep;
	HTAtom *encoding;
	float	quality;
} HTSuffix;

#ifdef USE_DIRENT		/* Set this for Sys V systems */
#define STRUCT_DIRENT struct dirent
#else
#define STRUCT_DIRENT struct direct
#endif

#include "HTML.h"		/* For directory object building */

#define PUTC(c) (*target->isa->put_character)(target, c)
#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)

struct _HTStructured {
	WWW_CONST HTStructuredClass *isa;
	/* ... */
};

/* For Solaris, etc. */
#ifndef NGROUPS
#define NGROUPS 64
#endif

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/*                   Controlling globals
**
*/
PUBLIC int HTDirAccess = HT_DIR_OK;
PUBLIC int HTDirReadme = HT_DIR_README_TOP;

#ifdef vms
PRIVATE char *HTCacheRoot = "/WWW$SCRATCH/";    /* Where to cache things */
#else
PRIVATE char *HTCacheRoot = "/tmp/W3_Cache_";   /* Where to cache things */
#endif

/* Passive mode FTP support */
extern BOOL ftp_passive;

#ifdef VMS
#define JPI$_PAGFILCNT 1044
#define JPI$_PGFLQUOTA 1038
void outofmem(WWW_CONST char *fname, WWW_CONST char *func)
{
  int status, pagfilcnt, pgflquota;
  int jpi_count = JPI$_PAGFILCNT;
  int jpi_quota = JPI$_PGFLQUOTA;

  status = lib$getjpi((void *) &jpi_count, 0, 0, (void *) &pagfilcnt, 0, 0);
  status = lib$getjpi((void *) &jpi_quota, 0, 0, (void *) &pgflquota, 0, 0);
  fprintf(stderr, "Remaining page file quota %d, maximum virtual size %d.\n",
	  pagfilcnt, pgflquota);
  fprintf(stderr,
	  "Ask your system manager to have your page file quota increased.\n");
  fprintf(stderr, "%s %s: out of memory.\nProgram aborted.\n", fname, func);
  exit(1);
}
#endif /* VMS, BSN, GEC */

/*	Suffix registration
*/
PUBLIC HTList *HTSuffixes = NULL;
PRIVATE HTSuffix no_suffix = { "*", NULL, NULL, 1.0 };
PRIVATE HTSuffix unknown_suffix = { "*.*", NULL, NULL, 1.0 };

/*	Define the representation associated with a file suffix
**	-------------------------------------------------------
**
**	Calling this with suffix set to "*" will set the default
**	representation.
**	Calling this with suffix set to "*.*" will set the default
**	representation for unknown suffix files which contain a ".".
*/
PUBLIC void HTSetSuffix (WWW_CONST char *suffix, WWW_CONST char *representation,
			 WWW_CONST char *encoding, float quality)
{
    HTSuffix *suff;
    static HTAtom *binary;
    static int init = 0;

    if (!init) {
	binary = HTAtom_for("binary");
	init = 1;
    }
    
    if (!strcmp(suffix, "*")) {
	suff = &no_suffix;
    } else if (!strcmp(suffix, "*.*")) {
	suff = &unknown_suffix;
    } else {
	suff = (HTSuffix *) calloc(1, sizeof(HTSuffix));
	if (!suff)
	    outofmem(__FILE__, "HTSetSuffix");
	if (!HTSuffixes)
	    HTSuffixes = HTList_new();
	HTList_addObject(HTSuffixes, suff);
	
	StrAllocCopy(suff->suffix, suffix);
    }

    suff->rep = HTAtom_for(representation);
    
    if (!strcmp(encoding, "binary")) {
	suff->encoding = binary;
    } else {
    	char *enc = NULL;
	char *p;

	StrAllocCopy(enc, encoding);
	for (p = enc; *p; p++)
	    *p = TOLOWER(*p);
	suff->encoding = HTAtom_for(enc);
        free(enc);
    }
    
    suff->quality = quality;
}


/*	Version stripping for VMS Filenames
**	-----------------------------------
**
** On entry,
**	filename	Filename, possibly including version number
** On exit,
**			filename parameter is modified
**
** Author: David Lewis (lewis@axp.mits.com.au) 6 June, 1996
**	   (Code dragged from VMS port, V2.6-1)
*/
void strip_VMS_version (char *filename)
{
    /* FTP Server is VMS */
    char *ptr;

    /* Remove the version number if necessary */
    ptr = strchr(filename, ';');

    /*
     * If we didn't find a semi-colon, then if the rightmost and
     * leftmost '.'s are not in the same location we take the position
     * of the rightmost dot to be by the version number.
     */
    if (!ptr && (strchr(filename, '.') != strrchr(filename, '.')))
        ptr = strrchr(filename, '.');

    if (ptr)
        *ptr = '\0';
}


/*	Send README file
**
**  If a README file exists, then it is inserted into the document here.
*/

#ifdef GOT_READ_DIR
PRIVATE void do_readme (HTStructured *target, WWW_CONST char *localname)
{ 
    FILE *fp;
    char *readme_file_name = malloc(strlen(localname) + 1 +
				    strlen(HT_DIR_README_FILE) + 1);

    strcpy(readme_file_name, localname);
    strcat(readme_file_name, "/");
    strcat(readme_file_name, HT_DIR_README_FILE);
    
    fp = fopen(readme_file_name, "r");
    
    if (fp) {
	HTStructuredClass targetClass;
	
	targetClass = *target->isa;	/* (Can't init agregate in K&R) */
	START(HTML_PRE);
	for (;;) {
	    char c = fgetc(fp);

	    if (c == (char)EOF)
		break;
	    switch (c) {
	    	case '&':
		case '<':
		case '>':
		    PUTC('&');
		    PUTC('#');
		    PUTC((char)(c / 10));
		    PUTC((char)(c % 10));
		    PUTC(';');
		    break;
		/*
	    	case '\n':
		    PUTC('\r');    
		Bug removed thanks to joe@athena.mit.edu
		*/
		default:
		    PUTC(c);
	    }
	}
	END(HTML_PRE);
	fclose(fp);
    } 
}
#endif


/*	Make the cache file name for a W3 document
**	------------------------------------------
**	Make up a suitable name for saving the node in
**
**	E.g.	/tmp/WWW_Cache_news/1234@cernvax.cern.ch
**		/tmp/WWW_Cache_http/crnvmc/FIND/xx.xxx.xx
**
** On exit,
**	returns	a malloc'ed string which must be freed by the caller.
*/
PUBLIC char *HTCacheFileName (WWW_CONST char *name)
{
    char *access = HTParse(name, "", PARSE_ACCESS);
    char *host = HTParse(name, "", PARSE_HOST);
    char *path = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);
    char *result;
    
    result = (char *)malloc(strlen(HTCacheRoot) + strlen(access) +
			    strlen(host) + strlen(path) + 6 + 1);
    if (!result)
	outofmem(__FILE__, "HTCacheFileName");
    sprintf(result, "%s/WWW/%s/%s%s", HTCacheRoot, access, host, path);
    free(path);
    free(access);
    free(host);
    return result;
}


/*	Open a file for write, creating the path
**	----------------------------------------
*/
#ifdef NOT_IMPLEMENTED
PRIVATE int HTCreatePath (WWW_CONST char *path)
{
    return -1;
}
#endif

/*	Convert filenames between local and WWW formats
**	-----------------------------------------------
** On exit,
**	returns	a malloc'ed string which must be freed by the caller.
*/
PUBLIC char *HTLocalName (WWW_CONST char *name)
{
  char *access = HTParse(name, "", PARSE_ACCESS);
  char *host = HTParse(name, "", PARSE_HOST);
  char *path = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);
  
  HTUnEscape(path);	/* Interpret % signs */
  
  if (!strcmp(access, "file")) {
      free(access);	
      if (!host || !*host || !my_strcasecmp(host, HTHostName()) ||
          !my_strcasecmp(host, "localhost")) {
          if (host)
              free(host);
#ifndef DISABLE_TRACE
          if (www2Trace) 
              fprintf(stderr, "Node `%s' means path `%s'\n", name, path);
#endif
          return(path);
      } else {
          free(host);
          if (path)
              free(path);
          return NULL;
      }
  }
  
  /* Not file */
  if (host)
      free(host);
  free(access);
  if (path)
      free(path);
  return NULL;
}


/*	Make a WWW name from a full local path name
**
** Bugs:
**	At present, only the names of two network root nodes are hand-coded
**	in and valid for the NeXT only.  This should be configurable in
**	the general case.
*/
PUBLIC char *WWW_nameOfFile (WWW_CONST char *name)
{
    char *result = (char *)malloc(7 + strlen(HTHostName()) + strlen(name) + 1);

    if (!result)
	outofmem(__FILE__, "WWW_nameOfFile");
    sprintf(result, "file://%s%s", HTHostName(), name);
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "File `%s'\n\tmeans node `%s'\n", name, result);
#endif
    return result;
}


/*	Determine a suitable suffix, given the representation
**	-----------------------------------------------------
**
** On entry,
**	rep	is the atomized MIME style representation
**
** On exit,
**	returns	a pointer to a suitable suffix string if one has been
**		found, else "".
*/
PUBLIC WWW_CONST char *HTFileSuffix (HTAtom *rep)
{
    HTSuffix *suff;
    int i, n;

    if (!HTSuffixes)
	HTFileInit();
    n = HTList_count(HTSuffixes);
    for (i = 0; i < n; i++) {
	suff = HTList_objectAt(HTSuffixes, i);
	if (suff->rep == rep)
	    return suff->suffix;		/* OK -- found */
    }
    return "";		/* Dunno */
}


/*	Determine file format from file name
**	------------------------------------
**
**	This version will return the representation and also set
**	a variable for the encoding.
**
**	It will handle for example  x.txt, x.txt.Z, x.Z
*/
PUBLIC HTFormat HTFileFormat (char *filename, HTAtom **pencoding,
                	      HTAtom *default_type, int *compressed)
{
  HTSuffix *suff;
  int n, i, lf;

  if (!filename)
      return NULL;

  /* Make a copy to hack and slash. */
  filename = strdup(filename);

  lf = strlen(filename);

  /* Step backward through filename, looking for '?'. */
  for (i = lf - 1; i >= 0; i--) {
      if (filename[i] == '?') {
          /* Clip query. */
          filename[i] = '\0';
          /* Get new strlen, since we just changed it. */
          lf = strlen(filename);
          goto ok_ready;
      }
  }

  *compressed = COMPRESSED_NOT;

  /* Check for .Z and .z, etc. */
  if (lf > 2) {
#ifndef VMS   /* Treat _Z the same as .Z, PGE */
      if (!strcmp(&filename[lf - 2], ".Z")) {
#else
      if (!strcmp(&filename[lf - 2], ".Z") ||
          !strcmp(&filename[lf - 2], "_z") ||
          !strcmp(&filename[lf - 2], "_Z")) {
#endif /* PGE */

          *compressed = COMPRESSED_BIGZ;
          filename[lf - 2] = '\0';
          lf = strlen(filename);
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[HTFileFormat] Got hit on .Z; filename '%s'\n",
                      filename);
#endif
      } else if (!strcmp(&filename[lf - 2], ".z")) {
          *compressed = COMPRESSED_GNUZIP;
          filename[lf - 2] = '\0';
          lf = strlen(filename);
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "[HTFileFormat] Got hit on .z; filename '%s'\n",
                      filename);
#endif
      } else if (lf > 3) {
#ifndef VMS
          if (!strcmp(&filename[lf - 3], ".gz")) {
#else
	  /*
	   * Treat -gz the same as .gz
	   */
          if (!strcmp(&filename[lf - 3], ".gz") ||
              !strcmp(&filename[lf - 3], "-gz") ||
              !strcmp(&filename[lf - 3], "-GZ")) {
#endif /* BSN */
              *compressed = COMPRESSED_GNUZIP;
              filename[lf - 3] = '\0';
              lf = strlen(filename);
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, 
                          "[HTFileFormat] Got hit on .gz; filename '%s'\n",
                          filename);
#endif
#ifndef VMS
          } else if (!strcmp(&filename[lf - 4], ".tgz")) {
#else
          } else if (!my_strcasecmp(&filename[lf - 4], ".tgz")) {
#endif
              *compressed = COMPRESSED_GNUZIP;
              filename[lf - 1] = 'r';
              filename[lf - 2] = 'a';
              lf = strlen(filename);
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, 
                          "[HTFileFormat] Got hit on .tgz; filename '%s'\n",
                          filename);
#endif
	  }
      }      
  }      
  
 ok_ready:
  if (!HTSuffixes) 
      HTFileInit();

  *pencoding = NULL;

  n = HTList_count(HTSuffixes);

  for (i = 0; i < n; i++) {
      int ls;

      suff = HTList_objectAt(HTSuffixes, i);
      ls = strlen(suff->suffix);
      if ((ls <= lf) && !my_strcasecmp(suff->suffix, filename + lf - ls)) {
          int j;

          *pencoding = suff->encoding;
          if (suff->rep) 
              goto done;
          
          for (j = 0; j < n; j++) {  /* Got encoding, need representation */
              int ls2;

              suff = HTList_objectAt(HTSuffixes, j);
              ls2 = strlen(suff->suffix);
              if ((ls <= lf) && 
                  !my_strncasecmp(suff->suffix, filename + lf - ls -ls2, ls2)) {
                  if (suff->rep) 
                      goto done;
	      }
	  }
      }
  }
  
  suff = strchr(filename, '.') ? 	/* Unknown suffix */
         (unknown_suffix.rep ? &unknown_suffix : &no_suffix) : &no_suffix;
  
  /* For now, assuming default is 8bit text/plain.
   * We also want default 8bit text/html for http connections. */
  
  /* Set default encoding unless found with suffix already */
  if (!*pencoding)
      *pencoding = suff->encoding ? suff->encoding : HTAtom_for("8bit");

 done:
  /* Free our copy. */
  free(filename);
  return suff->rep ? suff->rep : default_type;
}


/*	Determine file format from file name -- string version
**	------------------------------------------------------
*/
PUBLIC char *HTFileMimeType (WWW_CONST char *filename,
			     WWW_CONST char *default_type)
{
  HTAtom *pencoding;
  HTFormat format;
  int compressed;

  format = HTFileFormat(filename, &pencoding, HTAtom_for(default_type),
                        &compressed);
  if (HTAtom_name(format)) {
      return HTAtom_name(format);
  } else {
      return default_type;
  }
}

/* This doesn't do Gopher typing yet. */
/* This assumes we get a canonical URL and that HTParse works. */
char *HTDescribeURL(char *url)
{
  char line[512];
  char *type, *t, *host, *access;
  char *st = NULL;
  int i;

  if (!url || !*url)
      return "Completely content-free.";

  if (!strncmp("http:", url, 5)) {
      type = HTFileMimeType(url, "text/html");
  } else {
      type = HTFileMimeType(url, "text/plain");
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "DESCRIBE: type '%s'\n", type);
#endif

  t = strdup(type);
  for (i = 0; i < strlen(t); i++) {
      if (t[i] == '/') {
          t[i] = '\0';
          if (t[i + 1] && t[i + 1] != '*')
              st = &t[i + 1];
          break;
      }
  }

  access = HTParse(url, "", PARSE_ACCESS);
  if (!strcmp(access, "http")) {
      access[0] = 'H';
      access[1] = 'T';
      access[2] = 'T';
      access[3] = 'P';
  } else if (!strcmp(access, "ftp")) {
      access[0] = 'F';
      access[1] = 'T';
      access[2] = 'P';
  } else {
      access[0] = toupper(access[0]);
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "DESCRIBE: url '%s'\n", url);
#endif

  host = HTParse(url, "", PARSE_HOST);

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "DESCRIBE: host '%s'\n", host);
#endif

  if (st) {
      /* Uppercase type, to start sentence. */
      *t = toupper(*t);
      /* Crop x- from subtype. */
      if (*st == 'x' && st[1] == '-')
          st = &st[2];
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, 
                  "DESCRIBE: in if (st); pasting together %s %s %s %s %s\n", t,
                  !strcmp(t, "Application") ? " data" : "", 
                  st, host, access);
#endif
      sprintf(line, "%s%s, type %s, on host %s, via %s.", t,
              !strcmp(t, "Application") ? " data" : "", 
              st, host, access);
  } else {
      sprintf(line, "Type %s, on host %s, via %s.", type, host, access);
  }

  free(access);
  free(host);
  free(t);

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "DESCRIBE: returning '%s'\n", line);
#endif

  return strdup(line);
}


/*	Determine value from file name
**	------------------------------
**
*/
PUBLIC float HTFileValue (WWW_CONST char *filename)
{
    HTSuffix *suff;
    int i, n;
    int lf = strlen(filename);

    if (!HTSuffixes)
        HTFileInit();
    n = HTList_count(HTSuffixes);
    for (i = 0; i < n; i++) {
        int ls;

	suff = HTList_objectAt(HTSuffixes, i);
	ls = strlen(suff->suffix);
	if ((ls <= lf) && !strcmp(suff->suffix, filename + lf - ls)) {
#ifndef DISABLE_TRACE
	    if (www2Trace)
		fprintf(stderr, "File: Value of %s is %.3f\n",
			filename, suff->quality);
#endif
	    return suff->quality;		/* OK -- found */
	}
    }
    return 0.3;		/* Dunno! */
}


/*	Determine write access to a file
**	--------------------------------
**
** On exit,
**	return value	YES if file can be accessed and can be written to.
**
** Bugs:
**	1.	No code for non-unix systems.
**	2.	Isn't there a quicker way?
*/

#ifdef vms
#define NO_GROUPS
#endif
#ifdef NO_UNIX_IO
#define NO_GROUPS
#endif
#ifdef PCNFS
#define NO_GROUPS
#endif

PUBLIC BOOL HTEditable (WWW_CONST char *filename)
{
#ifdef NO_GROUPS
    return NO;		/* Safe answer till we find the correct algorithm */
#else
    int groups[NGROUPS];	
    uid_t myUid;
    int	ngroups;				/* The number of groups */
    int	i;
    struct stat	fileStatus;
        
    if (stat(filename, &fileStatus))		/* Get details of filename */
    	return NO;				/* Can't even access file! */

    /* The group stuff appears to be coming back garbage on IRIX... why? */
    ngroups = getgroups(NGROUPS, groups);	/* Groups to which I belong */
    myUid = geteuid();				/* Get my user identifier */

#ifndef DISABLE_TRACE
    if (www2Trace) {
	fprintf(stderr, 
	        "File mode is 0%o, uid=%d, gid=%d. My uid=%d, %d groups (",
    	        (unsigned int) fileStatus.st_mode, fileStatus.st_uid,
	        fileStatus.st_gid, myUid, ngroups);
	for (i = 0; i < ngroups; i++)
	    fprintf(stderr, " %d", groups[i]);
	fprintf(stderr, ")\n");
    }
#endif
    
    if (fileStatus.st_mode & 0002)		/* I can write anyway? */
    	return YES;
	
    if ((fileStatus.st_mode & 0200) &&		/* I can write my own file? */
        (fileStatus.st_uid == myUid))
    	return YES;

    if (fileStatus.st_mode & 0020) {		/* Group I am in can write? */
   	for (i = 0; i < ngroups; i++) {
            if (groups[i] == fileStatus.st_gid)
	        return YES;
	}
    }
#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "\tFile is not editable.\n");
#endif
    return NO;					/* If no excuse, can't do */
#endif
}


/*      Output one directory entry
**
*/
PUBLIC void HTDirEntry (HTStructured *target, WWW_CONST char *tail,
			WWW_CONST char *entry)
{
    char *relative;
    char *escaped = NULL;

    if (!strcmp(entry, "../")) {
	/*
	**  Undo slash appending for anchor creation.
	*/
	StrAllocCopy(escaped, "..");
    } else {
	int len;

	escaped = HTEscape(entry);
	if (((len = strlen(escaped)) > 2) && escaped[(len - 3)] == '%' &&
	    escaped[(len - 2)] == '2' && TOUPPER(escaped[(len - 1)]) == 'F')
	    escaped[(len - 3)] = '\0';
    }

    PUTS("<A HREF=\"");
    if (!tail || !*tail) {
	/*
	**  Handle extra slash at end of path.
	*/
	PUTS(*escaped ? escaped : "/");
    } else {
	/*
	**  If empty tail, gives absolute ref below.
	*/
	relative = (char *)malloc(strlen(tail) + strlen(escaped) + 2);
	if (!relative)
	    outofmem(__FILE__, "HTDirEntry");
	sprintf(relative, "%s%s%s", tail, *escaped ? "/" : "", escaped);
	PUTS(relative);
	free(relative);
    }
    PUTS("\">");
    free(escaped);
}
 
/*    Output parent directory entry
**
**    This gives the TITLE and H1 header, and also a link
**    to the parent directory if appropriate.
*/
PUBLIC void HTDirTitles (HTStructured *target, HTAnchor *anchor)
{
    char *logical = HTAnchor_address(anchor);
    char *path = HTParse(logical, "", PARSE_PATH + PARSE_PUNCTUATION);
    char *current = strrchr(path, '/');	 /* Last part or "" */
    char *printable = NULL;

    free(logical);

    StrAllocCopy(printable, current + 1);
    HTUnEscape(printable);
    START(HTML_TITLE);
    PUTS(*printable ? printable : "Welcome ");
    PUTS(" directory");
    END(HTML_TITLE);    
  
    START(HTML_H1);
    PUTS(*printable ? printable : "Welcome");
    END(HTML_H1);
    free(printable);

    /*  Make link back to parent directory
     */
    if (current && current[1]) {   /* Was a slash AND something else too */
        char *parent, *relative;

	*current++ = '\0';
        parent = strrchr(path, '/');  /* Penultimate slash */
	relative = (char *) malloc(strlen(current) + 4);
	if (!relative)
	    outofmem(__FILE__, "DirRead");
	sprintf(relative, "%s/..", current);
        PUTS("<A HREF=\"");
        PUTS(relative);
        PUTS("\">");
	free(relative);

	PUTS("Up to ");
	if (parent) {
	    printable = NULL;
	    StrAllocCopy(printable, parent + 1);
	    HTUnEscape(printable);
	    PUTS(printable);
	    free(printable);
	} else {
	    PUTS("/");
	}
        PUTS("</A>");
    }
    free(path);
}
		
/*	Load a document
**	---------------
**
** On entry,
**	addr		must point to the fully qualified hypertext reference.
**			This is the physical address of the file
**
** On exit,
**	returns		<0		Error has occured.
**			HTLOADED	OK 
**
*/
PUBLIC int HTLoadFile (WWW_CONST char *addr, HTParentAnchor *anchor,
		       HTFormat format_out, HTStream *sink)
{
    HTProtocol *p;
    HTFormat format;
    int fd = -1;		/* Unix file descriptor number = INVALID */
    char *filename, *nodename;
    HTAtom *encoding;
    int compressed;
#ifdef VMS
    int vmsdir = 0;
    int unixdir = 0;
    char *vmsname, *wwwname;
#if (stat != decc$stat) || !defined(MULTINET)
    struct stat stat_info;
#else
#undef stat
    struct stat stat_info;
#define stat decc$stat
#endif /* VMS MultiNet work around */
#endif
    static HTAtom *plaintext, *put, *www_source;
    static int init = 0;

    if (!init) {
        plaintext = WWW_PLAINTEXT;
	put = HTAtom_for("PUT");
	www_source = HTAtom_for("www/source");
	init = 1;
    }

    nodename = HTParse(addr, "", PARSE_HOST);
    /*
    **	If access is ftp, or file is on another host, invoke ftp now.
    */
    p = HTAnchor_protocol(anchor);
    if (!strcmp("ftp", p->name) ||
        (*nodename && strcmp("localhost", nodename) &&
#ifdef VMS
	my_strcasecmp(nodename, HTHostName()))) {
#else
	strcmp(nodename, HTHostName()))) {
#endif /* VMS */
	int status;
	char *file = strdup(addr);

        if (strstr(file, "%20")) {
	    /* Convert escaped spaces to real spaces */ 
	    char *p = file;
	    char *q = file;

	    while (*p) {
	        if ((*p == '%') && (*(p + 1) == '2') && (*(p + 2) == '0')) {
		    *q++ = ' ';
		    p += 3;
	        } else {
		    *q++ = *p++;
	        }
	    }
	    *q = '\0';
        }
	status = HTFTPLoad(file, anchor, format_out, sink);
	/* Try passive mode if soft failure and not already tried passive */
	if ((status < 0) && !ftp_passive && (status != HT_INTERRUPTED) &&
 	    (status != HT_NO_ACCESS) && (status != HT_FORBIDDEN)) {
	    ftp_passive = TRUE;
	    status = HTFTPLoad(file, anchor, format_out, sink);
	}
        ftp_passive = FALSE;
	free(file);
	free(nodename);
	return status;
    }

    /*	Reduce the filename to a basic form (hopefully unique!)
     */
    filename = HTParse(addr, "", PARSE_PATH | PARSE_PUNCTUATION);
#ifdef VMS
    HTUnEscape(filename);
#endif /* VMS */
    
#ifndef VMS   /* New code for VMS port, PGE for DL */
    format = HTFileFormat(filename, &encoding, plaintext, &compressed);
#else
    {
       /* Running on VMS, so strip version before testing file format */
       char *temp_filename = strdup(filename);

       strip_VMS_version(temp_filename);
       format = HTFileFormat(temp_filename, &encoding, plaintext, &compressed);
       free(temp_filename);
    }
#endif

#ifdef VMS
    /* Get VMS style file name */
    vmsname = strchr(filename + 1, '/') ?
	      HTVMS_name(nodename, filename) : filename + 1;
    /*
    **	Check to see if the 'filename' is in fact a directory.	If it is
    **	create a new hypertext object containing a list of files and
    **	subdirectories contained in the directory.  All of these are links
    **	to the directories or files listed.
    */
    if (HTStat(filename, &stat_info) != -1) {
	unixdir = 1;
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr, "HTFile: stat as Unix dir %s\n", filename);
#endif
    } else {
	wwwname = HTVMS_wwwName(vmsname);
	if (HTStat(vmsname, &stat_info) != -1) {
	    vmsdir = 1;
#ifndef DISABLE_TRACE
	    if (www2Trace)
		fprintf(stderr, "HTFile: stat as VMS dir %s\n", vmsname);
#endif
	} else if (HTStat(wwwname, &stat_info) != -1) {
	    /* Need to check wwwname because stat of vmsname fails on
	     * versions of VMS prior to 7.x */
	    vmsdir = 1;
#ifndef DISABLE_TRACE
	    if (www2Trace)
		fprintf(stderr, "HTFile: stat as WWW dir %s\n", wwwname);
#endif
	}
    }
    if (unixdir || vmsdir) {
	if (((stat_info.st_mode) & S_IFMT) == S_IFDIR) {
	    if (HTDirAccess == HT_DIR_FORBID) {
		free(filename);
		free(nodename);
		return HTLoadError(sink, 403,
		    		   "Directory browsing is not allowed.");
	    }
	    if (HTDirAccess == HT_DIR_SELECTIVE) {
		char *enable_file_name = malloc(strlen(filename) +
						strlen(HT_DIR_ENABLE_FILE) + 2);

		if (!enable_file_name)
		    outofmem(__FILE__, "HTLoadFile");
		strcpy(enable_file_name, filename);
		strcat(enable_file_name, "/");
		strcat(enable_file_name, HT_DIR_ENABLE_FILE);
		if (HTStat(enable_file_name, &stat_info) == -1) {
		    free(filename);
		    free(nodename);
		    return HTLoadError(sink, 403,
		          "Selective access is not enabled for this directory");
		}
	    }
	    free(nodename);
	    if (unixdir) {
		free(filename);
		return HTVMSBrowseDir(addr, anchor, format_out, sink);
	    } else {
		int status;

		/* Must start and end with slash */
		if (*wwwname != '/') {
		    vmsname = strdup("/");
		    StrAllocCat(vmsname, wwwname);
		} else {
		    StrAllocCopy(vmsname, wwwname);
		}
		if (*(vmsname + strlen(vmsname) - 1) != '/')
		    StrAllocCat(vmsname, "/");
		status = HTVMSBrowseDir(vmsname, anchor, format_out, sink);
		free(vmsname);
		free(filename);
		return status;
	    }
	}
    }
    /* Assume that the file is in Unix-style syntax if it contains a '/'
     * after the leading one @@
     */
    {
	FILE *fp;
	char *localname = HTLocalName(addr);

        if (!localname) {
	    free(filename);
	    free(nodename);
	    goto suicide;
	}
	fp = fopen(vmsname, "r", "shr=put", "shr=upd");
	
	/*  If the file wasn't VMS syntax, then perhaps it is ultrix
	 */
	if (!fp) {
	    char ultrixname[INFINITY];

#ifndef DISABLE_TRACE
	    if (www2Trace)
		fprintf(stderr, "HTFile: Can't open as %s\n", vmsname);
#endif
	    if (nodename) {
		sprintf(ultrixname, "%s::\"%s\"", nodename, filename);
	    } else {
		sprintf(ultrixname, "\"%s\"", filename);
	    }
	    fp = fopen(ultrixname, "r", "shr=put", "shr=upd");
	    if (!fp) {
#ifndef DISABLE_TRACE
		if (www2Trace)
		    fprintf(stderr, "HTFile: Can't open as %s\n", ultrixname);
#endif
		free(filename);
		free(nodename);
		free(localname);
       		return HTLoadError(sink, 403,	/* List formats? */
		    		   "Could not open local file.");
	    }
	}
	/*
	 * The next is duplicated from a previous version.
	 */
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr, "HTFile: Opening `%s' gives %p\n",
		    localname, (void *)fp);
#endif
	if (HTEditable(localname)) {
	    HTList *methods = HTAnchor_methods(anchor);

	    if (HTList_indexOf(methods, put) == (-1))
		HTList_addObject(methods, put);
	}
	free(filename);
	free(nodename);
	free(localname);
	HTParseFile(format, format_out, anchor, fp, sink, compressed);  /*F.Z.*/
	fclose(fp);
	return HT_LOADED;
    }
#else
    free(filename);
    
    /*	For unix, we try to translate the name into the name of a
    **  transparently mounted file.
    **
    **	Not allowed in secure (HTClienntHost) situations TBL 921019
    */
#ifndef NO_UNIX_IO
    /*  Need protection here for telnet server but not httpd server */
	 
    {	/* Try local file system */
	char *localname = HTLocalName(addr);
	struct stat dir_info;

        if (!localname)
            goto suicide;
	
#ifdef GOT_READ_DIR
	/*		  Multiformat handling
	**
	**	If needed, scan directory to find a good file.
	**      Bug:  we don't stat the file to find the length
	*/
	if ((strlen(localname) > strlen(MULTI_SUFFIX)) &&
	    (!strcmp(localname + strlen(localname) - strlen(MULTI_SUFFIX),
	             MULTI_SUFFIX))) {
	    DIR *dp;
	    STRUCT_DIRENT *dirbuf;
	    float best = NO_VALUE_FOUND;	/* So far best is bad */
	    HTFormat best_rep = NULL;		/* Set when rep found */
	    STRUCT_DIRENT best_dirbuf;		/* Best dir entry so far */
	    char *base = strrchr(localname, '/');
	    int baselen;

	    if (!base || (base == localname))
	        goto forget_multi;
	    *base++ = '\0';		/* Just got directory name */
	    baselen = strlen(base) - strlen(MULTI_SUFFIX);
	    base[baselen] = '\0';	/* Chop off suffix */

	    dp = opendir(localname);
	    if (!dp) {
 forget_multi:
		free(localname);
		return HTLoadError(sink, 500,
			           "Multiformat: directory scan failed.");
	    }
	    
	    /* While there are directory entries to be read */
	    while (dirbuf = readdir(dp)) {
		/* If the entry is not being used, skip it */
		if (dirbuf->d_ino == 0)
		    continue;
		if (!strncmp(dirbuf->d_name, base, baselen)) {	
		    HTFormat rep = HTFileFormat(dirbuf->d_name, &encoding,
                                                plaintext, &compressed);
		    float value = HTStackValue(rep, format_out,
		    			       HTFileValue(dirbuf->d_name), 0);

		    if (value != NO_VALUE_FOUND) {
#ifndef DISABLE_TRACE
		        if (www2Trace)
			    fprintf(stderr,
				    "HTFile: value of presenting %s is %f\n",
				    HTAtom_name(rep), value);
#endif
			if  (value > best) {
			    best_rep = rep;
			    best = value;
			    best_dirbuf = *dirbuf;
		        }
		    }	/* If best so far */ 		    
		}  /* If match */  
	    }
	    closedir(dp);
	    
	    if (best_rep) {
		format = best_rep;
		base[-1] = '/';		/* Restore directory name */
		base[0] = '\0';
		StrAllocCat(localname, best_dirbuf.d_name);
		goto open_file;
	    } else { 			/* If not found suitable file */
		free(localname);
		return HTLoadError(sink, 403,	/* List formats? */
		    "Could not find suitable representation for transmission.");
	    }
	}  /* If multi suffix */
        /*
        ** Check to see if the 'localname' is in fact a directory.  If it is
        ** create a new hypertext object containing a list of files and 
        ** subdirectories contained in the directory.  All of these are links
        ** to the directories or files listed.
        ** NB This assumes the existance of a type 'STRUCT_DIRENT', which will
        ** hold the directory entry, and a type 'DIR' which is used to point
        ** to the current directory being read.
        */
	if (stat(localname, &dir_info) == -1) {     /* Get file information */
	    /* If can't read file information */
#ifndef DISABLE_TRACE
	    if (www2Trace)
		fprintf(stderr, "HTFile: can't stat %s\n", localname);
#endif
	}  else {		/* Stat was OK */
	    if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) {
		/* If localname is a directory */	
 		/*
		** Read the localdirectory and present a nicely formatted list
		** to the user.  Re-wrote most of the read directory code here,
		** excepting for the checking access.
		**
		** Author: Charles Henrich (henrich@crh.cl.msu.edu)   10-09-93
		**
		** This is still pretty messy, need to clean it up at some point
		*/
                char filepath[MAXPATHLEN];
                char buffer[4096];
                char *ptr, *dataptr;
                HText *HT;
                HTFormat format;
                HTAtom *pencoding;
		struct stat statbuf;
		STRUCT_DIRENT *dp;
		DIR *dfp;
                int cmpr, count;

#ifndef DISABLE_TRACE
		if (www2Trace)
		    fprintf(stderr, "%s is a directory\n", localname);
#endif			
		/*	Check directory access.
		**	Selective access means only those directories
		**	containing a marker file can be browsed
		*/
		if (HTDirAccess == HT_DIR_FORBID) {
		    free(localname);
		    return HTLoadError(sink, 403,
		                       "Directory browsing is not allowed.");
		}
		if (HTDirAccess == HT_DIR_SELECTIVE) {
		    char *enable_file_name = malloc(strlen(localname) +
			 			strlen(HT_DIR_ENABLE_FILE) + 2);

		    strcpy(enable_file_name, localname);
		    strcat(enable_file_name, "/");
		    strcat(enable_file_name, HT_DIR_ENABLE_FILE);
		    if (stat(enable_file_name, &statbuf) != 0) {
			free(localname);
			return HTLoadError(sink, 403,
			 "Selective access is not enabled for this directory.");
		    }
		}

		dfp = opendir(localname);
		if (!dfp) {
		    free(localname);
		    return HTLoadError(sink, 403,
				       "This directory is not readable.");
		}

		/* Suck the directory up into a list to be sorted */
                HTSortInit();

                for (dp = readdir(dfp); dp; dp = readdir(dfp)) {
                    ptr = malloc(strlen(dp->d_name) + 1);
                    if (!ptr)
		        return HTLoadError(sink, 403,
					"Ran out of memory in directory read!");
                    strcpy(ptr, dp->d_name);
                     
                    HTSortAdd(ptr);
                }
                closedir(dfp);

		/* Sort the dir list */
                HTSortSort();

		/* Start a new HTML page */
                HT = HText_new();
                HText_beginAppend(HT);
                HText_appendText(HT, "<H1>Local Directory ");
                HText_appendText(HT, localname);
                HText_appendText(HT, "</H1>\n");
                HText_appendText(HT, "<DL>\n"); 

		/* Sort the list and then spit it out in a nice form */

		/* How this for a disgusting loop :) */
                for (count = 0, dataptr = HTSortFetch(count); dataptr; 
                    free(dataptr), count++, dataptr = HTSortFetch(count)) {

		    /* We do not want to see . */
                    if (!strcmp(dataptr, "."))
			continue;
 
		    /* If it's .. *and* the current directory is / don't show
		    ** anything, otherwise print out a nice Parent Directory
		    ** entry.
		    */
                    if (!strcmp(dataptr, "..")) {
                        if (strcmp(localname, "/")) {
                            strcpy(buffer, localname);

                            ptr = strrchr(buffer, '/');
                            if (ptr)
				*ptr = '\0'; 
                            if (!*buffer)
				strcpy(buffer, "/");
                            HText_appendText(HT, "<DD><A HREF=\"");
                            HText_appendText(HT, buffer);

                            HText_appendText(HT, "\"><IMG SRC=\"");
                            HText_appendText(HT,
					     HTgeticonname(NULL, "directory"));
                            HText_appendText(HT, "\"> Parent Directory</a>");
                            continue;
                        } else {
                            continue;
                        }
                    }
                      
		    /* Get the filesize information from a stat.  If we
		     * cannot stat it, we probably cannot read it either,
		     * so ignore it.
		     */
                    sprintf(filepath, "%s/%s", localname, dataptr);

                    if (stat(filepath, &statbuf) == -1)
			continue;
                    HText_appendText(HT, "<DD><A HREF=\"");
                    HText_appendText(HT, localname);

                    if (localname[strlen(localname) - 1] != '/')
                        HText_appendText(HT, "/");

                    HText_appendText(HT, dataptr);
                    HText_appendText(HT, "\">");

   		    /* If it's a directory, dump out a dir icon, don't
		     * bother with anything else.  If it is a file, try and
		     * figure out what type of file it is and grab the
		     * appropriate icon.  If we can't figure it out, call
		     * it text.  If it's a compressed file, call it binary
		     * no matter what.
                     */
                    if (statbuf.st_mode & S_IFDIR) {
                        sprintf(buffer, "%s", dataptr);
                        HText_appendText(HT, "<IMG SRC=\"");
                        HText_appendText(HT, HTgeticonname(NULL, "directory"));
                        HText_appendText(HT, "\"> ");
                    } else {
                        sprintf(buffer, "%s (%d bytes)",
				dataptr, statbuf.st_size);
                        format = HTFileFormat(dataptr, &pencoding, 
                                     	      www_source, &cmpr);

			/* If it's executable then call it application,
			 * else it might as well be text */
                        if (cmpr == 0) {
                            HText_appendText(HT, "<IMG SRC=\"");
                            if ((statbuf.st_mode & S_IXUSR) ||
                                (statbuf.st_mode & S_IXGRP) || 
                                (statbuf.st_mode & S_IXOTH)) {
                                HText_appendText(HT, 
                                          HTgeticonname(format, "application"));
                            } else {
                                HText_appendText(HT, 
                                                 HTgeticonname(format, "text"));
                            }
                            HText_appendText(HT, "\"> ");
                        } else {
                            HText_appendText(HT, "<IMG SRC=\"");
                            HText_appendText(HT,
				            HTgeticonname(NULL, "application"));
                            HText_appendText(HT, "\"> ");
                        }
                    }
		    /* Spit out the anchor */
                    HText_appendText(HT, buffer);
                    HText_appendText(HT, "</A>\n");
		}
	        /* End of list, clean up and we are done */
                HText_appendText(HT, "</DL>\n");
                HText_endAppend(HT);
                free(localname);
                return HT_LOADED;
	    }  /* End if localname is directory */
	}  /* End if file stat worked */
	
 /* End of directory reading section */
#endif
 open_file:
	{
	    FILE *fp = fopen(localname, "r");

#ifndef DISABLE_TRACE
	    if (www2Trace)
		fprintf(stderr, "HTFile: Opening `%s' gives %p\n",
			localname, (void *)fp);
#endif
	    if (fp) {		/* Good! */
		if (HTEditable(localname)) {
		    HTList *methods = HTAnchor_methods(anchor);

		    if (HTList_indexOf(methods, put) == (-1))
			HTList_addObject(methods, put);
		}
		free(localname);
		HTParseFile(format, format_out, anchor, fp, sink, compressed);
		return HT_LOADED;
	    }
	}
    }  /* Local unix file system */    
#endif
#endif

 suicide:
    /*
    return HTFTPLoad(addr, anchor, format_out, sink);
     */
    /* Sorry Charlie...if we are given a file:// URL and it fails, then it
     * fails! Do NOT FTP!!
     */
    return HT_NOT_LOADED;
}

/*		Protocol descriptors
*/
PUBLIC HTProtocol HTFTP  = { "ftp", HTLoadFile, NULL };
PUBLIC HTProtocol HTFile = { "file", HTLoadFile, NULL };
