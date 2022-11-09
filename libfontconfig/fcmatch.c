/*
 * $RCSId: xc/lib/fontconfig/src/fcmatch.c,v 1.20 2002/08/31 22:17:32 keithp Exp $
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

#include "fcint.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static double
FcCompareNumber (FcValue *value1, FcValue *value2)
{
    double  v1, v2, v;
    
    switch (value1->type) {
    case FcTypeInteger:
	v1 = (double) value1->u.i;
	break;
    case FcTypeDouble:
	v1 = value1->u.d;
	break;
    default:
	return -1.0;
    }
    switch (value2->type) {
    case FcTypeInteger:
	v2 = (double) value2->u.i;
	break;
    case FcTypeDouble:
	v2 = value2->u.d;
	break;
    default:
	return -1.0;
    }
    v = v2 - v1;
    if (v < 0)
	v = -v;
    return v;
}

static double
FcCompareString (FcValue *v1, FcValue *v2)
{
    return (double) FcStrCmpIgnoreCase (fc_value_string(v1), fc_value_string(v2)) != 0;
}

static double
FcCompareFamily (FcValue *v1, FcValue *v2)
{
    /* rely on the guarantee in FcPatternAddWithBinding that
     * families are always FcTypeString. */
    const FcChar8* v1_string = fc_value_string(v1);
    const FcChar8* v2_string = fc_value_string(v2);

    if (FcToLower(*v1_string) != FcToLower(*v2_string))
       return 1.0;

    return (double) FcStrCmpIgnoreBlanksAndCase (v1_string, v2_string) != 0;
}

static double
FcCompareLang (FcValue *v1, FcValue *v2)
{
    FcLangResult    result;
    FcValue value1 = FcValueCanonicalize(v1), value2 = FcValueCanonicalize(v2);
    
    switch (value1.type) {
    case FcTypeLangSet:
	switch (value2.type) {
	case FcTypeLangSet:
	    result = FcLangSetCompare (value1.u.l, value2.u.l);
	    break;
	case FcTypeString:
	    result = FcLangSetHasLang (value1.u.l, 
				       value2.u.s);
	    break;
	default:
	    return -1.0;
	}
	break;
    case FcTypeString:
	switch (value2.type) {
	case FcTypeLangSet:
	    result = FcLangSetHasLang (value2.u.l, value1.u.s);
	    break;
	case FcTypeString:
	    result = FcLangCompare (value1.u.s, 
				    value2.u.s);
	    break;
	default:
	    return -1.0;
	}
	break;
    default:
	return -1.0;
    }
    switch (result) {
    case FcLangEqual:
	return 0;
    case FcLangDifferentCountry:
	return 1;
    case FcLangDifferentLang:
    default:
	return 2;
    }
}

static double
FcCompareBool (FcValue *v1, FcValue *v2)
{
    if (fc_storage_type(v2) != FcTypeBool || fc_storage_type(v1) != FcTypeBool)
	return -1.0;
    return (double) v2->u.b != v1->u.b;
}

static double
FcCompareCharSet (FcValue *v1, FcValue *v2)
{
    return (double) FcCharSetSubtractCount (fc_value_charset(v1), fc_value_charset(v2));
}

static double
FcCompareSize (FcValue *value1, FcValue *value2)
{
    double  v1, v2, v;

    switch (value1->type) {
    case FcTypeInteger:
	v1 = value1->u.i;
	break;
    case FcTypeDouble:
	v1 = value1->u.d;
	break;
    default:
	return -1;
    }
    switch (value2->type) {
    case FcTypeInteger:
	v2 = value2->u.i;
	break;
    case FcTypeDouble:
	v2 = value2->u.d;
	break;
    default:
	return -1;
    }
    if (v2 == 0)
	return 0;
    v = v2 - v1;
    if (v < 0)
	v = -v;
    return v;
}

typedef struct _FcMatcher {
    const char	    *object;
    FcObjectPtr	    objectPtr;
    double	    (*compare) (FcValue *value1, FcValue *value2);
    int		    strong, weak;
} FcMatcher;

/*
 * Order is significant, it defines the precedence of
 * each value, earlier values are more significant than
 * later values
 */
static FcMatcher _FcMatchers [] = {
    { FC_FOUNDRY,	0, FcCompareString,	0, 0 },
#define MATCH_FOUNDRY	    0
#define MATCH_FOUNDRY_INDEX 0
    
    { FC_CHARSET,	0, FcCompareCharSet,	1, 1 },
#define MATCH_CHARSET	    1
#define MATCH_CHARSET_INDEX 1
    
    { FC_FAMILY,    	0, FcCompareFamily,	2, 4 },
#define MATCH_FAMILY	    2
#define MATCH_FAMILY_STRONG_INDEX   2
#define MATCH_FAMILY_WEAK_INDEX	    4
    
    { FC_LANG,		0, FcCompareLang,	3, 3 },
#define MATCH_LANG	    3
#define MATCH_LANG_INDEX    3
    
    { FC_SPACING,	0, FcCompareNumber,	5, 5 },
#define MATCH_SPACING	    4
#define MATCH_SPACING_INDEX 5
    
    { FC_PIXEL_SIZE,	0, FcCompareSize,	6, 6 },
#define MATCH_PIXEL_SIZE    5
#define MATCH_PIXEL_SIZE_INDEX	6
    
    { FC_STYLE,		0, FcCompareString,	7, 7 },
#define MATCH_STYLE	    6
#define MATCH_STYLE_INDEX   7
    
    { FC_SLANT,		0, FcCompareNumber,	8, 8 },
#define MATCH_SLANT	    7
#define MATCH_SLANT_INDEX   8
    
    { FC_WEIGHT,	0, FcCompareNumber,	9, 9 },
#define MATCH_WEIGHT	    8
#define MATCH_WEIGHT_INDEX  9
    
    { FC_WIDTH,		0, FcCompareNumber,	10, 10 },
#define MATCH_WIDTH	    9
#define MATCH_WIDTH_INDEX   10
    
    { FC_ANTIALIAS,	0, FcCompareBool,	11, 11 },
#define MATCH_ANTIALIAS	    10
#define MATCH_ANTIALIAS_INDEX	    11
    
    { FC_RASTERIZER,	0, FcCompareString,	12, 12 },
#define MATCH_RASTERIZER    11
#define MATCH_RASTERIZER_INDEX    12

    { FC_OUTLINE,	0, FcCompareBool,	13, 13 },
#define MATCH_OUTLINE	    12
#define MATCH_OUTLINE_INDEX	    13

    { FC_FONTVERSION,	0, FcCompareNumber,	14, 14 },
#define MATCH_FONTVERSION   13
#define MATCH_FONTVERSION_INDEX   14
};

#define NUM_MATCH_VALUES    15

static FcBool matchObjectPtrsInit = FcFalse;

static void
FcMatchObjectPtrsInit (void)
{
    _FcMatchers[MATCH_FOUNDRY].objectPtr = FcObjectToPtr(FC_FOUNDRY);
    _FcMatchers[MATCH_CHARSET].objectPtr = FcObjectToPtr(FC_CHARSET);
    _FcMatchers[MATCH_FAMILY].objectPtr = FcObjectToPtr(FC_FAMILY);
    _FcMatchers[MATCH_LANG].objectPtr = FcObjectToPtr(FC_LANG);
    _FcMatchers[MATCH_SPACING].objectPtr = FcObjectToPtr(FC_SPACING);
    _FcMatchers[MATCH_PIXEL_SIZE].objectPtr = FcObjectToPtr(FC_PIXEL_SIZE);
    _FcMatchers[MATCH_STYLE].objectPtr = FcObjectToPtr(FC_STYLE);
    _FcMatchers[MATCH_SLANT].objectPtr = FcObjectToPtr(FC_SLANT);
    _FcMatchers[MATCH_WEIGHT].objectPtr = FcObjectToPtr(FC_WEIGHT);
    _FcMatchers[MATCH_WIDTH].objectPtr = FcObjectToPtr(FC_WIDTH);
    _FcMatchers[MATCH_ANTIALIAS].objectPtr = FcObjectToPtr(FC_ANTIALIAS);
    _FcMatchers[MATCH_RASTERIZER].objectPtr = FcObjectToPtr(FC_RASTERIZER);
    _FcMatchers[MATCH_OUTLINE].objectPtr = FcObjectToPtr(FC_OUTLINE);
    _FcMatchers[MATCH_FONTVERSION].objectPtr = FcObjectToPtr(FC_FONTVERSION);
    matchObjectPtrsInit = FcTrue;
}

static FcMatcher*
FcObjectPtrToMatcher (FcObjectPtr o)
{
    int 	i;
    const char  *object = FcObjectPtrU(o);

    i = -1;
    switch (object[0]) {
    case 'f':
	switch (object[1]) {
	case 'o':
	    switch (object[2]) {
	    case 'u':
		i = MATCH_FOUNDRY; break;
	    case 'n':
		i = MATCH_FONTVERSION; break;
	    }
	    break;
	case 'a':
	    i = MATCH_FAMILY; break;
	}
	break;
    case 'c':
	i = MATCH_CHARSET; break;
    case 'a':
	i = MATCH_ANTIALIAS; break;
    case 'l':
	i = MATCH_LANG; break;
    case 's':
	switch (object[1]) {
	case 'p':
	    i = MATCH_SPACING; break;
	case 't':
	    i = MATCH_STYLE; break;
	case 'l':
	    i = MATCH_SLANT; break;
	}
	break;
    case 'p':
	i = MATCH_PIXEL_SIZE; break;
    case 'w':
	switch (object[1]) {
	case 'i':
	    i = MATCH_WIDTH; break;
	case 'e':
	    i = MATCH_WEIGHT; break;
	}
	break;
    case 'r':
	i = MATCH_RASTERIZER; break;
    case 'o':
	i = MATCH_OUTLINE; break;
    }

    if (i < 0)
	return 0;

    if (!matchObjectPtrsInit)
        FcMatchObjectPtrsInit();

    if (o != _FcMatchers[i].objectPtr)
	return 0;

    return _FcMatchers+i;
}

static FcBool
FcCompareValueList (FcObjectPtr o,
		    FcValueListPtr v1orig,	/* pattern */
		    FcValueListPtr v2orig,	/* target */
		    FcValue	*bestValue,
		    double	*value,
		    FcResult	*result)
{
    FcValueListPtr  v1, v2;
    FcValueList     *v1_ptrU, *v2_ptrU;
    double    	    v, best, bestStrong, bestWeak;
    int		    j;
    const char	    *object = FcObjectPtrU(o);
    FcMatcher       *match = FcObjectPtrToMatcher(o);

    if (!match)
    {
	if (bestValue)
	    *bestValue = FcValueCanonicalize(&FcValueListPtrU(v2orig)->value);
	return FcTrue;
    }

    best = 1e99;
    bestStrong = 1e99;
    bestWeak = 1e99;
    j = 0;
    for (v1 = v1orig, v1_ptrU = FcValueListPtrU(v1); v1_ptrU;
	 v1 = v1_ptrU->next, v1_ptrU = FcValueListPtrU(v1))
    {
	for (v2 = v2orig, v2_ptrU = FcValueListPtrU(v2); v2_ptrU;
	     v2 = v2_ptrU->next, v2_ptrU = FcValueListPtrU(v2))
	{
	    v = (match->compare) (&v1_ptrU->value, &v2_ptrU->value);
	    if (v < 0)
	    {
		*result = FcResultTypeMismatch;
		return FcFalse;
	    }
	    v = v * 100 + j;
	    if (v < best)
	    {
		if (bestValue)
		    *bestValue = FcValueCanonicalize(&v2_ptrU->value);
		best = v;
	    }
	    if (v1_ptrU->binding == FcValueBindingStrong)
	    {
		if (v < bestStrong)
		    bestStrong = v;
	    }
	    else
	    {
		if (v < bestWeak)
		    bestWeak = v;
	    }
	}
	j++;
    }
    if (FcDebug () & FC_DBG_MATCHV)
    {
	printf (" %s: %g ", object, best);
	FcValueListPrint (v1orig);
	printf (", ");
	FcValueListPrint (v2orig);
	printf ("\n");
    }
    if (value)
    {
	int weak    = match->weak;
	int strong  = match->strong;
	if (weak == strong)
	    value[strong] += best;
	else
	{
	    value[weak] += bestWeak;
	    value[strong] += bestStrong;
	}
    }
    return FcTrue;
}

/*
 * Return a value indicating the distance between the two lists of
 * values
 */

static FcBool
FcCompare (FcPattern	*pat,
	   FcPattern	*fnt,
	   double	*value,
	   FcResult	*result)
{
    int		    i, i1, i2;
    
    for (i = 0; i < NUM_MATCH_VALUES; i++)
	value[i] = 0.0;
    
    i1 = 0;
    i2 = 0;
    while (i1 < pat->num && i2 < fnt->num)
    {
	FcPatternElt *elt_i1 = FcPatternEltU(pat->elts)+i1;
	FcPatternElt *elt_i2 = FcPatternEltU(fnt->elts)+i2;

	i = FcObjectPtrCompare(elt_i1->object, elt_i2->object);
	if (i > 0)
	    i2++;
	else if (i < 0)
	    i1++;
	else
	{
	    if (!FcCompareValueList (elt_i1->object,
				     elt_i1->values, elt_i2->values,
				     0, value, result))
		return FcFalse;
	    i1++;
	    i2++;
	}
    }
    return FcTrue;
}

FcPattern *
FcFontRenderPrepare (FcConfig	    *config,
		     FcPattern	    *pat,
		     FcPattern	    *font)
{
    FcPattern	    *new;
    int		    i;
    FcPatternElt    *fe, *pe;
    FcValue	    v;
    FcResult	    result;
    
    new = FcPatternCreate ();
    if (!new)
	return 0;
    for (i = 0; i < font->num; i++)
    {
	fe = FcPatternEltU(font->elts)+i;
	pe = FcPatternFindElt (pat, FcObjectPtrU(fe->object));
	if (pe)
	{
	    if (!FcCompareValueList (pe->object, pe->values, 
				     fe->values, &v, 0, &result))
	    {
		FcPatternDestroy (new);
		return 0;
	    }
	}
	else
	    v = FcValueCanonicalize(&FcValueListPtrU(fe->values)->value);
	FcPatternAdd (new, FcObjectPtrU(fe->object), v, FcFalse);
    }
    for (i = 0; i < pat->num; i++)
    {
	pe = FcPatternEltU(pat->elts)+i;
	fe = FcPatternFindElt (font, FcObjectPtrU(pe->object));
	if (!fe)
	    FcPatternAdd (new, FcObjectPtrU(pe->object), 
                          FcValueCanonicalize(&FcValueListPtrU(pe->values)->value), FcTrue);
    }

    if (FcPatternFindElt (font, FC_FILE))
	FcPatternTransferFullFname (new, font);

    FcConfigSubstituteWithPat (config, new, pat, FcMatchFont);
    return new;
}

FcPattern *
FcFontSetMatch (FcConfig    *config,
		FcFontSet   **sets,
		int	    nsets,
		FcPattern   *p,
		FcResult    *result)
{
    double	    score;
    double	    bestscore;
    int		    f;
    FcFontSet	    *s;
    FcPattern	    *best;
    int		    scoring_index;
    int		    *sets_offset;
    int		    set;
    int		    nfonts;
    int		    fonts_left;
    FcMatcher	    *matcher;
    FcMatcher	    *strong_matchers[NUM_MATCH_VALUES];
    FcMatcher	    *weak_matchers[NUM_MATCH_VALUES];
    FcPatternElt    *pat_elts[NUM_MATCH_VALUES];
    int		    pat_elt;
    int		    *match_blocked;
    int		    block_start;

    if (!nsets || !sets || !p)
    {
	*result = FcResultNoMatch;
	return 0;
    }

    if (FcDebug () & FC_DBG_MATCH)
    {
	printf ("Match ");
	FcPatternPrint (p);
    }
    if (!config)
    {
	config = FcConfigGetCurrent ();
	if (!config)
	{
	    *result = FcResultOutOfMemory;
	    return 0;
	}
    }

    sets_offset = (int *)calloc(nsets, sizeof (int));

    nfonts = 0;
    for (set = 0; set < nsets; ++set)
    {
	sets_offset[set] = nfonts;
	if (sets[set]) 
	    nfonts += sets[set]->nfont;
    }

    fonts_left = nfonts;

    match_blocked = (int*)calloc(nfonts, sizeof(int));

    /* Find out all necessary matchers first, so we don't need to find them
     * in every loop.
     */

    memset(strong_matchers, 0, sizeof (FcMatcher*) * NUM_MATCH_VALUES);
    memset(weak_matchers, 0, sizeof (FcMatcher*) * NUM_MATCH_VALUES);
    memset(pat_elts, 0, sizeof (FcPatternElt*) * NUM_MATCH_VALUES);

    for (pat_elt = 0; pat_elt < p->num; ++pat_elt)
    {
	matcher = FcObjectPtrToMatcher
			((FcPatternEltU(p->elts)+pat_elt)->object);
	if (matcher)
	{
	    strong_matchers[matcher->strong] = matcher;
	    weak_matchers[matcher->weak] = matcher;
	    pat_elts [matcher->strong] = pat_elts [matcher->weak] =
		    (FcPatternEltU(p->elts)+pat_elt);
	}
    }

    /* The old algorithm checked if each font beat 'best', 
     * scanning all of the value lists for all of the pattern elts. */
    /* This algorithm checks each font on a element-by-element basis
     * and blocks fonts that have already lost on some element from
     * further consideration from being best.  Basically, we've
     * swapped the order of loops and short-circuited fonts that
     * are out of contention right away.
     * This saves a lot of time! */
    best = 0;
    block_start = 0;
    for (scoring_index = 0; scoring_index < NUM_MATCH_VALUES; ++scoring_index)
    {
	FcValueListPtr	v1;
	FcValueList	*v1_ptrU;
	int		v1_offset = 0;

	if (!strong_matchers [scoring_index] && !weak_matchers [scoring_index])
	    continue;

	for (v1 = pat_elts[scoring_index]->values, v1_ptrU = FcValueListPtrU(v1);
	     v1_ptrU;
	     v1 = v1_ptrU->next, v1_ptrU = FcValueListPtrU(v1), ++v1_offset)
	{
	    matcher = (v1_ptrU->binding == FcValueBindingWeak) ?
		weak_matchers[scoring_index] : strong_matchers[scoring_index];

	    if (!matcher) continue;

	    bestscore = 1e99;

	    if (FcDebug () & FC_DBG_MATCHV)
	    {
		printf("Scoring Index %d, Value %d: %d(%d) fonts left\n",
			scoring_index, v1_offset, fonts_left, nfonts);
	    }

	    for (set = 0; set < nsets; ++set)
	    {
		s = sets[set];
		if (!s) continue;

		/* All fonts before block_start should have been knocked out. */
		for (f = (block_start > sets_offset[set]) ? (block_start - sets_offset[set]) : 0;
		     f < s->nfont; ++f)
		{
		    int		    cand_elt;
		    FcPatternElt    *cand_elts;

		    if (match_blocked[f + sets_offset[set]] == 1)
			continue;

		    score = 1e99;
		    cand_elts = FcPatternEltU(s->fonts[f]->elts);

		    /* Look for the appropriate element in this candidate
		     * pattern 'f' and evaluate its score wrt 'p'. */
		    for (cand_elt = 0; cand_elt < s->fonts[f]->num; ++cand_elt)
		    {
			if (cand_elts[cand_elt].object == pat_elts[scoring_index]->object)
			{
			    FcValueListPtr  v2;
			    FcValueList	    *v2_ptrU;

			    for (v2 = cand_elts[cand_elt].values, v2_ptrU = FcValueListPtrU(v2);
				 v2_ptrU;
				 v2 = v2_ptrU->next, v2_ptrU = FcValueListPtrU(v2))
			    {
				double v = (matcher->compare)(&v1_ptrU->value, &v2_ptrU->value);

				if (v < 0)
				{
				    *result = FcResultTypeMismatch;
				    free (match_blocked);
				    free (sets_offset);
				    return 0;
				}

				/* I'm actually kind of surprised that
				 * this isn't v + 100 * v1_offset. -PL */
				v = v * 100 + v1_offset;
				/* The old patch said score += v, which
				 * seems to be wrong when you have
				 * multiple matchers.  This takes the
				 * best score it can find for that font. */
				if (v < score)
				    score = v;
			    }
			}
		    }

		    /* We had no matching, just try the next one */
		    if (score == 1e99)
		    {
			match_blocked[f + sets_offset[set]] = 2;
			continue;
		    }
		    match_blocked[f + sets_offset[set]] = 0;
		    /* If there's a previous champion, and current score
		     * beats previous best score, on this element, then
		     * knock out the previous champion and anything
		     * else that we would have visited previous to f;
		     * clearly anything previous to f would have been
		     * less than f on this score. */
		    if (!best || score < bestscore)
		    {
			if (best) 
			{
			    int b;
			    for (b = block_start; b < f + sets_offset[set]; ++b)
				if (!match_blocked[b])
				{
				    match_blocked[b] = 1;
				    --fonts_left;
				}
			}

			bestscore = score;
			best = s->fonts[f];
			/* This kills too many fonts, unfortunately. */
			/* block_start = f + sets_offset[set]; */
		    }

		    /* If f loses, then it's out too. */
		    if (best && score > bestscore)
		    {
			match_blocked[f + sets_offset[set]] = 1;
			--fonts_left;
		    }

		    /* If there is only 1 font left and the best is set,
		     * then just return this font
		     */
		    if (fonts_left == 1 && best)
			goto end;

		    /* Otherwise, f is equal to best on this element.
		     * Carry on to next pattern element. */
		}
	    }
	    if ((FcDebug () & FC_DBG_MATCHV) && best)
	    {
		printf ("Best match (scoring index %d) candidate %d ", scoring_index, block_start);
		FcPatternPrint (best);
	    }
	}
    }

end:
    free (match_blocked);
    free (sets_offset);

    if ((FcDebug () & FC_DBG_MATCH) && best)
    {
	printf ("Best match (scoring index %d) %d ", scoring_index, block_start);
	FcPatternPrint (best);
    }
    if (!best)
    {
	*result = FcResultNoMatch;
	return 0;
    }
    return FcFontRenderPrepare (config, p, best);
}

FcPattern *
FcFontMatch (FcConfig	*config,
	     FcPattern	*p, 
	     FcResult	*result)
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
    return FcFontSetMatch (config, sets, nsets, p, result);
}

typedef struct _FcSortNode {
    FcPattern	*pattern;
    double	score[NUM_MATCH_VALUES];
} FcSortNode;

static int
FcSortCompare (const void *aa, const void *ab)
{
    FcSortNode  *a = *(FcSortNode **) aa;
    FcSortNode  *b = *(FcSortNode **) ab;
    double	*as = &a->score[0];
    double	*bs = &b->score[0];
    double	ad = 0, bd = 0;
    int         i;

    i = NUM_MATCH_VALUES;
    while (i-- && (ad = *as++) == (bd = *bs++))
	;
    return ad < bd ? -1 : ad > bd ? 1 : 0;
}

static FcBool
FcSortWalk (FcSortNode **n, int nnode, FcFontSet *fs, FcCharSet **cs, FcBool trim, FcBool build_cs)
{
    FcCharSet	*ncs;
    FcSortNode	*node;

    while (nnode--)
    {
	node = *n++;
	if (FcPatternGetCharSet (node->pattern, FC_CHARSET, 0, &ncs) == 
	    FcResultMatch)
	{
	    /*
	     * If this font isn't a subset of the previous fonts,
	     * add it to the list
	     */
	    if (!trim || !*cs || !FcCharSetIsSubset (ncs, *cs))
	    {
                if (trim || build_cs)
                {
                    if (*cs)
                    {
                        ncs = FcCharSetUnion (ncs, *cs);
                        if (!ncs)
                            return FcFalse;
                        FcCharSetDestroy (*cs);
                    }
                    else
                        ncs = FcCharSetCopy (ncs);
                    *cs = ncs;
                }

		FcPatternReference (node->pattern);
		if (FcDebug () & FC_DBG_MATCH)
		{
		    printf ("Add ");
		    FcPatternPrint (node->pattern);
		}
		if (!FcFontSetAdd (fs, node->pattern))
		{
		    FcPatternDestroy (node->pattern);
		    return FcFalse;
		}
	    }
	}
    }
    return FcTrue;
}

void
FcFontSetSortDestroy (FcFontSet *fs)
{
    FcFontSetDestroy (fs);
}

FcFontSet *
FcFontSetSort (FcConfig	    *config,
	       FcFontSet    **sets,
	       int	    nsets,
	       FcPattern    *p,
	       FcBool	    trim,
	       FcCharSet    **csp,
	       FcResult	    *result)
{
    FcFontSet	    *ret;
    FcFontSet	    *s;
    FcSortNode	    *nodes;
    FcSortNode	    **nodeps, **nodep;
    int		    nnodes;
    FcSortNode	    *new;
    FcCharSet	    *cs;
    int		    set;
    int		    f;
    int		    i;
    int		    nPatternLang;
    FcBool    	    *patternLangSat;
    FcValue	    patternLang;

    if (FcDebug () & FC_DBG_MATCH)
    {
	printf ("Sort ");
	FcPatternPrint (p);
    }
    nnodes = 0;
    for (set = 0; set < nsets; set++)
    {
	s = sets[set];
	if (!s)
	    continue;
	nnodes += s->nfont;
    }
    if (!nnodes)
	goto bail0;
    
    for (nPatternLang = 0;
	 FcPatternGet (p, FC_LANG, nPatternLang, &patternLang) == FcResultMatch;
	 nPatternLang++)
	;
	
    /* freed below */
    nodes = malloc (nnodes * sizeof (FcSortNode) + 
		    nnodes * sizeof (FcSortNode *) +
		    nPatternLang * sizeof (FcBool));
    if (!nodes)
	goto bail0;
    nodeps = (FcSortNode **) (nodes + nnodes);
    patternLangSat = (FcBool *) (nodeps + nnodes);
    
    new = nodes;
    nodep = nodeps;
    for (set = 0; set < nsets; set++)
    {
	s = sets[set];
	if (!s)
	    continue;
	for (f = 0; f < s->nfont; f++)
	{
	    if (FcDebug () & FC_DBG_MATCHV)
	    {
		printf ("Font %d ", f);
		FcPatternPrint (s->fonts[f]);
	    }
	    new->pattern = s->fonts[f];
	    if (!FcCompare (p, new->pattern, new->score, result))
		goto bail1;
	    if (FcDebug () & FC_DBG_MATCHV)
	    {
		printf ("Score");
		for (i = 0; i < NUM_MATCH_VALUES; i++)
		{
		    printf (" %g", new->score[i]);
		}
		printf ("\n");
	    }
	    *nodep = new;
	    new++;
	    nodep++;
	}
    }

    nnodes = new - nodes;
    
    qsort (nodeps, nnodes, sizeof (FcSortNode *),
	   FcSortCompare);
    
    for (i = 0; i < nPatternLang; i++)
	patternLangSat[i] = FcFalse;
    
    for (f = 0; f < nnodes; f++)
    {
	FcBool	satisfies = FcFalse;
	/*
	 * If this node matches any language, go check
	 * which ones and satisfy those entries
	 */
	if (nodeps[f]->score[MATCH_LANG_INDEX] < nPatternLang)
	{
	    for (i = 0; i < nPatternLang; i++)
	    {
		FcValue	    nodeLang;
		
		if (!patternLangSat[i] &&
		    FcPatternGet (p, FC_LANG, i, &patternLang) == FcResultMatch &&
		    FcPatternGet (nodeps[f]->pattern, FC_LANG, 0, &nodeLang) == FcResultMatch)
		{
		    double  compare = FcCompareLang (&patternLang, &nodeLang);
		    if (compare >= 0 && compare < 2)
		    {
			if (FcDebug () & FC_DBG_MATCHV)
			{
			    FcChar8 *family;
			    FcChar8 *style;

			    if (FcPatternGetString (nodeps[f]->pattern, FC_FAMILY, 0, &family) == FcResultMatch &&
				FcPatternGetString (nodeps[f]->pattern, FC_STYLE, 0, &style) == FcResultMatch)
				printf ("Font %s:%s matches language %d\n", family, style, i);
			}
			patternLangSat[i] = FcTrue;
			satisfies = FcTrue;
			break;
		    }
		}
	    }
	}
	if (!satisfies)
	    nodeps[f]->score[MATCH_LANG_INDEX] = 1000.0;
    }

    /*
     * Re-sort once the language issues have been settled
     */
    qsort (nodeps, nnodes, sizeof (FcSortNode *),
	   FcSortCompare);

    ret = FcFontSetCreate ();
    if (!ret)
	goto bail1;

    cs = 0;

    if (!FcSortWalk (nodeps, nnodes, ret, &cs, trim, (csp!=0)))
	goto bail2;

    if (csp)
	*csp = cs;
    else
    {
        if (cs)
            FcCharSetDestroy (cs);
    }

    free (nodes);

    return ret;

bail2:
    if (cs)
	FcCharSetDestroy (cs);
    FcFontSetDestroy (ret);
bail1:
    free (nodes);
bail0:
    return 0;
}

FcFontSet *
FcFontSort (FcConfig	*config,
	    FcPattern	*p, 
	    FcBool	trim,
	    FcCharSet	**csp,
	    FcResult	*result)
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
    return FcFontSetSort (config, sets, nsets, p, trim, csp, result);
}
