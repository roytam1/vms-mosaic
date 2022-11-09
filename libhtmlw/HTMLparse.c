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
#include "../config.h"

/* Copyright (C) 1997 - G.Dauphin
 * Copyright (C) 1998, 1999, 2000 - The VMS Mosaic Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "HTMLparse.h"
#include "HTMLmiscdefs.h"
#include "HTMLP.h"
#include "HTMLPutil.h"

#include "../libnut/str-tools.h"

extern int appletSupportEnabled;
extern int tableSupportEnabled;

#ifndef DISABLE_TRACE
extern int reportBugs;
extern int htmlwTrace;
#endif

/* A hack to speed up caseless_equal.  Thanks to Quincey Koziol for
 * developing it.
 */
unsigned char map_table[256] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
    24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
    45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,97,98,
    99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,
    116,117,118,119,120,121,122,91,92,93,94,95,96,97,98,99,100,101,102,
    103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,
    120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,
    137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,
    154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,
    171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,
/*
    188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,
    205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,
    222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,
 ah, but 192-214 and 216 to 222 are also uppercase.  Stellan */
    188,189,190,191,224,225,226,227,228,229,230,231,232,233,234,235,236,
    237,238,239,240,241,242,243,244,245,246,215,248,249,250,251,252,253,
    254,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,
    239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};

#define TOLOWER(x)	(map_table[x])

/* Locale-independent /Stellan */
#define ISSPACE(x)	( (x)>0 && ( (x)<=' ' ) || ((x)>=127) && ((x)<160) )

typedef struct amp_esc_rec {
	char *tag;
	char value;
} AmpEsc;

#if (defined(VMS) && (__DECC_VER >= 50790004))
#pragma message disable(INTCONSTTRUNC)
#endif
static AmpEsc AmpEscapes[] = {
	{"lt", '<'},
	{"LT", '<'},
	{"gt", '>'},
	{"GT", '>'},
	{"amp", '&'},
	{"AMP", '&'},
	{"quot", QUOT_CONST},
	{"QUOT", QUOT_CONST},
	{"nbsp", NBSP_CONST},
	{"iexcl", '\241'},
	{"cent", '\242'},
	{"pound", '\243'},
	{"curren", '\244'},
	{"yen", '\245'},
	{"brvbar", '\246'},
	{"sect", '\247'},
	{"uml", '\250'},
	{"copy", '\251'},
	{"ordf", '\252'},
	{"laquo", '\253'},
	{"not", '\254'},
	{"shy", '\255'},
	{"reg", '\256'},
	{"macr", '\257'},
	{"hibar", '\257'},
	{"deg", '\260'},
	{"plusmn", '\261'},
	{"sup2", '\262'},
	{"sup3", '\263'},
	{"acute", '\264'},
	{"micro", '\265'},
	{"para", '\266'},
	{"middot", '\267'},
	{"cedil", '\270'},
	{"sup1", '\271'},
	{"ordm", '\272'},
	{"raquo", '\273'},
	{"frac14", '\274'},
	{"frac12", '\275'},
	{"frac34", '\276'},
	{"iquest", '\277'},
	{"Agrave", '\300'},
	{"Aacute", '\301'},
	{"Acirc", '\302'},
	{"Atilde", '\303'},
	{"Auml", '\304'},
	{"Aring", '\305'},
	{"AElig", '\306'},
	{"Ccedil", '\307'},
	{"Egrave", '\310'},
	{"Eacute", '\311'},
	{"Ecirc", '\312'},
	{"Euml", '\313'},
	{"Igrave", '\314'},
	{"Iacute", '\315'},
	{"Icirc", '\316'},
	{"Iuml", '\317'},
	{"ETH", '\320'},
	{"Ntilde", '\321'},
	{"Ograve", '\322'},
	{"Oacute", '\323'},
	{"Ocirc", '\324'},
	{"Otilde", '\325'},
	{"Ouml", '\326'},
	{"times", '\327'},
	{"Oslash", '\330'},
	{"Ugrave", '\331'},
	{"Uacute", '\332'},
	{"Ucirc", '\333'},
	{"Uuml", '\334'},
	{"Yacute", '\335'},
	{"THORN", '\336'},
	{"szlig", '\337'},
	{"agrave", '\340'},
	{"aacute", '\341'},
	{"acirc", '\342'},
	{"atilde", '\343'},
	{"auml", '\344'},
	{"aring", '\345'},
	{"aelig", '\346'},
	{"ccedil", '\347'},
	{"egrave", '\350'},
	{"eacute", '\351'},
	{"ecirc", '\352'},
	{"euml", '\353'},
	{"igrave", '\354'},
	{"iacute", '\355'},
	{"icirc", '\356'},
	{"iuml", '\357'},
	{"eth", '\360'},
	{"ntilde", '\361'},
	{"ograve", '\362'},
	{"oacute", '\363'},
	{"ocirc", '\364'},
	{"otilde", '\365'},
	{"ouml", '\366'},
	{"divide", '\367'},
	{"oslash", '\370'},
	{"ugrave", '\371'},
	{"uacute", '\372'},
	{"ucirc", '\373'},
	{"uuml", '\374'},
	{"yacute", '\375'},
	{"thorn", '\376'},
	{"yuml", '\377'},
	{NULL, '\0'},
};
#if (defined(VMS) && (__DECC_VER >= 50790004))
#pragma message enable(INTCONSTTRUNC)
#endif

static MarkType ParseMarkType(char *str);

/* Check if two strings are equal, ignoring case.  The strings must be of the
 * same length to be equal.  Return 1 if equal, 0 otherwise.
 */
int caseless_equal(char *str1, char *str2)
{
	if (!str1 || !str2)
		return(0);
	while (*str1 && *str2) {
		if (TOLOWER(*str1) != TOLOWER(*str2))
			return(0);
		str1++;
		str2++;
	}
	if (!*str1 && !*str2)
		return(1);
	return(0);
}

#if 0
/* Check if two strings are equal in the first count characters, ignoring case.
 * The strings must both be at least of length count to be equal.
 * return 1 if equal, 0 otherwise.
 */
static int caseless_equal_prefix(char *str1, char *str2, int cnt)
{
	int i;

	if (!str1 || !str2)
		return(0);
	if (cnt < 1)
		return(1);
	for (i=0; i < cnt; i++) {
		if (TOLOWER(*str1) != TOLOWER(*str2))
			return(0);
		str1++;
		str2++;
	}
	return(1);
}
#endif

/* Clean up the white space in a string.  Remove all leading and trailing
 * whitespace, and turn all internal whitespace into single spaces separating
 * words.  The cleaning is done by rearranging the chars in the passed txt
 * buffer.  The resultant string will probably be shorter, it can never get
 * longer.
 *
 * NOTE! For 8-bit characters this only works if isspace() works.
 * When using the Posix isspace() and C locale
 * it does not, identifying chars>127 as space!
 * This messes up an <OPTION> tag with characters with value >127.
 * We should NOT use a potentially Locale-dependent function for parsing
 * HTML! /Stellan
 */
void clean_white_space(char *txt)
{
	char *ptr;
	char *start;

	start = txt;
	ptr = txt;
	while (ISSPACE((int)*ptr))  /* Remove leading white space */
		ptr++;

	/* Find a word, copying if we removed some space already */
	if (start == ptr) {
		while ((!ISSPACE((int)*ptr)) && *ptr)
			ptr++;
		start = ptr;
	} else {
		while ((!ISSPACE((int)*ptr)) && *ptr)
			*start++ = *ptr++;
	}
	while (*ptr) {
		while (ISSPACE((int)*ptr))  /* Remove trailing whitespace */
			ptr++;
		if (!*ptr)
			break;
		/* If there are more words, insert a space and if space was 
		 * removed, move up remaining text.
		 */
		*start++ = ' ';
		if (start == ptr) {
			while ((!ISSPACE((int)*ptr)) && *ptr)
				ptr++;
			start = ptr;
		} else {
			while ((!ISSPACE((int)*ptr)) && *ptr)
				*start++ = *ptr++;
		}
	}
	*start = '\0';
}

/*
 * Parse an amperstand escape, and return the appropriate character, or
 * '\0' on error.
 * We should really only use caseless_equal_prefix for unterminated, and use
 * caseless_equal otherwise, but since there are so many escapes, and I
 * don't want to type everything twice, I always use caseless_equal_prefix.
 * Turns out the escapes are case sensitive, use strncmp.
 */
char ExpandEscapes(esc, endp)
	char *esc;
	char **endp;
{
	int cnt;
	char val;

	esc++;
	if (*esc == '#') {
		char *tptr;
		char tchar;

		tptr = (char *)(esc + 1);
		while (isdigit((int)*tptr)) {
			tptr++;
		}
		tchar = *tptr;
		*tptr = '\0';
		val = (char)atoi((esc + 1));
		*tptr = tchar;
		*endp = tptr;
	} else {
		int escLen, ampLen;
		cnt = 0;
		escLen = strlen(esc);	
		while (AmpEscapes[cnt].tag) {
			ampLen = strlen(AmpEscapes[cnt].tag);
			if (!strncmp(esc, AmpEscapes[cnt].tag, ampLen)) {
				val = AmpEscapes[cnt].value;
				*endp = (char *)(esc + ampLen);
				break;
			}
			cnt++;
		}
		if (AmpEscapes[cnt].tag == NULL) {
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr, "Error bad & string\n");
			}
#endif
			val = '\0';
			*endp = (char *)NULL;
		}
	}

	return(val);
}


/*
 * Clean the special HTML character escapes out of the text and replace
 * them with the appropriate characters "&lt;" = "<", "&gt;" = ">",
 * "&amp;" = "&"
 * GAG:  apparently &lt etc. can be left unterminated, what a nightmare.
 * The '&' character must be immediately followed by a letter to be
 * a valid escape sequence.  Other &'s are left alone.
 * The cleaning is done by rearranging chars in the passed txt buffer.
 * If any escapes are replaced, the string becomes shorter.
 */
void clean_text(txt)
	char *txt;
{
	char *ptr;
	char *ptr2;
	char *start;
	char *text;
	char *tend;
	char tchar;
	char val;

	if (!txt)
		return;

	/*
	 * Quick scan to find escape sequences.
	 * Escape is '&' followed by a letter (or a hash mark).
	 * Return if there are none.
	 */
	ptr = txt;
	while (*ptr) {
		if ((*ptr == '&') &&
			((isalpha((int)*(ptr + 1))) || (*(ptr + 1) == '#'))) {
			break;
		}
		ptr++;
	}
	if (!*ptr)
		return;

	/*
	 * Loop, replacing escape sequences, and moving up remaining text.
	 */
	ptr2 = ptr;
	while (*ptr) {

		/*
		 * Extract the escape sequence from start to ptr
		 */
		start = ptr;
		ptr++;
		while ((*ptr != ';') && !ISSPACE((int)*ptr) &&
		       *ptr && (*ptr != '&')) {
			ptr++;
		}
		if (!*ptr || (*ptr != ';')) {
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr,
					"Warning: unterminated & (%s)\n",
					start);
			}
#endif
		}

		/*
		 * Copy the escape sequence into a separate buffer.
		 * Then clean spaces so the "& lt ;" = "&lt;" etc.
		 * The cleaning should be unnecessary.
		 */
		tchar = *ptr;
		*ptr = '\0';
		text = (char *)malloc(strlen(start) + 1);
		if (!text) {
#ifndef DISABLE_TRACE
			if (htmlwTrace || reportBugs) {
				fprintf(stderr,
					"Cannot malloc space for & text\n");
			}
#endif
			*ptr = tchar;
			return;
		}
		strcpy(text, start);
		*ptr = tchar;
		clean_white_space(text);

		/*
		 * Replace escape sequence with appropriate character
		 */
		val = ExpandEscapes(text, &tend);
		if (val) {
			*tend = '\0';
			ptr = (char *)(start + strlen(text));
			if (*ptr == ';') {
				ptr++;
			}
			*ptr2 = val;
		} else {
		/*
		 * Invalid escape sequence.  Skip it.
		 */
#ifndef DISABLE_TRACE
			if (htmlwTrace) {
				fprintf(stderr, "Error bad & string\n");
			}
#endif
			ptr = start;
			*ptr2 = *ptr;
			ptr++;
		}
		free(text);

		/*
		 * Copy forward remaining text until the next
		 * escape sequence
		 */
		ptr2++;
		while (*ptr) {
			if ((*ptr == '&') &&
			    (isalpha((int)*(ptr + 1)) || (*(ptr + 1) == '#'))) {
				break;
			}
			*ptr2++ = *ptr++;
		}
	}
	*ptr2 = '\0';
}

/* Get a block of text from an HTML document.  All text from start to the end,
 * or the first mark (a mark is '<' followed by any char)
 * is returned in a malloced buffer.  Also, endp returns a pointer to the
 * next '<' or '\0'.  The returned text has already expanded '&' escapes.
 */
static char *get_text(char *start, char **endp, int *is_white)
{
	char *ptr;
	char *text;
	int len;

	*is_white = 1;
	len = 0;
	/* Copy text up to beginning of a mark, or the end */
	ptr = start;
	while (*ptr) {
		if ((*ptr == '<') &&
		    (isalpha((int)*(ptr + 1)) || (*(ptr + 1) == '!') ||
		     ((*(ptr + 1) == '/') && (isalpha((int)*(ptr + 2)) ||
		      (*(ptr + 2) == '!') ||
		      ((*(ptr + 2) == '\n') && isalpha((int)*(ptr + 3)))))))
			break;
		if (!ISSPACE(*ptr))
			*is_white = 0;
		ptr++;
		len++;
	}
	*endp = ptr;
	if (ptr == start)
		return(NULL);
	/* Copy the text into its own buffer, and remove escape sequences */
	text = (char *)malloc(len + 1);
	CHECK_OUT_OF_MEM(text);
	strncpy(text, start, len);
	text[len] = '\0';
	clean_text(text);
	return(text);
}

/*
 * Get the mark text between '<' and '>'.  From the text, determine
 * its type, and fill in a mark_up structure to return.  Also returns
 * endp pointing to the trailing '>' in the original string.
 */
static char *basetarget = NULL;

static MarkInfo *get_mark(HTMLWidget hw, char *start, char **endp)
{
	char *ptr;
	char *text;
	char *tptr;
	char tchar;
	MarkInfo *mark;
	int  comment = 0;       /* comment == 1 if we are in a comment */
	char *first_gt = NULL;  /* Keep track of ">" for old broken comments */
	int  quoted = 0;
	int  squoted = 0;
	int  equal = 0;

	if (!start || (*start != '<'))
		return(NULL);

	/* Check if we are in a comment, start tag is <!-- */
	/* Believe it or not, </!-- is also used */
	if (!strncmp(start, "<!--", 4) || !strncmp(start, "</!--", 5))
		comment = 1;
	start++;
	mark = GetMarkRec();
	ptr = start; 	/* Grab the mark text */

	/* Skip over the comment text */
	/* End tag is --*>, where * is zero or more spaces (ugh) */
	if (comment) {
		while (*ptr) {
			if ((*ptr == '>') && (!first_gt))
				first_gt = ptr;
			if (!strncmp(ptr, "--", 2)) {
				/* Found double dash(--) */
				ptr += 2;
				while (*ptr && ((*ptr == ' ') || 
				       (*ptr == '\n') || (*ptr == '-') ))
					ptr++;      /* Skip spaces and LFs */ 
				if (*ptr == '>') {  /* Completed end comment */
					*endp = ptr;
					mark->is_end = 1;
					mark->type = M_COMMENT;
					mark->start = NULL;
					mark->text = NULL;
					mark->end = NULL;
					mark->next = NULL;
					return(mark);
				}
			} else {  /* If no double dash (--) found */
				ptr++;
			}
		} /* If we get here, this document must use the old broken
		   * comment style */
		if (first_gt) {
			ptr = first_gt;
		}
	} /* End of: if (comment) */

	/* Find end of mark while ignoring <> in quotes */
	while (*ptr) {
		if (!quoted && !squoted && ((*ptr == '>') || (*ptr == '<')))
			break;
		/* Must be an '=' before having a quoted string */
		if (equal) {
			if ((*ptr == '\"') && !squoted) {
				if (quoted) {
					quoted = 0;
					equal = 0;
				} else {
					quoted = 1;
				}
			} else if ((*ptr == '\'') && !quoted) {
				if (squoted) {
					squoted = 0;
					equal = 0;
				} else {
					squoted = 1;
				}
			/* Must be only white space between '=' and first '"' */
			} else if (!quoted && !squoted && !ISSPACE(*ptr)) {
				equal = 0;
			}
		} else if (!quoted && !squoted && (*ptr == '&')) {
			/* Believe it or not! */
			if (!my_strncasecmp(ptr, "&gt;", 4)) {
				*ptr++ = ' ';
				*ptr++ = ' ';
				*ptr++ = ' ';
				*ptr = '>';
				break;
			}
		} else if (!quoted && !squoted && (*ptr == '=')) {
			equal = 1;
		}
		ptr++;
	}
	if (*ptr) {		/* Is on '>' or '<' */
		/* Backup if found '<' */
		if (*ptr == '<') {
			*endp = ptr - 1;
		} else {
			*endp = ptr;
		}
	} else {
		/* EOF and no closing '>' */
		*endp = NULL;
		free(mark);
		return(NULL);
	}

	/* Copy the mark text to its own buffer, and
	 * clean it of escapes and odd white space.
	 */
	tchar = *ptr;
	*ptr = '\0';
	text = (char *)malloc(strlen(start) + 1);
	CHECK_OUT_OF_MEM(text);
	strcpy(text, start);
	*ptr = tchar;
	clean_text(text);

	/* Set whether this is the start or end of a mark
	 * block, as well as determine its type.
	 */
	if (*text == '/') {
		mark->is_end = 1;
		if (*(text + 1) == '\n')
			mark->type = ParseMarkType((char *)(text + 2));
		else
			mark->type = ParseMarkType((char *)(text + 1));
		mark->start = NULL;
		mark->text = NULL;
		mark->end = text;
	} else {
	    mark->is_end = 0;
	    mark->type = ParseMarkType(text);
	    mark->start = text;
	    mark->text = NULL;
	    mark->end = NULL;
	    /* Check for tags specifying background colors and do the color
	     * map allocation.  Ignore body tag here since it comes early
	     * enough in the formatting to get its colors okay.  Also
	     * do USEMAPs here since fools tend to put them where they
	     * are hard to process later (like between table cells).
	     */
	     if (hw) switch(mark->type) { /* hw is NULL when in hotfile.c */
		case M_FONT:
			if (hw->html.font_colors) {
				tptr = ParseMarkTag(mark->start, MT_FONT,
						"color");
				if (tptr) {
					hw_do_color(hw,	"preallo", tptr, NULL);
					free(tptr);
				}
			}
			break;
		case M_BASE:
			if (!basetarget) {
				if (basetarget = ParseMarkTag(mark->start,
					 	MT_BASE, "target")) {
					if (!*basetarget) {
						free(basetarget);
						basetarget = NULL;
					}
				}
			}
			break;
		case M_BASEFONT:
			if (hw->html.font_colors) {
				tptr = ParseMarkTag(mark->start, MT_BASEFONT,
						"color");
				if (tptr) {
					hw_do_color(hw,	"preallo", tptr, NULL);
					free(tptr);
				}
			}
			break;
		case M_TABLE:
			if (hw->html.body_colors) {
				tptr = ParseMarkTag(mark->start, MT_TABLE,
						"bgcolor");
				if (tptr) {
					hw_do_color(hw,	"preallo", tptr, NULL);
					free(tptr);
				}
			}
			break;
		case M_TABLE_DATA:
			if (hw->html.body_colors) {
				tptr = ParseMarkTag(mark->start, MT_TABLE_DATA,
						"bgcolor");
				if (tptr) {
					hw_do_color(hw,	"preallo", tptr, NULL);
					free(tptr);
				}
			}
			break;
		case M_TABLE_HEADER:
			if (hw->html.body_colors) {
				tptr = ParseMarkTag(mark->start,
						MT_TABLE_HEADER, "bgcolor");
				if (tptr) {
					hw_do_color(hw, "preallo", tptr, NULL);
					free(tptr);
				}
			}
			break;
		case M_TABLE_ROW:
			if (hw->html.body_colors) {
				tptr = ParseMarkTag(mark->start, MT_TABLE_ROW,
						"bgcolor");
				if (tptr) {
					hw_do_color(hw,	"preallo", tptr, NULL);
					free(tptr);
				}
			}
			break;
		case M_AREA:
		    if (hw->html.cur_map) {
			AreaInfo *area;

			area = (AreaInfo *) malloc(sizeof(AreaInfo));
			area->next = NULL;
			if (hw->html.cur_map->areaList) {
				hw->html.cur_area->next = area;
			} else {
				hw->html.cur_map->areaList = area;
			}
			hw->html.cur_area = area;
			tptr = ParseMarkTag(mark->start, MT_AREA, "shape");
			if (tptr) {
				if (caseless_equal(tptr, "rect"))
					area->shape = AREA_RECT;
				else if (caseless_equal(tptr, "circle"))
					area->shape = AREA_CIRCLE;
				else if (!my_strncasecmp(tptr, "poly", 4))
					area->shape = AREA_POLY;
				else
					area->shape = -1;
				free(tptr);
			} else {
				area->shape = AREA_RECT;
			}
			area->alt = ParseMarkTag(mark->start, MT_AREA, "alt");
			area->href = ParseMarkTag(mark->start, MT_AREA, "href");
			area->target = ParseMarkTag(mark->start, MT_AREA,
						    "target");
			if (!area->target && basetarget) {
				area->target = strdup(basetarget);
			}
			tptr = ParseMarkTag(mark->start, MT_AREA, "nohref");
			if (tptr) {
				area->href = NULL;
				free(tptr);
			}
			tptr = ParseMarkTag(mark->start, MT_AREA, "coords");
			if (tptr && *tptr) {
				CoordInfo *coord;
				char *x, *y;

				coord = (CoordInfo *) malloc(sizeof(CoordInfo));
				area->coordList = coord;
				coord->next = NULL;
	     			/* Must have at least one x,y */
				if (!(x = strtok(tptr, ",")) ||
				    !(y = strtok(NULL, ", "))) {
					area->shape = -1;
					free(tptr);
					break;
				}
				coord->x = atoi(x);
				coord->y = atoi(y);
				coord->next = (CoordInfo *)malloc(
					sizeof(CoordInfo));
				coord = coord->next;
				coord->next = NULL;
				/* Must have at least another x */
				if (!(x = strtok(NULL, ","))) {
					area->shape = -1;
					free(tptr);
					break;
				} else {
					coord->x = atoi(x);
					/* If circle, then have enough */
					if (area->shape == AREA_CIRCLE) {
						free(tptr);
						break;
					}
				}
				if (!(y = strtok(NULL, ","))) {
					area->shape = -1;
					free(tptr);
					break;
				}
				coord->y = atoi(y);
				if (area->shape == AREA_RECT) {
					free(tptr);
					break;
				}
				/* Get rest of coords for a polygon */
				while (x = strtok(NULL, ",")) {
					if (y = strtok(NULL, ", ")) {
						coord->next =
						    (CoordInfo *)malloc(
							sizeof(CoordInfo));
						coord = coord->next;
						coord->x = atoi(x);
						coord->y = atoi(y);
						coord->next = NULL;
					} else {
						/* Missing the y */
						area->shape = -1;
						free(tptr);
						break;
					}
				}
				/* Mark the end of the polygon coords */
				coord->next = (CoordInfo *)malloc(
					sizeof(CoordInfo));
				coord = coord->next;
				coord->x = -1;
				coord->next = NULL;
				free(tptr);
			} else {
				if (tptr)
					free(tptr);
				area->coordList = NULL;
				area->shape = -1;
			}
		    }
		    break;
		default:
		    break;
	    }
	}
	/* Do MAP here because has both start and end tags */
	if (hw && (mark->type == M_MAP)) {
		if (mark->is_end) {
			hw->html.cur_map = NULL;
		} else {
			hw->html.cur_map = (MapInfo *) malloc(sizeof(MapInfo));
			if (hw->html.map_list) {
				hw->html.cur_map->next = hw->html.map_list;
			} else {
				hw->html.cur_map->next = NULL;
			}
			hw->html.map_list = hw->html.cur_map;
			hw->html.cur_map->areaList = NULL;
			hw->html.cur_map->name = ParseMarkTag(mark->start,
				MT_MAP, "name");
		}
	}
	mark->text = NULL;
	mark->next = NULL;
	return(mark);
}

/* Special version of get_text.  It reads all text up to the
 * end of the plain text mark, or the end of the file.
 */
static char *get_plain_text(HTMLWidget hw, char *start, char **endp)
{
	char *ptr;
	char *text;
	char tchar;

	if (!start)
		return(NULL);
	/* Read until stopped by end plain text mark. */
	ptr = start;
	while (*ptr) {
		/* Beginning of a mark is '<' followed by any letter, or
		 * followed by '!' for a comment, or '</' followed by any
		 * letter.
 		 */
		if ((*ptr == '<') &&
		    (isalpha((int)(*(ptr + 1))) ||
		     (*(ptr + 1) == '!') ||
		     ((*(ptr + 1) == '/') && (isalpha((int)(*(ptr + 2))))))) {
			MarkInfo *mp;
			char *ep;

			/* We think we found a mark.  If it is the end of
			 * plain text, break out
			 */
			mp = get_mark(hw, ptr, &ep);
			if (mp) {
				if (((mp->type == M_PLAIN_TEXT) ||
				     (mp->type == M_LISTING_TEXT)) &&
				    (mp->is_end)) {
					if (mp->end)
						free((char *)mp->end);
					free((char *)mp);
					break;
				}
				if (mp->start)
					free((char *)mp->start);
				if (mp->end)
					free((char *)mp->end);
				free((char *)mp);
			}
		}
		ptr++;
	}
	*endp = ptr;
	if (ptr == start)
		return(NULL);
	/* Copy text to its own buffer, and clean it of HTML escapes. */
	tchar = *ptr;
	*ptr = '\0';
	text = (char *)malloc(strlen(start) + 1);
	CHECK_OUT_OF_MEM(text);
	strcpy(text, start);
	*ptr = tchar;
	clean_text(text);
	return(text);
}

/* Add an object to the parsed object list.  Return a pointer to the
 * current (end) position in the list.  If the object is a normal text object
 * containing nothing but white space, throw it out, unless we have been
 * told to keep white space.
 */
static MarkInfo *AddObj(MarkInfo **listp, MarkInfo *current, MarkInfo *mark)
{
	if (!mark)
		return(current);

	/* Add object to either the head of the list for a new list,
	 * or at the end after the current pointer.
	 */
	if (*listp == NULL) {
		*listp = mark;
		current = *listp;
	} else {
		current->next = mark;
		current = current->next;
	}
	current->next = NULL;
	return(current);
}

/* Main parser of HTML text.  Takes raw text, and produces a linked
 * list of mark objects.  Mark objects are either text strings, or
 * starting and ending mark delimiters.
 */
MarkInfo *HTMLParse(HTMLWidget hw, char *str)
{
	char *start, *end;
	char *text, *tptr;
	MarkInfo *mark = NULL;
	MarkInfo *list = NULL;
	MarkInfo *current = NULL;
	int is_white = 0;		/* Is white text? */

	if (!str)
		return(NULL);

	if (hw) {
		hw->html.cur_map = NULL;
		hw->html.cur_area = NULL;
	}

	start = str;
	end = str;
	while (*start) {

	/* Get some text (if any).  If our last mark was a begin plain text
	 * we call a different function.  If last mark was <PLAINTEXT> we
	 * lump all the rest of the text in.
	 */
		if (mark && (mark->type == M_PLAIN_FILE) && (!mark->is_end)) {
			text = start;
			end = text;
			while (*end)
				end++;
			/* Copy text to its own malloced buffer,
			 * and clean it of HTML escapes. */
			tptr = (char *)malloc(strlen(text) + 1);
			CHECK_OUT_OF_MEM(tptr);
			strcpy(tptr, text);
			text = tptr;
		} else {
			if (mark && ((mark->type == M_PLAIN_TEXT) ||
			     (mark->type == M_LISTING_TEXT)) &&
			    (!mark->is_end)) {
				is_white = 0;
				text = get_plain_text(hw, start, &end);
			} else {
				text = get_text(start, &end, &is_white);
			}
		}
		/* If text is OK, put it into a mark structure,
		 * and add it to the linked list. */
		if (text) {
			mark = GetMarkRec();
			mark->type = M_NONE;	/* It's text */
			mark->is_end = 0;
			mark->start = NULL;
			mark->text = text;
			mark->is_white_text = is_white;
			mark->end = NULL;
			mark->next = NULL;
			mark->s_aps = NULL;
			mark->s_ats = NULL;
			mark->s_picd = NULL;
			mark->t_p1 = NULL;
			mark->anc_name = NULL;
			mark->anc_href = NULL;
			mark->anc_title = NULL;
			mark->anc_target = NULL;
			current = AddObj(&list, current, mark);
		}
		/* end is on '<' or '\0' */
		start = end;
		if (!*start)
			break;		/* End of html string, parse is done */

		/* Get the next mark if any, and if it is valid,
		 * add it to the linked list.  start is on '<'
		 * Loop until valid mark or end of string.
		 */
		while (!(mark = get_mark(hw, start, &end))) {
#ifndef DISABLE_TRACE
			if (htmlwTrace || reportBugs) {
				fprintf(stderr,
					"Error parsing, missing final '>'\n");
			}
#endif
			return(list);
		}
		/* end is on '>' or character prior to '<' if no '>' */

		mark->is_white_text = is_white = 0;
		mark->next = NULL;
		mark->s_aps = NULL;
		mark->s_ats = NULL;
		mark->s_picd = NULL;
		mark->t_p1 = NULL;
		mark->anc_name = NULL;
		mark->anc_href = NULL;
		mark->anc_title = NULL;
		mark->anc_target = NULL;
		current = AddObj(&list, current, mark);
		start = (char *)(end + 1);
		/* start is a pointer after the '>' character */
		if (mark && (mark->type == M_PLAIN_FILE || 
		     mark->type == M_PLAIN_TEXT || mark->type == M_PREFORMAT ||
		     mark->type == M_LISTING_TEXT) && (!mark->is_end)) {
		     /* A linefeed immediately after a <PLAINTEXT>, <XMP>, <PRE>
		      * or <LISTING> mark is to be ignored.
		      */
			if (*start == '\n')
				start++;
		} 
	}
	return(list);
}

/* Determine mark type from the identifying string passed */
static MarkType ParseMarkType(char *str)
{
	MarkType type;
	char *tptr;
	char tchar;
#if defined(VAXC) && !defined(__DECC)
	int vaxc_hack = 0;
#endif
	if (!str)
		return(M_NONE);
	type = M_UNKNOWN;
	tptr = str;
	while (*tptr) {
		if (ISSPACE((int)*tptr))
			break;
		tptr++;
	}
	tchar = *tptr;
	*tptr = '\0';
	if (caseless_equal(str, MT_ANCHOR)) {
		type = M_ANCHOR;
	} else if (caseless_equal(str, MT_TITLE)) {
		type = M_TITLE;
	} else if (caseless_equal(str, MT_FIXED)) {
		type = M_FIXED;
	} else if (caseless_equal(str, MT_BOLD)) {
		type = M_BOLD;
	} else if (caseless_equal(str, MT_ITALIC)) {
		type = M_ITALIC;
	} else if (caseless_equal(str, MT_EMPHASIZED)) {
		type = M_EMPHASIZED;
	} else if (caseless_equal(str, MT_STRONG)) {
		type = M_STRONG;
	} else if (caseless_equal(str, MT_CODE)) {
		type = M_CODE;
	} else if (caseless_equal(str, MT_SAMPLE)) {
		type = M_SAMPLE;
	} else if (caseless_equal(str, MT_KEYBOARD)) {
		type = M_KEYBOARD;
	} else if (caseless_equal(str, MT_VARIABLE)) {
		type = M_VARIABLE;
	} else if (caseless_equal(str, MT_CITATION)) {
		type = M_CITATION;
	} else if (caseless_equal(str, MT_DEFINE)) {
		type = M_DEFINE;
	} else if (caseless_equal(str, MT_STRIKEOUT) ||
		   caseless_equal(str, MT_STRIKEOUT2)) {
		type = M_STRIKEOUT;
	} else if (caseless_equal(str, MT_HEADER_1)) {
		type = M_HEADER_1;
	} else if (caseless_equal(str, MT_HEADER_2)) {
		type = M_HEADER_2;
	} else if (caseless_equal(str, MT_HEADER_3)) {
		type = M_HEADER_3;
	} else if (caseless_equal(str, MT_HEADER_4)) {
		type = M_HEADER_4;
	} else if (caseless_equal(str, MT_HEADER_5)) {
		type = M_HEADER_5;
	} else if (caseless_equal(str, MT_HEADER_6)) {
		type = M_HEADER_6;
	} else if (caseless_equal(str, MT_ADDRESS)) {
		type = M_ADDRESS;
	} else if (caseless_equal(str, MT_LISTING_TEXT)) {
		type = M_LISTING_TEXT;
	} else if (caseless_equal(str, MT_PLAIN_TEXT)) {
		type = M_PLAIN_TEXT;
	} else if (caseless_equal(str, MT_PLAIN_FILE)) {
		type = M_PLAIN_FILE;
	} else if (caseless_equal(str, MT_PARAGRAPH)) {
		type = M_PARAGRAPH;
	} else if (caseless_equal(str, MT_UNUM_LIST)) {
		type = M_UNUM_LIST;
	} else if (caseless_equal(str, MT_NUM_LIST)) {
		type = M_NUM_LIST;
	} else if (caseless_equal(str, MT_MENU)) {
		type = M_MENU;
	} else if (caseless_equal(str, MT_DIRECTORY)) {
		type = M_DIRECTORY;
	} else if (caseless_equal(str, MT_LIST_ITEM)) {
		type = M_LIST_ITEM;
	} else if (caseless_equal(str, MT_DESC_LIST)) {
		type = M_DESC_LIST;
	} else if (caseless_equal(str, MT_DESC_TITLE)) {
		type = M_DESC_TITLE;
	} else if (caseless_equal(str, MT_DESC_TEXT)) {
		type = M_DESC_TEXT;
	} else if (caseless_equal(str, MT_PREFORMAT)) {
		type = M_PREFORMAT;
	} else if (caseless_equal(str, MT_BLOCKQUOTE)) {
		type = M_BLOCKQUOTE;
	} else if (caseless_equal(str, MT_INDEX)) {
		type = M_INDEX;
	} else if (caseless_equal(str, MT_HRULE)) {
		type = M_HRULE;
	} else if (caseless_equal(str, MT_BASE)) {
		type = M_BASE;
	} else if (caseless_equal(str, MT_LINEBREAK)) {
		type = M_LINEBREAK;
	} else if (caseless_equal(str, MT_IMAGE) ||
		/* Hack to handle mis-spelled "img" */
		   caseless_equal(str, "image")) {
		type = M_IMAGE;
	} else if (caseless_equal(str, MT_FIGURE)) {
		type = M_FIGURE;
	} else if (caseless_equal(str, MT_SELECT)) {
		type = M_SELECT;
	} else if (caseless_equal(str, MT_OPTION)) {
		type = M_OPTION;
	} else if (caseless_equal(str, MT_INPUT)) {
		type = M_INPUT;
	} else if (caseless_equal(str, MT_TEXTAREA)) {
		type = M_TEXTAREA;
	} else if (caseless_equal(str, MT_FORM)) {
		type = M_FORM;
	} else if (caseless_equal(str, MT_SUP)) {
                type = M_SUP;
        } else if (caseless_equal(str, MT_SUB)) {
                type = M_SUB;
        } else if (caseless_equal(str, MT_DOC_HEAD)) {
	        type = M_DOC_HEAD;
        } else if (caseless_equal(str, MT_UNDERLINED)) {
	        type = M_UNDERLINED;
        } else if (caseless_equal(str, MT_DOC_BODY)) {
	        type = M_DOC_BODY;
        } else if (caseless_equal(str, MT_TABLE)) {
		if (tableSupportEnabled) {
			type = M_TABLE;
		} else {
			type = M_UNKNOWN;
		}
	} else if (caseless_equal(str, MT_CAPTION)) {
		type = M_CAPTION;
	} else if (caseless_equal(str, MT_TABLE_ROW)) {
		if (tableSupportEnabled) {
			type = M_TABLE_ROW;
		} else {
			type = M_LINEBREAK;
		}
	} else if (caseless_equal(str, MT_TABLE_HEADER)) {
		if (tableSupportEnabled) {
			type = M_TABLE_HEADER;
		} else {
			type = M_UNKNOWN;
		}
	} else if (caseless_equal(str, MT_TABLE_DATA)) {
		if (tableSupportEnabled) {
			type = M_TABLE_DATA;
		} else {
			type = M_UNKNOWN;
		}
	} else if (caseless_equal(str, MT_FRAME)) {
		type = M_FRAME;
	} else if (caseless_equal(str, MT_FRAMESET)) {
		type = M_FRAMESET;
	} else if (caseless_equal(str, MT_NOFRAMES) ||
		/* Hack to handle mis-spelled "noframes" */
		   caseless_equal(str, "noframe")) {
		type = M_NOFRAMES;
	} else if (caseless_equal(str, MT_APROG)){
		type = M_APROG;
	} else if (caseless_equal(str, MT_APPLET)){
		if (appletSupportEnabled) {
			type = M_APPLET;
		} else {
			type = M_UNKNOWN;
		}
	} else if (caseless_equal(str, MT_PARAM)) {
		type = M_PARAM;
	} else if (caseless_equal(str, MT_HTML)) {
		type = M_HTML;
	} else if (caseless_equal(str, MT_CENTER)) {
		type = M_CENTER;
	} else if (caseless_equal(str, MT_DIV)) {
		type = M_DIV;
	} else if (caseless_equal(str, MT_DOCTYPE)) {
		type = M_DOCTYPE;
	} else if (caseless_equal(str, MT_BIG)) {
		type = M_BIG;
	} else if (caseless_equal(str, MT_SMALL)) {
		type = M_SMALL;
	} else if (caseless_equal(str, MT_FONT)) {
		type = M_FONT;
	} else if (caseless_equal(str, MT_BASEFONT)) {
		type = M_BASEFONT;
#if defined(VAXC) && !defined(__DECC)
	/* Hack to prevent compiler stack overflow */
	} else {
		vaxc_hack = 1;
	}
	if (!vaxc_hack) {
		/* We found it already */
#endif
	} else if (caseless_equal(str, MT_MAP)) {
		type = M_MAP;
	} else if (caseless_equal(str, MT_AREA)) {
		type = M_AREA;
	} else if (caseless_equal(str, MT_LINK)) {
		type = M_LINK;
	} else if (caseless_equal(str, MT_META)) {
		type = M_META;
	} else if (caseless_equal(str, MT_SCRIPT)) {
		type = M_SCRIPT;
	} else if (caseless_equal(str, MT_NOSCRIPT)) {
		type = M_NOSCRIPT;
	} else if (caseless_equal(str, MT_STYLE)) {
		type = M_STYLE;
	} else if (caseless_equal(str, MT_NOBR)) {
		type = M_NOBR;
	} else if (caseless_equal(str, MT_SPACER)) {
		type = M_SPACER;
	} else if (caseless_equal(str, MT_BLINK)) {
		type = M_BLINK;
	} else if (caseless_equal(str, MT_IFRAME)) {
		type = M_IFRAME;
	} else {
#ifndef DISABLE_TRACE
		if (htmlwTrace || reportBugs) {
			fprintf(stderr, "Warning: unknown mark (%s)\n", str);
		}
#endif
		type = M_UNKNOWN;
	}
	*tptr = tchar;
	return(type);
}

/*
 * Parse a single anchor tag.  ptrp is a pointer to a pointer to the
 * string to be parsed.  On return, the ptr should be changed to
 * point to after the text we have parsed.
 * On return start and end should point to the beginning, and just
 * after the end of the tag's name in the original anchor string.
 * Finally the function returns the tag value in a malloced buffer.
 */
static char *AnchorTag( char **ptrp, char **startp, char **endp)
{
	char *tag_val;
	char *ptr;
	char *start;
	char tchar;
	int quoted, squoted;
	int has_value;

	squoted = quoted = 0;
	ptr = *ptrp; 		/* Remove leading spaces, and set start */
	while (ISSPACE((int)*ptr))
		ptr++;
	*startp = ptr;
				/* Find and set the end of the tag */
	while ((!ISSPACE((int)*ptr)) && (*ptr != '=') && *ptr)
		ptr++;
	*endp = ptr;
        has_value = 0;
	if (!*ptr) {
		*ptrp = ptr;
	} else {    	/* Move to the start of tag value, if there is one. */
            while ((ISSPACE((int)*ptr)) || (*ptr == '=')) {
		if (*ptr == '=')
                    has_value = 1;
		ptr++;
            }
        }
	/* For a tag with no value, this is a boolean flag.
	 * Return the string "" so we know the tag is there.
	 */
	if (!has_value) {	/* Set a tag value of "" */
		*ptrp = *endp;
		tag_val = (char *)malloc(1);
		CHECK_OUT_OF_MEM(tag_val);
		*tag_val = '\0';
		return(tag_val);
	}
	if (*ptr == '\"') {
		quoted = 1;
		ptr++;
	} else if (*ptr == '\'') {
		squoted = 1;
		ptr++;
	}
	start = ptr;
	if (quoted) { /* Value is either a quoted string or a single word */
		while ((*ptr != '\"') && *ptr)
			ptr++;
	} else if (squoted) {
		while ((*ptr != '\'') && *ptr)
			ptr++;
	} else {
		/* If find quote, then start quote was missing */
		while ((!ISSPACE((int)*ptr)) && (*ptr != '\"') && *ptr)
			ptr++;
	}

	/* Copy the tag value out into a malloced string */
	tchar = *ptr;
	*ptr = '\0';
	tag_val = (char *)malloc(strlen(start) + 1);
	CHECK_OUT_OF_MEM(tag_val);
	strcpy(tag_val, start);
	*ptr = tchar;

	/* If you forgot the end quote, you need to make sure you aren't
	 * indexing ptr past the end of its own array
	 */
	if ((quoted || squoted) && *ptr)
		ptr++;
	*ptrp = ptr;
	return(tag_val);
}

/* Parse mark text for the value associated with the passed mark tag.
 * If the passed tag is not found, return NULL.
 * If the passed tag is found but has no value, return "".
 */
char *ParseMarkTag(char *text, char *mtext, char *mtag)
{
	char *ptr;
	char *start;
	char *end;
	char *tag_val;
	char tchar;

	if (!text || !mtext || !mtag)
		return(NULL);

	ptr = (char *)(text + strlen(mtext));

	while (*ptr) {
		tag_val = AnchorTag(&ptr, &start, &end);
		tchar = *end;
		*end = '\0';
		if (caseless_equal(start, mtag)) {
			*end = tchar;
			if (!tag_val) {
				tag_val = (char *)malloc(1);
				*tag_val = '\0';
				return(tag_val);
			}
			/* Now fixup any &quot characters */
			start = tag_val;
			while (*start) {
				if (*start == QUOT_CONST)
					*start = '\"';
				start++;
			}
			return(tag_val);
		}
		*end = tchar;
		if (tag_val)
			free(tag_val);
	}
	return(NULL);
}
