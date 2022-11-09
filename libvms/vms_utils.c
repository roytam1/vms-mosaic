/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%                            V   V  M   M  SSSSS                              %
%                            V   V  MM MM  SS                                 %
%                            V   V  M M M   SSS                               %
%                             V V   M   M     SS                              %
%                              V    M   M  SSSSS                              %
%                                                                             %
%                    VMS Utility Routines for ImageMagick.                    %
%                                                                             %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                October 1994                                 %
%                                                                             %
%                                                                             %
%  Copyright 1997 E. I. du Pont de Nemours and Company                        %
%                                                                             %
%  Permission to use, copy, modify, distribute, and sell this software and    %
%  its documentation for any purpose is hereby granted without fee,           %
%  provided that the above Copyright notice appear in all copies and that     %
%  both that Copyright notice and this permission notice appear in            %
%  supporting documentation, and that the name of E. I. du Pont de Nemours    %
%  and Company not be used in advertising or publicity pertaining to          %
%  distribution of the software without specific, written prior               %
%  permission.  E. I. du Pont de Nemours and Company makes no representations %
%  about the suitability of this software for any purpose.  It is provided    %
%  "as is" without express or implied warranty.                               %
%                                                                             %
%  E. I. du Pont de Nemours and Company disclaims all warranties with regard  %
%  to this software, including all implied warranties of merchantability      %
%  and fitness, in no event shall E. I. du Pont de Nemours and Company be     %
%  liable for any special, indirect or consequential damages or any           %
%  damages whatsoever resulting from loss of use, data or profits, whether    %
%  in an action of contract, negligence or other tortious action, arising     %
%  out of or in connection with the use or performance of this software.      %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The directory routines are strongly based on similiar routines written
%  by Rich Salz.
%
*/

/* Modified for use with VMS Mosaic by George Cook, August 2008 */

#include "../config.h"

#ifndef DISABLE_TRACE
extern int reportBugs;
extern int srcTrace;
#endif

#include <ssdef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vms_utils.h"
#include "../libnut/str-tools.h"

#include <ctype.h>
#include <descrip.h>
#include <file.h>
#include <lib$routines.h>
#include <lnmdef.h>
#include <starlet.h>
#if defined(__DECC) && (__VMS_VER >= 70000000)
#define _VMS_V6_SOURCE
#endif    /* avoid __UTC_STAT in VMS V7.0, GEC */
#include <stat.h>
#if defined(__DECC) && (__VMS_VER >= 70000000)
#undef _VMS_V6_SOURCE
#endif
#include <types.h>
#include <unixio.h>

#define $NEW_DESCRIPTOR(name) \
	struct dsc$descriptor_s name = { \
		0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 \
	}

/* Use builtin strdup when appropriate. */
/* DEC C V5.2 string.h has for VMS V7.0 */
#if (defined(VMS) && (!defined(__GNUC__) || defined(vax)) && (!defined(__DECC) || (__VMS_VER < 70000000) || (__DECC_VER <= 50230003)))
extern char *strdup(char *str);
#endif

char *VMSopendir_dirname = NULL;

/* Returns static memory in ret */
int trans_VMSlogical(char **ret, char *name)
{
  typedef struct {
    unsigned short int  length;
    unsigned short int  code;
    char               *buffer;
    unsigned short int *return_length;
  } ITEMLIST;

  int                status;
  ITEMLIST           item_list[2];
  static char        translated_home_data[256];
  unsigned short int translated_home_length;
  unsigned long      attrib = LNM$M_CASE_BLIND;
  static char        logical_table_data[] = "LNM$FILE_DEV";
  $NEW_DESCRIPTOR(logical_table);
  $NEW_DESCRIPTOR(home_logical);

  /* Setup values to pass into sys$trnlnm */
  logical_table.dsc$w_length = strlen(logical_table_data);
  logical_table.dsc$a_pointer = logical_table_data;
  home_logical.dsc$w_length = strlen(name);
  home_logical.dsc$a_pointer = name;

  /* Setup values to return from sys$trnlnm */
  item_list[0].code = LNM$_STRING;
  item_list[0].length = 256;
  item_list[0].buffer = translated_home_data;
  item_list[0].return_length = &translated_home_length;
  item_list[1].code = 0;
  item_list[1].length = 0;

  /* Translate logical to DISK:[DIRECTORY] */
  status = sys$trnlnm(&attrib, &logical_table, &home_logical, 0, item_list);
  if (status == SS$_NOLOGNAM)
      return(0);

  if (status != SS$_NORMAL) {
#ifndef DISABLE_TRACE
      if (reportBugs)
          fprintf(stderr, "trans_VMSlogical failed with status = %d\n", status);
#endif
      return(0);
  }

  /* Convert to null terminated C style string. */
  translated_home_data[translated_home_length] = '\0';

  *ret = translated_home_data;   /* He better not free it */
  return(1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   c l o s e d i r                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function closedir closes the named directory stream and frees the DIR
%  structure.
%
%  The format of the closedir routine is:
%
%      VMSclosedir(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
void VMSclosedir(DIR *directory)
{
  if (!directory)
      return;
  (void) lib$find_file_end(&directory->context);
  free(directory->pattern);
  free((char *) directory);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   o p e n d i r                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function opendir opens the directory named by filename and associates
%  a directory stream with it.
%
%  The format of the opendir routine is:
%
%      VMSopendir(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
DIR *VMSopendir(char *name)
{
  DIR *directory = (DIR *) malloc(sizeof(*directory));
  char *newname;

  if (!directory || !name)
      return(NULL);

  /* So we can free it as needed */
  name = strdup(name);

  if (!strcmp(".", name)) {
      *name = '\0';
  } else {
      int len = strlen(name);
      char *ptr;
      char *new2;

      /* Make Unix name into VMS name */
      if (*name == '/') {
	  newname = malloc(len + 2);
	  strcpy(newname, name + 1);
	  if (!(ptr = strchr(newname, '/')) || !*(ptr + 1)) {
	      /* No more / or only one at the end */
	      strcat(newname, ":");
	  } else {
	      *ptr = '\0';
	      new2 = malloc(len + 4);
	      strcpy(new2, newname);
	      free(newname);
	      newname = new2;
	      strcat(new2, ":[");
	      strcat(new2, ptr + 1);
	      len = strlen(new2);
	      if (*(new2 + len - 1) == '/') {
		  *(new2 + len - 1) = ']';
	      } else {
		  *(new2 + len) = ']';
		  *(new2 + len + 1) = '\0';
	      }
	      while (ptr = strchr(new2, '/'))
		  *ptr = '.';
	  }
 	  free(name);
	  name = newname;
          len = strlen(name);
      }
      /* Turn .dir file into directory format if needed */
      if ((len > 4) && !my_strcasecmp((char *)((int)name + len - 4), ".dir")) {
	  newname = calloc(1, len);
	  *(char *)((int)name + len - 4) = '\0';
	  if (ptr = strchr(name, ']')) {
	      *ptr = '.';
	      strcpy(newname, name);
	  } else if (ptr = strchr(name, ':')) {
	      *ptr = '\0';
	      strcpy(newname, name);
	      strcat(newname, ":[");
	      strcat(newname, ptr + 1);
	  } else {
	      *new2 = '[';
	      strcat(newname, name);
	  }
	  strcat(newname, "]");
	  free(name);
	  name = newname;
          len = strlen(name);
      }
      /* Now translate logical name if nothing after ':' */
      if (*(ptr = name + len - 1) == ':') {
	  *ptr = '\0';
	  /* trans_VMSlogical returns static memory */
	  if (trans_VMSlogical(&new2, name)) {
	      free(name);
	      name = strdup(new2);
	  } else {
              *ptr = ':';
	  }
      }
  }

#ifndef DISABLE_TRACE
  if (srcTrace)
      fprintf(stderr, "VMSopendir: dir = %s\n", name);
#endif

  directory->pattern = malloc((unsigned int)(strlen(name) + sizeof("*.*") + 1));
  if (!directory->pattern) {
      free((char *) directory);
      free(name);
      return(NULL);
  }

  /* Save the translated name */
  if (VMSopendir_dirname)
      free(VMSopendir_dirname);
  VMSopendir_dirname = name;

  /*
    Initialize descriptor.
  */
  (void) sprintf(directory->pattern, "%s*.*", name);
  directory->context = 0;
  directory->pat.dsc$a_pointer = directory->pattern;
  directory->pat.dsc$w_length = strlen(directory->pattern);
  directory->pat.dsc$b_dtype = DSC$K_DTYPE_T;
  directory->pat.dsc$b_class = DSC$K_CLASS_S;
  return(directory);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   r e a d d i r                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function readdir returns a pointer to a structure representing the
%  directory entry at the current position in the directory stream to
%  which entry refers.
%
%  The format of the readdir
%
%      VMSreaddir(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
struct dirent *VMSreaddir(DIR *directory)
{
  char buffer[sizeof(directory->entry.d_name)];
  int status;
  register char *p;
  register int i;
  struct dsc$descriptor_s result;

  /*
    Initialize the result descriptor.
  */
  result.dsc$a_pointer = buffer;
  result.dsc$w_length = sizeof(buffer) - 2;
  result.dsc$b_dtype = DSC$K_DTYPE_T;
  result.dsc$b_class = DSC$K_CLASS_S;
  status = lib$find_file(&directory->pat, &result, &directory->context);
  if ((status == RMS$_NMF) || (status == RMS$_DNF) ||
      (directory->context == 0L))
      return((struct dirent *) NULL);
  /*
    Lowercase all filenames.
  */
  buffer[sizeof(buffer) - 1] = '\0';
  for (p = buffer; *p; p++) {
      if (isupper(*p))
          *p = tolower(*p);
  }
  /*
    Skip any directory component and just copy the name.
  */
  p = buffer;
  while (!isspace(*p))
      p++;
  *p = '\0';
  p = strchr(buffer, ']');
  if (p) {
      (void) strcpy(directory->entry.d_name, p + 1);
  } else {
      (void) strcpy(directory->entry.d_name, buffer);
  }
  /*
    Remove the file version number.
  */
  if (p = strchr(directory->entry.d_name, ';'))
      *p = '\0';
  directory->entry.d_namlen = strlen(directory->entry.d_name);
  return(&directory->entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   r e w i n d d i r                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function rewinddir restarts the directory named by filename and
%  associates a directory stream with it.
%
%  The format of the opendir routine is:
%
%      VMSrewinddir(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%
*/
void VMSrewinddir(DIR *directory)
{
  if (!directory)
      return;

  (void) lib$find_file_end(&directory->context);
  /*
    Reinitialize descriptor.
  */
  directory->context = 0;
}

/* Basename routine for VMS */
char *VMSbasename(char *path)
{
  char *cp;

  if (!path || !*path)
      return(".");
  if ((*path == '/') && !*(path + 1))
      return path;
  if (cp = strrchr(path, '/'))
      return cp + 1;
  return path;
}
