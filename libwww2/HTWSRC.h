/*
                                 WAIS SOURCE FILE PARSER
                                             
   This converter returns a stream object into which a WAIS source file can
   be written.  The result is put via a structured stream into whatever format
   was required for the output stream.
   
   See also: HTWAIS protocol interface module
   
 */
#ifndef HTWSRC_H
#define HTWSRC_H

#include "HTUtils.h"
#include "HTFormat.h"

extern  HTStream *HTWSRCConvert PARAMS((
        HTPresentation         *pres,
        HTParentAnchor         *anchor,
        HTStream               *sink,
        HTFormat                format_in,
        int                     compressed));

#endif

/* Tim BL */         
