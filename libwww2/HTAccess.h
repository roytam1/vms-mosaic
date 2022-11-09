/*          HTAccess:  Access manager for libwww
                             ACCESS MANAGER
                                             
   This module keeps a list of valid protocol (naming scheme)
   specifiers with associated access code.  It allows documents to be
   loaded given various combinations of parameters.  New access
   protocols may be registered at any time.
   
   Part of the libwww library.
   
 */
#ifndef HTACCESS_H
#define HTACCESS_H

/*      Definition uses:
*/
#ifndef HTUTILS_H
#include "HTUtils.h"
#endif
#ifndef TCP_H
#include "tcp.h"
#endif
#ifndef HTANCHOR_H
#include "HTAnchor.h"
#endif
#ifndef HTFORMAT_H
#include "HTFormat.h"
#endif

/*      Return codes from load routines:
**
**      These codes may be returned by the protocol modules,
**      and by the HTLoad routines.
**      In general, positive codes are OK and negative ones are bad.
*/

#define HT_NO_DATA -9999        /* Return code: OK but no data was loaded */
                                /* Typically, other app started or forked */

/*
 * Flag which may be set to control this module
 */
extern char *HTClientHost;              /* Name or number of telnetting host */


/*
Load a document from relative name

 ON ENTRY,
  relative_name           The relative address of the file to be accessed.
  here                    The anchor of the object being searched
                         
 ON EXIT,
  returns    YES          Success in opening file
  NO                      Failure
                         
 */
extern BOOL HTLoadRelative(WWW_CONST char *relative_name, HTParentAnchor *here);

/*
Load a document from absolute name

 ON ENTRY,
  addr                    The absolute address of the document to be accessed.
  filter                  if YES, treat document as HTML
                         
 ON EXIT,
  returns YES             Success in opening document
  NO                      Failure
                         
 */
extern int HTLoadAbsolute (WWW_CONST char *addr);


/*
Load a document from absolute name to a stream

 ON ENTRY,
  addr                    The absolute address of the document to be accessed.
  filter                  if YES, treat document as HTML
                         
 ON EXIT,
  returns YES             Success in opening document
                         
  NO                      Failure
                         
   Note: This is equivalent to HTLoadDocument
 */
extern BOOL HTLoadToStream (WWW_CONST char *addr, BOOL filter, HTStream *sink);

/*
Make a stream for Saving object back

 ON ENTRY,
  anchor                  is valid anchor which has previously beeing loaded
                         
 ON EXIT,
  returns                 0 if error else a stream to save the object to.
 */
extern HTStream *HTSaveStream (HTParentAnchor *anchor);


/*
 * Register an access method
 */

typedef struct _HTProtocol {
        char *name;
        int (*load)(WWW_CONST char *full_address,
                    HTParentAnchor *anchor,
                    HTFormat        format_out,
                    HTStream       *sink);
        HTStream *(*saveStream)(HTParentAnchor *anchor);
} HTProtocol;

extern BOOL HTRegisterProtocol (HTProtocol *protocol);

#endif  /* HTACCESS_H */
