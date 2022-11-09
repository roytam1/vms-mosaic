/*             
 *             HTML generator
 */

#ifndef HTMLGEN_H
#define HTMLGEN_H

#include "HTML.h"
#include "HTStream.h"

/* Special Creation:
*/
extern HTStructured *HTMLGenerator (HTStream *output);

extern HTStream *HTPlainToHTML (HTPresentation *pres,
        			HTParentAnchor *anchor,
        			HTStream       *sink,
        			HTFormat        format_in,
        			int             compressed);

#endif
