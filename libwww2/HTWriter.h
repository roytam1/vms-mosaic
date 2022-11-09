
/*              Unix File or Socket Writer                      HTWriter.c
**              --------------------------
**
**      This version of the stream object just writes to a socket.
**      The socket is assumed open and closed afterward.
**
**      Bugs:
**              strings written must be less than buffer size.
*/

#ifndef HTWRITE_H
#define HTWRITE_H

#ifndef HTSTREAM
#include "HTStream.h"
#endif

extern HTStream *HTWriter_new (int soc);

#endif
