/* WIDE AREA INFORMATION SERVER SOFTWARE
   No guarantees or restrictions.  See the readme file for the full standard
   disclaimer.    
   3.26.90
*/

/* Copyright (c) CNIDR (see ../COPYRIGHT) */


/* This file is for reading and writing the wais packet header.
 * Morris@think.com
 */

#ifndef lint
static char *RCSid = "$Header: /usr/users/freewais/FreeWAIS-0.1/ir/wmessage.c,v 1.1 1993/02/16 15:05:35 freewais Exp $";
#endif

/* Change log:
 * $Log: wmessage.c,v $
 * Revision 1.1  1993/02/16  15:05:35  freewais
 * Initial revision
 *
 * Revision 1.2  92/02/12  13:56:35  jonathan
 * Added "$Log" so RCS will put the log message in the header
 * 
 * 
*/

/* to do:
 *  add check sum
 *  what do you do when checksum is wrong?
 */

#include <string.h>
#include "wmessage.h"
#include "ustubs.h"
#include "cutil.h"

/*---------------------------------------------------------------------*/

void 
readWAISPacketHeader(msgBuffer,header_struct)
     char* msgBuffer;
     WAISMessage *header_struct;
{
  /* msgBuffer is a string containing at least HEADER_LENGTH bytes. */
		    
  memmove(header_struct->msg_len,msgBuffer,(size_t)10); 
  header_struct->msg_type = char_downcase((unsigned long)msgBuffer[10]);
  header_struct->hdr_vers = char_downcase((unsigned long)msgBuffer[11]);
  memmove(header_struct->server,(void*)(msgBuffer + 12),(size_t)10);
  header_struct->compression = char_downcase((unsigned long)msgBuffer[22]);
  header_struct->encoding = char_downcase((unsigned long)msgBuffer[23]);
  header_struct->msg_checksum = char_downcase((unsigned long)msgBuffer[24]);
}

/*---------------------------------------------------------------------*/

long
getWAISPacketLength(header)
     WAISMessage* header;
     /* interpret the length field, this is necessary since the lenght in the
	message is not null terminated, so atol() may get confused.
	*/
{ 
  char lenBuf[11];
  memmove(lenBuf,header->msg_len,(size_t)10);
  lenBuf[10] = '\0';
  return(atol(lenBuf));
}

/*---------------------------------------------------------------------*/

#ifdef NOTUSEDYET

static char checkSum _AP((char* string,long len));

static char
checkSum(string,len)
     char* string;
     long len;
     /* XXX the problem with this routine is that it can generate 
	non-ascii values.  Since these values are not being hexized,
	they can (and will) hang up some communication channels.
	*/
{
  register long i;
  register char chSum = '\0';
	  
  for (i = 0; i < len; i++)
    chSum = chSum ^ string[i];
	    
  return(chSum);
}	
#endif /* def NOTUSEDYET */

/* this modifies the header argument.  See wais-message.h for the different
 * options for the arguments.
 */

void
writeWAISPacketHeader(header,
		      dataLen,
		      type,
		      server,
		      compression,
		      encoding,
		      version)
     char* header;
     long dataLen;
     long type;
     char* server;
     long compression;
     long encoding;
     long version;
     /* Puts together the new wais before-the-z39-packet header. */
{
  char lengthBuf[11];
  char serverBuf[11];

  long serverLen = strlen(server);
  if (serverLen > 10)
    serverLen = 10;

  sprintf(lengthBuf, "%010ld", dataLen);  
  strncpy(header,lengthBuf,10);

  header[10] = type & 0xFF; 
  header[11] = version & 0xFF;

  strncpy(serverBuf,server,serverLen);       
  strncpy((char*)(header + 12),serverBuf,serverLen);

  header[22] = compression & 0xFF;    
  header[23] = encoding & 0xFF;    
  header[24] = '0';		/* checkSum(header + HEADER_LENGTH,dataLen);   XXX the result must be ascii */	
}              

/*---------------------------------------------------------------------*/




