/* MODULE							HTVMSUtil.c
**		VMS Utility Routines
**
** AUTHORS:
**	MD	Mark Donszelmann    duns@vxdeop.cern.ch
**
** HISTORY:
**	14 Nov 93  MD	Written
**
** BUGS:
**
*/

/* Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"

/* Avoid __utc_stat routine */
#if defined(__DECC) && (__VMS_VER >= 70000000)
#define _VMS_V6_SOURCE
#endif

#include "HTUtils.h"
#include "tcp.h"
#include "HTFormat.h"
#include "HTStream.h"
#include "HTTCP.h"

#ifdef __GNUC__
#include <stat.h>
#endif

#include "HTVMSUtils.h"
#include <descrip.h>
#include <lib$routines.h>
#include <starlet.h>
#include <rmsdef.h>
#include "../libnut/str-tools.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

#define FREE(x) if (x) {free(x); x = NULL;}

#define INFINITY 1024            /* File name length @@ FIXME */

PUBLIC BOOL HTVMSFileVersions = FALSE;  /* Version numbers in listing? */

typedef struct {
   unsigned long BufferLength : 16;
   unsigned long ItemCode : 16;
   unsigned long BufferAddress : 32;
   unsigned long ReturnLengthAddress : 32;
} ItemStruct;


/* PUBLIC							HTVMS_wwwName()
**		CONVERTS VMS Name into WWW Name 
** ON ENTRY:
**	vmsname		VMS file specification (NO NODE)
**
** ON EXIT:
**	returns 	www file specification
**
** EXAMPLES:
**	vmsname				wwwname
**	DISK$USER 			disk$user
**	DISK$USER: 			/disk$user/
**	DISK$USER:[DUNS] 		/disk$user/duns
**	DISK$USER:[DUNS.ECHO] 		/disk$user/duns/echo
**	[DUNS] 				duns
**	[DUNS.ECHO] 			duns/echo
**	[DUNS.ECHO.-.TRANS] 		duns/echo/../trans
**	[DUNS.ECHO.--.TRANS] 		duns/echo/../../trans
**	[.DUNS] 			duns
**	[.DUNS.ECHO] 			duns/echo
**	[.DUNS.ECHO]TEST.COM 		duns/echo/test.com 
**	TEST.COM 			test.com
**	
*/
PUBLIC char *HTVMS_wwwName (char *vmsname)
{
   static char wwwname[256];
   char *src = vmsname;
   char *dst = wwwname;
   int dir = 0;

   if (strchr(src, ':'))
      *dst++ = '/';

   for (; *src; src++) {
      switch (*src) {
         case ':':
		*dst++ = '/';
		break;
         case '-':
		if (dir) {
	 	      if ((*(src - 1) == '[' || *(src - 1) == '.' ||
			   *(src - 1) == '-') && 
		          (*(src + 1) == '.' || *(src + 1) == '-')) {
		          *dst++ = '/';
                          *dst++ = '.'; 
                          *dst++ = '.';
		      } else {
		          *dst++ = '-';
		      }
		} else {
		      if (*(src - 1) == ']')
			  *dst++ = '/';
		      *dst++ = '-';
		}
                break;
         case '.':
		if (dir) {
                      if (*(src - 1) != '[')
			  *dst++ = '/';
                } else {
		      if (*(src - 1) == ']')
			  *dst++ = '/';
                      *dst++ = '.';
		}
                break;
         case '[':
		dir = 1;
		break;
         case ']':
		dir = 0;
		break;
         default:
		if (*(src - 1) == ']')
		      *dst++ = '/';
                *dst++ = *src;
                break;
      }
   }
   *dst = '\0';
   return(wwwname);
}


/* PUBLIC							HTVMS_name()
**		CONVERTS WWW name into a VMS name
** ON ENTRY:
**	nn		Node Name (optional)
**	fn		WWW file name
**
** ON EXIT:
**	returns 	vms file specification
**
** Bug:	Returns pointer to static -- non-reentrant
*/
/*	We try converting the filename into Files-11 syntax.  That is, we
**	assume first that the file is, like us, on a VMS node.  We try remote
**	(or local) DECnet access.  Files-11, VMS, VAX and DECnet are
**	trademarks of Digital Equipment Corporation. 
**	The node is assumed to be local if the hostname WITHOUT DOMAIN
**	matches the local one. @@@
*/
PUBLIC char *HTVMS_name (WWW_CONST char *nn, WWW_CONST char *fn)
{
    static char vmsname[INFINITY];	/* Returned */
    char *filename = (char *)malloc(strlen(fn) + 1);
    char *nodename = nn ? (char *)malloc(strlen(nn) + 2 + 1) :
			  (char *)malloc(2 + 1);
    char *second;		/* 2nd slash */
    char *last;			/* Last slash */
    char *hostname = (char *)HTHostName();

    if (!filename || !nodename)
	outofmem(__FILE__, "HTVMSname");
    strcpy(filename, fn);
    *nodename = '\0';

    /* On same node?  Yes if node names match */
    if (nn && strncmp(nn, "localhost", 9)) {
        char *p, *q;

        for (p = hostname, q = (char *)nn;
	     *p && *p != '.' && *q && *q != '.'; p++, q++) {
	    if (TOUPPER(*p) != TOUPPER(*q)) {
	        strcpy(nodename, nn);
		q = strchr(nodename, '.');	/* Mismatch */
		if (q)
		    *q = '\0';			/* Chop domain */
		strcat(nodename, "::");		/* Try decnet anyway */
		break;
	    }
	}
    }
    second = strchr(filename + 1, '/');		/* 2nd slash */
    last = strrchr(filename, '/');		/* Last slash */
        
    if (!second) {				/* Only one slash */
	sprintf(vmsname, "%s%s", nodename, filename + 1);
    } else if (second == last) {		/* Exactly two slashes */
	*second = '\0';		/* Split filename from disk */
	sprintf(vmsname, "%s%s:%s", nodename, filename + 1, second + 1);
	*second = '/';		/* Restore */
    } else { 					/* More than two slashes */
	char *p;

	*second = '\0';		/* Split disk from directories */
	*last = '\0';		/* Split dir from filename */
	sprintf(vmsname, "%s%s:[%s]%s",
		nodename, filename + 1, second + 1, last + 1);
	*second = *last = '/';	/* Restore filename */
	for (p = strchr(vmsname, '['); *p != ']'; p++) {
	    if (*p == '/')
	        *p = '.';	/* Convert dir sep. to dots */
	}
    }
    free(nodename);
    free(filename);
    return vmsname;
}

/*
**	The code below is for directory browsing by VMS Curses clients.
**	It is based on the newer WWWLib's HTDirBrw.c. - Foteos Macrides
*/
PUBLIC int HTStat (WWW_CONST char *filename, stat_t *info)
{
   /* 
    * The following stuff does not work in VMS with a normal stat...
    * -->   /disk$user/duns/www if www is a directory
    *		is statted like: 	/disk$user/duns/www.dir 
    *		after a normal stat has failed
    * -->   /disk$user/duns	if duns is a toplevel directory
    *		is statted like:	/disk$user/000000/duns.dir
    * -->   /disk$user since disk$user is a device
    *		is statted like:	/disk$user/000000/000000.dir
    * -->   /
    *		searches all devices, no solution yet...
    * -->   /vxcern!/disk$cr/wwwteam/login.com
    *		is not statted but granted with fake information...
    */
   int Result, Len;
   char *Ptr, *Ptr2;
   char Name[256];

   /* Try normal stat... */
   Result = stat((char *)filename, info);
   if (!Result)
      return(Result);

   /* Make local copy */
   strcpy(Name, filename);

   /* Failed, so do device search in case root is requested */
   if (!strcmp(Name, "/"))   /* Root requested */
      return(-1);
   
   /* Failed so this might be a directory, add '.dir' */
   Len = strlen(Name) - 1;
   if (Name[Len] == '/')
      Name[Len] = '\0';
   
   /* Fail in case of device */
   Ptr = strchr(Name + 1, '/');
   if (!Ptr && (Name[0] == '/'))   /* Device only... */
      strcat(Name, "/000000/000000");
   
   if (Ptr) {  /* Correct filename in case of toplevel dir */
      Ptr2 = strchr(Ptr + 1, '/');
      if (!Ptr2 && (Name[0] == '/')) {
         char End[256];

         strcpy(End, Ptr);
         *(Ptr + 1) = '\0';
         strcat(Name, "000000");
         strcat(Name, End);
      }
   }

   /* Try in case a file on toplevel directory or .DIR was already specified */
   Result = stat(Name, info);
   if (!Result)
      return(Result);

   /* Add .DIR and try again */
   strcat(Name, ".dir");
   Result = stat(Name, info);
   return(Result);
}

#ifndef	_POSIX_SOURCE
#define	d_ino	d_fileno	/* Compatability */
#ifndef	NULL
#define	NULL	0
#endif
#endif	/* !_POSIX_SOURCE */

typedef	struct __dirdesc {
    long context;	        /* Context descriptor for LIB$FIND_FILE calls */
    char dirname[255 + 1];      /* Keeps the directory name, including *.* */
    struct dsc$descriptor_s dirname_desc;	/* Descriptor of dirname */
} DIR;

PRIVATE	DIR *HTVMSopendir(char *dirname);
PRIVATE	struct dirent *HTVMSreaddir(DIR *dirp);
PRIVATE	int HTVMSclosedir(DIR *dirp);

struct dirent {
    unsigned long d_fileno;	/* File number of entry */
    unsigned short d_namlen;	/* Length of string in d_name */
    char d_name[255 + 1];	/* Name (up to MAXNAMLEN + 1) */
};


PRIVATE DIR *HTVMSopendir(char *dirname)
{
   static DIR dir;
   long status;
   struct dsc$descriptor_s entryname_desc;
   struct dsc$descriptor_s dirname_desc;
   char DirEntry[256];
   char VMSentry[256];
   char UnixEntry[256];
   char *dot;
   int index;

   /* Check if directory exists 
    * dirname can look like /disk$user/duns/www/test/multi
    * or like               /disk$user/duns/www/test/multi/
    * DirEntry should look like disk$user:[duns.www.test]multi in both cases
    * dir.dirname should look like disk$user:[duns.www.test.multi]
    */
   strcpy(UnixEntry, dirname);
   if (UnixEntry[strlen(UnixEntry) - 1] != '/')
      strcat(UnixEntry, "/");

   strcpy(DirEntry, HTVMS_name("", UnixEntry));
   strcpy(dir.dirname, DirEntry);
   index = strlen(DirEntry) - 1;

   if (DirEntry[index] == ']')
      DirEntry[index] = '\0';

   if (!(dot = strrchr(DirEntry, '.'))) {
      /* Convert disk$user:[duns] into disk$user:[000000]duns.dir */
      char *openbr = strrchr(DirEntry, '[');

      if (!openbr) {
	 /* Convert disk$user: into disk$user:[000000]000000.dir */
         strcpy(dir.dirname, DirEntry);
         strcat(dir.dirname, "[000000]");
         strcat(DirEntry, "[000000]000000.dir");
      } else {
         char End[256];

         strcpy(End, openbr + 1);
         *(openbr + 1) = '\0';
         strcat(DirEntry, "000000]");
         strcat(DirEntry, End);
         strcat(DirEntry, ".dir");
      }
   } else {
      *dot = ']';   
      strcat(DirEntry, ".dir");
   }

   dir.context = 0;
   dirname_desc.dsc$w_length = strlen(DirEntry);
   dirname_desc.dsc$b_dtype = DSC$K_DTYPE_T;
   dirname_desc.dsc$b_class = DSC$K_CLASS_S;
   dirname_desc.dsc$a_pointer = (char *)&DirEntry;

   /* Look for the directory */
   entryname_desc.dsc$w_length = 255;
   entryname_desc.dsc$b_dtype = DSC$K_DTYPE_T;
   entryname_desc.dsc$b_class = DSC$K_CLASS_S;
   entryname_desc.dsc$a_pointer = VMSentry;

   status = lib$find_file(&dirname_desc, &entryname_desc, 
                          &dir.context, 0, 0, 0, 0);
   if (!(status & 0x01))   /* Directory not found */
      return(NULL);

   if (HTVMSFileVersions) {
      strcat(dir.dirname, "*.*;*");
   } else {
      strcat(dir.dirname, "*.*");
   }
   dir.context = 0;
   dir.dirname_desc.dsc$w_length = strlen(dir.dirname);
   dir.dirname_desc.dsc$b_dtype = DSC$K_DTYPE_T;
   dir.dirname_desc.dsc$b_class = DSC$K_CLASS_S;
   dir.dirname_desc.dsc$a_pointer = (char *)&dir.dirname;
   return(&dir);
}

PRIVATE struct dirent *HTVMSreaddir(DIR *dirp)
{
   static struct dirent entry;
   long status;
   struct dsc$descriptor_s entryname_desc;
   char *space, *slash, *UnixEntry;
   char VMSentry[256];

   entryname_desc.dsc$w_length = 255;
   entryname_desc.dsc$b_dtype = DSC$K_DTYPE_T;
   entryname_desc.dsc$b_class = DSC$K_CLASS_S;
   entryname_desc.dsc$a_pointer = VMSentry;

   status = lib$find_file(&dirp->dirname_desc, &entryname_desc, 
                          &dirp->context, 0,0,0,0);
   if (status == RMS$_NMF) {  /* No more files */
      return(NULL);
   } else {
      /* ok */
      if (!(status & 0x01))
	 return(0);
      if (HTVMSFileVersions) {
         space = strchr(VMSentry, ' ');
      } else {
         space = strchr(VMSentry, ';');
      }
      if (space)
         *space = '\0';

      /* Convert to unix style... */
      UnixEntry = HTVMS_wwwName(VMSentry);
      slash = strrchr(UnixEntry, '/') + 1;
      strcpy(entry.d_name, slash);
      entry.d_namlen = strlen(entry.d_name);
      entry.d_fileno = 1;
      return(&entry);
   }
}

PRIVATE int HTVMSclosedir(DIR *dirp)
{
   long status = lib$find_file_end(&dirp->context);

   if (!(status & 0x01))
      exit(status);
   dirp->context = 0;
   return(0);
}

#include "HTAnchor.h"
#include "HTParse.h"
#include "HTBTree.h"
#include "HTFile.h"	/* For HTFileFormat() */
#include "HTAlert.h"
/*
**  Hypertext object building machinery.
*/
#include "HTML.h"
#define PUTC(c) (*targetClass.put_character)(target, c)
#define PUTS(s) (*targetClass.put_string)(target, s)
#define START(e) (*targetClass.start_element)(target, e, 0, 0)
#define END(e) (*targetClass.end_element)(target, e)
#define FREE_TARGET (*targetClass.free)(target)

struct _HTStructured {
	WWW_CONST HTStructuredClass *isa;
	/* ... */
};

#define STRUCT_DIRENT struct dirent

PRIVATE char *months[12] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

typedef struct _VMSEntryInfo {
    char *filename;
    char *type;
    char *date;
    unsigned int size;
    BOOLEAN display;  /* Show this entry? */
} VMSEntryInfo;

PRIVATE void free_VMSEntryInfo_contents (VMSEntryInfo *entry_info)
{
    if (entry_info) {
	FREE(entry_info->filename);
	FREE(entry_info->type);
	FREE(entry_info->date);
    }
    /* Do not free the struct */
}

#define FILE_BY_NAME 0 
#define FILE_BY_TYPE 1
#define FILE_BY_SIZE 2
#define FILE_BY_DATE 3
/* Specifies the method of sorting */
PRIVATE BOOLEAN HTfileSortMethod = FILE_BY_NAME;

PRIVATE int compare_VMSEntryInfo_structs (VMSEntryInfo *entry1, 
					  VMSEntryInfo *entry2)
{
    int i, status;
    char date1[16], date2[16], time1[8], time2[8], month[4];

    switch (HTfileSortMethod) {
        case FILE_BY_SIZE:
	    /* Both equal or both 0 */
            if (entry1->size == entry2->size) {
		return(my_strcasecmp(entry1->filename, entry2->filename));
	    } else if (entry1->size > entry2->size) {
		return(1);
	    } else {
		return(-1);
	    }
        case FILE_BY_TYPE:
            if (entry1->type && entry2->type) {
                status = my_strcasecmp(entry1->type, entry2->type);
		if (status)
		    return(status);
		/* Else fall to filename comparison */
	    }
            return (my_strcasecmp(entry1->filename, entry2->filename));
        case FILE_BY_DATE:
            if (entry1->date && entry2->date) {
	        /*
		** Make sure we have the correct length.
		*/
		if (strlen(entry1->date) != 12 || strlen(entry2->date) != 12)
		    return (my_strcasecmp(entry1->filename, entry2->filename));
	        /*
		** Set up for sorting in reverse
		** chronological order.
		*/
		if (entry1->date[7] != ' ') {
		    strcpy(date1, "9999");
		    strcpy(time1, (char *)&entry1->date[7]);
		} else {
		    strcpy(date1, (char *)&entry1->date[8]);
		    strcpy(time1, "00:00");
		}
		strncpy(month, entry1->date, 3);
		month[3] = '\0';
		for (i = 0; i < 12; i++) {
		    if (!my_strcasecmp(month, months[i]))
			break;
		}
		i++;
		sprintf(month, "%s%d", i < 10 ? "0" : "", i);
		strcat(date1, month);
		strncat(date1, (char *)&entry1->date[4], 2);
		date1[8] = '\0';
		if (date1[6] == ' ')
		    date1[6] = '0';
		strcat(date1, time1);
		if (entry2->date[7] != ' ') {
		    strcpy(date2, "9999");
		    strcpy(time2, (char *)&entry2->date[7]);
		} else {
		    strcpy(date2, (char *)&entry2->date[8]);
		    strcpy(time2, "00:00");
		}
		strncpy(month, entry2->date, 3);
		month[3] = '\0';
		for (i = 0; i < 12; i++) {
		    if (!my_strcasecmp(month, months[i]))
			break;
		}
		i++;
		sprintf(month, "%s%d", i < 10 ? "0" : "", i);
		strcat(date2, month);
		strncat(date2, (char *)&entry2->date[4], 2);
		date2[8] = '\0';
		if (date2[6] == ' ')
		    date2[6] = '0';
		strcat(date2, time2);
		/*
		** Do the comparison.
		*/
                status = my_strcasecmp(date2, date1);
		if (status)
		    return(status);
		/* Else fall to filename comparison */
	    }
            return (my_strcasecmp(entry1->filename, entry2->filename));
        case FILE_BY_NAME:
        default:
            return (strcmp(entry1->filename, entry2->filename));
    }
}

/*						    	HTVMSBrowseDir()
**
**	This function generates a directory listing as an HTML-object
**	for local file URL's.  It assumes the first two elements of
**	of the path are a device followed by a directory:
**
**		file://localhost/device/directory[/[foo]]
**
**	Will not accept 000000 as a directory name.
**	Will offer links to parent through the top directory, unless
**	a terminal slash was included in the calling URL.
**
**	Returns HT_LOADED on success, HTLoadError() messages on error.
**
**	Developed for Lynx by Foteos Macrides (macrides@sci.wfeb.edu).
*/
PUBLIC int HTVMSBrowseDir (WWW_CONST char *address,
			   HTParentAnchor *anchor,
			   HTFormat format_out,
			   HTStream *sink)
{
    HTStructured *target;
    HTStructuredClass targetClass;
    char *pathname = HTParse(address, "", PARSE_PATH + PARSE_PUNCTUATION);
    char *tail = NULL;
    char *title = NULL;
    char *header = NULL;
    char *parent = NULL;
    char *relative = NULL;
    char *backpath = NULL;
    char *cp, *cp1, *cp2;
    char string_buffer[64];
    int  pathend, len;
    DIR  *dp;
#if (stat != decc$stat) || !defined(MULTINET)
    struct stat file_info;
#else
#undef stat
    struct stat file_info;
#define stat decc$stat
#endif /* VMS MultiNet work around */
    time_t NowTime;
    VMSEntryInfo *entry_info = 0;
    BOOLEAN no_dotfiles = 1;
    BOOLEAN show_dotfiles = 0;
    static char ThisYear[8];

    HTUnEscape(pathname);
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "HTVMSBrowseDir: Browsing `%s\'\n", pathname);
#endif
    /*
     *  Require at least two elements (presumably a device and directory)
     *  and disallow the device root (000000 directory).  Symbolic paths
     *  (e.g. sys$help) should have been translated and expanded (e.g.
     *  to /sys$sysroot/syshlp) before calling this routine.
     */
    if ((*pathname != '/') || !(cp = strchr(pathname + 1, '/')) ||
	!*(cp + 1) || !strncmp(cp + 1, "000000", 6) ||
        !(dp = HTVMSopendir(pathname))) {
        free(pathname);
    	return HTLoadError(sink, 403, "Could not access directory.");
    }
    /*
     *  Set up the output stream.
     */
    HTProgress("Building directory listing...");
    target = HTML_new(anchor, format_out, sink);
    targetClass = *(target->isa);
    /*
     *  Set up the offset string of the anchor reference,
     *  and strings for the title and header.
     */
    cp = strrchr(pathname, '/');  /* Find last slash */
    StrAllocCopy(tail, cp + 1);
    /* This is a mess because tail's usage is overloaded, but it works */
    if (!*tail && !my_strcasecmp(pathname, address)) {
	*cp = '\0';
	StrAllocCopy(tail, pathname);
	cp1 = strrchr(pathname, '/');
	if (cp1 && (cp1 != pathname) && strncmp(cp1 + 1, "000000", 6)) {
	    *cp1 = '\0';
	    cp2 = strrchr(pathname, '/');
	    /* Top level directory has no parent (ignoring [000000]) */ 
	    if (cp2 && (cp2 != pathname)) {
		StrAllocCopy(parent, cp2 + 1);
		StrAllocCopy(backpath, pathname);
	    }
	    *cp1 = '/';
	}
	StrAllocCopy(title, cp1 + 1);
	*cp = '/';
    } else if (*tail) {
        StrAllocCopy(title, tail);
	*cp = '\0';
	if ((cp1 = strrchr(pathname, '/')) && (cp1 != pathname) &&
	    strncmp(cp1 + 1, "000000", 6))
	    StrAllocCopy(parent, cp1 + 1);
	*cp = '/';
    } else {
        pathname[strlen(pathname) - 1] = '\0';
	cp = strrchr(pathname, '/');
	StrAllocCopy(title, cp + 1);
	pathname[strlen(pathname)] = '/';
    }
    StrAllocCopy(header, pathname);
    /*
     *  Initialize path name for HTStat().
     */
    pathend = strlen(pathname);
    if (*(pathname + pathend - 1) != '/') {
	StrAllocCat(pathname, "/");
	pathend++;
    }
    /*
     *  Output the title and header.
     */
    START(HTML_HTML);
    START(HTML_HEAD);
    HTUnEscape(title);
    START(HTML_TITLE);
    PUTS(title);
    PUTS(" directory");
    END(HTML_TITLE);
    free(title);
    END(HTML_HEAD);
    START(HTML_BODY);
    HTUnEscape(header);
    START(HTML_H1);
    PUTS(header);
    END(HTML_H1);
    PUTS("\n");
    if (HTDirReadme == HT_DIR_README_TOP) {
        FILE *fp;

	if (header[strlen(header) - 1] != '/')
	    StrAllocCat(header, "/");
	StrAllocCat(header, HT_DIR_README_FILE);
        if (fp = fopen(header, "r")) {
	    START(HTML_PRE);
	    for (;;) {
	        char c = fgetc(fp);

	        if (c == (char)EOF)
		    break;
#ifdef NOTDEFINED
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
		    default:
			PUTC(c);
	        }
#else
		PUTC(c);
#endif /* NOTDEFINED */
	    }
	    END(HTML_PRE);
	    fclose(fp);
        } 
    }
    free(header);
    if (parent) {
	if (!(relative = (char *)malloc(strlen(tail) + 4)))
	    outofmem(__FILE__, "HTVMSBrowseDir");
	if (backpath) {
	    sprintf(relative, "%s", backpath);
	    free(backpath);
	} else {
	    sprintf(relative, "%s/..", tail);
	}
	PUTS("<A HREF=\"");
	PUTS(relative);
	PUTS("\">");
	PUTS("Up to ");
	HTUnEscape(parent);
	PUTS(parent);
	END(HTML_A);
	START(HTML_P);
	free(relative);
	free(parent);
    }
    /*
     *  Set up the date comparison.
     */
    NowTime = time(NULL);
    strcpy(ThisYear, (char *)ctime(&NowTime) + 20);
    ThisYear[4] = '\0';
    /*
     * Now, generate the Btree and put it out to the output stream.
     */
    {
	char dottest = 2;	/* To avoid two strcmp() each time */
	STRUCT_DIRENT *dirbuf;
	HTBTree *bt;
	int compressed;
	static HTAtom *www_plaintext;
	static int init = 0;

	if (!init) {
	    www_plaintext = WWW_PLAINTEXT;
	    init = 1;
	}

	/* Set up sort key and initialize BTree */
	bt = HTBTree_new((HTComparer) compare_VMSEntryInfo_structs);

	/* Build tree */
	while (dirbuf = HTVMSreaddir(dp)) {
	    HTAtom *encoding = NULL;
	    HTFormat format;

	    /* Skip if not used */
	    if (!dirbuf->d_ino)
		continue;
	    
	    /* Current and parent directories are never shown in list */
	    if (dottest && (!strcmp(dirbuf->d_name, ".") ||
			    !strcmp(dirbuf->d_name, ".."))) {
		dottest--;
		continue;
	    }

	    /* Don't show the selective enabling file
	     * unless version numbers are included */
	    if (!my_strcasecmp(dirbuf->d_name, HT_DIR_ENABLE_FILE))
		continue;

	    /* Skip files beginning with a dot? */
	    if ((no_dotfiles || !show_dotfiles) && *dirbuf->d_name == '.')
		continue;

	    /* OK, make an lstat() and get a key ready. */
	    *(pathname + pathend) = '\0';
	    StrAllocCat(pathname, dirbuf->d_name);
	    if (HTStat(pathname, &file_info))
		/* For VMS the failure here means the file is not readable...
		 * We however continue to browse through the directory... */
                continue;

            if (!(entry_info = (VMSEntryInfo *)calloc(1, sizeof(VMSEntryInfo))))
		outofmem(__FILE__, "HTVMSBrowseDir");
	    /** calloc zeros
	    entry_info->type = 0;
	    entry_info->size = 0;
	    entry_info->date = 0;
	    entry_info->filename = 0;
	    **/
	    entry_info->display = TRUE;

	    /* Get the type */
	    format = HTFileFormat(dirbuf->d_name, &encoding, www_plaintext,
				  &compressed);
	    if (!strncmp(HTAtom_name(format), "application", 11)) {
		cp = HTAtom_name(format) + 12;
		if (!strncmp(cp, "x-", 2))
		    cp += 2;
	    } else {
		cp = HTAtom_name(format);
	    }
	    StrAllocCopy(entry_info->type, cp);

	    StrAllocCopy(entry_info->filename, dirbuf->d_name);
	    if ((file_info.st_mode & S_IFMT) == S_IFDIR) {
	        /* Strip .DIR part... */
                char *ptr = strstr(entry_info->filename, ".DIR");

                if (ptr)
                   *ptr = '\0';
		ptr = entry_info->filename;
		while (*ptr) {
		    *ptr = TOLOWER(*ptr);
		    ptr++;
		}
		StrAllocCopy(entry_info->type, "Directory");
	    } else {
	        if (!(cp = strstr(entry_info->filename, "READ"))) {
	            cp = entry_info->filename;
		} else {
		    cp += 4;
		    if (!strncmp(cp, "ME", 2)) {
		        cp += 2;
			while (cp && *cp && (*cp != '.'))
			    cp++;
		    } else if (!strncmp(cp, ".ME", 3)) {
		        cp = entry_info->filename +
			     strlen(entry_info->filename);
		    } else {
		        cp = entry_info->filename;
		    }
		}
		while (*cp) {
		    *cp = TOLOWER(*cp);
		    cp++;
		}
		if (((len = strlen(entry_info->filename)) > 2) &&
		    entry_info->filename[len - 1] == 'z') {
		    if (entry_info->filename[len - 2] == '.' ||
		        entry_info->filename[len - 2] == '_')
			entry_info->filename[len - 1] = 'Z';
		}
	    }

	    /* Get the date */
	    {
	        char *t = (char *)ctime((WWW_CONST time_t *)
					&file_info.st_ctime);

		*(t + 24) = '\0';
	        StrAllocCopy(entry_info->date, t + 4);
		*(entry_info->date + 7) = '\0';
		if (atoi(t + 19) < atoi(ThisYear)) {
		    StrAllocCat(entry_info->date, t + 19);
		} else {
		    StrAllocCat(entry_info->date, t + 11);
		    *(entry_info->date + 12) = '\0';
		}
	    }

	    /* Get the size */
	    if ((file_info.st_mode & S_IFMT) != S_IFDIR) {
	        entry_info->size = (unsigned int)file_info.st_size;
	    } else {
	        entry_info->size = 0;
	    }
	    /* Now, update the BTree etc. */
	    if (entry_info->display) {
#ifndef DISABLE_TRACE
		if (www2Trace)
		    fprintf(stderr, "Adding file to BTree: %s\n",
			    entry_info->filename);
#endif
	        HTBTree_add(bt, (VMSEntryInfo *)entry_info); 
	    }
	}  /* End while HTVMSreaddir() */

	free(pathname);
	HTVMSclosedir(dp);

	START(HTML_PRE);
	/*
	 * Run through the BTree printing out in order
	 */
	{
	    HTBTElement *ele;
	    int i;

	    for (ele = HTBTree_next(bt, NULL); ele;
		 ele = HTBTree_next(bt, ele)) {
		entry_info = (VMSEntryInfo *)HTBTree_object(ele);

		/* Output the date */
		if (entry_info->date) {
		    PUTS(entry_info->date);
		    PUTS("  ");
		} else {
		    PUTS("     * ");
		}
		/* Output the type */
		if (entry_info->type) {
		    for (i = 0; entry_info->type[i] && (i < 15); i++)
		        PUTC(entry_info->type[i]);
		    for (; i < 17; i++)
		        PUTC(' ');
		}

		/* Output the link for the name */
		HTDirEntry(target, tail, entry_info->filename);  
		PUTS(entry_info->filename);
		END(HTML_A);

                /* Output the size */
		if (entry_info->size) {
		    if (entry_info->size < 1024) {
			sprintf(string_buffer, "  %d bytes", entry_info->size);
		    } else {
			sprintf(string_buffer, "  %dKb",
				entry_info->size / 1024);
		    }
		    PUTS(string_buffer);
		}
		PUTC('\n');  /* End of this entry */

		free_VMSEntryInfo_contents(entry_info);
	    }
	}
	HTBTreeAndObject_free(bt);
    }  /* End of both BTree loops */
    /*
     *  Complete the output stream.
     */
    END(HTML_PRE);
    END(HTML_BODY);
    END(HTML_HTML);
    free(tail);
    FREE_TARGET;

    return HT_LOADED;
}  /* End of directory reading section */
