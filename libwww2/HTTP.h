/*      HyperText Tranfer Protocol                                      HTTP.h
**      ==========================
*/

#ifndef HTTP_H
#define HTTP_H

extern HTProtocol HTTP;
extern HTProtocol HTTPS;
extern void HT_SetExtraHeaders(char **headers);
extern void HTClose_HTTP_Socket(int sock, void *handle);

/* Encrypt verification status */
typedef enum {
    HTSSL_OFF,
    HTSSL_OK,
    HTSSL_NOCERTS,
    HTSSL_VERI_FAILED,
    HTSSL_CERT_ERROR
} HTSSL_Status;

typedef struct _ssl_host {
    char *host;
    BOOLEAN perm;
    struct _ssl_host *next;
} HT_SSL_Host;

#endif
