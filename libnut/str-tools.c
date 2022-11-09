/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright (C) 1993, Board of Trustees of the University of Illinois      *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/

/* Copyright (C) 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#if defined(MULTINET) && defined(__DECC) && (__VMS_VER >= 70000000)
#define strdup  decc$strdup
#endif /* VMS V7.0 has, do before string.h, GEC */
#include <string.h>

#ifdef VMS   /* PGE */
#include <ctype.h>
#endif

#ifndef DEBUG

/* Use builtin strdup when appropriate -- code duplicated in tcp.h. */
/* DEC C V5.2 string.h has for VMS V7.0, GEC */
#if defined(ultrix) || (defined(VMS) && (!defined(__GNUC__) || defined(vax)) && (!defined(__DECC) || (__VMS_VER < 70000000) || (__DECC_VER <= 50230003))) || defined(NeXT)
extern char *strdup(char *str);
#endif

#else

#include "str-tools.h"

/* Copied from mo-www.c PGE */
/* VMS version 7.0 has strdup, GEC */
#if defined(ultrix) || (defined(VMS) && (!defined(__GNUC__) || defined(vax)) && (!defined(__VMS_VER) || (__VMS_VER < 70000000))) || defined(NeXT) || defined(M4310)
char *strdup(char *str)
{
  char *dup;

  if (!str || !(dup = (char *)malloc(strlen(str) + 1)))
    return NULL;

  return(strcpy(dup, str));
}
#endif

int main()
{
	char *bob, *newstr;

	bob = strdup("This is test %d.");
	newstr = strstrdup(bob, "%d", "1");

	printf("bob[%s]\nnew[%s]\n\n", bob, newstr);

	free(bob);
	free(newstr);

	bob = strdup("%d) This is test %d.");
	newstr = strstrdup(bob, "%d", "2");

	printf("bob[%s]\nnew[%s]\n\n", bob, newstr);

	free(bob);
	free(newstr);

	bob = strdup("This is test %d.");
	newstr = strstrdup(bob, "%d", "003");

	printf("bob[%s]\nnew[%s]\n\n", bob, newstr);

	bob = strdup("%d) This is test %d.");
	newstr = strstrdup(bob, "%d", "004");

	printf("bob[%s]\nnew[%s]\n\n", bob, newstr);

	bob = strdup("qwerty");
	printf("src[%s]\n", bob);
	newstr = my_chop(bob);
	printf("chopped[%s]\n\n", newstr);

	bob = strdup("qwerty  ");
	printf("src[%s]\n", bob);
	newstr = my_chop(bob);
	printf("chopped[%s]\n\n", newstr);

	bob = strdup("  qwerty  ");
	printf("src[%s]\n", bob);
	newstr = my_chop(bob);
	printf("chopped[%s]\n\n", newstr);

	exit(0);
}
#endif


char *getFileName(char *file_src)
{
	char *ptr;

	if (!file_src || !*file_src)
		return(NULL);

	ptr = strrchr(file_src, '/');

	if (!ptr || !*ptr)
		return(file_src);

	if (*ptr == '/' && *(ptr + 1))
		ptr++;

#ifdef VMS /* GEC */
        /*
        ** Handle special case of starting mosaic with an URL on the command
        ** line like "http://hostname/" in order to prevent a file name of
        ** "device:[directory]/".
        */
        if (*ptr == '/')
        	return("");
        /*
        ** Handle case of filename having multiple dots, which is invalid on
        ** VMS. Remove all but the last dot (like unzip).
        */
        {
           char *ptr2;
	   char *ptr3 = strrchr(ptr, '.');   /* Pointer to last dot. */

           /* Handle special cases of compressed files, PGE */
           if (ptr3 && !strcmp(ptr3, ".gz")) {
              *ptr3 = '-';               /* a.b.c.gz -> a_b.c-gz */
              ptr3 = strrchr(ptr, '.');  /* Pointer to last dot. */
           } else if (ptr3 && !strcmp(ptr3, ".Z")) {
              *ptr3 = '_';               /* a.b.c.Z -> a_b.c_Z */
              ptr3 = strrchr(ptr, '.');  /* Pointer to last dot. */
           }

           ptr2 = strchr(ptr, '.');    /* Pointer to first dot. */
           while (ptr2 != ptr3) {
              *ptr2 = '_';
              ptr2 = strchr(ptr, '.');
           }

           /* Remove version number of VMS files, PGE */
           ptr3 = strrchr(ptr, ';');   /* Pointer to last semicolon. */
           if (ptr3)
              *ptr3 = '\0';

	   /* Remove dir spec, if one exists */
           ptr3 = strrchr(ptr, ']');   /* Pointer to end of dir spec. */
	   if (ptr3)
	      ptr = ++ptr3;
        }
#endif
	return(ptr);
}


/*
 * Will casefully search forward through a string for a character.
 *
 * Must be a null-terminated string.
 *
 * SWP
 */
char *strcasechr(char *src, char srch)
{
	char *ptr;
	char tmp;

	if (!src || !*src)
		return(NULL);

	tmp = toupper(srch);

	for (ptr = src; (*ptr && toupper(*ptr) != tmp); ptr++)
		;
	/*
	 * At this point, either *ptr == \0 (failure) or toupper(*ptr) is
	 *   == to tmp (success).  Return accordingly.
	 */
	if (*ptr)
		return(ptr);

	return(NULL);
}


/*
 * Will casefully search backward through a string for a character.
 *
 * Must be a null-terminated string.
 *
 * SWP
 */
char *strrcasechr(char *src, char srch)
{
	char *ptr;
	char tmp;

	if (!src || !*src)
		return(NULL);

	tmp = toupper(srch);

	for (ptr = (src + strlen(src) - 1);
	     (ptr > src && toupper(*ptr) != tmp); ptr--)
		;
	/*
	 * At this point we have either found toupper(*ptr) == to tmp, or we
	 * are at the very begining of the string.  So, if ptr is != to src,
	 * we found a match...or...we need to test to make sure the first
	 * char in the string is not the match.  Return accordingly.
	 */
	if ((ptr != src) || (toupper(*ptr) == tmp))
		return(ptr);

	return(NULL);
}


char *strstrdup(char *src, char *srch, char *rplc)
{
	char *dest, *start, *local, *next, *found;
	int rplcLen = 0;
	int i, srchLen;

	if (!src || !*src || !srch || !*srch)
		return(NULL);

	if (rplc && *rplc)
		rplcLen = strlen(rplc);

	srchLen = strlen(srch);

	if (rplcLen > srchLen) {
		dest = (char *)malloc(sizeof(char));
	} else {
		dest = strdup(src);
	}
	*dest = '\0';

	start = local = strdup(src);
	while (*start) {
		if (!(found = strstr(start, srch))) {
			if (rplcLen > srchLen) {
				realloc((void *)dest,
					(strlen(dest) + strlen(start) + 4) *
					sizeof(char));
				strcat(dest, start);
			} else {
				strcat(dest, start);
			}
			free(local);
			return(dest);
		}

		for (i = 0, next = found; i < srchLen; i++, next++)
			;
		*found = '\0';
		if (rplcLen > srchLen) {
			realloc((void *)dest,
				(rplcLen + strlen(dest) + strlen(start) + 4) *
				 sizeof(char));
			strcat(dest, start);
			if (rplcLen)
				strcat(dest, rplc);
		} else {
			strcat(dest, start);
			strcat(dest, rplc);
		}
		start = next;
	}
	return(dest);
}


char **string_to_token_array(char *str, char *delimiter)
{
  char **array;
  char *tmp;
  int num = 1;
  int i = 0;

  if (!str || !*str || !delimiter || !*delimiter)
      return NULL;

  /* First get number of tokens */
  tmp = strstr(str, delimiter);
  tmp++;
  while (tmp = strstr(tmp, delimiter)) {
      tmp++;
      num++;
  }

  array = malloc(sizeof(char *) * (num + 2));
  *array = strdup(strtok(str, delimiter));

  while (array[++i] = strdup(strtok((char *) NULL, delimiter)))
      ;

  free(str);
  return array;
}

char *my_strndup(char *str, int num)
{
  char *nstr;

  if (!str)
      return NULL;

  nstr = malloc(sizeof(char) * (num + 1));

  strncpy(nstr, str, num);
  nstr[num] = '\0';
  return nstr;
}

char *my_chop(char *str)
{
  char *ptr;

  if (!str || !*str)
      return str;

  /* Remove blank space from end of string. */
  ptr = str + strlen(str) - 1;
  while ((ptr >= str) && isspace(*ptr))
      *ptr-- = '\0';

  /* Remove blank space from start of string. */
  ptr = str;
  while(isspace(*ptr))
      ptr++;

  /*
  ** If there was blank space at start of string then move string back to the
  ** beginning.  This prevents memory freeing problems later if pointer is
  ** moved.  memmove is used because it is safe for overlapping regions.
  */
  if (ptr != str)
      memmove(str, ptr, strlen(ptr) + 1);

  return str;
}

/* Remove both spaces and tabs from a string */
int removeblanks(char *str)
{
  int i;
  int j = 0;

  if (!str || !*str)
      return 0;

  for (i = 0; str[i]; i++) {
      if ((str[i] != ' ') && (str[i] != '\t'))
          str[j++] = str[i];
  }

  if (i > j) {
      str[j] = '\0';
      return 1;
  }
  return 0;
}
 
int my_strcasecmp(char *str1, char *str2)
{
  int i, min, offset1, offset2;
  int length1, length2;

  if (!str1 || !str2 || !*str1 || !*str2)
      return 1;

  /* Find shortest string to make sure we don't go past null */
  min = length1 = strlen(str1);
  if ((length2 = strlen(str2)) < min)
      min = length2;

  for (i = 0; i < min; i++) {
      /* Use offsets to make everything lower case */

      if ((str1[i] >= 'A') && (str1[i] <= 'Z')) {
	  offset1 = 32;
      } else {
	  offset1 = 0;
      }
      if ((str2[i] >= 'A') && (str2[i] <= 'Z')) {
	  offset2 = 32;
      } else {
	  offset2 = 0;
      }
      if (str1[i] + offset1 != str2[i] + offset2)
	  return (str1[i] + offset1) - (str2[i] + offset2);
  }
  /* They're equal up to min */
  return length1 - length2;
}


int my_strncasecmp(char *str1, char *str2, int n)
{
  int i, offset1, offset2, length1, length2;
  int min = n;

  if (!str1 || !str2 || !*str1 || !*str2 || !n)
      return 1;

  if ((length1 = strlen(str1)) < min)
      min = length1;

  if ((length2 = strlen(str2)) < min)
      min = length2;

  /* If they provide an n too big, go with the smallest safe value (min)
   * This is how it works on my system at least */
  if (min > n)
      min = n;

  for (i = 0; i < min; i++) {
      if ((str1[i] >= 'A') && (str1[i] <= 'Z')) {
	  offset1 = 32;
      } else {
	  offset1 = 0;
      }
      if ((str2[i] >= 'A') && (str2[i] <= 'Z')) {
	  offset2 = 32;
      } else {
	  offset2 = 0;
      }
      if (str1[i] + offset1 != str2[i] + offset2)
	  return (str1[i] + offset1) - (str2[i] + offset2);
  }

  /* They're equal up to min */
  if (min >= n)
      return 0; 

  /* If they specified n greater than min return the shorter string
   * as lexiographically smaller */
  return length1 - length2;
}
