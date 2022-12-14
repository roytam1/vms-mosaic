/*
 * $RCSId: xc/lib/fontconfig/src/fclist.c,v 1.11tsi Exp $
 *
 * Copyright © 2000 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include "fcint.h"

FcObjectSet *
FcObjectSetCreate (void)
{
    FcObjectSet    *os;

    os = (FcObjectSet *) malloc (sizeof (FcObjectSet));
    if (!os)
	return 0;
    FcMemAlloc (FC_MEM_OBJECTSET, sizeof (FcObjectSet));
    os->nobject = 0;
    os->sobject = 0;
    os->objects = 0;
    return os;
}

FcBool
FcObjectSetAdd (FcObjectSet *os, const char *object)
{
    int		s;
    const char	**objects;
    int		high, low, mid, c;
    
    if (os->nobject == os->sobject)
    {
	s = os->sobject + 4;
	if (os->objects)
	    objects = (const char **) realloc ((void *) os->objects,
					       s * sizeof (const char *));
	else
	    objects = (const char **) malloc (s * sizeof (const char *));
	if (!objects)
	    return FcFalse;
	if (os->sobject)
	    FcMemFree (FC_MEM_OBJECTPTR, os->sobject * sizeof (const char *));
	FcMemAlloc (FC_MEM_OBJECTPTR, s * sizeof (const char *));
	os->objects = objects;
	os->sobject = s;
    }
    high = os->nobject - 1;
    low = 0;
    mid = 0;
    c = 1;
    object = (char *)FcStrStaticName ((FcChar8 *)object);
    while (low <= high)
    {
	mid = (low + high) >> 1;
	c = os->objects[mid] - object;
	if (c == 0)
	    return FcTrue;
	if (c < 0)
	    low = mid + 1;
	else
	    high = mid - 1;
    }
    if (c < 0)
	mid++;
    memmove (os->objects + mid + 1, os->objects + mid, 
	     (os->nobject - mid) * sizeof (const char *));
    os->objects[mid] = object;
    os->nobject++;
    return FcTrue;
}

void
FcObjectSetDestroy (FcObjectSet *os)
{
    if (os->objects)
    {
	FcMemFree (FC_MEM_OBJECTPTR, os->sobject * sizeof (const char *));
	free ((void *) os->objects);
    }
    FcMemFree (FC_MEM_OBJECTSET, sizeof (FcObjectSet));
    free (os);
}

FcObjectSet *
FcObjectSetVaBuild (const char *first, va_list va)
{
    FcObjectSet    *ret;

    FcObjectSetVapBuild (ret, first, va);
    return ret;
}

FcObjectSet *
FcObjectSetBuild (const char *first, ...)
{
    va_list	    va;
    FcObjectSet    *os;

    va_start (va, first);
    FcObjectSetVapBuild (os, first, va);
    va_end (va);
    return os;
}

/*
 * Font must have a containing value for every value in the pattern
 */
static FcBool
FcListValueListMatchAny (FcValueListPtr patOrig,	    /* pattern */
			 FcValueListPtr fntOrig)	    /* font */
{
    FcValueListPtr	 pat, fnt;

    for (pat = patOrig; FcValueListPtrU(pat); 
	 pat = FcValueListPtrU(pat)->next)
    {
	for (fnt = fntOrig; FcValueListPtrU(fnt); 
	     fnt = FcValueListPtrU(fnt)->next)
	{
	    /*
	     * make sure the font 'contains' the pattern.
	     * (OpListing is OpContains except for strings
	     *  where it requires an exact match)
	     */
	    if (FcConfigCompareValue (&FcValueListPtrU(fnt)->value,
				      FcOpListing, 
				      &FcValueListPtrU(pat)->value)) 
		break;
	}
	if (!FcValueListPtrU(fnt))
	    return FcFalse;
    }
    return FcTrue;
}

static FcBool
FcListValueListEqual (FcValueListPtr v1orig,
		      FcValueListPtr v2orig)
{
    FcValueListPtr	    v1, v2;

    for (v1 = v1orig; FcValueListPtrU(v1); 
	 v1 = FcValueListPtrU(v1)->next)
    {
	for (v2 = v2orig; FcValueListPtrU(v2); 
	     v2 = FcValueListPtrU(v2)->next)
	    if (FcValueEqual (FcValueCanonicalize(&FcValueListPtrU(v1)->value),
			      FcValueCanonicalize(&FcValueListPtrU(v2)->value)))
		break;
	if (!FcValueListPtrU(v2))
	    return FcFalse;
    }
    for (v2 = v2orig; FcValueListPtrU(v2); 
	 v2 = FcValueListPtrU(v2)->next)
    {
	for (v1 = v1orig; FcValueListPtrU(v1); 
	     v1 = FcValueListPtrU(v1)->next)
	    if (FcValueEqual (FcValueCanonicalize(&FcValueListPtrU(v1)->value),
			      FcValueCanonicalize(&FcValueListPtrU(v2)->value)))
		break;
	if (!FcValueListPtrU(v1))
	    return FcFalse;
    }
    return FcTrue;
}

static FcBool
FcListPatternEqual (FcPattern	*p1,
		    FcPattern	*p2,
		    FcObjectSet	*os)
{
    int		    i;
    FcPatternElt    *e1, *e2;

    for (i = 0; i < os->nobject; i++)
    {
	e1 = FcPatternFindElt (p1, os->objects[i]);
	e2 = FcPatternFindElt (p2, os->objects[i]);
	if (!e1 && !e2)
	    continue;
	if (!e1 || !e2)
	    return FcFalse;
	if (!FcListValueListEqual (e1->values, e2->values))
	    return FcFalse;
    }
    return FcTrue;
}

/*
 * FcTrue iff all objects in "p" match "font"
 */

FcBool
FcListPatternMatchAny (const FcPattern *p,
		       const FcPattern *font)
{
    int		    i;
    FcPatternElt   *e;

    for (i = 0; i < p->num; i++)
    {
	e = FcPatternFindElt (font, 
			      FcObjectPtrU((FcPatternEltU(p->elts)+i)->object));
	if (!e)
	    return FcFalse;
	if (!FcListValueListMatchAny ((FcPatternEltU(p->elts)+i)->values,    /* pat elts */
				      e->values))	    /* font elts */
	    return FcFalse;
    }
    return FcTrue;
}

static FcChar32
FcListMatrixHash (const FcMatrix *m)
{
    int	    xx = (int) (m->xx * 100), 
	    xy = (int) (m->xy * 100), 
	    yx = (int) (m->yx * 100),
	    yy = (int) (m->yy * 100);

    return ((FcChar32) xx) ^ ((FcChar32) xy) ^ ((FcChar32) yx) ^ ((FcChar32) yy);
}

static FcChar32
FcListValueHash (FcValue    *value)
{
    FcValue v = FcValueCanonicalize(value);
    switch (v.type) {
    case FcTypeVoid:
	return 0;
    case FcTypeInteger:
	return (FcChar32) v.u.i;
    case FcTypeDouble:
	return (FcChar32) (int) v.u.d;
    case FcTypeString:
	return FcStrHashIgnoreCase (v.u.s);
    case FcTypeBool:
	return (FcChar32) v.u.b;
    case FcTypeMatrix:
	return FcListMatrixHash (v.u.m);
    case FcTypeCharSet:
	return FcCharSetCount (v.u.c);
    case FcTypeFTFace:
	return (long) v.u.f;
    case FcTypeLangSet:
	return FcLangSetHash (v.u.l);
    }
    return 0;
}

static FcChar32
FcListValueListHash (FcValueListPtr list)
{
    FcChar32	h = 0;
    
    while (FcValueListPtrU(list))
    {
	h = h ^ FcListValueHash (&FcValueListPtrU(list)->value);
	list = FcValueListPtrU(list)->next;
    }
    return h;
}

static FcChar32
FcListPatternHash (FcPattern	*font,
		   FcObjectSet	*os)
{
    int		    n;
    FcPatternElt    *e;
    FcChar32	    h = 0;

    for (n = 0; n < os->nobject; n++)
    {
	e = FcPatternFindElt (font, os->objects[n]);
	if (e)
	    h = h ^ FcListValueListHash (e->values);
    }
    return h;
}

typedef struct _FcListBucket {
    struct _FcListBucket    *next;
    FcChar32		    hash;
    FcPattern		    *pattern;
} FcListBucket;

#define FC_LIST_HASH_SIZE   4099

typedef struct _FcListHashTable {
    int		    entries;
    FcListBucket    *buckets[FC_LIST_HASH_SIZE];
} FcListHashTable;
    
static void
FcListHashTableInit (FcListHashTable *table)
{
    table->entries = 0;
    memset (table->buckets, '\0', sizeof (table->buckets));
}

static void
FcListHashTableCleanup (FcListHashTable *table)
{
    int	i;
    FcListBucket    *bucket, *next;

    for (i = 0; i < FC_LIST_HASH_SIZE; i++)
    {
	for (bucket = table->buckets[i]; bucket; bucket = next)
	{
	    next = bucket->next;
	    FcPatternDestroy (bucket->pattern);
	    FcMemFree (FC_MEM_LISTBUCK, sizeof (FcListBucket));
	    free (bucket);
	}
	table->buckets[i] = 0;
    }
    table->entries = 0;
}

static int
FcGetDefaultObjectLangIndex (FcPattern *font, const char *object)
{
    FcChar8	   *lang = FcGetDefaultLang ();
    FcPatternElt   *e = FcPatternFindElt (font, object);
    FcValueListPtr  v;
    FcValue         value;
    int             idx = -1;
    int             i;

    if (e)
    {
	for (v = e->values, i = 0; FcValueListPtrU(v); v = FcValueListPtrU(v)->next, ++i)
	{
	    value = FcValueCanonicalize (&FcValueListPtrU (v)->value);

	    if (value.type == FcTypeString)
	    {
		FcLangResult res = FcLangCompare (value.u.s, lang);
		if (res == FcLangEqual || (res == FcLangDifferentCountry && idx < 0))
		    idx = i;
	    }
	}
    }

    return (idx > 0) ? idx : 0;
}

static FcBool
FcListAppend (FcListHashTable	*table,
	      FcPattern		*font,
	      FcObjectSet	*os)
{
    int		    o;
    FcPatternElt    *e;
    FcValueListPtr  v;
    FcChar32	    hash;
    FcListBucket    **prev, *bucket;
    int             familyidx = -1;
    int             fullnameidx = -1;
    int             styleidx = -1;
    int             defidx = 0;
    int             idx;

    hash = FcListPatternHash (font, os);
    for (prev = &table->buckets[hash % FC_LIST_HASH_SIZE];
	 (bucket = *prev); prev = &(bucket->next))
    {
	if (bucket->hash == hash && 
	    FcListPatternEqual (bucket->pattern, font, os))
	    return FcTrue;
    }
    bucket = (FcListBucket *) malloc (sizeof (FcListBucket));
    if (!bucket)
	goto bail0;
    FcMemAlloc (FC_MEM_LISTBUCK, sizeof (FcListBucket));
    bucket->next = 0;
    bucket->hash = hash;
    bucket->pattern = FcPatternCreate ();
    if (!bucket->pattern)
	goto bail1;
    
    for (o = 0; o < os->nobject; o++)
    {
	if (!strcmp (os->objects[o], FC_FAMILY) || !strcmp (os->objects[o], FC_FAMILYLANG))
	{
	    if (familyidx < 0)
		familyidx = FcGetDefaultObjectLangIndex (font, FC_FAMILYLANG);
	    defidx = familyidx;
	}
	else if (!strcmp (os->objects[o], FC_FULLNAME) || !strcmp (os->objects[o], FC_FULLNAMELANG))
	{
	    if (fullnameidx < 0)
		fullnameidx = FcGetDefaultObjectLangIndex (font, FC_FULLNAMELANG);
	    defidx = fullnameidx;
	}
	else if (!strcmp (os->objects[o], FC_STYLE) || !strcmp (os->objects[o], FC_STYLELANG))
	{
	    if (styleidx < 0)
		styleidx = FcGetDefaultObjectLangIndex (font, FC_STYLELANG);
	    defidx = styleidx;
	}
	else
	    defidx = 0;

        /* Also, copy over the full path info. */
	if (!strcmp (os->objects[o], FC_FILE))
	    FcPatternTransferFullFname (bucket->pattern, font);

	e = FcPatternFindElt (font, os->objects[o]);
	if (e)
	{
	    for (v = e->values, idx = 0; FcValueListPtrU(v); 
		 v = FcValueListPtrU(v)->next, ++idx)
	    {
		if (!FcPatternAdd (bucket->pattern, 
				   os->objects[o], 
				   FcValueCanonicalize(&FcValueListPtrU(v)->value), defidx != idx))
		    goto bail2;
	    }
	}
    }
    *prev = bucket;
    ++table->entries;

    return FcTrue;
    
bail2:
    FcPatternDestroy (bucket->pattern);
bail1:
    FcMemFree (FC_MEM_LISTBUCK, sizeof (FcListBucket));
    free (bucket);
bail0:
    return FcFalse;
}

FcFontSet *
FcFontSetList (FcConfig	    *config,
	       FcFontSet    **sets,
	       int	    nsets,
	       FcPattern    *p,
	       FcObjectSet  *os)
{
    FcFontSet	    *ret;
    FcFontSet	    *s;
    int		    f;
    int		    set;
    FcListHashTable table;
    int		    i;
    FcListBucket    *bucket;

    if (!config)
    {
	if (!FcInitBringUptoDate ())
	    goto bail0;

	config = FcConfigGetCurrent ();
	if (!config)
	    goto bail0;
    }
    FcListHashTableInit (&table);
    /*
     * Walk all available fonts adding those that
     * match to the hash table
     */
    for (set = 0; set < nsets; set++)
    {
	s = sets[set];
	if (!s)
	    continue;
	for (f = 0; f < s->nfont; f++)
	    if (FcListPatternMatchAny (p,		/* pattern */
				       s->fonts[f]))	/* font */
		if (!FcListAppend (&table, s->fonts[f], os))
		    goto bail1;
    }
#if 0
    {
	int	max = 0;
	int	full = 0;
	int	ents = 0;
	int	len;
	for (i = 0; i < FC_LIST_HASH_SIZE; i++)
	{
	    if ((bucket = table.buckets[i]))
	    {
		len = 0;
		for (; bucket; bucket = bucket->next)
		{
		    ents++;
		    len++;
		}
		if (len > max)
		    max = len;
		full++;
	    }
	}
	printf ("used: %d max: %d avg: %g\n", full, max, 
		(double) ents / FC_LIST_HASH_SIZE);
    }
#endif
    /*
     * Walk the hash table and build
     * a font set
     */
    ret = FcFontSetCreate ();
    if (!ret)
	goto bail0;
    for (i = 0; i < FC_LIST_HASH_SIZE; i++)
	while ((bucket = table.buckets[i]))
	{
	    if (!FcFontSetAdd (ret, bucket->pattern))
		goto bail2;
	    table.buckets[i] = bucket->next;
	    FcMemFree (FC_MEM_LISTBUCK, sizeof (FcListBucket));
	    free (bucket);
	}
    
    return ret;

bail2:
    FcFontSetDestroy (ret);
bail1:
    FcListHashTableCleanup (&table);
bail0:
    return 0;
}

FcFontSet *
FcFontList (FcConfig	*config,
	    FcPattern	*p,
	    FcObjectSet *os)
{
    FcFontSet	*sets[2];
    int		nsets;

    if (!config)
    {
	config = FcConfigGetCurrent ();
	if (!config)
	    return 0;
    }
    nsets = 0;
    if (config->fonts[FcSetSystem])
	sets[nsets++] = config->fonts[FcSetSystem];
    if (config->fonts[FcSetApplication])
	sets[nsets++] = config->fonts[FcSetApplication];
    return FcFontSetList (config, sets, nsets, p, os);
}
