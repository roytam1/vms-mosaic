/* FTP access module for libwww
                                   FTP ACCESS FUNCTIONS
                                             
   This isn't really a valid protocol module -- it is lumped together
   with HTFile.  That could be changed easily.
   
   Author: Tim Berners-Lee.  Public Domain.
   
 */
#ifndef HTFTP_H
#define HTFTP_H

#include "HTAnchor.h"
#include "HTStream.h"
#include "HTAlert.h"

/*
 * Retrieve File from Server
 *
 *  returns                 Socket number for file if good.  <0 if bad.
 */
extern int HTFTPLoad(char *name, HTParentAnchor *anchor, HTFormat format_out,
		     HTStream *sink);

/* Send file to server */
extern int  HTFTPSend(char *name);

extern void HTFTPClearCache(void);
extern int  HTFTPMkDir(char *name);
extern int  HTFTPRemove(char *name);

#endif
