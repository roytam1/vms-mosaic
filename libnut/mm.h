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

/* Copyright (C) 2004, 2006 - The VMS Mosaic Project */

typedef struct memory_struct {
	int memoryType; /* Type of chunk to use */
	char *memory;   /* Chunk(s) of memory */
	int sizeCnt;    /* Number of chunks of size */
	int size;       /* Size of _1_ chunk */
	int fullSize;   /* size * sizeCnt */
	int nextFree;   /* Next free location (end of last allocation) */
} mem_block;

int memSize[] = {
	4096,  /* Lex Tree */
	8192   /* Parse Tree */
};

#define MEM_LEX 0
#define MEM_PARSE 1
#define MEM_MAX_ENTRY 2

#define MEM_ALIGN_2 2
#define MEM_ALIGN_4 4

#if defined(VMS) && defined(__DECC) && (defined(__alpha) || defined(__ia64))
#define MEM_ALIGN_8 8
#define MEM_ALIGN MEM_ALIGN_8
#else
#define MEM_ALIGN MEM_ALIGN_4
#endif

#define MEM_FAIL 0
#define MEM_SUCCEED 1

#define MEM_INDEX_SIZE(block) ((block)->fullSize / 4)  /* 1/4 chunk size */


void freeBlock(mem_block *block);
mem_block *allocateBlock(int type);
int reallocateBlock(mem_block *block);
void clearBlock(mem_block *block);
void *balloc(mem_block *block, int size);
int blockSize(mem_block *block);
