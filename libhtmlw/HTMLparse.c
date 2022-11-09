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
 *
 * Copyright (C) 1998, 1999, 2000, 2003, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "HTMLparse.h"
#include "HTMLmiscdefs.h"
#include "HTMLP.h"
#include "HTMLPutil.h"

#if defined(MULTINET) && defined(__DECC) && (__VMS_VER >= 70000000)
#define strdup decc$strdup
#endif
extern char *strdup();

#include "../libnut/str-tools.h"

extern int appletSupportEnabled;
extern int tableSupportEnabled;

extern int Vclass;

extern MarkInfo *NULL_ANCHOR_PTR;

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
 ah, but 192-214 and 216 to 222 are also uppercase.  Stellan
*/
    188,189,190,191,224,225,226,227,228,229,230,231,232,233,234,235,236,
    237,238,239,240,241,242,243,244,245,246,215,248,249,250,251,252,253,
    254,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,
    239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};

#define TOLOWER(x) (map_table[x])

/* Locale-independent /Stellan */
#define ISSPACE(x) ((x) > 0 && ((x) <= ' ') || ((x) >= 127) && ((x) < 160))

static char *GreekCaps = "ABGDEZHQIKLMNXOPR STUFCYW";
static char *GreekLower = "abgdezhqiklmnxoprVstufcyw";

typedef struct amp_esc_rec {
	char *tag;
	unsigned char value;
	char *string;
} AmpEsc;

static char *basetarget;
static MarkInfo *mlist;
static MarkInfo *currentmark;
static charset_is_UTF8 = 0;
static int in_plain_text = 0;
 
#if (defined(VMS) && (__DECC_VER >= 50790004))
#pragma message disable(INTCONSTTRUNC)
#endif

static AmpEsc AmpEscapesUpper[] = {
	{"LT", '<', NULL},
	{"GT", '>', NULL},
	{"AMP", '&', NULL},
	{"QUOT", QUOT_CONST, NULL},
	{"Agrave", '\300', NULL},
	{"Aacute", '\301', NULL},
	{"Acirc", '\302', NULL},
	{"Atilde", '\303', NULL},
	{"Auml", '\304', NULL},
	{"Aring", '\305', NULL},
	{"AElig", '\306', NULL},
	{"Ccedil", '\307', NULL},
	{"Egrave", '\310', NULL},
	{"Eacute", '\311', NULL},
	{"Ecirc", '\312', NULL},
	{"Euml", '\313', NULL},
	{"Igrave", '\314', NULL},
	{"Iacute", '\315', NULL},
	{"Icirc", '\316', NULL},
	{"Iuml", '\317', NULL},
	{"ETH", '\320', NULL},
	{"Ntilde", '\321', NULL},
	{"Ograve", '\322', NULL},
	{"Oacute", '\323', NULL},
	{"Ocirc", '\324', NULL},
	{"Otilde", '\325', NULL},
	{"Ouml", '\326', NULL},
	{"Oslash", '\330', NULL},
	{"Ugrave", '\331', NULL},
	{"Uacute", '\332', NULL},
	{"Ucirc", '\333', NULL},
	{"Uuml", '\334', NULL},
	{"Yacute", '\335', NULL},
	{"THORN", '\336', NULL},
	/* Specials */
	{"Dagger", '+', "++"},
	{"OElig", '-', "OE"},
	{"Scaron", '-', "S("},
	{"Yuml", '-', "Y\237"},
	{"Prime", '-', "SYM 178"},
	/* Greek capitals */
	{"Alpha", '-', "SYMC A"},
	{"Beta", '-', "SYMC B"},
	{"Gamma", '-', "SYMC G"},
	{"Delta", '-', "SYMC D"},
	{"Epsilon", '-', "SYMC E"},
	{"Zeta", '-', "SYMC Z"},
	{"Eta", '-', "SYMC H"},
	{"Theta", '-', "SYMC Q"},
	{"Iota", '-', "SYMC I"},
	{"Kappa", '-', "SYMC K"},
	{"Lambda", '-', "SYMC L"},
	{"Mu", '-', "SYMC M"},
	{"Nu", '-', "SYMC N"},
	{"Xi", '-', "SYMC X"},
	{"Omicron", '-', "SYMC O"},
	{"Pi", '-', "SYMC P"},
	{"Rho", '-', "SYMC R"},
	{"Sigma", '-', "SYMC S"},
	{"Tau", '-', "SYMC T"},
	{"Upsilon", '-', "SYMC U"},
	{"Phi", '-', "SYMC F"},
	{"Chi", '-', "SYMC C"},
	{"Psi", '-', "SYMC Y"},
	{"Omega", '-', "SYMC W"},
	{NULL, '\0', NULL}
};
static AmpEsc AmpEscapesLower[] = {
	{"lt", '<', NULL},
	{"gt", '>', NULL},
	{"amp", '&', NULL},
	{"apos", '\47', NULL},
	{"ldquo", QUOT_CONST, NULL},
	{"lsaquo", '<', NULL},
	{"lsquo", '\47', NULL},
	{"quot", QUOT_CONST, NULL},
	{"rdquo", QUOT_CONST, NULL},
	{"rsaquo", '>', NULL},
	{"rsquo", '\47', NULL},
	{"tilde", '\176', NULL},
	{"nbsp", NBSP_CONST, NULL},	/* '\240' */
	{"iexcl", '\241', NULL},
	{"cent", '\242', NULL},
	{"pound", '\243', NULL},
	{"curren", '\244', NULL},
	{"yen", '\245', NULL},
	{"brvbar", '\246', NULL},
	{"sect", '\247', NULL},
	{"uml", '\250', NULL},
	{"copy", '\251', NULL},
	{"ordf", '\252', NULL},
	{"laquo", '\253', NULL},
	{"not", '\254', NULL},

	/* Ambiguous usage problem - used as a browser hyphenation hint
	 * by some browsers */
	/* {"shy", '\255', NULL}, */

	{"reg", '\256', NULL},
	{"macr", '\257', NULL},
	{"hibar", '\257', NULL},
	{"deg", '\260', NULL},
	{"plusmn", '\261', NULL},
	{"sup2", '\262', NULL},
	{"sup3", '\263', NULL},
	{"acute", '\264', NULL},
	{"micro", '\265', NULL},
	{"para", '\266', NULL},
	{"middot", '\267', NULL},
	{"cedil", '\270', NULL},
	{"sup1", '\271', NULL},
	{"ordm", '\272', NULL},
	{"raquo", '\273', NULL},
	{"frac14", '\274', NULL},
	{"frac12", '\275', NULL},
	{"frac34", '\276', NULL},
	{"iquest", '\277', NULL},
	{"times", '\327', NULL},
	{"szlig", '\337', NULL},
	{"agrave", '\340', NULL},
	{"aacute", '\341', NULL},
	{"acirc", '\342', NULL},
	{"atilde", '\343', NULL},
	{"auml", '\344', NULL},
	{"aring", '\345', NULL},
	{"aelig", '\346', NULL},
	{"ccedil", '\347', NULL},
	{"egrave", '\350', NULL},
	{"eacute", '\351', NULL},
	{"ecirc", '\352', NULL},
	{"euml", '\353', NULL},
	{"igrave", '\354', NULL},
	{"iacute", '\355', NULL},
	{"icirc", '\356', NULL},
	{"iuml", '\357', NULL},
	{"eth", '\360', NULL},
	{"ntilde", '\361', NULL},
	{"ograve", '\362', NULL},
	{"oacute", '\363', NULL},
	{"ocirc", '\364', NULL},
	{"otilde", '\365', NULL},
	{"ouml", '\366', NULL},
	{"divide", '\367', NULL},
	{"oslash", '\370', NULL},
	{"ugrave", '\371', NULL},
	{"uacute", '\372', NULL},
	{"ucirc", '\373', NULL},
	{"uuml", '\374', NULL},
	{"yacute", '\375', NULL},
	{"thorn", '\376', NULL},
	{"yuml", '\377', NULL},
	/* Specials */
	{"bull", '*', "MOSAIC BULLET"},
	{"circ", '^', NULL},
	{"dagger", '+', NULL},
	{"ndash", '-', NULL},
	{"oelig", '-', "oe"},
	{"permil", '-', "0/00"},
	{"scaron", '-', "s("},
	{"forall", '-', "SYM 34"},
	{"exist", '-', "SYM 36"},
	{"lowast", '*', "SYM 42"},
	{"minus", '-', "SYM 45"},
	{"hyphen", '-', "SYM 45"},
	{"cong", '-', "SYM 64"},
	{"there4", '-', "SYM 92"},
	{"perp", '-', "SYM 94"},
	{"sim", '-', "SYM 126"},
	{"prime", '-', "SYM 162"},
	{"le", '-', "SYM 163"},
	{"frasl", '-', "SYM 164"},
	{"infin", '-', "SYM 165"},
	{"fnof", 'f', "SYM 166"},
	{"clubs", '-', "SYM 167"},
	{"diams", '-', "SYM 168"},
	{"hearts", '-', "SYM 169"},
	{"spades", '-', "SYM 170"},
	{"harr", '-', "SYM 171"},
	{"larr", '<', "SYM 172"},
	{"uarr", '^', "SYM 173"},
	{"rarr", '>', "SYM 174"},
	{"darr", 'V', "SYM 175"},
	{"ge", '-', "SYM 179"},
	{"prop", '-', "SYM 181"},
	{"part", '-', "SYM 182"},
	{"ne", '-', "SYM 185"},
	{"equiv", '-', "SYM 186"},
	{"asymp", '-', "SYM 187"},
	{"hellip", '.', "SYM 188"},
	{"mdash", '-', "SYM 190"},
	{"crarr", '-', "SYM 191"},
	{"alefsym", '-', "SYM 192"},
	{"image", '-', "SYM 193"},
	{"real", '-', "SYM 194"},
	{"weierp", '-', "SYM 195"},
	{"otimes", '-', "SYM 196"},
	{"oplus", '-', "SYM 197"},
	{"empty", '-', "SYM 198"},
	{"cap", '-', "SYM 199"},
	{"cup", '-', "SYM 200"},
	{"sup", '-', "SYM 201"},
	{"supe", '-', "SYM 202"},
	{"nsub", '-', "SYM 203"},
	{"sub", '-', "SYM 204"},
	{"sube", '-', "SYM 205"},
	{"isin", '-', "SYM 206"},
	{"notin", '-', "SYM 207"},
	{"ang", '-', "SYM 208"},
	{"nabla", '-', "SYM 209"},
	{"trade", '-', "SYM 212"},
	{"prod", '-', "SYM 213"},
	{"radic", '-', "SYM 214"},
	{"sdot", '-', "SYM 215"},
	{"and", '-', "SYM 217"},
	{"or", '-', "SYM 218"},
	{"hArr", '-', "SYM 219"},
	{"lArr", '<', "SYM 220"},
	{"uArr", '^', "SYM 221"},
	{"rArr", '>', "SYM 222"},
	{"dArr", 'V', "SYM 223"},
	{"loz", '-', "SYM 224"},
	{"lozenge", '-', "SYM 224"},
	{"lang", '-', "SYM 225"},
	{"sum", '-', "SYM 229"},
	{"lceil", '-', "SYM 233"},
	{"lfloor", '-', "SYM 235"},
	{"rang", '-', "SYM 241"},
	{"int", '-', "SYM 242"},
	{"rceil", '-', "SYM 249"},
	{"rfloor", '-', "SYM 251"},
	{"euro", '-', "EUR"},
	/* Greek */
	{"alpha", '-', "SYMC a"},
	{"beta", '-', "SYMC b"},
	{"gamma", '-', "SYMC g"},
	{"delta", '-', "SYMC d"},
	{"epsilon", '-', "SYMC e"},
	{"zeta", '-', "SYMC z"},
	{"eta", '-', "SYMC h"},
	{"theta", '-', "SYMC q"},
	{"iota", '-', "SYMC i"},
	{"kappa", '-', "SYMC k"},
	{"lambda", '-', "SYMC l"},
	{"mu", '-', "SYMC m"},
	{"nu", '-', "SYMC n"},
	{"xi", '-', "SYMC x"},
	{"omicron", '-', "SYMC o"},
	{"pi", '-', "SYMC p"},
	{"rho", '-', "SYMC r"},
	{"sigmaf", '-', "SYMC V"},
	{"sigma", '-', "SYMC s"},
	{"tau", '-', "SYMC t"},
	{"upsilon", '-', "SYMC u"},
	{"phi", '-', "SYMC f"},
	{"chi", '-', "SYMC c"},
	{"psi", '-', "SYMC y"},
	{"omega", '-', "SYMC w"},
	{"thetasym", '-', "SYMC J"},
	{"piv", '-', "SYMC v"},
	{"upsih", '-', "SYM 161"},
	{NULL, '\0', NULL}
};
#if (defined(VMS) && (__DECC_VER >= 50790004))
#pragma message enable(INTCONSTTRUNC)
#endif

static MarkType ParseMarkType(char *str);

/* Check if two strings are equal, ignoring case.  The strings must be of
 * the same length to be equal.  Return 1 if equal, 0 otherwise.
 */
int caseless_equal(char *str1, char *str2)
{
	if (!str1 || !str2)
		return(0);

	while (*str1 && *str2) {
		if (TOLOWER(*str1++) != TOLOWER(*str2++))
			return(0);
	}
	if (!*str1 && !*str2)
		return(1);
	return(0);
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
	if (!*listp) {
		*listp = mark;
		current = *listp;
	} else {
		current->next = mark;
	     	current = current->next;
	}
	current->next = NULL;
	return(current);
}

/* Add a text mark to the list */
static void text_mark(char *text, int is_white)
{
	MarkInfo *mark = GetMarkRec();

	mark->type = M_NONE;    /* It's text */
	mark->text = GetMarkText(text);
	mark->is_white_text = is_white;
	currentmark = AddObj(&mlist, currentmark, mark);
}

/* Clean up the white space in a string.  Remove all leading and trailing
 * whitespace, and turn all internal whitespace into single spaces separating
 * words.  The cleaning is done by rearranging the chars in the passed txt
 * buffer.  The resultant string will probably be shorter, it can never get
 * longer.
 *
 * NOTE!  For 8-bit characters this only works if isspace() works.
 * When using the Posix isspace() and C locale
 * it does not, identifying chars>127 as space!
 * This messes up an <OPTION> tag with characters with value >127.
 * We should NOT use a potentially Locale-dependent function for parsing
 * HTML! /Stellan
 */
void clean_white_space(char *txt)
{
	char *ptr = txt;
	char *start = txt;

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
		/* If there are more words, insert a space and if space
		 * was removed, move up remaining text.
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
 * Parse an amperstand escape, and return the appropriate character,
 * string or '\0' on error.
 */
static char ExpandEscape(char *esc, char **endp, char **new)
{
	unsigned char val = ' ';

	if (*++esc == '#') {
		char *tptr = esc;
		char tchar;
		int value = 0;

		if (*++tptr == 'x') {
			/* Hex */
			while (isxdigit((int)*++tptr))
				;
			tchar = *tptr;
			*tptr = '\0';
			sscanf(esc + 2, "%x", &value);
#ifndef DISABLE_TRACE
			if (htmlwTrace)
				fprintf(stderr, "Hex & string = %d\n", value);
#endif
		} else {
			while (isdigit((int)*tptr))
				tptr++;
			tchar = *tptr;
			*tptr = '\0';
			value = atoi(esc + 1);
		}
		*new = NULL;
		*tptr = tchar;
		*endp = tptr;

		if (value < 256) {
			val = (char)value;
			if ((val > 127) && (val < 160)) {
				/* Windows extensions */
				switch (val) {
				    case 130:
					/* Comma */
					val = ',';
					break;
				    case 131:
					/* Florin */
					*new = "SYM 166";
					break;
				    case 132:
					/* ldquor */
					val = QUOT_CONST;
					break;
				    case 133:
					/* Ellipsis */
					*new = "SYM 188";
					break;
				    case 134:
					/* Dagger */
					val = '+';
					break;
				    case 135:
					/* Double dagger */
					*new = "++";
					break;
				    case 136:
					/* Circumflex accent */
					val = '^';
					break;
				    case 137:
					/* permil */
					*new = "0/00";
					break;
				    case 138:
					/* Scaron */
					*new = "S(";
					break;
				    case 139:
					/* lsaquo */
					val = '<';
					break;
				    case 140:
					/* Capital OE ligature */
					*new = "OE";
					break;
				    case 145:
				    case 146:
					/* L/R quote marks */
					val = 39;
					break;
				    case 147:
				    case 148:
					/* L/R double quote marks */
					val = QUOT_CONST;
					break;
				    case 149:
					/* Bullet */
					*new = "SYM 183";
					break;
				    case 150:
					/* En dash */
					val = '-';
					break;
				    case 151:
					/* Em dash */
					*new = "SYM 190";
					break;
				    case 152:
					/* Tilde */
					val = '~';
					break;
				    case 153:
					/* Trademark */
					*new = "SYM 212";
					break;
				    case 154:
					/* scaron */
					*new ="s(";
					break;
				    case 155:
					/* rsaquo */
					val = '>';
					break;
				    case 156:
					/* Small oe ligature */
					*new ="oe";
					break;
				    case 159:
					/* Yuml */
					*new = "Y\237";
					break;
				    default:
					/* Invalid */
					val = '\0';
					*endp = (char *)NULL;
				}
			}
		} else {
			/* Unicode punctuation, etc. */
			switch (value) {
			    case 956:
				/* mu */
				*new = "SYMC m";
				break;
			    case 8210:
				/* Figure dash */
			    case 8211:
				/* En dash */
				val = '-';
				break;
			    case 8212:
				/* Em dash */
			    case 8213:
				/* Quotation dash */
				*new = "SYM 190";
				break;
			    case 8216:
			    case 8217:
				/* lsquo and rsquo */
				val = 39;
				break;
			    case 8220:
			    case 8221:
				/* ldquo and rdquo*/
				val = QUOT_CONST;
				break;
			    case 8224:
				/* Dagger */
				val = '+';
				break;
			    case 8225:
				/* Double Dagger */
				*new = "++";
				break;
			    case 8226:
			    case 9679:
				/* Bullet and Black Circle */
				*new = "SYM 183";
				break;
			    case 8230:
				/* Ellipsis */
				*new = "SYM 188";
				break;
			    case 8243:
				/* Prime */
				*new = "SYM 178";
				break;
			    case 8249:
				/* lsaquo */
			        val = '<';
				break;
			    case 8250:
			        /* rsaquo */
			        val = '>';
				break;
			    case 8482:
				/* Trademark */
				*new = "SYM 212";
				break;
			    case 8810:
				/* Much Less Than */
				*new = "<<";
				break;
			    case 8811:
				/* Much Greater Than */
				*new = ">>";
				break;
			    case 9650:
				/* Up pointing triangle */
				*new = "SYM 221";
				break;
			    case 9660:
				/* Down pointing triangle */
				*new = "SYM 223";
				break;
			    default:
				/* No support */
				val = '\0';
				*endp = (char *)NULL;
			}
		}
	} else {
		AmpEsc *AmpEscapes;

		if (islower((int)*esc)) {
			AmpEscapes = AmpEscapesLower;
		} else {
			AmpEscapes = AmpEscapesUpper;
		}
		while (AmpEscapes->tag) {
			if ((*esc == *AmpEscapes->tag) &&
			    !strcmp(esc, AmpEscapes->tag)) {
				val = AmpEscapes->value;
				*new = AmpEscapes->string;
				*endp = (char *)(esc + strlen(esc));
				break;
			}
			AmpEscapes++;
		}
		if (!AmpEscapes->tag) {
#ifndef DISABLE_TRACE
			if (htmlwTrace)
				fprintf(stderr, "Unknown & string\n");
#endif
			val = '\0';
			*new = NULL;
			*endp = (char *)NULL;
		}
	}
	return((char)val);
}

/* Create Symbol font tags for symbol */
static void symbol_marks(int sym)
{
	unsigned char symbol[2];
	MarkInfo *mark = GetMarkRec();

	/* Make marks for the symbol */
	mark->type = M_FONT;
	mark->start = GetMarkText("FONT FACE=SYMBOL");
	currentmark = AddObj(&mlist, currentmark, mark);
	symbol[0] = (unsigned char)sym;
	symbol[1] = '\0';
	text_mark((char *)symbol, 0);
	mark = GetMarkRec();
	mark->type = M_FONT;
	mark->is_end = True;
	currentmark = AddObj(&mlist, currentmark, mark);
}

/*
 * Clean the special HTML character escapes out of the text and replace
 * them with the appropriate characters "&lt;" = '<', "&gt;" = '>',
 * "&amp;" = '&', etc.
 * GAG:  apparently &lt etc. can be left unterminated, what a nightmare.
 * The '&' character must be immediately followed by a letter or '#' to
 * be a valid escape sequence.  Other &'s are left alone.
 * The cleaning is done by rearranging chars into a new buffer.
 * If any escapes are replaced, the string changes in length.
 */
static char *expand_escapes(char *txt, int expand, int is_white)
{
	char *ptr = txt;
	char *new, *newtext, *start, *text, *tend, *ptr2;
	char tchar, val;
	int cnt = 0;

	/*
	 * Quick scan to find escape sequences.
	 * Escape is '&' followed by a letter (or a hash mark).
	 * Return if there are none.
	 */
	while (*ptr) {
		if ((*ptr == '&') &&
		    ((isalpha((int)*(ptr + 1))) || (*(ptr + 1) == '#')))
			break;
		ptr++;
		cnt++;
	}
	if (!*ptr)
		return(txt);
	/*
	 * Get replacement string started.
	 */
	new = ptr2 = strdup(txt);
	ptr2 += cnt;
	/*
	 * Loop replacing escape sequences into new buffer.
	 */
	while (*ptr) {
		/*
		 * Extract the escape sequence from start to ptr
		 */
		start = ptr++;
		if (*ptr == '#')
			ptr++;
		while (*ptr && !ispunct((int)*ptr) && !ISSPACE((int)*ptr))
			ptr++;
#ifndef DISABLE_TRACE
		if (htmlwTrace && (!*ptr || (*ptr != ';')))
			fprintf(stderr, "Unterminated & (%s)\n", start);
#endif
		/*
		 * Copy the escape sequence into a separate buffer.
		 * Then clean spaces so the "&lt ;" = "&lt;" etc.
		 * The cleaning should be unnecessary.
		 */
		tchar = *ptr;
		*ptr = '\0';
		text = strdup(start);
		CHECK_OUT_OF_MEM(text);
		*ptr = tchar;
		clean_white_space(text);
		/*
		 * Replace escape sequence with appropriate string
		 */
		if (val = ExpandEscape(text, &tend, &newtext)) {
			*tend = '\0';
			ptr = (char *)(start + strlen(text));
			if (*ptr == ';')
			    ptr++;
			if (newtext && expand) {
			    if (!strncmp(newtext, "MOS", 3)) {
				/* Special MOSAIC tag */
				MarkInfo *mark = GetMarkRec();

				/* Make mark for previous text */
				*ptr2 = '\0';
				text_mark(new, is_white);
				/* Restart output text string */
				free(new);
				ptr2 = new = malloc(strlen(ptr) + 1);

				/* Make special mark */
				mark->type = M_MOSAIC;
				mark->start = GetMarkText(newtext);
				currentmark = AddObj(&mlist, currentmark, mark);
			    } else if (!strncmp(newtext, "SYM", 3)) {
				/* Create Symbol font tags for symbol */
				MarkInfo *mark = GetMarkRec();

				/* Make mark for previous text */
				*ptr2 = '\0';
				text_mark(new, is_white);
				/* Restart output text string */
				free(new);
				ptr2 = new = malloc(strlen(ptr) + 1);

				if (*(newtext + 3) == 'C') {
				    /* Character */
				    symbol_marks((int)*(newtext + 5));
				} else {
				    /* Integer */
				    symbol_marks(atoi(newtext + 4));
				}
			    } else {
				int newlen = strlen(newtext);

				if (newlen > strlen(text)) {
				    int offset = ptr2 - new;

				    new = realloc(new, strlen(new) + 1 +
						  (newlen - strlen(text)));
				    ptr2 = new + offset;
				}
				*ptr2 = '\0';
				strcat(new, newtext);
				ptr2 += newlen;
			    }
			} else {
			    *ptr2++ = val;
			}
		} else {
			/*
			 * Invalid escape sequence.  Skip it.
			 */
#ifndef DISABLE_TRACE
			if (htmlwTrace)
				fprintf(stderr, "Invalid & string\n");
#endif
			ptr = start;
			*ptr2++ = *ptr++;
		}
		free(text);
		/*
		 * Copy forward remaining text until the next
		 * escape sequence
		 */
		while (*ptr) {
			if ((*ptr == '&') &&
			    (isalpha((int)*(ptr + 1)) || (*(ptr + 1) == '#')))
				break;
			*ptr2++ = *ptr++;
		}
	}
	*ptr2 = '\0';
	free(txt);

	return(new);
}

static void trace_utf8(char *ptr)
{
#ifndef DISABLE_TRACE
	if (reportBugs || htmlwTrace)
		fprintf(stderr, "Unknown UTF-8 code (%X,%X,%X)\n",
			(unsigned char)*ptr, (unsigned char)*(ptr + 1),
			(unsigned char)*(ptr + 2));
#endif
	return; 
}

static char *clean_text(char *txt, int expand, int is_white)
{
	if (!txt)
		return(NULL);

	/* Fixup UTF-8 encoding of ASCII characters 160 thru 255 and specials */
	if (charset_is_UTF8) {
	    char *newtext;
	    char *ptr = txt;
	    char *ptr2 = txt;

	    while (*ptr) {
		unsigned char c1 = (unsigned char)*ptr;
		unsigned char c2 = (unsigned char)*(ptr + 1);

		if (!c2) {	/* At end */
		    *ptr2++ = *ptr++;
		    break;
		}
		switch (c1) {
		  case 0xC2:
		    /* 160 thru 191 */
		    if ((c2 >= 0xA0) && (c2 <= 0xBF)) {
			if (c2 != 0xAD)  /* Skip soft hyphen */
			    *ptr2++ = c2;
		    } else {
			trace_utf8(ptr);
			/* Probably a control, so skip it */
		    }
		    ptr += 2;
		    break;
		  case 0xC3:
		    /* 192 thru 255 */
		    if ((c2 >= 0x80) &&	(c2 <= 0xBF)) {
			*ptr2++ = 64 + c2;
		    } else {
			trace_utf8(ptr);
			/* Encoding error, so display it */
			*ptr2++ = c1;
			*ptr2++ = c2;
		    }
		    ptr += 2;
		    break;
		  case 0xCB:
		    if (c2 == 0x9c) {
			/* Small tilde */
			*ptr2++ = '~';
		    } else {
			trace_utf8(ptr);
			*ptr2++ = '?';
		    }
		    ptr += 2;
		    break;
		  case 0xCE: {
		    int do_symbol = 0;

		    /* Greek capitals and first lowers */
		    if ((c2 >= 0x91) && (c2 <= 0xA9) && (c2 != 0xA2)) {
			do_symbol = GreekCaps[c2 - 0x91];
		    } else if ((c2 >= 0xB1) && (c2 <= 0xBF)) {
			do_symbol = GreekLower[c2 - 0xB1];
		    }
		    if (do_symbol > 0) {
			*ptr = '\0';
			newtext = expand_escapes(strdup(txt), expand, is_white);
			text_mark(newtext, is_white);
			free(newtext);
			txt = ptr2 = ptr += 2;
			symbol_marks(do_symbol);
		    } else {
			trace_utf8(ptr);
			ptr += 2;
			*ptr2++ = '?';
		    }
		    break;
		  }
		  case 0xCF: {
		    int do_symbol = 0;

		    /* Rest of Greek lowers */
		    if ((c2 >= 0x80) && (c2 <= 0x89)) {
			do_symbol = GreekLower[15 + (c2 - 0x80)];
		    } else if (c2 == 0x91) {
			do_symbol = 'J';
		    } else if (c2 == 0x92) {
			do_symbol = 161;
		    } else if (c2 == 0x95) {
			do_symbol = 'j';
		    } else if (c2 == 0x96) {
			do_symbol = 'v';
		    }
		    if (do_symbol > 0) {
			*ptr = '\0';
			newtext = expand_escapes(strdup(txt), expand, is_white);
			text_mark(newtext, is_white);
			free(newtext);
			txt = ptr2 = ptr += 2;
			symbol_marks(do_symbol);
		    } else {
			trace_utf8(ptr);
			ptr += 2;
			*ptr2++ = '?';
		    }
		    break;
		  }
		  case 0xE2: {
		    unsigned char c3 = (unsigned char)*(ptr + 2);
		    int do_symbol = 0;

		    switch (c2) {
		      case 0x80:
			switch (c3) {
			  case 0x92:
			    /* Figure dash */
			  case 0x93:
			    /* En dash */
			    *ptr2++ = '-';
			    ptr += 3;
			    break;
			  case 0x94:
			    /* Em dash */
			  case 0x95:
			    /* Quotation dash */
			    do_symbol = 190;
			    break;
			  case 0x99:
			    /* Right single quote */
			    *ptr2++ = '\47';
			    ptr += 3;
			    break;
			  case 0x9C:
			    /* Left double quote */
			    *ptr2++ = QUOT_CONST;
			    ptr += 3;
			    break;
			  case 0x9D:
			    /* Right double quote */
			    *ptr2++ = QUOT_CONST;
			    ptr += 3;
			    break;
			  case 0xA0:
			    /* Dagger */
			    *ptr2++ = '+';
			    ptr += 3;
			    break;
			  case 0xA2:
			    /* Bullet */
			    do_symbol = 183;
			    break;
			  case 0xA6:
			    /* Ellipsis */
			    do_symbol = 188;
			    break;
			  case 0xB2:
			    /* prime */
			    do_symbol = 162;
			    break;
			  case 0xB3:
			    /* Prime */
			    do_symbol = 178;
			    break;
			  default:
			    do_symbol = -1;
			}
			break;
		      case 0x81:
			if (c3 == 0x84) {
			    /* Fraction slash */
			    do_symbol = 164;
			    break;
			} else {
			    do_symbol = -1;
			}
			break;
		      case 0x82:
			if (c3 == 0xAC) {
			    /* Euro */
			    *ptr2++ = 'E';
			    *ptr2++ = 'U';
			    *ptr2++ = 'R';
			    ptr += 3;
			} else {
			    do_symbol = -1;
			}
			break;
		      case 0x84:
			if (c3 == 0x91) {
			    /* image */
			    do_symbol = 193;
			} else if (c3 == 0x98) {
			    /* weierp */
			    do_symbol = 195;
			} else if (c3 == 0x9C) {
			    /* real */
			    do_symbol = 194;
			} else if (c3 == 0xA2) {
			    /* Trademark */
			    do_symbol = 212;
			} else if (c3 == 0xB5) {
			    /* alefsym */
			    do_symbol = 192;
			} else {
			    do_symbol = -1;
			}
			break;
		      case 0x86:
			if (c3 == 0x90) {
			    /* Left arrow */
			    do_symbol = 172;
			} else if (c3 == 0x91) {
			    /* Up arrow */
			    do_symbol = 173;
			} else if (c3 == 0x92) {
			    /* Right arrow */
			    do_symbol = 174;
			} else if (c3 == 0x93) {
			    /* Down arrow */
			    do_symbol = 175;
			} else if (c3 == 0x94) {
			    /* Left and right arrow */
			    do_symbol = 171;
			} else if (c3 == 0xB5) {
			    /* Carriage return arrow */
			    do_symbol = 191;
			} else {
			    do_symbol = -1;
			}
			break;
		      case 0x87:
			if (c3 == 0x90) {
			    /* Left double arrow */
			    do_symbol = 220;
			} else if (c3 == 0x91) {
			    /* Up double arrow */
			    do_symbol = 221;
			} else if (c3 == 0x92) {
			    /* Right double arrow */
			    do_symbol = 222;
			} else if (c3 == 0x93) {
			    /* Down arrow */
			    do_symbol = 223;
			} else if (c3 == 0x94) {
			    /* Left and right double arrow */
			    do_symbol = 219;
			} else {
			    do_symbol = -1;
			}
			break;
		      case 0x88:
			switch (c3) {
			  case 0x80:
			    /* forall */
			    do_symbol = 34;
			    break;
			  case 0x82:
			    /* part */
			    do_symbol = 182;
			    break;
			  case 0x83:
			    /* exist */
			    do_symbol = 36;
			    break;
			  case 0x85:
			    /* empty */
			    do_symbol = 198;
			    break;
			  case 0x87:
			    /* nabla */
			    do_symbol = 209;
			    break;
			  case 0x88:
			    /* isin */
			    do_symbol = 206;
			    break;
			  case 0x89:
			    /* notin */
			    do_symbol = 207;
			    break;
			  case 0x8D:
			    /* Small contains as member */
			    do_symbol = 39;
			    break;
			  case 0x8F:
			    /* prod */
			    do_symbol = 213;
			    break;
			  case 0x91:
			    /* sum */
			    do_symbol = 229;
			    break;
			  case 0x92:
			    /* minus */
			    do_symbol = 45;
			    break;
			  case 0x97:
			    /* lowast */
			    do_symbol = 42;
			    break;
			  case 0x99:
			    /* Bullet operator */
			    do_symbol = 183;
			    break;
			  case 0x9A:
			    /* radic */
			    do_symbol = 214;
			    break;
			  case 0x9D:
			    /* prop */
			    do_symbol = 181;
			    break;
			  case 0x9E:
			    /* Infinity */
			    do_symbol = 165;
			    break;
			  case 0xA0:
			    /* ang */
			    do_symbol = 208;
			    break;
			  case 0xA7:
			    /* and */
			    do_symbol = 217;
			    break;
			  case 0xA8:
			    /* or */
			    do_symbol = 218;
			    break;
			  case 0xA9:
			    /* cap */
			    do_symbol = 199;
			    break;
			  case 0xAA:
			    /* cup */
			    do_symbol = 200;
			    break;
			  case 0xAB:
			    /* int */
			    do_symbol = 242;
			    break;
			  case 0xB4:
			    /* Therefore */
			    do_symbol = 92;
			    break;
			  case 0xBC:
			    /* sim */
			    do_symbol = 126;
			    break;
			  default:
			    do_symbol = -1;
			}
			break;
		      case 0x89:
			switch (c3) {
			  case 0x85:
			    /* cong */
			    do_symbol = 64;
			    break;
			  case 0x88:
			    /* asymp */
			    do_symbol = 187;
			    break;
			  case 0xA0:
			    /* Not equal */
			    do_symbol = 185;
			    break;
			  case 0xA1:
			    /* Equivalent */
			    do_symbol = 186;
			    break;
			  case 0xA4:
			    /* Less than or equal to */
			    do_symbol = 163;
			    break;
			  case 0xA5:
			    /* Greater than or equal to */
			    do_symbol = 179;
			    break;
			  default:
			    do_symbol = -1;
			}
			break;
		      case 0x8A:
			switch (c3) {
			  case 0x82:
			    /* sub */
			    do_symbol = 204;
			    break;
			  case 0x83:
			    /* sup */
			    do_symbol = 201;
			    break;
			  case 0x84:
			    /* nsub */
			    do_symbol = 203;
			    break;
			  case 0x86:
			    /* sube */
			    do_symbol = 205;
			    break;
			  case 0x87:
			    /* supe */
			    do_symbol = 202;
			    break;
			  case 0x95:
			    /* oplus */
			    do_symbol = 197;
			    break;
			  case 0x97:
			    /* otimes */
			    do_symbol = 196;
			    break;
			  case 0xA5:
			    /* perp */
			    do_symbol = 94;
			    break;
			  default:
			    do_symbol = -1;
			}
			break;
		      case 0x8B:
			if (c3 == 0x85) {
			    /* Dot operator */
			    do_symbol = 215;
			} else {
			    do_symbol = -1;
			}
			break;
		      case 0x8C:
			if (c3 == 0x88) {
			    /* lceil */
			    do_symbol = 233;
			} else if (c3 == 0x89) {
			    /* rceil */
			    do_symbol = 249;
			} else if (c3 == 0x8A) {
			    /* lfloor */
			    do_symbol = 235;
			} else if (c3 == 0x8B) {
			    /* rfloor */
			    do_symbol = 251;
			} else if (c3 == 0xA9) {
			    /* Left angle bracket */
			    do_symbol = 225;
			} else if (c3 == 0xAA) {
			    /* Right angle bracket */
			    do_symbol = 241;
			} else {
			    do_symbol = -1;
			}
			break;
		      case 0x97:
			if (c3 == 0x8A) {
			    /* Lozenge */
			    do_symbol = 224;
			} else {
			    do_symbol = -1;
			}
			break;
		      case 0x99:
			if (c3 == 0xA0) {
			    /* Spades */
			    do_symbol = 170;
			} else if (c3 == 0xA3) {
			    /* Clubs */
			    do_symbol = 167;
			} else if (c3 == 0xA5) {
			    /* Hearts */
			    do_symbol = 169;
			} else if (c3 == 0xA6) {
			    /* Diamonds */
			    do_symbol = 168;
			} else {
			    do_symbol = -1;
			}
			break;
		      default:
			do_symbol = -1;
		    }
		    if (do_symbol > 0) {
			*ptr = '\0';
			newtext = expand_escapes(strdup(txt), expand, is_white);
			text_mark(newtext, is_white);
			free(newtext);
			txt = ptr2 = ptr += 3;
			symbol_marks(do_symbol);
		    } else if (do_symbol < 0) {
			trace_utf8(ptr);
			ptr += 3;
			*ptr2++ = '?';
		    }
		    break;
		  }
		  case 0xEF:
		    if ((c2 == 0xBB) && ((unsigned char)*(ptr + 2) == 0xBF)) {
			/* Zero width NBSP */
		    } else {
			trace_utf8(ptr);
			*ptr2++ = '?';
		    }
		    ptr += 3;
		    break;
		  default:
		    if (c1 > 0x7F) {
		        trace_utf8(ptr++);
		        if (c1 > 0xEF) {
			    ptr += 3;
		        } else if (c1 > 0xDF) {
			    ptr += 2;
		        } else if (c1 > 0x7F) {
			    ptr++;
		        }
		        *ptr2++ = '?';
		    } else {
			/* First 128 ASCII */
			*ptr2++ = *ptr++;
		    }
		}
	    }
	    *ptr2 = '\0';
	}
	return expand_escapes(txt, expand, is_white);
}

/* Get a block of text from an HTML document.  All text from start to
 * the end, or the first mark (a mark is '<' followed by any char)
 * is returned in a malloced buffer.  Also, endp returns a pointer to the
 * next '<' or '\0'.  The returned text has already expanded '&' escapes.
 */
static char *get_text(char *start, char **endp, int *is_white)
{
	char *ptr = start;
	char *text;
	int len = 0;

	*is_white = 1;
	/* Copy text up to beginning of a mark, or the end */
	while (*ptr) {
		if ((*ptr == '<') &&
		    (isalpha((int)*(ptr + 1)) || (*(ptr + 1) == '!') ||
		     (*(ptr + 1) == '?') ||
		     ((*(ptr + 1) == '/') && (isalpha((int)*(ptr + 2)) ||
		       (*(ptr + 2) == '!') || (*(ptr + 2) == '/') ||
		       (((*(ptr + 2) == '\n') || (*(ptr + 2) == ' ')) &&
		        isalpha((int)*(ptr + 3)))))))
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
	text = clean_text(text, 1, *is_white);
	return(text);
}

/*
 * Get the mark text between '<' and '>'.  From the text, determine
 * its type, and fill in a mark_up structure to return.  Also returns
 * endp pointing to the trailing '>' in the original string.
 */
static MarkInfo *get_mark(HTMLWidget hw, char *start, char **endp)
{
	char *ptr, *text, *tptr;
	char tchar;
	MarkInfo *mark;
	int  comment = 0;       /* comment == 1 if we are in a comment */
	int  quoted = 0;
	int  squoted = 0;
	int  equal = 0;

	if (!start || (*start != '<'))
		return(NULL);

	/* Check if we are in a comment, start tag is <!-- */
	/* Believe it or not, </!-- is also used */
	if (!strncmp(start, "<!--", 4) || !strncmp(start, "</!--", 5))
		comment = 1;
	mark = GetMarkRec();
	ptr = ++start; 	/* Grab the mark text */

	/* Skip over the comment text */
	/* End tag is --*>, where * is zero or more spaces (ugh) */
	if (comment) {
		char *first_gt = NULL;  /* Track ">" for old broken comments */

		while (*ptr) {
			if ((*ptr == '>') && !first_gt)
				first_gt = ptr;
			if (!strncmp(ptr, "--", 2)) {
				/* Found double dash(--) */
				ptr += 2;
				while (*ptr && ((*ptr == ' ') || 
				       (*ptr == '\n') || (*ptr == '-')))
					ptr++;      /* Skip spaces and LFs */ 
				if (*ptr == '>') {  /* Completed end comment */
					*endp = ptr;
					mark->is_end = 1;
					mark->type = M_COMMENT;
					return(mark);
				}
			} else {  /* If no double dash (--) found */
				ptr++;
			}
		}
		/* If we get here, this document must use the old broken
		 * comment style */
		if (first_gt)
			ptr = first_gt;
	}

	/* Find end of mark while ignoring <> in quotes */
	while (*ptr) {
		if (!quoted && !squoted && ((*ptr == '>') || (*ptr == '<')))
			break;
		/* Must be an '=' before having a quoted string */
		if (equal) {
			if ((*ptr == '\"') && !squoted) {
				if (quoted) {
					quoted = equal = 0;
				} else {
					quoted = 1;
				}
			} else if ((*ptr == '\'') && !quoted) {
				if (squoted) {
					squoted = equal = 0;
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
		FreeMarkRec(mark);
		return(NULL);
	}

	/* Copy the mark text to its own buffer, and
	 * clean it of escapes and odd white space.
	 */
	tchar = *ptr;
	*ptr = '\0';
	tptr = clean_text(strdup(start), 0, 0);
	*ptr = tchar;

	text = GetMarkText(tptr);
	free(tptr);

	/* Set whether this is the start or end of a mark
	 * block, as well as determine its type.
	 */
	if (*text == '/') {
		mark->is_end = 1;
		if ((*(text + 1) == '\n') || (*(text + 1) == ' ')) {
			mark->type = ParseMarkType((char *)(text + 2));
		} else {
			mark->type = ParseMarkType((char *)(text + 1));
		}
		mark->start = mark->text = NULL;
		FreeMarkText(text);
	} else {
	    mark->is_end = 0;
	    mark->type = ParseMarkType(text);
	    mark->start = text;
	    mark->text = NULL;
	    /* Check for tags specifying background colors and do the color
	     * map allocation.  Ignore body tag here since it comes early
	     * enough in the formatting to get its colors okay.  Also
	     * do USEMAPs here since fools tend to put them where they
	     * are hard to process later (like between table cells) and
	     * check Meta tags for character set.
	     */
	    if (hw && !in_plain_text) {   /* hw is NULL when in hotfile.c */
		char *cptr = NULL;

		switch(mark->type) {
		    case M_FONT:
			if (hw->html.font_colors && (Vclass != TrueColor))
			    cptr = ParseMarkTag(mark->start, MT_FONT, "color");
			break;
		    case M_BASE:
			if (!basetarget &&
			    (basetarget = ParseMarkTag(mark->start, MT_BASE,
						       "target"))) {
			    if (!*basetarget) {
				free(basetarget);
				basetarget = NULL;
			    }
			}
			break;
		    case M_BASEFONT:
			if (hw->html.font_colors && (Vclass != TrueColor))
			    cptr = ParseMarkTag(mark->start, MT_BASEFONT,
						"color");
			break;
		    case M_IMAGE:
		        if (hw->html.multi_image_load &&
			    (tptr = ParseMarkTag(mark->start,
						 MT_IMAGE, "src"))) {
			    if (!hw->html.image_src)
				hw->html.image_src = HTBTree_new((HTComparer)
								 strcmp);
			    HTBTree_add_new(hw->html.image_src, tptr);
			}
			break;
		    case M_TABLE:
			if (hw->html.body_colors && (Vclass != TrueColor))
			    cptr = ParseMarkTag(mark->start, MT_TABLE,
						"bgcolor");
			break;
		    case M_TABLE_DATA:
			if (hw->html.body_colors && (Vclass != TrueColor))
			    cptr = ParseMarkTag(mark->start, MT_TABLE_DATA,
						"bgcolor");
			break;
		    case M_TABLE_HEADER:
			if (hw->html.body_colors && (Vclass != TrueColor))
			    cptr = ParseMarkTag(mark->start, MT_TABLE_HEADER,
						"bgcolor");
			break;
		    case M_TABLE_ROW:
			if (hw->html.body_colors && (Vclass != TrueColor))
			    cptr = ParseMarkTag(mark->start, MT_TABLE_ROW,
						"bgcolor");
			break;
		    case M_TEXTAREA:
			currentmark = AddObj(&mlist, currentmark, mark);
			/* Create <XMP> to keep markup as text */
			mark = GetMarkRec();
			mark->type = M_PLAIN_TEXT;
			mark->start = GetMarkText("XMP TEXTAREA=1");
			/* GetMarkRec zeros the rest of the structure */
			break;
		    case M_META:
			tptr = ParseMarkTag(mark->start, MT_META, "http-equiv");
			if (tptr) {
			    if (!my_strcasecmp(tptr, "Content-Type")) {
				char *sptr = ParseMarkTag(mark->start, MT_META,
							  "content");

				if (sptr) {
				    char *cset = ParseMarkTag(sptr, "",
							      "charset");

				    if (cset) {
					if (!my_strcasecmp(cset, "UTF-8"))
					    charset_is_UTF8 = 1;
					free(cset);
				    }
				    free(sptr);
				}
			    }
			    free(tptr);
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
				if (caseless_equal(tptr, "rect")) {
				    area->shape = AREA_RECT;
				} else if (caseless_equal(tptr, "circle")) {
				    area->shape = AREA_CIRCLE;
				} else if (!my_strncasecmp(tptr, "poly", 4)) {
				    area->shape = AREA_POLY;
				} else {
				    area->shape = -1;
				}
				free(tptr);
			    } else {
				area->shape = AREA_RECT;
			    }
			    area->alt = ParseMarkTag(mark->start, MT_AREA,
						     "alt");
			    area->href = ParseMarkTag(mark->start, MT_AREA,
						      "href");
			    area->target = ParseMarkTag(mark->start, MT_AREA,
						        "target");
			    if (!area->target && basetarget)
				area->target = strdup(basetarget);
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
	     			/* Must have at least one x, y */
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
					coord->next = (CoordInfo *)malloc(
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
	        if (cptr) {
			hw_do_color(hw,	"preallo", cptr, NULL);
			free(cptr);
		}
	    }
	}
	/* Do these here because have both start and end tags to deal with */
	if (hw && !in_plain_text) switch(mark->type) {
	    case M_MAP:
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
		break;
	    case M_FIELDSET:
		mark->type = M_TABLE;
		if (!mark->is_end) {
			FreeMarkText(mark->start);
			mark->start =GetMarkText("<TABLE FIELDSET=1 BORDER=1>");
		}
		break;
	    case M_LEGEND:
		mark->type = M_CAPTION;
		if (mark->is_end) {
			currentmark = AddObj(&mlist, currentmark, mark);
			/* Table marks to make table code not complain */
			mark = GetMarkRec();
			mark->type = M_TABLE_ROW;
			mark->start = GetMarkText("TR");
			/* GetMarkRec zeros the rest of the structure */
			currentmark = AddObj(&mlist, currentmark, mark);
			mark = GetMarkRec();
			mark->type = M_TABLE_DATA;
			mark->start = GetMarkText("TD");
		} else {
			FreeMarkText(mark->start);
			mark->start =GetMarkText("CAPTION LEGEND=1 ALIGN=LEFT");
		}
		break;
	    default:
		break;
	}
	mark->text = NULL;
	mark->next = NULL;
	return(mark);
}

/* Special version of get_text.  It reads all text up to the
 * end of the plain text mark, or the end of the file.
 */
static char *get_plain_text(HTMLWidget hw, char *start, char **endp,
			    int textarea)
{
	char *ptr = start;
	char *text;
	char tchar;
	MarkInfo *mp;

	if (!start)
		return(NULL);

	in_plain_text = 1;

	/* Read until stopped by end plain text mark. */
	while (*ptr) {
		/* Beginning of a mark is '<' followed by any letter, or
		 * followed by '!' for a comment, or '</' followed by any
		 * letter.
 		 */
		if ((*ptr == '<') &&
		    (isalpha((int)(*(ptr + 1))) ||
		     (*(ptr + 1) == '!') ||
		     ((*(ptr + 1) == '/') && (isalpha((int)(*(ptr + 2))))))) {
			char *ep;

			/* We think we found a mark.  If it is the end of
			 * plain text, break out
			 */
			if (mp = get_mark(hw, ptr, &ep)) {
				if (((mp->type == M_PLAIN_TEXT) ||
				     (mp->type == M_LISTING_TEXT) ||
				     ((mp->type == M_TEXTAREA) && textarea)) &&
				    mp->is_end) {
					FreeMarkRec(mp);
					break;
				}
				if (mp->start)
					FreeMarkText(mp->start);
				FreeMarkRec(mp);
			}
		}
		ptr++;
	}
	*endp = ptr;

	in_plain_text = 0;

	if (ptr == start)
		return(NULL);
	/* Copy text to its own buffer, and clean it of HTML escapes. */
	tchar = *ptr;
	*ptr = '\0';
	text = strdup(start);
	CHECK_OUT_OF_MEM(text);
	*ptr = tchar;
	text = clean_text(text, 1, 0);
	return(text);
}

/* Main parser of HTML text.  Takes raw text, and produces a linked
 * list of mark objects.  Mark objects are either text strings, or
 * starting and ending mark delimiters.
 */
MarkInfo *HTMLParse(HTMLWidget hw, char *str, char *charset)
{
	char *start = str;
	char *end = str;
	char *text, *tptr;
	MarkInfo *mark = NULL_ANCHOR_PTR;  /* Dummy to start */
	int is_white = 0;		/* Is white text? */

	if (!str)
		return(NULL);

	if (hw) {
		hw->html.cur_map = NULL;
		hw->html.cur_area = NULL;
	}

	if (charset && !my_strcasecmp(charset, "UTF-8")) {
		charset_is_UTF8 = 1;
	} else {
		charset_is_UTF8 = 0;
	}
	basetarget = NULL;
	mlist = NULL;
	currentmark = NULL;

	while (*start) {
	        /* Get some text (if any).  If our last mark was a begin plain
		 * text we call a different function.  If last mark was
		 * <PLAINTEXT> we lump all the rest of the text in.
	         */
		if (!mark->is_end) {
			switch (mark->type) {
			    case M_PLAIN_FILE:
				end = text = start;
				while (*end)
					end++;
				/* Copy text to its own buffer */
				tptr = strdup(text);
				CHECK_OUT_OF_MEM(tptr);
				text = tptr;
				break;
			    case M_PLAIN_TEXT:
			    case M_LISTING_TEXT: {
				int ta = 0;

				/* Special XMP tag for TEXTAREA */ 
				if ((mark->type == M_PLAIN_TEXT) &&
				    (tptr = ParseMarkTag(mark->start,
							 MT_PLAIN_TEXT,
							 "textarea"))) {
					ta = 1;
					free(tptr);
				}
				is_white = 0;
				text = get_plain_text(hw, start, &end, ta);
				break;
			    }
			    case M_SCRIPT:
				/* Skip over script */
				text = NULL;
				end = start;
				while (*end) {
					if ((*end == '<') &&
					    !my_strncasecmp(end, "</script", 8))
						break;
					end++;
				}
				break;
			    case M_STYLE:
				/* Skip over style */
				text = NULL;
				end = start;
				while (*end) {
					if ((*end == '<') &&
					    !my_strncasecmp(end, "</style", 7))
						break;
					end++;
				}
				break;
			    default:
				text = get_text(start, &end, &is_white);
			}
		} else {
			text = get_text(start, &end, &is_white);
		}
		/* If text is OK, create a text mark */
		if (text) {
			text_mark(text, is_white);
			free(text);
		}
		/* end is on '<' or '\0' */
		start = end;
		if (!*start)
			break;		/* End of html string, parse is done */

		/* Get the next mark if any, and if it is valid,
		 * add it to the linked list.  start is on '<'.
		 * Loop until valid mark or end of string.
		 */
		if (!(mark = get_mark(hw, start, &end))) {
#ifndef DISABLE_TRACE
			if (htmlwTrace || reportBugs)
				fprintf(stderr,
					"Error parsing, missing final '>'\n");
#endif
			return(mlist);
		}
		/* end is on '>' or character prior to '<' if no '>' */

		mark->is_white_text = is_white = 0;
		currentmark = AddObj(&mlist, currentmark, mark);
		start = (char *)(end + 1);
		/* start is a pointer after the '>' character */
		if (mark && (mark->type == M_PLAIN_FILE || 
		    mark->type == M_PLAIN_TEXT || mark->type == M_PREFORMAT ||
		    mark->type == M_LISTING_TEXT) && (!mark->is_end)) {
		        /* A linefeed immediately after a <PLAINTEXT>, <XMP>,
			 * <PRE> or <LISTING> mark is to be ignored.
		         */
			if (*start == '\n')
				start++;
		} 
	}
	return(mlist);
}

/* Determine mark type from the identifying string passed */
static MarkType ParseMarkType(char *str)
{
	MarkType type;
	char *tptr = str;
	char tchar;

	if (!str)
	    return(M_NONE);

	/* Find end of mark type */
	while (*tptr) {
	    if (ISSPACE((int)*tptr))
		break;
	    tptr++;
	}
	tchar = *tptr;
	*tptr = '\0';

	type = M_UNKNOWN;

	switch (TOLOWER(*str)) {
	    case '!':
		if (caseless_equal(str, MT_DOCTYPE))         /* <!DOCTYPE> */
		    type = M_DOCTYPE;
		break;
	    case 'a':
		if (caseless_equal(str, MT_ANCHOR)) {
		    type = M_ANCHOR;
		} else if (caseless_equal(str, MT_ADDRESS)) {
		    type = M_ADDRESS;
		} else if (caseless_equal(str, MT_AREA)) {
		    type = M_AREA;
		} else if (caseless_equal(str, MT_APPLET) &&
			   appletSupportEnabled) {
                        type = M_APPLET;
		}
		break;
	    case 'b':
		if (caseless_equal(str, MT_LINEBREAK) ||         /* <BR> */
		    caseless_equal(str, "br/")) {                /* XHTML */
		    type = M_LINEBREAK;
		} else if (caseless_equal(str, MT_DOC_BODY)) {   /* <BODY> */
		    type = M_DOC_BODY;
		} else if (caseless_equal(str, MT_BLINK)) {
		    type = M_BLINK;
		} else if (caseless_equal(str, MT_BOLD)) {
		    type = M_BOLD;
		} else if (caseless_equal(str, MT_BASE)) {
		    type = M_BASE;
		} else if (caseless_equal(str, MT_BASEFONT)) {
		    type = M_BASEFONT;
		} else if (caseless_equal(str, MT_BIG)) {
		    type = M_BIG;
		} else if (caseless_equal(str, MT_BLOCKQUOTE)) {
		    type = M_BLOCKQUOTE;
		} else if (caseless_equal(str, MT_BUTTON)) {
		    type = M_BUTTON;
		}
		break;
	    case 'c':
		if (caseless_equal(str, MT_CENTER)) {
		    type = M_CENTER;
		} else if (caseless_equal(str, MT_CODE)) {
		    type = M_CODE;
		} else if (caseless_equal(str, MT_COL)) {
		    type = M_COL;
		} else if (caseless_equal(str, MT_COLGROUP)) {
		    type = M_COLGROUP;
		} else if (caseless_equal(str, MT_CITATION)) {
		    type = M_CITATION;
		} else if (caseless_equal(str, MT_CAPTION)) {
		    type = M_CAPTION;
		}
		break;
	    case 'd':
		if (caseless_equal(str, MT_DIV)) {
		    type = M_DIV;
		} else if (caseless_equal(str, MT_DEFINE)) {
		    type = M_DEFINE;
		} else if (caseless_equal(str, MT_DELETED)) {
		    type = M_STRIKEOUT;
		} else if (caseless_equal(str, MT_DESC_LIST)) {
		    type = M_DESC_LIST;
		} else if (caseless_equal(str, MT_DESC_TEXT)) {
		    type = M_DESC_TEXT;
		} else if (caseless_equal(str, MT_DESC_TITLE)) {
		    type = M_DESC_TITLE;
		} else if (caseless_equal(str, MT_DIRECTORY)) {
		    type = M_DIRECTORY;
		}
		break;
	    case 'e':
		if (caseless_equal(str, MT_EMPHASIZED))
		    type = M_EMPHASIZED;
		break;
	    case 'f':
		if (caseless_equal(str, MT_FONT)) {
		    type = M_FONT;
		} else if (caseless_equal(str, MT_FORM)) {
		    type = M_FORM;
		} else if (caseless_equal(str, MT_FRAME)) {
		    type = M_FRAME;
		} else if (caseless_equal(str, MT_FRAMESET)) {
		    type = M_FRAMESET;
		} else if (caseless_equal(str, MT_FIELDSET)) {
		    type = M_FIELDSET;
		} else if (caseless_equal(str, MT_FIGURE)) {
		    type = M_FIGURE;
		}
		break;
	    case 'h':
		if (tptr == (str + 2)) {
		    /* H followed by single character */
		    switch (TOLOWER(*(str + 1))) {
			case '1':
			    type = M_HEADER_1;
			    break;
			case '2':
			    type = M_HEADER_2;
			    break;
			case '3':
			    type = M_HEADER_3;
			    break;
			case '4':
			    type = M_HEADER_4;
			    break;
			case '5':
			    type = M_HEADER_5;
			    break;
			case '6':
			    type = M_HEADER_6;
			    break;
			case 'r':
			    type = M_HRULE;
			    break;
		    }
		} else if (caseless_equal(str, MT_HTML)) {
		    type = M_HTML;
		} else if (caseless_equal(str, MT_DOC_HEAD)) {    /* <HEAD> */
		    type = M_DOC_HEAD;
		} else if (caseless_equal(str, "hr/")) {          /* XHTML */
		    type = M_HRULE;
		}
		break;
	    case 'i':
		if (caseless_equal(str, MT_IMAGE) ||
		    /* Hack to handle mis-spelled "img" */
		    caseless_equal(str, "image")) {
		    type = M_IMAGE;
		} else if (caseless_equal(str, MT_IFRAME)) {
		    type = M_IFRAME;
		} else if (caseless_equal(str, MT_ITALIC)) {
		    type = M_ITALIC;
		} else if (caseless_equal(str, MT_INPUT)) {
		    type = M_INPUT;
		} else if (caseless_equal(str, MT_INDEX)) {
		    type = M_INDEX;
		} else if (caseless_equal(str, MT_INSERTED)) {
		    type = M_UNDERLINED;
		}
		break;
	    case 'k':
		if (caseless_equal(str, MT_KEYBOARD))
		    type = M_KEYBOARD;
		break;
	    case 'l':
		if (caseless_equal(str, MT_LINK)) {
		    type = M_LINK;
		} else if (caseless_equal(str, MT_LISTING_TEXT)) {
		    type = M_LISTING_TEXT;
		} else if (caseless_equal(str, MT_LIST_ITEM)) {
		    type = M_LIST_ITEM;
		} else if (caseless_equal(str, MT_LABEL)) {
		    type = M_LABEL;
		} else if (caseless_equal(str, MT_LEGEND)) {
		    type = M_LEGEND;
		}
		break;
	    case 'm':
		if (caseless_equal(str, MT_MENU)) {
		    type = M_MENU;
		} else if (caseless_equal(str, MT_META)) {
		    type = M_META;
		} else if (caseless_equal(str, MT_MAP)) {
		    type = M_MAP;
		}
		break;
	    case 'n':
		if (caseless_equal(str, MT_NOBR)) {
		    type = M_NOBR;
		} else if (caseless_equal(str, MT_NOFRAMES) ||
			   /* Hack to handle mis-spelled "noframes" */
			   caseless_equal(str, "noframe")) {
		    type = M_NOFRAMES;
		} else if (caseless_equal(str, MT_NOSCRIPT)) {
		    type = M_NOSCRIPT;
		/* NCSA annotation tag */
		} else if (!my_strncasecmp(str, MT_NCSA, 5)) {
		    type = M_NCSA;
		/* Non_standard Atomz tag */ 
		} else if (caseless_equal(str, MT_NOINDEX)) {
		    type = M_NOINDEX;
		}
		break;
	    case 'o':
		if (caseless_equal(str, MT_OPTION)) {
		    type = M_OPTION;
		} else if (caseless_equal(str, MT_OPTGROUP)) {
		    type = M_OPTGROUP;
		} else if (caseless_equal(str, MT_NUM_LIST)) {   /* <OL> */
		    type = M_NUM_LIST;
		}
		break;
	    case 'p':
		if (caseless_equal(str, MT_PARAGRAPH)) {
		    type = M_PARAGRAPH;
		} else if (caseless_equal(str, MT_PLAIN_FILE)) {
		    type = M_PLAIN_FILE;
		} else if (caseless_equal(str, MT_PREFORMAT)) {
		    type = M_PREFORMAT;
		} else if (caseless_equal(str, MT_PARAM)) {
		    type = M_PARAM;
		}
		break;
	    case 's':
		if (caseless_equal(str, MT_SCRIPT)) {
		    type = M_SCRIPT;
		} else if (caseless_equal(str, MT_STYLE)) {
		    type = M_STYLE;
		} else if (caseless_equal(str, MT_SPACER)) {
		    type = M_SPACER;
		} else if (caseless_equal(str, MT_SPAN)) {
		    type = M_SPAN;
		} else if (caseless_equal(str, MT_SMALL)) {
		    type = M_SMALL;
		} else if (caseless_equal(str, MT_STRONG)) {
		    type = M_STRONG;
		} else if (caseless_equal(str, MT_SAMPLE)) {
		    type = M_SAMPLE;
		} else if (caseless_equal(str, MT_SELECT)) {
		    type = M_SELECT;
		} else if (caseless_equal(str, MT_STRIKEOUT)) {
		    type = M_STRIKEOUT;
		} else if (caseless_equal(str, MT_STRIKEOUT2)) {
		    type = M_STRIKEOUT;
		} else if (caseless_equal(str, MT_SUB)) {
		    type = M_SUB;
		} else if (caseless_equal(str, MT_SUP)) {
		    type = M_SUP;
		}
		break;
	    case 't':
		if (caseless_equal(str, MT_TABLE)) {
		    if (tableSupportEnabled) {
			type = M_TABLE;
		    } else {
			type = M_COMMENT;
		    }
		} else if (caseless_equal(str, MT_TABLE_ROW)) {
		    if (tableSupportEnabled) {
			type = M_TABLE_ROW;
		    } else {
			type = M_LINEBREAK;
		    }
		} else if (caseless_equal(str, MT_TABLE_DATA)) {
		    if (tableSupportEnabled) {
			type = M_TABLE_DATA;
		    } else {
			type = M_COMMENT;
		    }
		} else if (caseless_equal(str, MT_TABLE_HEADER)) {
		    if (tableSupportEnabled) {
			type = M_TABLE_HEADER;
		    } else {
			type = M_COMMENT;
		    }
		} else if (caseless_equal(str, MT_TITLE)) {
		    type = M_TITLE;
		} else if (caseless_equal(str, MT_FIXED)) {      /* <TT> */
		    type = M_FIXED;
		} else if (caseless_equal(str, MT_TEXTAREA)) {
		    type = M_TEXTAREA;
		}
		break;
	    case 'u':
		if (caseless_equal(str, MT_UNUM_LIST)) {
		    type = M_UNUM_LIST;
		} else if (caseless_equal(str, MT_UNDERLINED)) {
		    type = M_UNDERLINED;
		}
		break;
	    case 'v':
		if (caseless_equal(str, MT_VARIABLE))
		    type = M_VARIABLE;
		break;
	    case 'x':
		if (caseless_equal(str, MT_PLAIN_TEXT))         /* <XMP> */
		    type = M_PLAIN_TEXT;
		break;
	}
#ifndef DISABLE_TRACE
	if ((type == M_UNKNOWN) && (htmlwTrace || reportBugs))
	    fprintf(stderr, "Warning: unknown mark (%s)\n", str);
#endif
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
static char *AnchorTag(char **ptrp, char **startp, char **endp)
{
	char *tag_val, *ptr, *start;
	char tchar;
	int quoted = 0;
	int squoted = 0;
	int has_value = 0;

	ptr = *ptrp; 		/* Remove leading spaces, and set start */
	while (ISSPACE((int)*ptr))
		ptr++;
	*startp = ptr;
				/* Find and set the end of the tag */
	while ((!ISSPACE((int)*ptr)) && (*ptr != '=') && *ptr)
		ptr++;
	*endp = ptr;
	if (!*ptr) {
		*ptrp = ptr;
	} else {    	/* Move to the start of tag value, if there is one. */
        	while ((ISSPACE((int)*ptr)) || (*ptr == '=')) {
			if (*ptr++ == '=')
                		has_value = 1;
        	}
        }
	/* For a tag with no value, this is a boolean flag.
	 * Return the string "" so we know the tag is there.
	 */
	if (!has_value) {	/* Set a tag value of "" */
		*ptrp = *endp;
		tag_val = strdup("");
		CHECK_OUT_OF_MEM(tag_val);
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
	if (quoted) {  /* Value is either a quoted string or a single word */
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
	tag_val = strdup(start);
	CHECK_OUT_OF_MEM(tag_val);
	*ptr = tchar;

	/* If you forgot the end quote, you need to make sure you
	 * aren't indexing ptr past the end of its own array.
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
	char *ptr, *start, *end, *tag_val;
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
		free(tag_val);
	}
	return(NULL);
}
