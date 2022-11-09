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

/* Copyright (C) 1998, 1999, 2000, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include "../config.h"
#include "mosaic.h"
#include "globalhist.h"
#include "mo-www.h"
#include "../libhtmlw/HTMLP.h"
#include <time.h>
#include "../libnut/system.h"
#include "../libnut/str-tools.h"
#include "img.h"

/* For memset */
#ifndef VMS
#include <memory.h>
#endif

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int cacheTrace;
#endif

/* Old compilers choke on const... */
#ifdef __STDC__
#define MO_CONST const
#else
#define MO_CONST
#endif

/* ------------------------------------------------------------------------ */
/* ---------------------------- GLOBAL HISTORY ---------------------------- */
/* ------------------------------------------------------------------------ */

/* We save history list out to a file (~/.mosaic-global-history) and
   reload it on entry.

   Initially the history file format will look like this:

   ncsa-mosaic-history-format-1            [identifying string]
   Global                                  [title]
   url Fri Sep 13 00:00:00 1986            [first word is url;
                                            subsequent words are
                                            last-accessed date (GMT)]
   [1-line sequence for single document repeated as necessary]
   ...

 --Format 2--02/15/96--SWP
   ncsa-mosaic-history-format-2            [identifying string]
   Global                                  [title]
   url seconds                             [first word is url;
                                            subsequent word is the
					    seconds since unix birth
					    at last access]
   [1-line sequence for single document repeated as necessary]
   ...
*/

#define NCSA_HISTORY_FORMAT_COOKIE_ONE "ncsa-mosaic-history-format-1"
#define NCSA_HISTORY_FORMAT_COOKIE_TWO "ncsa-mosaic-history-format-2"

#define HASH_TABLE_SIZE 400

/* Cached data in a hash entry for a given URL; one or both
 * slots can be filled; non-filled slots will be NULL. */
typedef struct cached_data {
  void *image_data;
  cached_frame_data *frame_data;
  int last_access;
  int usage_count;
} cached_data;

/* An entry in a hash bucket, containing a URL (in canonical,
 * absolute form) and possibly cached info (right now, an ImageInfo
 * struct for inlined images). */
typedef struct entry {
  /* Canonical URL for this document. */
  char *url;
  char *lastdate;
  /* This could be one of a several things:
   * for an image, it's the ImageInfo struct */
  cached_data *cached_data;
  struct entry *next;
} entry;

/* A bucket in the hash table; contains a linked list of entries. */
typedef struct bucket {
  entry *head;
  int count;
} bucket;

static bucket hash_table[HASH_TABLE_SIZE];

static mo_status mo_cache_image_data(cached_data *cd, void *info);
static mo_status mo_uncache_image_data(cached_data *cd);
static int mo_kbytes_in_image_data(void *image_data);

static int access_counter = 0;
static int dont_nuke_after_me = 0;
static int kbytes_cached = 0;


/* Given a character string of time, is this older than URLEXPIRED? */
static int notExpired(char *lastdate)
{
  time_t curtime = time(NULL);
  static long expired;
  static int init = 0;

  if (!init) {
      expired = get_pref_int(eURLEXPIRED) * 86400;
      init = 1;
  }
  if (expired <= 0)
      return(1);

  if ((curtime - atol(lastdate)) >= expired) {
#ifndef DISABLE_TRACE
      if (srcTrace)
	  fprintf(stderr, "EXPIRED! [%ld] - [%ld] = [%ld] (%ld)\n",
		  curtime, atol(lastdate), curtime - atol(lastdate), expired);
#endif
      return(0);
  }
  return(1);
}


/* Given a URL, hash it and return the hash value, mod'd by the size
 * of the hash table. */
static int hash_url(char *url)
{
  int len, i;
  int val = 0;

  if (!url)
      return 0;
  len = strlen(url);
  for (i = 0; i < 10; i++)
      val += url[(i * val + 7) % len];

  return val % HASH_TABLE_SIZE;
}


/* Assume url isn't already in the bucket; add it by
 * creating a new entry and sticking it at the head of the bucket's
 * linked list of entries. */
static void add_url_to_bucket(int buck, char *url, char *lastdate)
{
  bucket *bkt = &hash_table[buck];
  entry *l = (entry *)calloc(1, sizeof(entry));

  if (url)
      l->url = strdup(url);
  if (lastdate)
      l->lastdate = strdup(lastdate);

  /** calloc zeroes them
  l->cached_data = NULL;
  l->next = NULL;
  **/
  
  if (bkt->head)
       l->next = bkt->head;
  bkt->head = l;

  bkt->count++;
}


/* This is the internal predicate that takes a URL, hashes it,
 * does a search through the appropriate bucket, and either returns
 * 1 or 0 depending on whether we've been there.
 */
static int been_here_before(char *url, int hash)
{
  if (hash_table[hash].count) {
      entry *l;

      for (l = hash_table[hash].head; l; l = l->next) {
          if (!strcmp(l->url, url)) {
	      time_t foo = time(NULL);
	      char ts[30];

	      /* We need to update the date */
	      sprintf(ts, "%ld", foo);

	      if (l->lastdate)
	          free(l->lastdate);
	      l->lastdate = strdup(ts);
              return 1;
	  }
      }
  }
  return 0;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

/****************************************************************************
 * name:    mo_been_here_before_huh_dad
 * purpose: Predicate to determine if we've visited this URL before.
 * inputs:  
 *   - char *url: The URL.
 * returns: 
 *   mo_succeed if we've been here before; mo_fail otherwise
 * remarks: 
 *   We canonicalize the URL keeping the target anchor, if one exists.
 ****************************************************************************/
mo_status mo_been_here_before_huh_dad(char *url)
{
  char *curl;
  mo_status status;

  /* Only http:, https: and file: can have target anchors */
  if (!my_strncasecmp(url, "http:", 5) || !my_strncasecmp(url, "https:", 6) ||
      !my_strncasecmp(url, "file:", 5)) {
      curl = mo_url_canonicalize_keep_anchor(url, "");
  } else {
      curl = mo_url_canonicalize(url, "");
  }
  if (been_here_before(curl, hash_url(curl))) {
      status = mo_succeed;
  } else {
      status = mo_fail;
  }
  free(curl);
  return status;
}


/****************************************************************************
 * name:    mo_here_we_are_son
 * purpose: Add a URL to the global history, if it's not already there.
 * inputs:  
 *   - char *url: URL to add.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   We canonicalize the URL (stripping out the target anchor, 
 *   if one exists).
 ****************************************************************************/
mo_status mo_here_we_are_son(char *url)
{
  char *curl = mo_url_canonicalize(url, "");
  int hash;

  hash = hash_url(curl);

  if (!been_here_before(curl, hash)) {
      time_t foo = time(NULL);
      char ts[30];

      sprintf(ts, "%ld", foo);
      add_url_to_bucket(hash, curl, ts);
  }
  free(curl);
  return mo_succeed;
}


/****************************************************************************
 * name:    mo_track_url_anchors_visited
 * purpose: Add an anchored URL to global history, if it's not already there.
 * inputs:  
 *   - char *url: URL to add.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   We canonicalize the URL keeping the target anchor.
 ****************************************************************************/
mo_status mo_track_url_anchors_visited(char *url)
{
  char *curl = mo_url_canonicalize_keep_anchor(url, "");
  int hash;

  hash = hash_url(curl);

  if (!been_here_before(curl, hash)) {
      time_t foo = time(NULL);
      char ts[30];

      sprintf(ts, "%ld", foo);
      add_url_to_bucket(hash, curl, ts);
  }
  free(curl);
  return mo_succeed;
}


/****************************************************************************
 * name:    mo_read_global_history (PRIVATE)
 * purpose: Given a filename, read the file's contents into the
 *          global history hash table.
 * inputs:  
 *   - char *filename: The file to read.
 * returns: 
 *   nothing
 * remarks: 
 *   
 ****************************************************************************/
static void mo_read_global_history(char *filename)
{
  FILE *fp;
  char line[MO_LINE_LENGTH];
  char *status, *url, *lastdate;
  int format;

  if (!(fp = fopen (filename, "r"))) {
      return;
  } else if (get_pref_boolean(eBACKUP_FILES)) {
      char *tf;
      char retBuf[BUFSIZ];

#ifndef VMS
      tf = (char *)calloc(strlen(filename) + strlen(".backup") + 5,
			  sizeof(char));
      sprintf(tf, "%s.backup", filename);
      if (my_copy(filename, tf, retBuf, BUFSIZ - 1, 1) != SYS_SUCCESS) {
#else
      tf = (char *)calloc(strlen(filename) + strlen("_backup") + 5,
			  sizeof(char));
      sprintf(tf, "%s_backup", filename);
      if (my_copy(filename, tf, retBuf, BUFSIZ - 1,
		  get_pref_int(eBACKUPFILEVERSIONS)) != SYS_SUCCESS) {
#endif /* VMS, Bob, GEC */
          fprintf(stderr, "%s\n", retBuf);
      }
      free(tf);
  }

  status = fgets(line, MO_LINE_LENGTH, fp);
  if (!status || !*line)
      goto screwed_with_file;
  
  /* See if it's our format. */
  if (strncmp(line, NCSA_HISTORY_FORMAT_COOKIE_ONE,
              strlen(NCSA_HISTORY_FORMAT_COOKIE_ONE))) {
      if (strncmp(line, NCSA_HISTORY_FORMAT_COOKIE_TWO,
		  strlen(NCSA_HISTORY_FORMAT_COOKIE_TWO))) {
	  goto screwed_with_file;
      } else {
	  format = 2;
      }
  } else {
      format = 1;
  }

  /* Go fetch the name on the next line. */
  status = fgets(line, MO_LINE_LENGTH, fp);
  if (!status || !*line)
      goto screwed_with_file;
  
  /* Start grabbing url's. */
  while (1) {
      status = fgets(line, MO_LINE_LENGTH, fp);
      if (!status || !*line)
          goto done;
      
      url = line;
      /* Must be at least one space, but could be more */
      lastdate = strrchr(line, ' ');
      if (!lastdate)
          goto screwed_with_file;

      /* Terminate the URL */
      *lastdate++ = '\0';
      lastdate = strtok(lastdate, "\n");
      if (!lastdate)
          goto screwed_with_file;

      if (notExpired(lastdate) || (format == 1))
	  add_url_to_bucket(hash_url(url), url, lastdate);
  }
  
 screwed_with_file:
 done:
  fclose(fp);
  return;
}


/****************************************************************************
 * name:    mo_init_global_history
 * purpose: Initialize the global history hash table.
 * inputs:  
 *   none
 * returns: 
 *   mo_succeed
 * remarks: 
 *   
 ****************************************************************************/
mo_status mo_init_global_history(void)
{
  memset(hash_table, 0, sizeof(hash_table));
  return mo_succeed;
}


/****************************************************************************
 * name:    mo_flush_image_cache
 * purpose: 
 * inputs:
 * returns: 
 *   nuthin
 * remarks: 
 ****************************************************************************/
mo_status mo_flush_image_cache(void)
{
  entry *l;
  int hash;

  for (hash = 0; hash < HASH_TABLE_SIZE; hash++) {
      for (l = hash_table[hash].head; l; l = l->next) {
          if (l->cached_data && l->cached_data->image_data)
              mo_uncache_image_data(l->cached_data);
      }
  }
  return mo_succeed;
}


/****************************************************************************
 * name:    mo_flush_frame_cache
 * purpose: Flush all frame data in the cache
 * inputs:  
 * returns: 
 * remarks: 
 ****************************************************************************/
static mo_status mo_flush_frame_cache(void)
{
  entry *l;
  int hash;

  for (hash = 0; hash < HASH_TABLE_SIZE; hash++) {
      for (l = hash_table[hash].head; l; l = l->next) {
          if (l->cached_data && l->cached_data->usage_count) {
	      if (l->cached_data->frame_data) {
		  cached_frame_data *data = l->cached_data->frame_data;

		  if (data->text)
		      free(data->text);
		  if (data->last_modified)
		      free(data->last_modified);
		  if (data->expires)
		      free(data->expires);
		  if (data->charset)
		      free(data->charset);
                  free(data);
	          l->cached_data->frame_data = NULL;
	      }
	      l->cached_data->usage_count = 0;
          }
      }
  }
  return mo_succeed;
}


/****************************************************************************
 * name:    mo_wipe_global_history
 * purpose: Wipe out the current global history.
 * inputs:  
 *   none
 * returns: 
 *   mo_succeed
 * remarks: 
 *   Huge memory hole here.  However, we now call
 *   mo_flush_image_cache to at least clear out the image structures.
 ****************************************************************************/
mo_status mo_wipe_global_history(mo_window *win)
{
  mo_flush_image_cache();
  mo_flush_frame_cache();

  /* Memory leak! @@@ */
  mo_init_global_history();

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_setup_global_history
 * purpose: Called on program startup to do the global history
 *          initialization stuff, including figuring out where the
 *          global history file is and reading it.
 * inputs:  
 *   none
 * returns: 
 *   mo_succeed
 * remarks: 
 *   
 ****************************************************************************/
static char *cached_global_hist_fname = NULL;

mo_status mo_setup_global_history(void)
{
  char *home = getenv("HOME");
  char *default_filename = get_pref_string(eGLOBAL_HISTORY_FILE);
  char *history_file = get_pref_string(eHISTORY_FILE);
  char *filename;
  FILE *fp;

  mo_init_global_history();

  /* This shouldn't happen. */
#ifndef VMS
  if (!home)
      home = "/tmp";
#endif
  
  filename = (char *)malloc((strlen(home) + strlen(history_file) + 8) *
	     		    sizeof(char));
#ifndef VMS
  sprintf (filename, "%s/%s", home, history_file);
#else
  sprintf (filename, "%s%s", home, history_file);
#endif /* VMS, BSN */

  /* The one we will write to at exit */
  cached_global_hist_fname = strdup(filename);

  if (!(fp = fopen(filename, "r"))) {
      char *tmpnam = (char *)malloc((strlen(home) + strlen(default_filename) +
		     		     8) * sizeof(char));

#ifndef VMS
      sprintf(tmpnam, "%s/%s", home, default_filename);
#else
      sprintf(tmpnam, "%s%s", home, default_filename);
#endif /* VMS, BSN */
      if (fp = fopen(tmpnam, "r")) {
	  printf("\n\n---------------New History Format---------------\n\n");
	  printf("Mosaic needs to update your history file to a new format\n");
	  printf("  which will enable links to expire after %d days (see\n",
		 get_pref_int(eURLEXPIRED));
	  printf("  the resource 'Mosaic*urlExpired').\n\n");
	  printf("Your current history file will still exist and will not\n");
	  printf("  be modified. However, it will no longer be updated.\n");
#ifndef VMS
	  printf("  Instead, the file '.mosaic-x-history' will be used.\n\n");
#else
	  printf("  Instead, the file '%s' will be used.\n\n", history_file);
#endif /* VMS, BSN */

	  free(filename);
	  filename = tmpnam;
          fclose(fp);
      } else {
	  free(tmpnam);
      }
  } else {
      fclose(fp);
  }
  mo_read_global_history(filename);

  free(filename);

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_write_global_history
 * purpose: Write the global history file out to disk.
 * inputs:  
 *   none
 * returns: 
 *   mo_succeed (usually)
 * remarks: 
 *   This assigns last-read times to all the entries in the history,
 *   which is a bad thing.
 *   --- Not anymore --- SWP
 ****************************************************************************/
mo_status mo_write_global_history(void)
{
  FILE *fp;
  int i;
  entry *l;
  time_t foo = time(NULL);
  char ts[30];

  if (!cached_global_hist_fname)
      return mo_fail;

  sprintf(ts, "%ld", foo);

#ifdef VMS
  remove(cached_global_hist_fname);
  fp = fopen(cached_global_hist_fname, "w", "shr = nil", "rop = WBH", "mbf = 4",
	     "mbc = 32", "deq = 64", "alq = 64", "fop = tef");
#else
  fp = fopen(cached_global_hist_fname, "w");
#endif
  if (!fp)
      return mo_fail;

  fprintf(fp, "%s\n%s\n", NCSA_HISTORY_FORMAT_COOKIE_TWO, "Global");
  
  for (i = 0; i < HASH_TABLE_SIZE; i++) {
      for (l = hash_table[i].head; l; l = l->next)
          fprintf(fp, "%s %s\n", l->url,
		  isdigit(*l->lastdate) ? l->lastdate : ts);
  }
  fclose(fp);
  
  return mo_succeed;
}


/****************************************************************************
 * name:    mo_fetch_cached_image_data
 * purpose: Retrieve a piece of cached data associated with a URL.
 * inputs:  
 *   - char *url: The URL.
 * returns: 
 *   The piece of cached data (void *).
 * remarks: 
 *   We do *not* do anything to the URL.  If there is a target
 *   anchor in it, fine with us.  This means the target anchor
 *   should have been stripped out someplace else if it needed to be.
 ****************************************************************************/
void *mo_fetch_cached_image_data(char *url)
{
  int hash = hash_url(url);
  entry *l;

  if (!hash_table[hash].count)
      return NULL;

  for (l = hash_table[hash].head; l; l = l->next) {
      if (!strcmp(l->url, url)) {
          if (l->cached_data && l->cached_data->image_data) {
#ifndef DISABLE_TRACE
              if (cacheTrace)
                  fprintf(stderr,
		         "[mo_fetch_cached_image_data] Hit '%s', data 0x%08x\n",
		         url, l->cached_data->image_data);
#endif
              l->cached_data->last_access = access_counter++;
              return l->cached_data->image_data;
          } else {
#ifndef DISABLE_TRACE
              if (cacheTrace)
                  fprintf(stderr,
			  "[mo_fetch_cached_image_data] Miss '%s'\n", url);
#endif
              break;
          }
      }
  }
  return NULL;
}


/****************************************************************************
 * name:    mo_fetch_cached_frame_data
 * purpose: Retrieve a piece of cached data associated with a URL.
 * inputs:  
 *   - char *url: The URL.
 * returns: 
 *   The piece of cached data (void *).
 * remarks: 
 *   We do *not* do anything to the URL.  If there is a target
 *   anchor in it, fine with us.  This means the target anchor
 *   should have been stripped out someplace else if it needed to be.
 ****************************************************************************/
cached_frame_data *mo_fetch_cached_frame_data(char *url)
{
  int hash = hash_url(url);

  if (hash_table[hash].count) {
      entry *l;

      for (l = hash_table[hash].head; l; l = l->next) {
          if (!strcmp(l->url, url)) {
              if (l->cached_data) {
                  return l->cached_data->frame_data;
              } else {
                  break;
	      }
          }
      }
  }  
  return NULL;
}


/****************************************************************************
 * name:    mo_replace_cached_frame_data
 * purpose: Replace cached frame data associated with a window.
 * inputs:  
 *   - mo_window *win
 * returns: 
 *   mo_succeed unless failed.
 * remarks:
 *
 ****************************************************************************/
mo_status mo_replace_cached_frame_data(mo_window *win)
{
  char *url = win->frameurl;
  int hash = hash_url(url);

  if (hash_table[hash].count) {
      entry *l;

      for (l = hash_table[hash].head; l; l = l->next) {
          if (!strcmp(l->url, url)) {
              if (l->cached_data && l->cached_data->frame_data) {
		  cached_frame_data *data = l->cached_data->frame_data;

		  if (data->text)
		      free(data->text);
		  if (data->last_modified)
		      free(data->last_modified);
		  if (data->expires)
		      free(data->expires);
		  if (data->charset)
		      free(data->charset);
	  	  data->text = win->frametext;
	  	  data->last_modified = win->framelast_modified;
	  	  data->expires = win->frameexpires;
	  	  data->charset = win->framecharset;
              } else {
		  /* Should have been there, but wasn't */
	  	  cached_frame_data *data = malloc(sizeof(cached_frame_data));

	  	  data->text = win->frametext;
	  	  data->last_modified = win->framelast_modified;
	  	  data->expires = win->frameexpires;
	  	  data->charset = win->framecharset;
          	  mo_cache_data(win->frameurl, data, mo_cache_frame);
	      }
	      return mo_succeed;
          }
      }
  }
  return mo_fail;
}


/****************************************************************************
 * name:    mo_deaccess_cached_frame_data
 * purpose: Deaccess a piece of cached data associated with a URL.
 * inputs:  
 *   - char *url: The URL.
 * returns: 
 *   mo_succeed, unless something goes badly wrong
 * remarks: 
 *   We do *not* do anything to the URL.
 ****************************************************************************/
mo_status mo_deaccess_cached_frame_data(char *url)
{
  int hash = hash_url(url);
  entry *l;

  if (!hash_table[hash].count)
      return mo_fail;
  for (l = hash_table[hash].head; l; l = l->next) {
      if (!strcmp(l->url, url)) {
          if (l->cached_data && l->cached_data->usage_count) {
	      l->cached_data->usage_count--;
	      if (!l->cached_data->usage_count &&
		  l->cached_data->frame_data) {
		  cached_frame_data *data = l->cached_data->frame_data;

		  if (data->text)
		      free(data->text);
		  if (data->last_modified)
		      free(data->last_modified);
		  if (data->expires)
		      free(data->expires);
		  if (data->charset)
		      free(data->charset);
                  free(data);
		  l->cached_data->frame_data = NULL;
	      }
              return mo_succeed;
          } else {
              return mo_fail;
	  }
      }
  }  
  return mo_fail;
}


/****************************************************************************
 * name:    mo_cache_data
 * purpose: Cache a piece of data associated with a given URL.
 * inputs:  
 *   - char  *url: The URL.
 *   - void *info: The piece of data to cache (currently an ImageInfo
 *		   struct for an image named as SRC in an IMG tag.
 *   - int   type: The type of data to cache (currently either
 *                 mo_cache_image for an ImageInfo struct or
 *		   mo_cache_frame for cached_frame_data struct).
 * returns: 
 *   mo_succeed, unless something goes badly wrong
 * remarks: 
 *   We do *not* do anything to the URL.  If there is a target
 *   anchor in it, fine with us.  This means the target anchor
 *   should have been stripped out someplace else if it needed to be.
 ****************************************************************************/
mo_status mo_cache_data(char *url, void *info, mo_cache_type type)
{
  int hash = hash_url(url);
  entry *l;

  /* First, register ourselves if we're not already registered.
   * Now, the same URL can be registered multiple times with different
   * (or, in one instance, no) internal anchor. */
  if (!been_here_before(url, hash)) {
      time_t foo = time(NULL);
      char ts[30];

      sprintf(ts, "%ld", foo);
      add_url_to_bucket(hash, url, ts);
  }
  /* Then, find the right entry. */
  if (hash_table[hash].count) {
      for (l = hash_table[hash].head; l; l = l->next) {
          if (!strcmp(l->url, url))
              goto found;
      }
  }
  return mo_fail;

 found:
  if (!l->cached_data) {
      l->cached_data = (cached_data *)calloc(1, sizeof(cached_data));
      /** Calloc zeros them
      l->cached_data->image_data = NULL;
      l->cached_data->frame_data = NULL;
      l->cached_data->last_access = 0;
      l->cached_data->usage_count = 0;
      **/
  }

  if (type == mo_cache_image) {
#ifndef DISABLE_TRACE
      if (cacheTrace)
          fprintf(stderr,
		  "[mo_cache_data] Caching '%s', data 0x%08x\n", url, info);
#endif
      mo_cache_image_data(l->cached_data, info);

  } else if (type == mo_cache_frame) {
      l->cached_data->usage_count++;
      if (info) {
          if (l->cached_data->frame_data)
	      free(l->cached_data->frame_data);
          l->cached_data->frame_data = (cached_frame_data *)info;
      }
  }
  return mo_succeed;
}


/* Cache frame data for all frames in this window */
mo_frame *mo_cache_frames(mo_window *win, int level)
{
  mo_frame *frame = (mo_frame *)malloc(sizeof(mo_frame));
  mo_frame *start;
  HTMLWidget hw;
  int num = 0;

  start = frame;
  while (win) {
      frame->next = NULL;
      if (mo_fetch_cached_frame_data(win->frameurl)) {
	  /* Just increment usage count if already in cache */
          mo_cache_data(win->frameurl, NULL, mo_cache_frame);
      } else {
	  cached_frame_data *data = malloc(sizeof(cached_frame_data));

	  data->text = win->frametext;
	  data->last_modified = win->framelast_modified;
	  data->expires = win->frameexpires;
	  data->charset = win->framecharset;
          mo_cache_data(win->frameurl, data, mo_cache_frame);
      }
      frame->url = strdup(win->frameurl);
      frame->docid = 1;
      frame->cached_widgets = NULL;
      frame->scrolled_win = win->scrolled_win;
      hw = (HTMLWidget) win->scrolled_win;
      hw->html.node_count++;
      frame->level = level;
      frame->num = num++;
      /* Cache frames inside a frame */
      if (win->frames) {
	  frame->next = mo_cache_frames(win->frames, level + 1);
	  while (frame->next)
	      frame = frame->next;
      }
      win = win->next_frame;
      if (win) {
          frame->next = (mo_frame *)malloc(sizeof(mo_frame));
	  frame = frame->next;
      }
  }
  return start;
}


mo_status mo_zap_cached_images_here(mo_window *win)
{
  int num;
  char **hrefs;

  /* Go fetch new hrefs. */
  hrefs = HTMLGetImageSrcs(win->scrolled_win, &num);

  if (num) {
      void *ptr;
      int i;
      char *url;

      for (i = 0; i < num; i++) {
          url = mo_url_canonicalize(hrefs[i], win->cached_url);
          ptr = mo_fetch_cached_image_data(url);
          if (ptr)
              mo_cache_data(url, NULL, mo_cache_image);
	  free(url);
      }

      /* All done; clean up. */
      for (i = 0; i < num; i++)
          free(hrefs[i]);
      free(hrefs);
  }
  return mo_succeed;
}


/* ------------------------------------------------------------------------ */
/* ------------------------- decent image caching ------------------------- */
/* ------------------------------------------------------------------------ */

/* CHUNK_OF_IMAGES determines the initial size of the array of cached
 * pointers to image data; if more images must be cached, the array is
 * grown with realloc by this amount.  It is good to keep the array as
 * small as possible, as it must occasionally be sorted. */
#define CHUNK_OF_IMAGES 10

static cached_data **cached_cd_array = NULL;
static int num_in_cached_cd_array = 0;
static int size_of_cached_cd_array = 0;

#ifndef DISABLE_TRACE
/* Only called if cacheTrace is true */
static mo_status mo_dump_cached_cd_array(void)
{
  int i;

  if (!cached_cd_array) {
      fprintf(stderr, "[mo_dump_cached_cd_array] No array; punting\n");
      return mo_fail;
  }

  fprintf(stderr, "+++++++++++++++++++++++++\n");

  for (i = 0; i < size_of_cached_cd_array; i++) {
      if (cached_cd_array[i]) {
	  fprintf(stderr, "  %02d data 0x%08x last_access %d\n", i,
		  cached_cd_array[i]->image_data, 
		  cached_cd_array[i]->last_access);
      } else {
	  fprintf(stderr, "  %02d NULL\n", i);
      }
  }

  fprintf(stderr, "---------------------\n");

  return mo_succeed;
}
#endif


static mo_status mo_init_cached_cd_array(void)
{
  cached_cd_array = (cached_data **)calloc(sizeof(cached_data *),
					   CHUNK_OF_IMAGES);
  size_of_cached_cd_array += CHUNK_OF_IMAGES;

#ifndef DISABLE_TRACE
  if (cacheTrace)
      fprintf(stderr, "[mo_init] Did it 0x%08x -- allocated %d pointers.\n",
              cached_cd_array, size_of_cached_cd_array);
#endif

  /** calloc zeros it
  memset((char *)cached_cd_array, 0,
         CHUNK_OF_IMAGES * sizeof(cached_cd_array[0]));
  **/

  return mo_succeed;
}


static mo_status mo_grow_cached_cd_array(void)
{
  cached_cd_array = (cached_data **)realloc(cached_cd_array,
           sizeof(cached_data *) * (size_of_cached_cd_array + CHUNK_OF_IMAGES));

#ifndef DISABLE_TRACE
  if (cacheTrace)
      fprintf(stderr,
	"[grow] cached_cd_array 0x%08x, size_of_cached_cd_array 0x%08x, sum 0x%08x\n",
        cached_cd_array, size_of_cached_cd_array, 
        cached_cd_array + size_of_cached_cd_array);
#endif
  memset((char *)(cached_cd_array + size_of_cached_cd_array), 0,
         CHUNK_OF_IMAGES * sizeof(cached_cd_array[0]));

  size_of_cached_cd_array += CHUNK_OF_IMAGES;

  return mo_succeed;
}


static int mo_sort_cd_for_qsort(MO_CONST void *a1, MO_CONST void *a2)
{
  cached_data **d1 = (cached_data **)a1;
  cached_data **d2 = (cached_data **)a2;

  /* NULL entries will be at end of array -- this may be good,
   * or may not be -- hmmmmmm. */
  if (!d1 || !*d1)
      return 1;
  if (!d2 || !*d2)
      return -1;

#ifndef DISABLE_TRACE
  if (cacheTrace)
      fprintf(stderr, "sort: hi there! %d %d\n",
              (*d1)->last_access, (*d2)->last_access);
#endif

  return ((*d1)->last_access < (*d2)->last_access ? -1 : 1);
}


static mo_status mo_sort_cached_cd_array(void)
{
  if (!cached_cd_array) {
#ifndef DISABLE_TRACE
      if (cacheTrace)
          fprintf(stderr, "[mo_sort_cached_cd_array] No array; punting\n");
#endif
      return mo_fail;
  }

  if (!num_in_cached_cd_array) {
#ifndef DISABLE_TRACE
      if (cacheTrace)
          fprintf(stderr,
		  "[mo_sort_cached_cd_array] Num in array 0; punting\n");
#endif
      return mo_fail;
  }

#ifndef DISABLE_TRACE
  if (cacheTrace) {
      fprintf(stderr, "[mo_sort_cached_cd_array] Sorting 0x%08x!\n",
              cached_cd_array);
      mo_dump_cached_cd_array();
  }
#endif

  qsort((void *)cached_cd_array, size_of_cached_cd_array, 
        sizeof(cached_cd_array[0]), mo_sort_cd_for_qsort);

#ifndef DISABLE_TRACE
  if (cacheTrace)
      mo_dump_cached_cd_array();
#endif
  
  return mo_succeed;
}


static mo_status mo_remove_cd_from_cached_cd_array(cached_data *cd)
{
  int i;
  int freed_kb = 0;
  
  if (!cached_cd_array)
      return mo_fail;

  for (i = 0; i < size_of_cached_cd_array; i++) {
      if (cached_cd_array[i] == cd) {
#ifndef DISABLE_TRACE
          if (cacheTrace)
              fprintf(stderr, 
                "[mo_remove_cd_from_cached_cd_array] Found data 0x%08x, location %d\n", 
                cached_cd_array[i]->image_data, i);
#endif
          freed_kb = mo_kbytes_in_image_data(cached_cd_array[i]->image_data);
          mo_free_image_data(cached_cd_array[i]->image_data);
          cached_cd_array[i] = NULL;
          goto done;
      }
  }
#ifndef DISABLE_TRACE
  if (cacheTrace)
      fprintf(stderr, "[mo_remove_cd] UH OH, DIDN'T FIND IT!!\n");
#endif

  return mo_fail;
  
 done:
  num_in_cached_cd_array--;
  kbytes_cached -= freed_kb;
  return mo_succeed;
}


static mo_status mo_add_cd_to_cached_cd_array(cached_data *cd)
{
  int i, num;
  int kbytes_in_new_image = mo_kbytes_in_image_data(cd->image_data);
  static int init = 0;
  static long cachesize;
  
  if (!init) {
      cachesize = get_pref_int(eIMAGE_CACHE_SIZE);
      init = 1;
  }

#ifndef DISABLE_TRACE
  if (cacheTrace)
      fprintf(stderr, "[mo_add_cd] New image is %d kbytes.\n",
              kbytes_in_new_image);
#endif

  if (!cached_cd_array) {
      mo_init_cached_cd_array();
#ifndef DISABLE_TRACE
      if (cacheTrace)
          fprintf(stderr, "[mo_add_cd] Init'd cached_cd_array.\n");
#endif
  } else {
      /* Maybe it's already in there. */
      for (i = 0; i < size_of_cached_cd_array; i++) {
          if (cached_cd_array[i] == cd)
              return mo_succeed;
      }
  }

  /* Here's the magic part. */
  if ((kbytes_cached + kbytes_in_new_image) > cachesize) {
      int num_to_remove = 0;

#ifndef DISABLE_TRACE
      if (cacheTrace)
          fprintf(stderr, "[mo_add_cd] Going to sort 0x%08x...\n", 
                  cached_cd_array);
#endif
      mo_sort_cached_cd_array();
#ifndef DISABLE_TRACE
      if (cacheTrace) {
          fprintf(stderr, 
                  "[mo_add_to] Just sorted in preparation for purging...\n");
	  mo_dump_cached_cd_array();
      }
#endif

      while ((kbytes_cached + kbytes_in_new_image) > cachesize) {
#ifndef DISABLE_TRACE
          if (cacheTrace)
              fprintf(stderr,
		      "[mo_add_cd] Trying to free another image (%d > %d).\n",
                      kbytes_cached + kbytes_in_new_image, cachesize);
#endif
          /* Try to remove one -- we rely on the fact that NULL
           * entries in cached_cd_array are at the end of the array. */
          if (num_to_remove < size_of_cached_cd_array &&
              cached_cd_array[num_to_remove]) {
#ifndef DISABLE_TRACE
              if (cacheTrace)
                  fprintf(stderr,
		    "        ** try to remove %d; last_access %d < dont_nuke_after_me %d??\n",
                    num_to_remove,
                    cached_cd_array[num_to_remove]->last_access,
                    dont_nuke_after_me);
#endif
              if (cached_cd_array[num_to_remove]->last_access <
                  dont_nuke_after_me) {
#ifndef DISABLE_TRACE
                  if (cacheTrace)
                      fprintf(stderr, "        ** really removing %d\n",
                              num_to_remove);
#endif
                  mo_uncache_image_data(cached_cd_array[num_to_remove]);
#ifndef DISABLE_TRACE
                  if (cacheTrace)
                      mo_dump_cached_cd_array();
#endif
              }
              num_to_remove++;
          } else {
#ifndef DISABLE_TRACE
              if (cacheTrace) {
                  fprintf(stderr, "        ** no more to remove\n");
                  mo_dump_cached_cd_array();
	      }
#endif
              break;
          }
      }
  }
  
  if (num_in_cached_cd_array == size_of_cached_cd_array) {
#ifndef DISABLE_TRACE
      if (cacheTrace)
          fprintf(stderr, "[mo_add_cd] Growing array... \n");
#endif
      num = size_of_cached_cd_array;
      mo_grow_cached_cd_array();
  } else {
      num = -1;
      for (i = 0; i < size_of_cached_cd_array; i++) {
          if (!cached_cd_array[i]) {
              num = i;
              goto got_num;
          }
      }
#ifndef DISABLE_TRACE
      if (cacheTrace)
          fprintf(stderr, 
                  "[mo_add_cd_to_cached_cd_array] Couldn't find empty slot!\n");
#endif
      /* Try to grow array -- flow of control should never reach here */
      num = size_of_cached_cd_array;
      mo_grow_cached_cd_array();
  }
  
 got_num:
  cached_cd_array[num] = cd;
  num_in_cached_cd_array++;

  kbytes_cached += kbytes_in_new_image;

#ifndef DISABLE_TRACE
  if (cacheTrace) {
      fprintf(stderr,
	      "[mo_add_cd_to_cached_cd_array] Added cd, data 0x%08x, num %d\n",
              cd->image_data, num);
      fprintf(stderr,
              "[mo_add_cd_to_cached_cd_array] Now cached %d kbytes.\n",
	      kbytes_cached);
      mo_dump_cached_cd_array();
  }
#endif

  return mo_succeed;
}


static int mo_kbytes_in_image_data(void *image_data)
{
  ImageInfo *img = (ImageInfo *)image_data;
  int bytes, kbytes;

  if (!img)
      return 0;

  bytes = img->width * img->height;
  kbytes = bytes >> 10;

#ifndef DISABLE_TRACE
  if (cacheTrace)
      fprintf(stderr, "[mo_kbytes_in_image_data] bytes %d, kbytes %d\n",
              bytes, kbytes);
#endif

  if (!kbytes)
      kbytes = 1;
  
  return kbytes;
}


static mo_status mo_cache_image_data(cached_data *cd, void *info)
{
  static int init = 0;

  /* Beeeeeeeeeeeeeeeeee smart! */
  if (!init && get_pref_int(eIMAGE_CACHE_SIZE) <= 0) {
      set_pref_int(eIMAGE_CACHE_SIZE, 1);
      init = 1;
  }
  /* It's possible we'll be getting NULL info here, so we
   * should uncache in this case... */
  if (!info)
      mo_uncache_image_data(cd);

  cd->image_data = info;
  cd->last_access = access_counter++;

  mo_add_cd_to_cached_cd_array(cd);

  return mo_succeed;
}


static mo_status mo_uncache_image_data(cached_data *cd)
{
  mo_remove_cd_from_cached_cd_array(cd);

  cd->image_data = NULL;
  return mo_succeed;
}


mo_status mo_set_image_cache_nuke_point(void)
{
#ifndef DISABLE_TRACE
  if (cacheTrace)
      fprintf(stderr, "[mo_set_nuke_point] Setting to %d\n", access_counter);
#endif

  dont_nuke_after_me = access_counter;
  return mo_succeed;
}
