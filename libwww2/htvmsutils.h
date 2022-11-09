/*             VMS specific routines
                                             
 */

#ifndef HTVMSUTIL_H
#define HTVMSUTIL_H

extern BOOL HTVMSFileVersions;	/* Include version numbers in listing? */

/* PUBLIC							HTVMS_wwwName()
**		CONVERTS VMS Name into WWW Name 
** ON ENTRY:
**	vmsname		VMS file specification (NO NODE)
**
** ON EXIT:
**	returns 	www file specification
**
** EXAMPLES:
**	vmsname				wwwname
**	DISK$USER 			disk$user
**	DISK$USER: 			/disk$user/
**	DISK$USER:[DUNS] 		/disk$user/duns
**	DISK$USER:[DUNS.ECHO] 		/disk$user/duns/echo
**	[DUNS] 				duns
**	[DUNS.ECHO] 			duns/echo
**	[DUNS.ECHO.-.TRANS] 		duns/echo/../trans
**	[DUNS.ECHO.--.TRANS] 		duns/echo/../../trans
**	[.DUNS] 			duns
**	[.DUNS.ECHO] 			duns/echo
**	[.DUNS.ECHO]TEST.COM 		duns/echo/test.com 
**	TEST.COM 			test.com
**
**	
*/
extern char *HTVMS_wwwName (char *vmsname);

/* PUBLIC							HTVMS_name()
**		CONVERTS WWW name into a VMS name
** ON ENTRY:
**	nn		Node Name (optional)
**	fn		WWW file name
**
** ON EXIT:
**	returns 	vms file specification
**
** Bug:	Returns pointer to static -- non-reentrant
*/
extern char *HTVMS_name (WWW_CONST char *nn, WWW_CONST char *fn);

extern int HTStat (WWW_CONST char *filename, stat_t *info);

extern int HTVMSBrowseDir (WWW_CONST char *address,
			   HTParentAnchor *anchor,
			   HTFormat        format_out,
			   HTStream       *sink);

#endif
