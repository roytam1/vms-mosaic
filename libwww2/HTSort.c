/* Simple string sorting support, thanks to qsort(). */
#include "../config.h"
#include "HTUtils.h"
#include "HTSort.h"
#include <string.h>

#define SIZE_OF_HUNK 100

static char **hunk = NULL;
static int size_of_hunk;
static int count;

void HTSortInit(void)
{
  count = 0;

  if (!hunk) {
      size_of_hunk = SIZE_OF_HUNK;
      hunk = (char **)malloc(size_of_hunk * sizeof(char *));
  }
  return;
}

static void expand_hunk(void)
{
  /* Make hunk bigger by SIZE_OF_HUNK elements. */
  size_of_hunk += SIZE_OF_HUNK;
  hunk = (char **)realloc(hunk, size_of_hunk * sizeof(char *));

  return;
}

void HTSortAdd(char *str)
{
  /* If we don't have room, expand. */
  if (count == size_of_hunk)
    expand_hunk();

  hunk[count++] = str;
  
  return;
}

static int dsortf(char **s1, char **s2)
{
  return (strcmp(*(char **)s1, *(char **)s2));
}

void HTSortSort(void)
{
  static int (*dsortf_ptr)() = dsortf;

  qsort((void *)hunk, count, sizeof(char *), dsortf_ptr);

  return;
}

int HTSortCurrentCount(void)
{
  return count;
}

char *HTSortFetch(int i)
{
  if (i < count)
    return hunk[i];

  return NULL;
}
