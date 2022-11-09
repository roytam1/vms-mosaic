/*	HyperText Tranfer Protocol	- Client implementation		HTTP.c
**	==========================
*/

/* Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"

#define HTTP_VERSION  " HTTP/1.0"
#define HTTP_PORT    80
#define HTTPS_PORT  443

#define INIT_LINE_SIZE		1024	/* Start with line buffer this big */
#define LINE_EXTEND_THRESH	256	/* Minimum read size */
#define VERSION_LENGTH 		20	/* For returned protocol version */

#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>
#ifndef DISABLE_TRACE
#include <openssl/err.h>
#endif
extern char *built_time;
#endif

#include "HTParse.h"
#include "HTUtils.h"
#include "HTTCP.h"
#include "HTFormat.h"
#include "HTFile.h"
#include "HTTP.h"
#include <ctype.h>
#include "HTAlert.h"
#include "HTMIME.h"
#include "HTML.h"
#include "HTInit.h"
#include "HTAABrow.h"
#include "HTCookie.h"
#include "HTMultiLoad.h"

#ifdef HAVE_SSL
#include "../src/mosaic.h"
extern mo_window *current_win;
extern XtAppContext app_context;
#endif

int useKeepAlive = 1;
extern int securityType;
int sendAgent = 1;
int sendReferer = 1;
extern int selectedAgent;
extern char **agent;
extern int retried_with_new_nonce;  /* For digest auth */
extern char *redirecting_url;
extern int broken_crap_hack;
extern int HTSetCookies;
extern MultiInfo *HTMultiLoading;

#ifndef DISABLE_TRACE
int httpTrace = 0;
int www2Trace = 0;
#endif

char **extra_headers = NULL;

extern WWW_CONST HTStreamClass HTMIME;

struct _HTStream {
    HTStreamClass *isa;
};

/* For browser to call */
void HT_SetExtraHeaders(char **headers)
{
    extra_headers = headers;
}

/* Defined in src/mo-www.c */
extern char *HTAppName;		/* Application name */
/* Defined in src/gui.c */
extern char *HTAppVersion;	/* Application version */
extern char *HTReferer;		/* HTTP referer field */

/* Variables that control whether we do a POST or a GET,
 * and if a POST, what and how we POST.  And HEAD */
int do_head = 0;
int do_post = 0;
int do_put = 0;
int put_file_size = 0;
int bad_location = 0;
FILE *put_fp;
char *post_content_type = NULL;
char *post_data = NULL;
extern BOOL using_gateway;    /* Are we using an HTTP gateway? */
extern char *proxy_host_fix;  /* For the Host: header */
extern BOOL using_proxy;      /* Are we using an HTTP proxy gateway? */
PUBLIC BOOL reloading = NO;   /* Did someone say, "RELOAD!?!?!" */
PUBLIC HT_SSL_Host *SSL_ignore_hosts = NULL;	/* Must be outside HAVE_SSL */

#ifdef HAVE_SSL
PUBLIC char *encrypt_cipher = NULL;
PUBLIC char *encrypt_issuer = NULL;
PUBLIC int encrypt_bits;
PUBLIC HTSSL_Status encrypt_status;

PRIVATE SSL *keepalive_handle = NULL;
PRIVATE HTSSL_Status keepalive_status;
PUBLIC SSL_CTX *ssl_ctx = NULL;		/* SSL ctx */
PUBLIC SSL *SSL_handle = NULL;
PRIVATE int ssl_ignore;
PRIVATE char *ssl_url;
PRIVATE BOOL try_tls;
/* Note: we never actually turn verification off, we just ignore the errors */
PUBLIC int verifyCertificates = 1;

PRIVATE void free_ssl_ctx (void)
{
    if (ssl_ctx)
        SSL_CTX_free(ssl_ctx);
}

PRIVATE int SSL_confirm(char *msg)
{
    int ch;
    HT_SSL_Host *hosts = SSL_ignore_hosts;
    char *host = HTParse(ssl_url, "", PARSE_HOST);

    /* Are we already ignoring errors for this host? */
    while (hosts) {
	if (!my_strcasecmp(host, hosts->host)) {
	    free(host);
	    return (hosts->perm ? 4 : 3);
	}
	hosts = hosts->next;
    }
    XmxSetButtonClueText("Ignore error once", "Abort connection",
			 "Ignore SSL errors for host this session",
			 "Ignore SSL errors for this host always", NULL);
    ch = XmxDoFourButtons(current_win->base, app_context,
		          "VMS Mosaic: SSL Certificate Error", msg,
		          "Yes", "No", "This session only", "Always", 520);
    XmxSetButtonClueText(NULL, NULL, NULL, NULL, NULL);

    ch = XmxExtractToken(ch);
    if (ch > 2) {
	HT_SSL_Host *shost = malloc(sizeof(HT_SSL_Host));

        shost->host = host;
	if (ch == 3) {
	    shost->perm = FALSE;
	} else {
	    shost->perm = TRUE;
	}
	shost->next = SSL_ignore_hosts;
	SSL_ignore_hosts = shost;
    } else {
	free(host);
    }
    return ch;
}

PRIVATE int HTSSLCallback(int preverify_ok, X509_STORE_CTX *x509_ctx)
{
    char msg[1024];
    int error;
    int result = 1;
    static int ssl_certs_not_okay = 0;

    if (!verifyCertificates) {
	encrypt_status = HTSSL_OFF;
	return 1;
    }
    if (preverify_ok)
	/* No error, but leave encrypt_status alone because previous
	 * callback may have set it */
	return 1;

    error = X509_STORE_CTX_get_error(x509_ctx);

    if (!ssl_certs_not_okay &&
	((error == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY) ||
	 (error == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN))) {
	static int checked_file = 0;

	if (!checked_file) {
	    FILE *fp;

	    if (fp = fopen(X509_get_default_cert_file_env(), "r")) {
	        fclose(fp);
	    } else if (fp = fopen(X509_get_default_cert_file(), "r")) {
	        fclose(fp);
	    } else {
		ssl_certs_not_okay = 1;
		ssl_ignore = 1;
	        /* No certificate file, so only ask about it once */
	        HTProgress("No local SSL certificates found");
	        application_warning(
	         "No local SSL root authority certificates found.\nContinuing without certificate verification.",
	         "VMS Mosaic: SSL Error");
	    }
	    checked_file = 1;
	}
	if (!ssl_ignore) {
	    int ch;

	    /* Stupidly this error can be due to no local certs */
	    if (error == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN) {
		sprintf(msg, "SSL error: %s\nContinue?",
			X509_verify_cert_error_string(error));
		ch = SSL_confirm(msg);
		if (ch > 2) {
		    /* One error message per connection */
		    ssl_ignore = 1;
		} else if (ch == 2) {
		    result = 0;
		}
	    } else {
	        ch = SSL_confirm(
		    "No local SSL certificate found for this host - Continue?");
	        if (ch > 2) {
	            /* One error message per connection */
		    ssl_ignore = 1;
	        } else if (ch == 2) {
	            result = 0;
	        }
	    }
	}
    } else if (!ssl_ignore && !ssl_certs_not_okay) {
	int ch;

	sprintf(msg, "SSL error: %s\nContinue?",
		X509_verify_cert_error_string(error));
	ch = SSL_confirm(msg);
	if (ch > 2) {
	    /* One error message per connection */
	    ssl_ignore = 1;
	} else if (ch == 2) {
	    result = 0;
	}
    }
    if (ssl_certs_not_okay) {
	encrypt_status = HTSSL_NOCERTS;
    } else {
	encrypt_status = HTSSL_VERI_FAILED;
    }
    if (!result)
	try_tls = FALSE;

    /* Return 1 to continue verification, 0 to abort */
    return result;
}

PRIVATE SSL *HTGetSSLHandle (char *url)
{
    static int verify = -1;

    if (!ssl_ctx) {
        /*
	 *  First time only.
	 */
	time_t foo = time(NULL);
	char *tmp = tmpnam(NULL);
	char *ts = ctime(&foo);

	SSL_library_init();
	ssl_ctx = SSL_CTX_new(SSLv23_client_method());
	SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL);
	SSL_CTX_set_default_verify_paths(ssl_ctx);
	atexit(free_ssl_ctx);

	/* 0.9.5 (and later) needs this (at least 128 bits of data) */
	RAND_seed(tmp, strlen(tmp));
	RAND_seed(url, strlen(url));
	RAND_seed(ts, strlen(ts));
	RAND_seed(built_time, strlen(built_time));
    }
    if (verify != verifyCertificates) {
	verify = verifyCertificates;
        if (verifyCertificates) {
	    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, HTSSLCallback);
        } else {
	    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
        }
    }
    if (!verifyCertificates)
	encrypt_status = HTSSL_OFF;
    ssl_ignore = 0;
    return(SSL_new(ssl_ctx));
}

#define HTTP_NETREAD(sock, buff, size, handle) \
    (handle ? SSL_read(handle, buff, size) : NETREAD(sock, buff, size))
#define HTTP_NETWRITE(sock, buff, size, handle) \
    (handle ? SSL_write(handle, buff, size) : NETWRITE(sock, buff, size))
#define HTTP_NETCLOSE(sock, handle) \
    { NETCLOSE(sock); if (handle) SSL_free(handle); SSL_handle = handle = NULL;}

#else

#define HTTP_NETREAD(a, b, c, d)   NETREAD(a, b, c)
#define HTTP_NETWRITE(a, b, c, d)  NETWRITE(a, b, c)
#define HTTP_NETCLOSE(a, b)	   NETCLOSE(a)

#endif  /* HAVE_SSL */

#define end_component(p) (*(p) == '.' || *(p) == '\0')

/*
 * Compare names as described in RFC 2818: ignore case, allow wildcards. 
 * Return zero on a match, nonzero on mismatch -TD
 *
 * From RFC 2818:
 * Names may contain the wildcard character * which is considered to match any
 * single domain name component or component fragment.  E.g., *.a.com matches
 * foo.a.com but not bar.foo.a.com.  f*.com matches foo.com but not bar.com.
 */
PRIVATE int strcasecomp_asterisk(char *a, char *b)
{
    char *p;
    int result = 0;
    int done = FALSE;

    while (!result && !done) {
	if (*a == '*') {
	    p = b;
	    for (;;) {
		if (end_component(p)) {
		    if (end_component(a + 1)) {
			b = p - 1;
			result = 0;
		    } else {
			result = 1;
		    }
		    break;
		} else if (strcasecomp_asterisk(a + 1, p)) {
		    p++;
		    result = 1;	 /* Could not match */
		} else {
		    b = p - 1;
		    result = 0;	 /* Found a match starting at 'p' */
		    done = TRUE;
		    break;
		}
	    }
	} else if (*b == '*') {
	    result = strcasecomp_asterisk(b, a);
	    done = (result == 0);
	} else if (*a == '\0' || *b == '\0') {
	    result = (*a != *b);
	    break;
	} else if (TOLOWER(*a) != TOLOWER(*b)) {
	    result = 1;
	    break;
	}
	a++;
	b++;
    }
    return result;
}

PRIVATE void TrimDoubleQuotes (char *value)
{
    int i;
    char *cp = value;

    if (!(cp && *cp) || *cp != '\"')
        return;

    i = strlen(cp);
    if (cp[i - 1] != '\"') {
        return;
    } else {
        cp[i - 1] = '\0';
    }
    for (i = 0; value[i]; i++)
        value[i] = cp[i + 1];
}

void HTClose_HTTP_Socket(int sock, void *handle)
{
	HTTP_NETCLOSE(sock, handle);
}

/*		Load Document from HTTP Server			HTLoadHTTP()
**		==============================
**
**	Given a hypertext address, this routine loads a document.
**
**
** On entry,
**	arg	is the hypertext reference of the article to be loaded.
**
** On exit,
**	returns	>=0	If no error, a good socket number
**		<0	Error.
**
*/

/* Where was our last connection to? */
static int lsocket = -1;
static char *addr = NULL;

PUBLIC int HTLoadHTTP (char *arg,
		       HTParentAnchor *anAnchor,
		       HTFormat format_out,
		       HTStream *sink)
{
  int s;			/* Socket number for returned data */
  WWW_CONST char *url = arg;
  char *command = NULL;		/* The whole command */
  char *eol;			/* End of line if found */
  char *start_of_data;		/* Start of body of reply */
  char *p, *begin_ptr, *tmp_ptr;
  int status;			/* tcp return */
  int bytes_already_read;
  HTStream *target;		/* Unconverted data */
  HTFormat format_in;		/* Format arriving in the message */
  BOOL had_header;		/* Have we had at least one header? */
  BOOL extensions;		/* Assume good HTTP server */
  char *line_buffer = NULL;
  char *line_kept_clean = NULL;
  char line[2048];		/* Bumped up to cover Kerb huge headers */
  int i, length, rawlength, done_length;
  int rv, return_nothing, env_length, compressed;
  int already_retrying = 0;
  int keepingalive = 0;
  int statusError = 0;
  BOOL auth_proxy = NO;         /* Generate a proxy authorization. */
#ifdef HAVE_SSL
  BOOL do_connect = FALSE;	/* Are we going to use a proxy tunnel ? */
  BOOL did_connect = FALSE;	/* Are we actually using a proxy tunnel ? */
  WWW_CONST char *connect_url = NULL; /* The URL being proxied */
  char *connect_host = NULL;	/* The host being proxied */
  SSL *handle;			/* The SSL handle */
#else
  void *handle = NULL;
#endif
  static char crlf[3];		/* CR LF string */
  static HTAtom *html_in, *mime_in, *plain_in, *www_present;
  static int init = 0;

  if (!init) {
      sprintf(crlf, "\r\n");
      html_in = HTAtom_for("text/html");
      mime_in = HTAtom_for("www/mime");
      plain_in = HTAtom_for("text/plain");
      www_present = WWW_PRESENT;
      init = 1;
  }
  if (!arg || !*arg) {
      if (!arg) {
          status = -3;
      } else {
          status = -2;
      }
      HTProgress("Bad request.");
      goto done;
  }

#ifdef HAVE_SSL
  try_tls = TRUE;
  SSL_handle = handle = NULL;
  ssl_url = arg;
  FREE(encrypt_cipher);
  /* Set encrypt initial status */
  if (verifyCertificates) {
      encrypt_status = HTSSL_OK;
  } else {
      encrypt_status = HTSSL_OFF;
  }
  if (using_proxy && !strncmp(arg, "http://", 7)) {
      if (connect_url = strstr(arg + 7, "https://")) {
	  do_connect = TRUE;
	  connect_host = HTParse(connect_url, "https", PARSE_HOST);
	  if (!strchr(connect_host, ':')) {
	      sprintf(line, ":%d", HTTPS_PORT);
	      StrAllocCat(connect_host, line);
	  }
#ifndef DISABLE_TRACE
	  if (httpTrace)
	      fprintf(stderr,
		      "HTTP: connect_url = '%s'\n      connect_host = '%s'\n",
		      connect_url, connect_host);
#endif
      }
  }
#endif

  /* At this point, we're talking HTTP/1.0. */
  extensions = YES;

 try_again:
  /* These initializations are moved down here from up above,
   * so we can start over here... */
  eol = 0;
  bytes_already_read = 0;
  had_header = NO;
  length = 0;
  compressed = 0;
  target = NULL;
  FREE(line_buffer);
  FREE(line_kept_clean);
  return_nothing = 0;

  /* Okay... addr looks like http://hagbard.ncsa.uiuc.edu/blah/etc.html 
   * lets crop it at the 3rd '/' */
  for (p = url, i = 0; *p && i != 3; p++) {
      if (*p == '/')
	  i++;
  }
  if (i == 3) {
      i = p - url;      /* i = length not counting last '/' */
  } else {
      i = 0;
  }
  if ((lsocket != -1) && i && addr && !strncmp(addr, url, i)) {
      /* Keepalive active and addresses match -- try the old socket */
      s = lsocket;
#ifdef HAVE_SSL
      SSL_handle = handle = keepalive_handle;
      if (handle) {
	  char ssl_info[256];

	  encrypt_status = keepalive_status;
 	  StrAllocCopy(encrypt_cipher, (char *)SSL_get_cipher(handle));
	  encrypt_bits = SSL_get_cipher_bits(handle, NULL);
          X509_NAME_oneline(
			 X509_get_issuer_name(SSL_get_peer_certificate(handle)),
			 ssl_info, sizeof(ssl_info));
	  StrAllocCopy(encrypt_issuer, ssl_info);
      }
#endif
      /* Flag in case of network error due to server timeout */ 
      keepingalive = 1;
      lsocket = -1;  /* Prevent looping on failure */
#ifndef DISABLE_TRACE
      if (httpTrace)
	  fprintf(stderr, "HTTP: Keep-Alive reusing '%s'\n", addr);
#endif
  } else {
      if (addr)
	  free(addr);
      /* Save the address for next time around */
      addr = malloc(i + 1);
      strncpy(addr, url, i);
      *(addr + i) = '\0';

      keepingalive = 0;  /* Just normal opening of the socket */
      if (lsocket != -1) {
	  /* No socket leaks here */
	  HTTP_NETCLOSE(lsocket, keepalive_handle);
	  lsocket = -1;  /* Wait server says okay */
      }
#ifndef DISABLE_TRACE
      if (httpTrace)
	  fprintf(stderr, "HTTP: Keep-Alive saving '%s'\n", addr);
#endif
  }

  if (!keepingalive) {
      if (!strncmp(url, "https", 5)) {
#ifdef HAVE_SSL
#ifndef DISABLE_TRACE
          if (httpTrace)
	      fprintf(stderr, "HTTP: Doing secure connect.\n");
#endif
	  status = HTDoConnect(url, "HTTPS", HTTPS_PORT, &s);
      } else {
          status = HTDoConnect(url, "HTTP", HTTP_PORT, &s);
      }
#else
          application_user_feedback(
			"This client does not contain support for https URLs.");
          status = HT_NOT_LOADED;
          goto done;
      }
      status = HTDoConnect(url, "HTTP", HTTP_PORT, &s);
#endif
      if (status == HT_INTERRUPTED) {
	  /* Interrupt cleanly. */
#ifndef DISABLE_TRACE
	  if (httpTrace)
	      fprintf(stderr,
		      "HTTP: Interrupted on connect; recovering cleanly.\n");
#endif
	  HTProgress("Connection interrupted.");
	  goto done;
      }
      if (status < 0) {
#ifndef DISABLE_TRACE
	  if (httpTrace)
	      fprintf(stderr, 
		      "HTTP: Cannot connect to host for `%s' (errno = %d).\n",
		      url, errno);
#endif
	  HTProgress("Unable to connect to remote host.");
	  status = HT_NO_DATA;
	  goto done;
      }   
  }
#ifdef HAVE_SSL
 use_tunnel:
  /*
  ** If this is an https document then do the SSL stuff here
  */
  if (did_connect || (!strncmp(url, "https", 5) && !keepingalive)) {
      X509 *peer_cert;
      char ssl_info[1024];

      SSL_handle = handle = HTGetSSLHandle(arg);
      if (!did_connect)
	  keepalive_handle = handle;
      SSL_set_fd(handle, s);
      if (!try_tls)
          handle->options |= SSL_OP_NO_TLSv1;
      status = SSL_connect(handle);
      if (status <= 0) {
	  if (try_tls) {
#ifndef DISABLE_TRACE
	      if (httpTrace) {
		  ERR_print_errors_fp(stderr);
	          fprintf(stderr, "HTTP: Retrying connection without TLS\n");
	      }
#endif
	      HTProgress("Retrying secure connection.");
	      try_tls = FALSE;
	      if (did_connect)
	          HTTP_NETCLOSE(s, handle);
      	      goto try_again;
	  } else {
#ifndef DISABLE_TRACE
	      if (httpTrace) {
                  fprintf(stderr,
                     "HTTP: Failed SSL handshake for host '%s' (SSLerr = %d)\n",
		     url, status);
		  ERR_print_errors_fp(stderr);
	      }
#endif
      	      HTProgress("Unable to make secure connection to remote host.");
	      if (did_connect)
	          HTTP_NETCLOSE(s, handle);
      	      status = HT_NOT_LOADED;
      	      goto done;
	  }
      }
      StrAllocCopy(encrypt_cipher, (char *)SSL_get_cipher(handle));
      encrypt_bits = SSL_get_cipher_bits(handle, NULL);
      peer_cert = SSL_get_peer_certificate(handle);
      X509_NAME_oneline(X509_get_issuer_name(peer_cert),
			ssl_info, sizeof(ssl_info));
#ifndef DISABLE_TRACE
      if (httpTrace)
          fprintf(stderr, "HTTP: Got SSL cert Issuer: '%s'\n", ssl_info);
#endif
      StrAllocCopy(encrypt_issuer, ssl_info);
      if (!ssl_ignore) {
	  char *cert_host, *ssl_host, *ssl_dn_start;
          char *ssl_all_cns = NULL;
	  int status_sslcertcheck = 0;

          X509_NAME_oneline(X509_get_subject_name(peer_cert),
			    ssl_info, sizeof(ssl_info));
#ifndef DISABLE_TRACE
          if (httpTrace)
              fprintf(stderr, "HTTP: Got SSL cert Subject: '%s'\n", ssl_info);
#endif
	  /*
	   * X.509 DN validation taking ALL CN fields into account
	   * (c) 2006 Thorsten Glaser <tg@mirbsd.de>
	   */
	  ssl_dn_start = ssl_info;
	  /* Get host we're connecting to */
	  ssl_host = HTParse(url, "", PARSE_HOST);
	  /* Strip port number or extract hostname component */
	  if (p = strchr(ssl_host, (*ssl_host == '[') ? ']' : ':'))
	      *p = '\0';
	  if (*ssl_host == '[')
	      ssl_host++;

	  /* Validate all CNs found in DN */
	  while (cert_host = strstr(ssl_dn_start, "/CN=")) {
	      status_sslcertcheck = 1;	/* 1 = could not verify CN */
	      /* Start of CommonName */
	      cert_host += 4;
	      /* Find next part of DistinguishedName */
	      if (p = strchr(cert_host, '/')) {
		  *p = '\0';
		  ssl_dn_start = p;	/* Yes, this points to the NULL byte */
	      } else {
		  ssl_dn_start = NULL;
	      }

	      /* Strip port number (XXX [ip]:port encap here too? -TG) */
	      if (p = strchr(cert_host, (*cert_host == '[') ? ']' : ':'))
		  *p = '\0';
	      if (*cert_host == '[')
		  cert_host++;

	      /* Verify this CN */
	      if (!strcasecomp_asterisk(ssl_host, cert_host)) {
		  status_sslcertcheck = 2;	/* 2 = verified peer */
		  /* I think this is cool to have in the logs --mirabilos */
#ifndef DISABLE_TRACE
	          if (httpTrace)
	              fprintf(stderr, "Verified connection to %s (cert=%s)\n",
			      ssl_host, cert_host);
#endif
		  /* No need to continue the verification loop */
		  break;
	      }
	      /* Add this CN to list of failed CNs */
	      if (!ssl_all_cns) {
		  StrAllocCopy(ssl_all_cns, "CN<");
	      } else {
		  StrAllocCat(ssl_all_cns, ":CN<");
	      }
	      StrAllocCat(ssl_all_cns, cert_host);
	      StrAllocCat(ssl_all_cns, ">");
	      /* If we cannot retry, don't try it */
	      if (!ssl_dn_start)
		  break;
	      /* Now retry next CN found in DN */
	      *ssl_dn_start = '/';	/* Formerly NULL byte */
	  }

	  /* Check the X.509v3 Subject Alternative Name */
	  if (status_sslcertcheck < 2) {
	      STACK_OF(GENERAL_NAME) *gens;
	      int i, numalts;
	      GENERAL_NAME *gn;

	      if (gens = X509_get_ext_d2i(peer_cert, NID_subject_alt_name,
					  NULL, NULL)) {
		  numalts = sk_GENERAL_NAME_num(gens);
		  for (i = 0; i < numalts; ++i) {
		      gn = sk_GENERAL_NAME_value(gens, i);
		      if (gn->type == GEN_DNS) {
			  cert_host = (char *) ASN1_STRING_data(gn->d.ia5);
		      } else if (gn->type == GEN_IPADD) {
			  /* XXX untested -TG */
			  size_t j = ASN1_STRING_length(gn->d.ia5);

			  cert_host = malloc(j + 1);
			  memcpy(cert_host, ASN1_STRING_data(gn->d.ia5), j);
			  cert_host[j] = '\0';
		      } else {
			  continue;
		      }
		      status_sslcertcheck = 1;	/* Got at least one */

		      /* Verify this SubjectAltName (see above) */
		      if (p = strchr(cert_host,
				     (*cert_host == '[') ? ']' : ':'))
			  *p = '\0';
		      if (*cert_host == '[')
			  cert_host++;
		      if (!(gn->type == GEN_IPADD ? my_strcasecmp :
			    strcasecomp_asterisk) (ssl_host, cert_host)) {
			  status_sslcertcheck = 2;
#ifndef DISABLE_TRACE
			  if (httpTrace)
			      fprintf(stderr,
				      "Verified connection to %s (subj=%s)\n",
				      ssl_host, cert_host);
#endif
			  if (gn->type == GEN_IPADD)
			      free(cert_host);
			  break;
		      }
		      /* Add to list of failed CNs */
		      if (!ssl_all_cns) {
			  StrAllocCopy(ssl_all_cns, "SAN<");
		      } else {
			  StrAllocCat(ssl_all_cns, ":SAN<");
		      }
		      if (gn->type == GEN_DNS) {
			  StrAllocCat(ssl_all_cns, "DNS=");
		      } else if (gn->type == GEN_IPADD) {
			  StrAllocCat(ssl_all_cns, "IP=");
		      }
		      StrAllocCat(ssl_all_cns, cert_host);
		      StrAllocCat(ssl_all_cns, ">");
		      if (gn->type == GEN_IPADD)
			  free(cert_host);
		  }
		  sk_GENERAL_NAME_free(gens);
	      }
	  }

	  /* If an error occurred, format the appropriate message */
	  if (!status_sslcertcheck) {
	      encrypt_status = HTSSL_CERT_ERROR;
	      if (verifyCertificates) {
		  int ch = SSL_confirm(
	               "Can't find common name in SSL certificate - Continue?");

	          if (ch > 2) {
		      ssl_ignore = 1;
	          } else if (ch == 2) {
	              status = HT_NOT_LOADED;
	              if (did_connect)
	                  HTTP_NETCLOSE(s, handle);
	              goto done;
	          }
	      }
	  } else if (status_sslcertcheck == 1) {
	      encrypt_status = HTSSL_CERT_ERROR;
	      if (verifyCertificates) {
		  int ch;

	          sprintf(line,
                      "Secure host (%s)\ndoes not match SSL certificate host (%s).\nContinue?",
	              ssl_host, ssl_all_cns);
	          ch = SSL_confirm(line);
	          if (ch > 2) {
		      ssl_ignore = 1;
	          } else if (ch == 2) {
		      status = HT_NOT_LOADED;
		      if (did_connect)
		          HTTP_NETCLOSE(s, handle);
		      FREE(ssl_all_cns);
		      goto done;
	          }
	      }
	  }
	  FREE(ssl_all_cns);
      }
      keepalive_status = encrypt_status;
  }
#endif

  /*	Ask the node for the document, omitting the host name and anchor
   */        
  {
    char *p1 = HTParse(url, "", PARSE_PATH | PARSE_PUNCTUATION);
    char *ctype;

#ifdef HAVE_SSL
    if (do_connect) {
	ctype = "CONNECT ";
    } else
#endif
    if (do_post) {
	if (do_put) {
	    ctype = "PUT ";
	} else {
	    ctype = "POST ";
	}
    } else if (do_head) {
	ctype = "HEAD ";
    } else {
	ctype = "GET ";
    }
    StrAllocCopy(command, ctype);
    /*
     * For a gateway, the beginning '/' on the request must
     * be stripped before appending to the gateway address.
     */
#ifdef HAVE_SSL
    if ((using_gateway || using_proxy) && !did_connect) {
	if (do_connect) {
	    StrAllocCat(command, connect_host);
	} else {
	    StrAllocCat(command, p1 + 1);
	}
#else
    if (using_gateway || using_proxy) {
        StrAllocCat(command, p1 + 1);
#endif
    } else {
        StrAllocCat(command, p1);
    }
    free(p1);
  }
  if (extensions)
      StrAllocCat(command, HTTP_VERSION);
  
  StrAllocCat(command, crlf);	/* CR LF, as in rfc 977 */

  if (extensions) {
      static char *accept = NULL;

      if (!HTPresentations) {
	  HTFormatInit();
	  /* Needs to be rebuilt */
	  if (accept) {
	      free(accept);
	      accept = NULL;
	  }
      }
      /* Only build it once */
      if (!accept) {
          int i;
          int count = 0;
          int n = HTList_count(HTPresentations);

          env_length = strlen("Accept:");
          StrAllocCat(accept, "Accept:");
          begin_ptr = accept;

          for (i = 0; i < n; i++) {
              HTPresentation *pres = HTList_objectAt(HTPresentations, i);

	      /* Don't send the ones we actually don't know how to handle */
              if ((pres->rep_out == www_present) && !pres->secs_per_byte) {
	          sprintf(line, " %s,", HTAtom_name(pres->rep));
	          env_length += strlen(line);
	          StrAllocCat(accept, line);
	          if (env_length > 200) {
		      if (tmp_ptr = strrchr(accept, ','))
		          *tmp_ptr = '\0';
		      StrAllocCat(accept, crlf);
		      begin_ptr = accept + strlen(accept);
		      env_length = strlen("Accept:");
		      StrAllocCat(accept, "Accept:");
	          }
	          count++;
              }
              /* More than 48 will cause some servers to abort the connection */
	      if (count == 47) {
		  /* Always send fallback (it's always at end of the list) */
		  if ((i + 1) < n)
		      StrAllocCat(accept, " */*,");
	          break;
	      }
          }
          /* This gets rid of the last comma. */
          if (tmp_ptr = strrchr(accept, ',')) {
	      *tmp_ptr = '\0';
	      StrAllocCat(accept, crlf);
          } else {  /* No accept stuff... get rid of "Accept:" */
	      begin_ptr = '\0';
          }
      } else {
	  StrAllocCat(command, accept);
      }

      /* If reloading, send no-cache pragma to proxy servers.
       * Original patch from Ari L. <luotonen@dxcern.cern.ch>
       *
       * Also send it as a Cache-Control header for HTTP/1.1. (from LYNX)
       */
      if (reloading)
	  StrAllocCat(command,
		      "Pragma: no-cache\r\nCache-Control: no-cache\r\n");

      /* This is just used for "not" sending this header on a proxy request */
      if (useKeepAlive && !do_head) { 
	  StrAllocCat(command, "Connection: Keep-Alive\r\n");
      } else if (do_head) {
	  StrAllocCat(command, "Connection: close\r\n");
      }
      if (sendAgent) {
	  sprintf(line, "User-Agent: %s\r\n", agent[selectedAgent]);
	  StrAllocCat(command, line);
      }

      /* HTTP Referer field, specifies back-link URL */
      if (sendReferer && HTReferer) {
	  sprintf(line, "Referer: %s\r\n", HTReferer);
	  StrAllocCat(command, line);
	  HTReferer = NULL;
      }

      /* addr is always in URL form */
      if (addr && !using_proxy && !using_gateway) {
	  char *startPtr;
	  char *tmp = strdup(addr);

	  if (startPtr = strchr(tmp, '/')) {
	      startPtr += 2;  /* Now at begining of hostname */
	      if (*startPtr) {
		  char *endPtr = strchr(startPtr, ':');

		  if (!endPtr) {
		      endPtr = strchr(startPtr, '/');
		      if (endPtr && *endPtr)
			  *endPtr = '\0';
		  } else {
		      *endPtr = '\0';
		  }
		  sprintf(line, "Host: %s\r\n", startPtr);
		  StrAllocCat(command, line);
	      }
	  }
	  free(tmp);
      } else if (using_proxy || using_gateway) {
	  sprintf(line, "Host: %s\r\n", proxy_host_fix);
	  StrAllocCat(command, line);
      }

      /* HTTP Extension headers */

      /* Domain Restriction */
      StrAllocCat(command, "Extension: Notify-Domain-Restriction\r\n");

      /* Allow arbitrary headers sent from browser */
      if (extra_headers) {
	  int h;

	  for (h = 0; extra_headers[h]; h++) {
	      sprintf(line, "%s\r\n", extra_headers[h]);
	      StrAllocCat(command, line);
	  }
      }

      {
        char *abspath, *docname, *hostname, *colon, *auth;
        int portnumber;
	char *cookie = NULL;
	BOOL secure = (strncmp(anAnchor->address, "https", 5) ? FALSE : TRUE);
        
        abspath = HTParse(arg, "", PARSE_PATH | PARSE_PUNCTUATION);
        docname = HTParse(arg, "", PARSE_PATH);
        hostname = HTParse(arg, "", PARSE_HOST);
        if (hostname && (colon = strchr(hostname, ':'))) {
            *colon++ = '\0';	/* Chop off port number */
            portnumber = atoi(colon);
	} else if (!strncmp(arg, "https", 5)) {
	    portnumber = HTTPS_PORT;
        } else {
	    portnumber = HTTP_PORT;
	}
	/*
	**  Add Authorization, Proxy-Authorization,
	**  and/or Cookie headers, if applicable.
	*/
	if (using_proxy) {
	    /*
	    **	If we are using a proxy, first determine if
	    **	we should include an Authorization header
	    **	for the ultimate target of this request.
	    */
	    char *host2 = HTParse(docname, "", PARSE_HOST);
	    char *path2 = HTParse(docname, "", PARSE_PATH | PARSE_PUNCTUATION);
	    int port2 = strncmp(docname, "https", 5) ? HTTP_PORT : HTTPS_PORT;

	    if (host2 && (colon = strchr(host2, ':'))) {
		/* Use non-default port number */
		*colon++ = '\0';
		port2 = atoi(colon);
	    }
	    /*
	    **	This composeAuth() does file access, i.e., for
	    **	the ultimate target of the request. - AJL
	    */
	    auth_proxy = NO;
	    if ((auth = HTAA_composeAuth(host2, port2, path2, auth_proxy)) &&
		*auth) {
		/*
		**  If auth is not NULL nor zero-length, it's
		**  an Authorization header to be included.
		*/
		sprintf(line, "%s\r\n", auth);
		StrAllocCat(command, line);
	    }
#ifndef DISABLE_TRACE
	    if (httpTrace) {
		if (auth && *auth) {
		    fprintf(stderr, "HTTP: Sending authorization: %s\n", auth);
		} else {
		    fprintf(stderr, "HTTP: Not sending authorization (yet)\n");
		}
            }
#endif
            /*
            **  Add 'Cookie:' header, if it's HTTP or HTTPS
            **  document being proxied.
            */
            if (!strncmp(docname, "http", 4))
                cookie = HTCookie(host2, path2, port2, secure);
            FREE(host2);
            FREE(path2);
            /*
            **  The next composeAuth() will be for the proxy.
            */
	    auth_proxy = YES;
	} else {
           /*
            **  Add cookie for a non-proxied request.
            */
            cookie = HTCookie(hostname, abspath, portnumber, secure);
	    auth_proxy = NO;
	}
	/*
	**  If we do have a cookie set, add it to the request buffer. - FM
	*/
	if (cookie) {
	    if (*cookie != '$') {
		/*
		**  It's an historical cookie, so signal to the
		**  server that we support modern cookies. - FM
		*/
		StrAllocCat(command, "Cookie2: $Version=\"1\"\r\n");
#ifndef DISABLE_TRACE
		if (httpTrace)
		    fprintf(stderr, "HTTP: Sending Cookie2: $Version =\"1\"\n");
#endif
	    }
	    if (*cookie) {
		/*
		**  It's not a zero-length string, so add the header.
		**  Note that any folding of long strings has been
		**  done already in HTCookie.c. - FM
		*/
		StrAllocCat(command, "Cookie: ");
		StrAllocCat(command, cookie);
		StrAllocCat(command, crlf);
#ifndef DISABLE_TRACE
		if (httpTrace)
		    fprintf(stderr, "HTTP: Sending Cookie: %s\n", cookie);
#endif
	    }
	    free(cookie);
	}
        if (auth = HTAA_composeAuth(hostname, portnumber, docname, auth_proxy)){
            sprintf(line, "%s\r\n", auth);
            StrAllocCat(command, line);
#ifndef DISABLE_TRACE
	    if (httpTrace) {
		if (auth_proxy) {
		    fprintf(stderr, "HTTP: Sending proxy authorization: %s\n",
			    auth);
		} else {
		    fprintf(stderr, "HTTP: Sending authorization: %s\n", auth);
		}
            }
#endif
        } else {
#ifndef DISABLE_TRACE
	    if (httpTrace) {
		if (auth_proxy) {
		    fprintf(stderr,
			    "HTTP: Not sending proxy authorization (yet).\n");
		} else {
		    fprintf(stderr, "HTTP: Not sending authorization (yet).\n");
		}
            }
#endif
	}
        FREE(abspath);
        FREE(hostname);
        FREE(docname);
      }
      auth_proxy = NO;
  }

#ifdef HAVE_SSL
  if (!do_connect && do_post && !do_put) {
#else
  if (do_post && !do_put) {
#endif
#ifndef DISABLE_TRACE
      if (httpTrace || www2Trace)
          fprintf(stderr, "HTTP: Doing post, content-type '%s'\n",
                  post_content_type);
#endif
      sprintf(line, "Content-type: %s\r\n",
              post_content_type ? post_content_type : "lose");
      StrAllocCat(command, line);
      {
        int content_length;

        if (!post_data) {
            content_length = 4;		/* 4 == "lose" :-) */
        } else {
            content_length = strlen(post_data);
	}
        sprintf(line, "Content-length: %d\r\n", content_length);
        StrAllocCat(command, line);
      }
      StrAllocCat(command, crlf);	/* Blank line means "end" */
      
      if (post_data) {
          StrAllocCat(command, post_data);
      } else {
          StrAllocCat(command, "lose");
      }
#ifdef HAVE_SSL
  } else if (!do_connect && do_post && do_put) {
#else
  } else if (do_post && do_put) {
#endif
      sprintf(line, "Content-length: %d\r\n\r\n", put_file_size);
      StrAllocCat(command, line);
  } else {
      StrAllocCat(command, crlf);	/* Blank line means "end" */
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "HTTP: Writing:\n%s----------------------------------\n",
              command);
#endif

  status = HTTP_NETWRITE(s, command, (int)strlen(command), handle);
  if (do_post && do_put) {
      char buf[BUFSIZ];
      int upcnt = 0;
      int n;

      while (status > 0) {
	  n = fread(buf, 1, BUFSIZ - 1, put_fp);

	  upcnt += status = HTTP_NETWRITE(s, buf, n, handle);
#ifndef DISABLE_TRACE
	  if (httpTrace || www2Trace)
	      fprintf(stderr, "[%d](%d) %s", status, n, buf);
#endif
	  if (feof(put_fp))
	      break;
      }
      if ((status < 0) || !feof(put_fp) || (upcnt != put_file_size)) {
	  sprintf(buf,
	    "Status: %d  --  EOF: %d  --  UpCnt/FileSize: %d/%d\n\nThe server you connected to either does not support\nthe PUT method, or an error occurred.\n\nYour upload was corrupted! Please try again!",
            status, feof(put_fp) ? 1 : 0, upcnt, put_file_size);
	  application_error(buf, "Upload Error!");
      }
  }

#ifndef DISABLE_TRACE
  if (httpTrace || www2Trace)
      fprintf(stderr, "%s", command);
#endif

  free(command);

  /* Twirl on each request to make things look nicer */
  if (HTCheckActiveIcon(1)) {
      HTTP_NETCLOSE(s, handle);
      status = HT_INTERRUPTED;
      HTProgress("Connection aborted.");
      goto done;
  }
  if (status <= 0) {
      if (status == 0) {
#ifndef DISABLE_TRACE
          if (httpTrace)
              fprintf(stderr, "HTTP: Got status 0 in initial write\n");
#endif
          /* Do nothing. */
      } else if 
#ifndef MULTINET
        ((errno == ENOTCONN || errno == ECONNRESET || errno == EPIPE) &&
#else
        ((socket_errno == ENOTCONN || socket_errno == ECONNRESET ||
          socket_errno == EPIPE) &&
#endif /* MULTINET, BSN */
         !already_retrying &&
	 /* Don't retry if we're posting. */
         !do_post) {

	  if (keepingalive) {
#ifndef DISABLE_TRACE
	      if (httpTrace)
		  fprintf(stderr,
			  "HTTP: Write error on Keep-Alive.  Retrying.\n");
#endif
 	      lsocket = -1;
 	      keepingalive = 0;
 	      HTTP_NETCLOSE(s, handle);
	      HTProgress("Server Error: Reconnecting");
	      goto try_again;
	  }

          /* Arrrrgh, HTTP 0/1 compatibility problem, maybe. */
#ifndef DISABLE_TRACE
          if (httpTrace)
              fprintf(stderr, "HTTP: Trying write again with HTTP0 request.\n");
#endif
          HTTP_NETCLOSE(s, handle);
          extensions = NO;
          already_retrying = 1;
          goto try_again;
      } else {
	  if (keepingalive) {
#ifndef DISABLE_TRACE
	      if (httpTrace)
		  fprintf(stderr,
			  "HTTP: Write timeout on Keep-Alive.  Retrying.\n");
#endif
 	      lsocket = -1;
 	      keepingalive = 0;
 	      HTTP_NETCLOSE(s, handle);
	      HTProgress("Server Timeout: Reconnecting");
	      goto try_again;
	  }
#ifndef DISABLE_TRACE
          if (httpTrace)
              fprintf(stderr,
	        "HTTP: Unexpected network WRITE error; aborting connection.\n");
#endif
          HTTP_NETCLOSE(s, handle);
          status = -1;
          HTProgress("Unexpected network write error; connection aborted.");
          goto done;
      }
  }
  
#ifndef DISABLE_TRACE
  if (httpTrace || www2Trace)
      fprintf(stderr, "HTTP: WRITE delivered OK\n");
#endif

#ifdef HAVE_SSl
  if (handle) {
      HTProgress("Done sending HTTPS request; waiting for response.");
  } else
#endif
      HTProgress("Done sending HTTP request; waiting for response.");

  /*	Read the first line of the response
   **	-----------------------------------
   */
  {
      /* Get numeric status, etc. */
      BOOL end_of_file = NO;
      int buffer_length = INIT_LINE_SIZE;
      char msgline[256];
    
      line_buffer = (char *) malloc(buffer_length * sizeof(char));
      do {
	  /* Loop to read in the first line */
	  /* Extend line buffer if necessary for those crazy WAIS URLs ;-) */
	  if (buffer_length - length < LINE_EXTEND_THRESH) {
	      buffer_length += buffer_length;
	      line_buffer = (char *) realloc(line_buffer,
					     buffer_length * sizeof(char));
          }
#ifndef DISABLE_TRACE
	  if (httpTrace)
	      fprintf(stderr, "HTTP: Trying to read %d\n",
		      buffer_length - length - 1);
#endif
	  status = HTTP_NETREAD(s, line_buffer + length,
			        buffer_length - length - 1, handle);
#ifndef DISABLE_TRACE
	  if (httpTrace)
	      fprintf(stderr, "\nHTTP: Read status %d\n", status);
#endif
	  if (status <= 0) {
	      /* Retry if we get nothing back too; 
	       * bomb out if we get nothing twice. */
	      if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
		  if (httpTrace)
		      fprintf(stderr, "HTTP: Interrupted initial read.\n");
#endif
		  HTProgress("Connection interrupted.");
		  HTTP_NETCLOSE(s, handle);
		  goto clean_up;
              } else {
		  if ((status < 0) &&
#ifndef MULTINET
		      (errno == ENOTCONN || errno == ECONNRESET ||
		       errno == EPIPE) &&
#else
		      (socket_errno == ENOTCONN || socket_errno == ECONNRESET ||
		       socket_errno == EPIPE) &&
#endif /* MULTINET, BSN, GEC */
		      !already_retrying && !do_post) {

		      if (keepingalive) {
#ifndef DISABLE_TRACE
			  if (httpTrace)
			      fprintf(stderr,
			             "HTTP: Error on Keep-Alive.  Retrying.\n");
#endif
			  lsocket = -1;
			  keepingalive = 0;
			  HTTP_NETCLOSE(s, handle);
	      		  HTProgress("Server Error: Reconnecting");
	      		  goto try_again;
	  	      }

		      /* Arrrrgh, HTTP 0/1 compatibility problem, maybe. */
#ifndef DISABLE_TRACE
		      if (httpTrace)
			  fprintf(stderr,
			          "HTTP: Trying again with HTTP0 request.\n");
#endif
		      HTTP_NETCLOSE(s, handle);
		      extensions = NO;
		      already_retrying = 1;
		      HTProgress("Retrying as HTTP0 request.");
		      goto try_again;
		  } else {
		      if (keepingalive) {
#ifndef DISABLE_TRACE
			  if (httpTrace)
			      fprintf(stderr,
				    "HTTP: Timeout on Keep-Alive. Retrying.\n");
#endif
 			  lsocket = -1;
 			  keepingalive = 0;
 			  HTTP_NETCLOSE(s, handle);
			  HTProgress("Server Timeout: Reconnecting");
			  goto try_again;
		      }
#ifndef DISABLE_TRACE
		      if (httpTrace)
			  fprintf(stderr,
			    "HTTP: read error; aborted connection; status %d\n",
			    status);
#endif
		      HTProgress(
			  "Unexpected network read error; connection aborted.");
		      HTTP_NETCLOSE(s, handle);
		      status = -1;
		      goto clean_up;
		  }
	      }
          }
	  bytes_already_read += status;

          sprintf(msgline, "Read %d bytes of data.", bytes_already_read);
	  HTProgress(msgline);
        
	  if (!status) {
	      end_of_file = YES;
	      break;
          }
	  line_buffer[length + status] = '\0';
        
	  if (line_buffer) {
	      if (line_kept_clean)
		  free(line_kept_clean);
	      line_kept_clean = (char *)malloc(buffer_length * sizeof(char));
	      memcpy(line_kept_clean, line_buffer, buffer_length);
          }
	  eol = strchr(line_buffer + length, LF);
	  /* Do we *really* want to do this? */
	  if (eol && (eol != line_buffer) && (*(eol - 1) == CR)) 
	      *(eol - 1) = ' '; 
        
	  length += status;
	  
	  /* Do we really want to do *this*? */
	  if (eol) 
	      *eol = '\0';		/* Terminate the line */
      /* All we need is the first line of the response.  If it's an HTTP/1.0
       * response, then the first line will be absurdly short and therefore
       * we can safely gate the number of bytes read through this code
       * (as opposed to below) to ~1000.
       * Well, let's try 100.
       */
      } while (!eol && !end_of_file && (bytes_already_read < 100));
  }  /* Scope of loop variables */

  /* Save total length, in case we decide later to show it all */
  rawlength = length;
    
  /*	We now have a terminated unfolded line.  Parse it.
   **	--------------------------------------------------
   */
  {
    char server_version[VERSION_LENGTH + 1];
    int fields, server_status;

    statusError = 0;
    server_version[0] = '\0';
    
    fields = sscanf(line_buffer, "%20s %d", server_version, &server_status);
    
#ifndef DISABLE_TRACE
    if (httpTrace || www2Trace) {
        fprintf(stderr, "HTTP: Rx: line_buffer '%s'\n", line_buffer);
        fprintf(stderr, "HTTP: Scanned %d fields from line_buffer\n", fields);
    }
#endif
    
    /* Rule out HTTP/1.0 reply as best we can. */
    if (fields < 2 || strncmp(server_version, "HTTP/", 5) ||
        server_version[6] != '.') {	
        HTAtom *encoding;

#ifndef DISABLE_TRACE
        if (httpTrace)
            fprintf(stderr, "--- Talking HTTP0.\n");
#endif
        format_in = HTFileFormat(url, &encoding, WWW_HTML, &compressed);
        start_of_data = line_kept_clean;
	done_length = length;
    } else {
        /* Decode full HTTP response */
	format_in = mime_in;
        /* We set start_of_data to "" when !eol here because there
         * will be a put_block done below; we do *not* use the value
         * of start_of_data (as a pointer) in the computation of
         * length or anything else in this situation. */
        start_of_data = eol ? eol + 1 : "";
        done_length = length = eol ? length - (start_of_data - line_buffer) : 0;
        
#ifndef DISABLE_TRACE
        if (httpTrace)
            fprintf(stderr, "--- Talking HTTP1.\n");
#endif
        switch (server_status / 100) {
	  case 1:
	    /*
	    **	HTTP/1.1 Informational statuses.
	    **	100 Continue.
	    **	101 Switching Protocols.
	    **	> 101 is unknown.
	    **	We should never get these, and they have only
	    **	the status line and possibly other headers,
	    **	so we'll deal with them by showing the full
	    **	header to the user as text/plain. - FM
	    */
	    HTProgress("Got unexpected Informational Status.");
	    do_head = 1;
	    break;

          case 2:		/* Good: Got MIME object */
	    if (do_head) {
		if (server_status == 200)
		    retried_with_new_nonce = 0;  /* Reset the toggle value */
		break;
	    }
            switch (server_status) {
              case 204:
                return_nothing = 1;
                format_in = html_in;
                break;
	      case 200:
		retried_with_new_nonce = 0;  /* Reset the toggle value */
              default:
		/*
		 *  200 OK.
		 *  201 Created.
		 *  202 Accepted.
		 *  203 Non-Authoritative Information.
		 *  > 206 is unknown.
		 *  All should return something to display.
		 */
#ifdef HAVE_SSL
	        if (do_connect) {
#ifndef DISABLE_TRACE
		    if (httpTrace)
		        fprintf(stderr,
			        "HTTP: Proxy tunnel to '%s' established.\n",
			        connect_host);
#endif
		    do_connect = FALSE;
		    url = connect_url;
		    FREE(line_buffer);
		    FREE(line_kept_clean);
		    did_connect = TRUE;
		    already_retrying = TRUE;
		    eol = 0;
		    bytes_already_read = 0;
		    had_header = NO;
		    length = 0;
		    target = NULL;
#ifndef DISABLE_TRACE
		    if (httpTrace)
		        fprintf(stderr,
			"      Will attempt handshake and resubmit headers.\n");
#endif
		    goto use_tunnel;
		}
#else
		break;
#endif
            }
            break;
            
          case 3:
	    /*
	    **	Various forms of Redirection.
	    **	300 Multiple Choices.
	    **	301 Moved Permanently.
	    **	302 Found (temporary; we can, and do, use GET).
	    **	303 See Other (temporary; always use GET).
	    **	304 Not Modified.
	    **	305 Use Proxy.
	    **	306 Set Proxy.
	    **	307 Temporary Redirect with method retained.
	    **	> 308 is unknown.
	    */
	    if (do_head)
		/* Just show it to user */
		break;

	    /* Redirect unless Multiple Choices or other bad status */
	    if ((server_status != 300) &&
		(server_status != 304) && (server_status != 305) &&
		(server_status != 306) && (server_status < 308)) {
	        /*
	         * We do not load the file, but read the headers for
	         * the "Location:", check out that redirecting_url
	         * and if it's acceptable (e.g. not a telnet URL
	         * when we have that disabled), initiate a new fetch.
	         * If that's another redirecting_url, we'll repeat the
	         * checks, and fetch initiations if acceptable, until
	         * we reach the actual URL, or the redirection limit
	         * set in HTAccess.c is exceeded.  If the status was 301
	         * indicating that the relocation is permanent, we set
	         * the permanent_redirection flag to make it permanent
	         * for the current anchor tree (i.e., will persist until
	         * the tree is freed or the client exits).  If the
	         * redirection would include POST content, we seek
	         * confirmation from an interactive user, with option to
	         * use 303 for 301 (but not for 307), and otherwise refuse
	         * the redirection.  We also don't allow permanent
	         * redirection if we keep POST content.  If we don't find
	         * the Location header or its value is zero-length, we
	         * display whatever the server returned, and the user
	         * should RELOAD that to try again, or make a selection
	         * from it if it contains links, or Left-Arrow to the
	         * previous document. - FM
	         */
	        char *cp;
		int finish_read = 1;

		/* Work around hung socket problem */
		if (strstr(line_kept_clean, "Server: Microsoft-IIS/4.0")) {
		    broken_crap_hack = 1;
		} else {
		    broken_crap_hack = 2;
		}
		cp = line_kept_clean;
		while (*cp) {
		    /*
		    **  Assume we are done reading if a CRLF pair terminates
		    **  the header section.  Prevents short hangs caused by
		    **  servers stupidly doing keepalive on redirects.
		    */
		    if (*cp == CR) {
		        if ((*(cp + 1) == LF) && (*(cp + 2) == CR) &&
			    (*(cp + 3) == LF)) {
		            finish_read = 0;
		            break;
		        }
		    }
		    cp++;
		}
		/*
		 *  Get the rest of the headers and data, if
		 *  any, and then close the connection.
		 */
		while (finish_read && (status = HTTP_NETREAD(s, line_buffer,
					   (INIT_LINE_SIZE - 1), handle)) > 0) {
		    line_buffer[status] = '\0';
		    StrAllocCat(line_kept_clean, line_buffer);

		    /* Broken server check */
		    cp = line_buffer;
		    while (*cp) {
			if (*cp == CR) {
			    if ((*(cp + 1) == LF) && (*(cp + 2) == CR) &&
				(*(cp + 3) == LF)) {
		 	        finish_read = 0;
				break;
		     	    }
		    	}
		    	cp++;
		    }
		}
		HTTP_NETCLOSE(s, handle);
		broken_crap_hack = 0;
		if (status == HT_INTERRUPTED) {
		    /*
		     *  Impatient user.
		     */
#ifndef DISABLE_TRACE
		    if (httpTrace)
		        fprintf(stderr, "HTTP: Interrupted followup read.\n");
#endif
		    HTProgress("Connection interrupted.");
		    goto clean_up;
		}
	        /*
	        **  Look for "Set-Cookie:" and "Set-Cookie2:" headers.
	        */
	        if (HTSetCookies == TRUE) {
		  char *value = NULL;
		  char *SetCookie = NULL;
		  char *SetCookie2 = NULL;

		  cp = line_kept_clean;
		  while (*cp) {
		      /*
		      **  Assume a CRLF pair terminates
		      **  the header section. - FM
		      */
		      if (*cp == CR) {
			  if ((*(cp + 1) == LF) &&
			      (*(cp + 2) == CR) && (*(cp + 3) == LF))
			      break;
		      }
		      if (tolower(*cp) != 's') {
			  cp++;
		      } else if (!my_strncasecmp(cp, "Set-Cookie:", 11)) {
			  char *cp1;
			  char *cp2 = NULL;

			  cp += 11;
 Cookie_continuation:
			  /*
			   *  Trim leading spaces. - FM
			   */
			  while (isspace((unsigned char)*cp))
			      cp++;
			  /*
			  **  Accept CRLF, LF, or CR as end of line. - FM
			  */
			  if ((cp1 = strchr(cp, LF)) ||
			      (cp2 = strchr(cp, CR))) {
			      if (*cp1) {
				  *cp1 = '\0';
				  if (cp2 = strchr(cp, CR))
				      *cp2 = '\0';
			      } else {
				  *cp2 = '\0';
			      }
			  }
			  if (!*cp) {
			      if (cp1)
				  *cp1 = LF;
			      if (cp2)
				  *cp2 = CR;
			      if (value) {
				  TrimDoubleQuotes(value);
				  if (SetCookie)
				      StrAllocCat(SetCookie, ", ");
				  StrAllocCat(SetCookie, value);
				  free(value);
			      }
			      break;
			  }
			  StrAllocCat(value, cp);
			  cp += strlen(cp);
			  if (cp1) {
			      *cp1 = LF;
			      cp1 = NULL;
			  }
			  if (cp2) {
			      *cp2 = CR;
			      cp2 = NULL;
			  }
			  cp1 = cp;
			  if (*cp1 == CR)
			     cp1++;
			  if (*cp1 == LF)
			     cp1++;
			  if ((*cp1 == ' ') || (*cp1 == '\t')) {
			      StrAllocCat(value, " ");
			      cp = cp1;
			      cp++;
			      cp1 = NULL;
			      goto Cookie_continuation;
			  }
			  TrimDoubleQuotes(value);
			  if (SetCookie)
			      StrAllocCat(SetCookie, ", ");
			  StrAllocCat(SetCookie, value);
			  FREE(value);
		      } else if (!my_strncasecmp(cp, "Set-Cookie2:", 12))  {
			  char *cp1;
			  char *cp2 = NULL;

			  cp += 12;
 Cookie2_continuation:
			  /*
			   *  Trim leading spaces. - FM
			   */
			  while (isspace((unsigned char)*cp))
			      cp++;
			  /*
			  **  Accept CRLF, LF, or CR as end of line. - FM
			  */
			  if ((cp1 = strchr(cp, LF)) ||
			      (cp2 = strchr(cp, CR))) {
			      if (*cp1) {
				  *cp1 = '\0';
				  if (cp2 = strchr(cp, CR))
				      *cp2 = '\0';
			      } else {
				  *cp2 = '\0';
			      }
			  }
			  if (!*cp) {
			      if (cp1)
				  *cp1 = LF;
			      if (cp2)
				  *cp2 = CR;
			      if (value) {
				  TrimDoubleQuotes(value);
				  if (SetCookie2)
				      StrAllocCat(SetCookie2, ", ");
				  StrAllocCat(SetCookie2, value);
				  FREE(value);
			      }
			      break;
			  }
			  StrAllocCat(value, cp);
			  cp += strlen(cp);
			  if (cp1) {
			      *cp1 = LF;
			      cp1 = NULL;
			  }
			  if (cp2) {
			      *cp2 = CR;
			      cp2 = NULL;
			  }
			  cp1 = cp;
			  if (*cp1 == CR)
			     cp1++;
			  if (*cp1 == LF)
			     cp1++;
			  if ((*cp1 == ' ') || (*cp1 == '\t')) {
			      StrAllocCat(value, " ");
			      cp = cp1;
			      cp++;
			      cp1 = NULL;
			      goto Cookie2_continuation;
			  }
			  TrimDoubleQuotes(value);
			  if (SetCookie2)
			      StrAllocCat(SetCookie2, ", ");
			  StrAllocCat(SetCookie2, value);
			  FREE(value);
		      } else {
			  cp++;
		      }
		  }
		  FREE(value);
		  if (SetCookie || SetCookie2) {
		      HTSetCookie(SetCookie, SetCookie2, anAnchor->address);
		      FREE(SetCookie);
		      FREE(SetCookie2);
		  }
	        }
	        /*
	         *  Look for "Location:" in the headers.
		 *  Ignore "Content-Location:
	         */
	        cp = line_kept_clean;
	        while (*cp) {
		  if (tolower(*cp) != 'l') {
		    cp++;
		  } else if (!my_strncasecmp(cp, "Location:", 9) &&
			     (*(cp - 1) != '-')) {
		    char *cp1;
		    char *cp2 = NULL;

		    cp += 9;
		    /*
		     *	Trim leading spaces.
		     */
		    while (isspace((unsigned char)*cp))
			cp++;
		    /*
		     *	Accept CRLF, LF, or CR as end of header.
		     */
		    if ((cp1 = strchr(cp, LF)) || (cp2 = strchr(cp, CR))) {
			if (*cp1) {
			    *cp1 = '\0';
			    if (cp2 = strchr(cp, CR))
				*cp2 = '\0';
			} else {
			    *cp2 = '\0';
			}
			/*
			 *  Load the new URL into redirecting_url,
			 *  and make sure it's not zero-length.
			 */
			StrAllocCopy(redirecting_url, cp);
			TrimDoubleQuotes(redirecting_url);
			if (!*redirecting_url) {
			    /*
			     *	The "Location:" value is zero-length, and
			     *	thus is probably something in the body, so
			     *	we'll show the user what was returned.
			     */
#ifndef DISABLE_TRACE
			    if (httpTrace)
				fprintf(stderr,
				        "HTTP: 'Location:' is zero-length!\n");
#endif
			    if (cp1)
				*cp1 = LF;
			    if (cp2)
				*cp2 = CR;
			    bad_location = 1;
			    free(redirecting_url);
			    redirecting_url = NULL;
			    HTProgress(
			         "Got redirection with a bad Location header.");
			    break;
			} else if (!strcmp(redirecting_url, url)) {
			    bad_location = 2;
			    free(redirecting_url);
			    redirecting_url = NULL;
			    HTProgress("Got redirection with duplicate URL.");
			    break;
			}
#ifndef DISABLE_TRACE
			if (httpTrace)
			    fprintf(stderr, "HTTP: Picked up location '%s'\n",
				    redirecting_url);
#endif
			if (cp1)
			    *cp1 = LF;
			if (cp2)
			    *cp2 = CR;
#if 0
			if (server_status == 305) {  /* Use Proxy */
			    /*
			     *	Make sure the proxy field ends with
			     *	a slash.
			     */
			    if (redirecting_url[strlen(redirecting_url) - 1] !=
				'/')
				StrAllocCat(redirecting_url, "/");
			    /*
			     *	Append our URL.
			     */
			    StrAllocCat(redirecting_url, anAnchor->address);
#ifndef DISABLE_TRACE
			    if (httpTrace)
			        fprintf(stderr, "HTTP: Proxy URL is '%s'\n",
					redirecting_url);
#endif
			}
#endif
			status = HT_REDIRECTING;
			goto clean_up;
		    }
		    break;
		  } else {
		    /*
		     *	Keep looking for the Location header.
		     */
		    cp++;
		  }
	        }
	        /*
	         *  If we get to here, we didn't find the Location
	         *  header, so we'll show the user what we got, if
	         *  anything.
	         */
#ifndef DISABLE_TRACE
		if (www2Trace || httpTrace)
		    fprintf(stderr, "HTTP: Failed to pick up location.\n");
#endif
		if (bad_location != 2) {
	            start_of_data = line_kept_clean;
	            length = rawlength;
		}
	        if (!bad_location)
		    HTProgress("Got redirection with no Location header.");
	    }
            statusError = 1;
            break;
            
          case 4:		/* "I think I goofed" */
            switch (server_status) {
              case 401:
                /* length -= start_of_data - text_buffer; */
                if (HTAA_shouldRetryWithAuth(start_of_data, length, s, NO)) {
                    HTTP_NETCLOSE(s, handle);
		    lsocket = -1;
#ifndef DISABLE_TRACE
                    if (httpTrace) 
                        fprintf(stderr, "HTTP: close socket %d %s\n", s,
                                "to retry with Access Authorization");
#endif
                    HTProgress(
			     "Retrying with access authorization information.");
#ifdef HAVE_SSL
		    if ((using_proxy || using_gateway) &&
			!strncmp(url, "https://", 8)) {
			url = arg;
			do_connect = TRUE;
			did_connect = FALSE;
		    }
#endif
                    goto try_again;
                } else {
		    statusError = 1;
                    break;
                }

              case 403:
		statusError = 1;
                /* 403 is "forbidden"; display returned text. */
                break;

	      case 404:
		HTProgress("Not Found");
		statusError = 1;

		/* Work around hung socket problem */
		broken_crap_hack = 2;

                /* 404 is "Not Found"; display returned text. */
                break;

	      case 407:
		/*
		 *  Authorization for proxy server required.
		 *  If we are not in fact using a proxy, proceed
		 *  to showing the 407 body.  Otherwise, if we can
		 *  set up authorization based on the Proxy-Authenticate
		 *  header, and the user provides a username and
		 *  password, try again.
		 */
		if (!using_proxy && !using_gateway) {
 		    statusError = 1;
#ifndef DISABLE_TRACE
                    if (httpTrace) 
                        fprintf(stderr,
				"HTTP: Got 407 but not using proxy\n");
#endif
		    break;
		}
		if (HTAA_shouldRetryWithAuth(start_of_data, length, s, YES)) {
		    HTTP_NETCLOSE(s, handle);
		    lsocket = -1;
#ifndef DISABLE_TRACE
                    if (httpTrace) 
                        fprintf(stderr, "%s %d %s\n", "HTTP: close socket", s,
                                "to retry (407) with Access Authorization");
#endif
		    HTProgress(
			      "Retrying with proxy authorization information.");
		    goto try_again;
                } else {
#ifndef DISABLE_TRACE
                    if (httpTrace) 
                        fprintf(stderr, "HTTP: Got 407 but should not retry\n");
#endif
		    statusError = 1;
		    break;
                }

	      case 408:
		/*
		 *  Request Timeout.  Show the status message
		 *  and restore the current document.
		 */
		HTProgress("Request Timeout");
		statusError = 1;
                break;

              default:
		/*
		 *  400 Bad Request.
		 *  402 Payment Required.
		 *  403 Forbidden.
		 *  404 Not Found.
		 *  405 Method Not Allowed.
		 *  406 Not Acceptable.
		 *  409 Conflict.
		 *  410 Gone.
		 *  411 Length Required.
		 *  412 Precondition Failed.
		 *  413 Request Entity Too Large.
		 *  414 Request-URI Too Long.
		 *  415 Unsupported Media Type.
		 *  416 List Response (for content negotiation).
		 *  > 416 is unknown.
		 */
		statusError = 1;
                break;
            }  /* case 4 switch */
            break;

          case 5:		/* I think you goofed */
	    statusError = 1;
            break;
            
          default:		/* Bad number */
	    statusError = 1;
            HTAlert("Unknown status reply from server!");
            break;
        }  /* Switch on server_status/100 */
      }	 /* Full HTTP reply */
  }  /* Scope of fields */

  if (do_head) {
      start_of_data = line_kept_clean;
      length = rawlength;
      format_in = plain_in;
  }

  /* Set up the stream stack to handle the body of the message */
  target = HTStreamStack(format_in, format_out, compressed, sink, anAnchor);

  if (!target) {
      char buffer[1024];

      sprintf(buffer, "Sorry, no known way of converting %s to %s.",
              HTAtom_name(format_in), HTAtom_name(format_out));
      HTProgress(buffer);
      status = -1;
      HTTP_NETCLOSE(s, handle);
      lsocket = -1;
      goto clean_up;
  }

  if (!return_nothing) {
#ifndef DISABLE_TRACE
      if (www2Trace || httpTrace)
          fprintf(stderr, "HTTP: Doing put_block, '%s'\n", start_of_data);
#endif
      /* Check for Keep-Alive */
      for (p = start_of_data; *p; p++) {
	  if ((*p == 'C') && !strncmp("Connection:", p, 11)) {
	      char *p2;

#ifndef DISABLE_TRACE
	      if (httpTrace)
		  fprintf(stderr, "HTTP: Server Sent Connection:\n");
#endif
	      for (p2 = p + 11; *p2; p2++) {
		  if (!my_strncasecmp("Keep-Alive", p2, 10)) {
		      lsocket = s;
		      break;
		  }
		  if (!my_strncasecmp("close", p2, 5)) {
#ifndef DISABLE_TRACE
		      if (httpTrace)
			  fprintf(stderr, "HTTP: Server Closes Connection\n");
#endif
		      break;
		  }
		  /* Check if found neither Keep-Alive or close */
		  if (*p2 == ':')
		      break;
	      }
	      break;
	  } else if ((*p == 'K') && !strncmp("Keep-Alive:", p, 11)) {
#ifndef DISABLE_TRACE
	      if (httpTrace)
		  fprintf(stderr, "HTTP: Server Sent Keep-Alive:\n");
#endif
	      lsocket = s;
	      break;
	  }
      }

#ifndef DISABLE_TRACE
      if (httpTrace && (lsocket == -1))
	  fprintf(stderr, "HTTP: Server does not agree to Keep-Alive\n");
#endif
      /* Recycle the first chunk of data, in all cases. */
      (*target->isa->put_block)(target, start_of_data, length);
      
      if (HTMultiLoading) {
	  HTMultiLoading->stream = (void *)target;
	  HTMultiLoading->socket = s;
	  HTMultiLoading->handle = (void *)handle;
	  HTMultiLoading->length = done_length;
	  status = HT_LOADED;
	  lsocket = -1;
	  /* Did we already read it all? */
	  if (target->isa == &HTMIME) {
	      int content_length = HTMIME_get_content_length(target);

      	      if ((content_length != -1) &&
		  (((HTMIME_get_header_length(target) + content_length) -
		    done_length) < 1)) {
		  HTMultiLoading->loaded = 1;
		  (*target->isa->end_document)(target);
		  HTTP_NETCLOSE(s, handle);
		  (*target->isa->free)(target);
	      }
	  }
          goto clean_up;
      }
      /* Go pull the bulk of the data down.
       * If we don't use length, header length is wrong due to the 
       * discarded first line.  HTCopy will get any loading length. */
      rv = HTCopy(s, target, done_length, handle, -1);
      if (rv < 0) {
	  if (rv == -1) {
	      (*target->isa->handle_interrupt)(target);
	      HTTP_NETCLOSE(s, handle);
	      (*target->isa->free)(target);
	      status = HT_INTERRUPTED;
	      lsocket = -1;
	      goto clean_up;
	  }
	  if (rv == -2 && !already_retrying && !do_post) {
	      (*target->isa->handle_interrupt)(target);
	      HTTP_NETCLOSE(s, handle);
	      (*target->isa->free)(target);
#ifndef DISABLE_TRACE
	      if (httpTrace)
		  fprintf(stderr, "HTTP: Trying again with HTTP0 request.\n");
#endif
	      extensions = NO;
	      already_retrying = 1;
	      HTProgress("Retrying as HTTP0 request.");
	      goto try_again;
	  }
      }
  } else {
      /* return_nothing is high. */
      (*target->isa->put_string)(target, "<mosaic-access-override>\n");
      HTProgress("No content was returned.");
      if (HTMultiLoading)
	  HTMultiLoading->loaded = 1;
  }

  (*target->isa->end_document)(target);

  /* Close socket before doing free. */
  if (lsocket == -1) {
      HTTP_NETCLOSE(s, handle);
#ifndef DISABLE_TRACE
      if (httpTrace)
	  fprintf(stderr, "HTTP: Closing connection\n");
#endif
  } else {
      HTProgress("Leaving Server Connection Open");
#ifndef DISABLE_TRACE
      if (httpTrace)
	  fprintf(stderr, "HTTP: Keeping connection alive\n");
#endif
  }

  (*target->isa->free)(target);

  status = HT_LOADED;

  /*	Clean up
   */
 clean_up: 
  FREE(line_buffer);
  FREE(line_kept_clean);

 done:
  /* Clear out on exit */
  do_post = do_head = 0;

  if (statusError) {
      securityType = HTAA_NONE;
#ifndef DISABLE_TRACE
      if (httpTrace)
	  fprintf(stderr, "Resetting security type to NONE.\n");
#endif
  }
#ifdef HAVE_SSL
  do_connect = FALSE;
  did_connect = FALSE;
  FREE(connect_host);
  if (lsocket == -1)
      keepalive_handle = NULL;
#endif

  return status;
}


/*	Protocol descriptor
*/
PUBLIC HTProtocol HTTP = { "http", HTLoadHTTP, NULL };
PUBLIC HTProtocol HTTPS = { "https", HTLoadHTTP, NULL };
