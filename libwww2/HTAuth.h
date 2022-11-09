/*                                   AUTHENTICATION MODULE
                                             
   This is the authentication module.  By modifying the function
   HTAA_authenticate() it can be made to support external authentication
   methods.
   
 */

#ifndef HTAUTH_H
#define HTAUTH_H

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif
#ifndef HTAAUTIL_H
#include "HTAAUtil.h"
#endif
#ifndef HTAAPROT_H
#include "HTAAProt.h"
#endif

/*
** Server's representation of a user (fields in authentication string)
*/
typedef struct {
    HTAAScheme  scheme;         /* Scheme used to authenticate this user */
    char       *username;
    char       *password;
    char       *inet_addr;
    char       *timestamp;
    char       *secret_key;
} HTAAUser;

/*
 * User Authentication
 */

/* SERVER PUBLIC                                        HTAA_authenticate()
**                      AUTHENTICATE USER
** ON ENTRY:
**      scheme          used authentication scheme.
**      scheme_specifics the scheme specific parameters
**                      (authentication string for Basic scheme)
**      prot            is the protection information structure
**                      for the file.
**
** ON EXIT:
**      returns         NULL, if authentication failed.
**                      Otherwise a pointer to a structure
**                      representing authenticated user,
**                      which should not be freed.
*/
PUBLIC HTAAUser *HTAA_authenticate (HTAAScheme   scheme,
                                    char        *scheme_specifics,
                                    HTAAProt    *prot);

#endif  /* HTAUTH_H */
