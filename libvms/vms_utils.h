/* Parts taken from IMAGEMAGICK V385 */

/*
 * VMS specific include declarations.
 */
#include <lib$routines.h>
#include <errno.h>
#include <descrip.h>
#include <rmsdef.h>
#include <ctype.h>

/*
 * Typedef declarations.
 */
struct dirent {
  char d_name[255];
  int d_namlen;
};

typedef struct _dirdesc {
  long context;
  char *pattern;
  struct dirent entry;
  struct dsc$descriptor_s pat;
} DIR;

/*
 * VMS utilities routines.
 */
extern DIR *VMSopendir(char *);

extern struct dirent *VMSreaddir(DIR *);

extern void VMSclosedir(DIR *);

extern void VMSrewinddir(DIR *);

extern char *VMSbasename(char *path);

extern int trans_VMSlogical(char **ret, char *name);
