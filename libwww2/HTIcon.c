/*
** Author: Charles Henrich (henrich@crh.cl.msu.edu) October 2, 1993
**
** This routine takes two parameters, Format (as returned by HTFileFormat)
** and a default icon type.  The type is the first half of the MIME type field
** (i.e., image from image/jpeg).  If the icon type cannot be determined from
** the format or from the default, then the unknown icon is returned.
**
** Note: This routine guarantee's to return something!
** 
*/
#include "../config.h"
#include "HTFile.h"
#include "HTAtom.h"
#include "HTIcon.h"

struct typemap {
  char *format;
  char *image;
};

static struct typemap type_map[] = {
  {"image",       "internal-gopher-image"},
  {"text",        "internal-gopher-text"},
  {"audio",       "internal-gopher-sound"},
  {"application", "internal-gopher-binary"},
  {"message",     "internal-gopher-text"},
  {"video",       "internal-gopher-movie"},
  {"directory",   "internal-gopher-menu"},
  {"unknown",     "internal-gopher-unknown"},
  {"EOFEOF",      "EOFEOF"}
};

char *HTgeticonname(HTFormat format, char *defaultformat)
{
  int count;
  char *ptr;
  char subtype[128];
  
  if (format) {
      strcpy(subtype, format->name);
      ptr = strchr(subtype, '/');
      if (ptr) 
          *ptr = '\0';
   } else {
      subtype[0] = '\0';
  }
  
  ptr = NULL;
  
  for (count = 0; strcmp(type_map[count].image, "EOFEOF"); count++) {
      if (!strcmp(type_map[count].format, subtype))
          return type_map[count].image;
      if (!strcmp(type_map[count].format, defaultformat))
          ptr = type_map[count].image;
  }
  
  if (ptr)
      return ptr;
  
  return "internal-gopher-unknown";
}
