/*
                               GENERIC TCP/IP COMMUNICATION
                                             
   This module has the common code for handling TCP/IP connections, etc.
   
 */
#ifndef HTTCP_H
#define HTTCP_H

/*      Produce a string for an internet address
**      ---------------------------------------
**
** On exit:
**           returns a pointer to a static string which must be copied if
**                it is to be kept.
*/
extern char *HTInetString(struct sockaddr_in *sin);


/*      Encode INET status (as in sys/errno.h)                    inet_status()
**      ------------------
**
** On entry:
**              where gives a description of what caused the error
**      global errno gives the error number in the unix way.
**
** On return:
**      returns a negative status in the unix way.
*/
extern int HTInetStatus(char *where);


/*      Parse a cardinal value                                 parse_cardinal()
**      ----------------------
**
** On entry:
**      *pp      points to first character to be interpreted, terminated by
**               non 0..9 character.
**      *pstatus points to status already valid,
**      maxvalue gives the largest allowable value.
**
** On exit:
**      *pp      points to first unread character,
**      *pstatus points to status updated iff bad
*/

extern unsigned int HTCardinal(int *pstatus, char **pp, unsigned int max_value);


/*      Parse an internet node address and port
**      ---------------------------------------
**
** On entry:
**               str points to a string with a node name or number,
**               with optional trailing colon and port number.
**               sin points to the binary internet or decnet address field.
**
** On exit:
**               *sin is filled in.  If no port is specified in str, that
**               field is left unchanged in *sin.
*/
extern int HTParseInet(struct sockaddr_in *sin, char *str);


/*      Get Name of This Machine
**      ------------------------
**
*/
extern char *HTHostName();

extern int HTDoConnect(char *, char *, int, int *);

extern int HTDoRead(int, void *, unsigned);

#endif   /* HTTCP_H */
