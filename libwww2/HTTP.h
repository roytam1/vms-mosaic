/*      HyperText Tranfer Protocol                                      HTTP.h
**      ==========================
*/

#ifndef HTTP_H
#define HTTP_H

#include "HTAccess.h"

extern HTProtocol HTTP;
extern HTProtocol HTTPS;
extern void HT_SetExtraHeaders(char **headers);

#endif /* HTTP_H */
