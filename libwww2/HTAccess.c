/*		Access Manager					HTAccess.c
**		==============
**
** Authors
**	TBL	Tim Berners-Lee timbl@info.cern.ch
**	JFG	Jean-Francois Groff jfg@dxcern.cern.ch
**	DD	Denis DeLaRoca (310) 825-4580  <CSP1DWD@mvs.oac.ucla.edu>
** History
**       8 Jun 92 Telnet hopping prohibited as telnet is not secure TBL
**	26 Jun 92 When over DECnet, suppressed FTP, Gopher and News. JFG
**	 6 Oct 92 Moved HTClientHost and logfile into here. TBL
**	17 Dec 92 Tn3270 added, bug fix. DD
**	 4 Feb 93 Access registration, Search escapes bad chars TBL
**		  PARAMETERS TO HTSEARCH AND HTLOADRELATIVE CHANGED
**	28 May 93 WAIS gateway explicit if no WAIS library linked in.
**
** Bugs
**	This module assumes that that the graphic object is hypertext, as it
**	needs to select it when it has been loaded.  A superclass needs to be
**	defined which accepts select and select_anchor.
*/

#include "../config.h"
#ifndef DEFAULT_WAIS_GATEWAY
#define DEFAULT_WAIS_GATEWAY "http://www.ncsa.uiuc.edu:8001/"
#endif

#include "HTAccess.h"
#include "HTParse.h"
#include "HTUtils.h"
#include "HTML.h"

#include <stdio.h>

#include "HTList.h"
#include "HText.h"	/* See bugs above */
#include "HTAlert.h"
#include "HTMime.h"
#include "HTTP.h"

#include "../libnut/str-tools.h"
#include "../src/proxy.h"
#include "HTMultiLoad.h"

#ifdef HAVE_SSL
#include <openssl/ssl.h>
#endif

#ifndef DISABLE_TRACE
extern int www2Trace;
extern int httpTrace;
#endif

struct _HTStream {
    HTStreamClass *isa;
};

/* In gui.c */
extern char *mo_check_for_proxy(char *);
/* In mo-www.c */
extern char *mo_url_canonicalize_keep_anchor(char *, char *);
/* Also used in HTTP.C */
char *redirecting_url = NULL;

char *currentURL = NULL;

/* In htmime.c */
extern MIMEInfo MIME_http;

/*	These may be set to modify the operation of this module
*/
PUBLIC char *HTClientHost = NULL;    /* Name of remote login host if any */
PUBLIC int HTMultiLoadLimit = 8;

/*	To generate other things, play with these
*/
PRIVATE HTFormat HTOutputFormat = NULL;
PRIVATE HTStream *HTOutputStream = NULL;  /* For non-interactive, set this */ 

PUBLIC BOOL using_gateway = NO;      /* Are we using a gateway? */
PUBLIC BOOL using_proxy = NO;        /* Are we using a proxy gateway? */
PUBLIC char *proxy_host_fix = NULL;  /* Host: header fix */

PRIVATE HTList *protocols = NULL;  /* List of registered protocol descriptors */

/* Multiple image stuff */
PUBLIC MultiInfo *HTMultiLoading = NULL;
PRIVATE MultiInfo *multi_loading = NULL;
PRIVATE int multi_count = 0;
PRIVATE HTBTree *multi_more = NULL;
extern int HTCopyOneRead; 
extern int force_dump_to_file;
extern char *force_dump_filename;

/*	Register a Protocol				HTRegisterProtocol
**	-------------------
*/

PUBLIC BOOL HTRegisterProtocol(HTProtocol *protocol)
{
    if (!protocols)
	protocols = HTList_new();
    HTList_addObject(protocols, protocol);
    return YES;
}

#if defined(VAXC) || defined(__GNUC__)
/*
 * Dummy routine for VMS with VAXC.  Call routines so that the modules
 * containing the various HTProtocol definitions are loaded.  An extern
 * declaration seems not to be enough do to this with VAXC.
 */
PUBLIC void SILLY_VAXC()
{
    HTLoadHTTP();
    HTLoadFile();
    HTLoadTelnet();
    HTLoadFinger();
    HTGetNewsHost();
    HTLoadGopher();
#ifdef HAVE_WAIS
    HTLoadWAIS();
#endif /* WAIS */
    HTSendMailTo();
}
#endif /* VAXC, BSN */


/*	Register all known protocols
**	----------------------------
**
**	Add to or subtract from this list if you add or remove protocol modules.
**	This routine is called the first time the protocol list is needed,
**	unless any protocols are already registered, in which case it is not
**	called.  Therefore the application can override this list.
*/
PRIVATE void HTAccessInit (void)			/* Call me once */
{
    extern HTProtocol HTTP, HTFile, HTFinger, HTTelnet, HTTn3270, HTRlogin;
    extern HTProtocol HTFTP, HTNews, HTGopher, HTMailto, HTNNTP;
    extern HTProtocol HTMosaicCookies, HTTPS;
#ifdef HAVE_WAIS
    extern HTProtocol HTWAIS;
#endif

    /* Most popular first so search is faster in most cases */
    HTRegisterProtocol(&HTTP);
    HTRegisterProtocol(&HTTPS);
    HTRegisterProtocol(&HTFTP);
    HTRegisterProtocol(&HTNews);
    HTRegisterProtocol(&HTMosaicCookies);
    HTRegisterProtocol(&HTFile);
    HTRegisterProtocol(&HTGopher);
    HTRegisterProtocol(&HTFinger);
    HTRegisterProtocol(&HTTelnet);
    HTRegisterProtocol(&HTTn3270);
    HTRegisterProtocol(&HTRlogin);
    HTRegisterProtocol(&HTMailto);
    HTRegisterProtocol(&HTNNTP);
#ifdef HAVE_WAIS
    HTRegisterProtocol(&HTWAIS);
#endif
}


/*		Find physical name and access protocol
**		--------------------------------------
**
**
** On entry,
**	addr		must point to the fully qualified hypertext reference.
**	anchor		a parent anchor whose address is addr
**
** On exit,
**	returns		HT_NO_ACCESS		Error has occured.
**			HT_OK			Success
**
*/
PRIVATE int get_physical (char *addr, HTParentAnchor *anchor, int bong)
{
    char *access = HTParse(addr, "file:", PARSE_ACCESS);
    char *host = HTParse(addr, "", PARSE_HOST);
    extern int useKeepAlive;
    
    HTAnchor_setPhysical(anchor, addr);

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "get_physical: addr = %s, host = %s, access = %s\n",
		addr, host, access);
#endif
    /*
    ** Set useKeepAlive to the default here because the last pass might
    ** have turned this off, and the default might have been on.
    */
    useKeepAlive = 1;

    /* Check whether gateway access has been set up for this */

    /* Make sure the using_proxy variable is false */
    using_proxy = NO;
    if (proxy_host_fix) {
	free(proxy_host_fix);
	proxy_host_fix = NULL;
    }

    {
	char *tmp_access = strdup(access);
	char *tmp_host = strdup(host);
	char *ptr;

	for (ptr = tmp_access; *ptr; ptr++)
	    *ptr = tolower(*ptr);
	for (ptr = tmp_host; *ptr; ptr++)
	    *ptr = tolower(*ptr);

	if (!GetNoProxy(tmp_access, tmp_host)) {
		char *gateway_parameter, *gateway, *proxy;
		struct Proxy *proxent = NULL;
		extern struct Proxy *proxy_list;
		char *proxyentry = NULL;

		proxy_host_fix = strdup(tmp_host);

		/* Search for gateways */
		gateway_parameter = (char *)malloc(strlen(tmp_access) + 20);
		if (!gateway_parameter)
		        outofmem(__FILE__, "HTLoad");
		strcpy(gateway_parameter, "WWW_");
		strcat(gateway_parameter, tmp_access);
		strcat(gateway_parameter, "_GATEWAY");
		/* Return value of getenv not freeable */
		gateway = (char *)getenv(gateway_parameter);

		/* Search for proxy servers */
		strcpy(gateway_parameter, tmp_access);
		strcat(gateway_parameter, "_proxy");
		proxy = (char *)getenv(gateway_parameter);
		free(gateway_parameter);
		/*
		 * Check the proxies list
		 */
		if (!proxy || !*proxy) {
			int fMatchEnd = 1;  /* Match hosts from the end */
			char *scheme_info = strdup(host);

			if (scheme_info && !*scheme_info) {
				free(scheme_info);
				scheme_info = HTParse(addr, "", PARSE_PATH);
				/* Match other scheme_info at beginning */
				fMatchEnd = 0;
			}
			if (bong) {  /* This one is bad - disable! */
#ifndef DISABLE_TRACE
				if (www2Trace)
		 			fprintf(stderr, 
						"Disabling proxy, bong = %d\n",
						bong);
#endif
				proxent = GetProxy(tmp_access, scheme_info,
					           fMatchEnd);
				if (proxent)
					proxent->alive = bong;
			}
			proxent = GetProxy(tmp_access, scheme_info, fMatchEnd);
			if (scheme_info)
				free(scheme_info);
			if (proxent) {
				useKeepAlive = 0;  /* Proxies don't keepalive */
				StrAllocCopy(proxyentry, proxent->transport);
				StrAllocCat(proxyentry, "://");
				StrAllocCat(proxyentry, proxent->address);
				StrAllocCat(proxyentry, ":");
				StrAllocCat(proxyentry, proxent->port);
				StrAllocCat(proxyentry, "/");
				proxy = proxyentry;
			}
		}
#ifndef DISABLE_TRACE
		if (www2Trace && proxy)
 			fprintf(stderr,	"Got proxy %s\n", proxy);
#endif
#ifndef HAVE_WAIS
		if (!gateway && !strcmp(tmp_access, "wais"))
			gateway = DEFAULT_WAIS_GATEWAY;
#endif
		/* Proxy servers have precedence over gateway servers */
		if (proxy) {
			char *gatewayed = NULL;

			StrAllocCopy(gatewayed, proxy);
			StrAllocCat(gatewayed, addr);
			using_proxy = YES;
			HTAnchor_setPhysical(anchor, gatewayed);
			free(access);
			if (proxyentry)
				free(proxyentry);
			access = HTParse(gatewayed, "http:", PARSE_ACCESS);
			free(gatewayed);
		} else if (gateway) {
			char *gatewayed = NULL;

			StrAllocCopy(gatewayed, gateway);
			StrAllocCat(gatewayed, addr);
			using_gateway = YES;
			HTAnchor_setPhysical(anchor, gatewayed);
			free(access);
			access = HTParse(gatewayed, "http:", PARSE_ACCESS);
			free(gatewayed);
		} else {
			if (proxy_host_fix) {
				free(proxy_host_fix);
				proxy_host_fix = NULL;
			}
			using_proxy = NO;
			using_gateway = NO;
			ClearTempBongedProxies();
		}
	}
	free(tmp_access);
	free(tmp_host);
    }
    free(host);

    /*	Search registered protocols to find suitable one */
    {
	int i, n;

        if (!protocols)
	    HTAccessInit();
	n = HTList_count(protocols);
	for (i = 0; i < n; i++) {
	    HTProtocol *p = HTList_objectAt(protocols, i);

            if (p->name && !my_strcasecmp(p->name, access)) {
		HTAnchor_setProtocol(anchor, p);
		free(access);
		return (HT_OK);
	    }
	}
    }
    free(access);
    return HT_NO_ACCESS;
}


/*		Load a document
**		---------------
**
**	This is an internal routine, which has an address AND a matching
**	anchor.  (The public routines are called with one OR the other.)
**
** On entry,
**	addr		must point to the fully qualified hypertext reference.
**	anchor		a pareent anchor with whose address is addr
**
** On exit,
**	returns		<0		Error has occured.
**			HT_LOADED	Success
**			HT_NO_DATA	Success, but no document loaded.
**					(telnet sesssion started etc)
**
*/
PRIVATE int HTLoad (WWW_CONST char *addr,
		    HTParentAnchor *anchor,
		    HTFormat format_out,
		    HTStream *sink)
{
    HTProtocol *p;
    int ret;
    int fallbacks = -1;
    int status = get_physical(addr, anchor, 0);
    int retry = 5;
    static char *buf1 = "Do you want to disable the proxy server:\n\n";
    static char *buf2 = "\n\nAlready attempted 5 contacts.";
    char *finbuf, *host;

    if (!(p = HTAnchor_protocol(anchor)))
	return(HT_NOT_LOADED);

    while (1) {
    	if (status < 0)
	    return status;	/* Can't resolve or forbidden */
	retry = 5;

 retry_proxy:
    	ret = (*p->load)(HTAnchor_physical(anchor), anchor, format_out, sink);

	if ((ret == HT_INTERRUPTED) ||
	    (!HTMultiLoading && HTCheckActiveIcon(0))) {
	    if (using_proxy)
		ClearTempBongedProxies();
	    return(HT_INTERRUPTED);
	}
	/*
	 * HT_REDIRECTING supplied by Dan Riley -- dsr@lns598.lns.cornell.edu
	 */
	if ((ret == HT_LOADED) || (ret == HT_REDIRECTING) || !using_proxy ||
	    !((fallbacks == -1) ?
	      (fallbacks = has_fallbacks(p->name)) : fallbacks) ||
	    (ret == HT_NO_DATA &&
	     !my_strncasecmp(p->name, "telnet", 6))) {
	    if (using_proxy)
		ClearTempBongedProxies();
	    return(ret);
	}
	if (retry-- > 0) {
	    HTProgress("Retrying proxy server...");
	    goto retry_proxy;
	}

	/* Must be using proxy and have a problem to get here! */

	host = HTParse(HTAnchor_physical(anchor), "", PARSE_HOST);

	finbuf = (char *)malloc(strlen(host) + strlen(buf1) + strlen(buf2) + 5);
	sprintf(finbuf, "%s%s?%s", buf1, host, buf2);
	if (HTConfirm(finbuf)) {
	    free(finbuf);
	    finbuf = (char *)malloc(strlen(host) +
				    strlen("Disabling proxy server ") +
				    strlen(" and trying again.") + 5);
	    sprintf(finbuf, "Disabling proxy server %s and trying again.",host);
	    HTProgress(finbuf);
	    application_user_feedback(finbuf);
	    status = get_physical(addr, anchor, 1);  /* Perm disable */

	} else if (HTConfirm("Try next fallback proxy server?")) {
	    status = get_physical(addr, anchor, 2);  /* Temp disable */
	}
	/* Else -- Try the same one again */

	if (finbuf)
	    free(finbuf);

	/* get_physical messes with anchor */ 
    	p = HTAnchor_protocol(anchor);
    }
}


/*		Get a save stream for a document
**		--------------------------------
*/
PUBLIC HTStream *HTSaveStream (HTParentAnchor *anchor)
{
    HTProtocol *p = HTAnchor_protocol(anchor);

    if (!p)
	return NULL;
    
    return (*p->saveStream)(anchor);
}


/*		Load a document - with logging, etc.
**		------------------------------------
**
**	- Checks or documents already loaded
**	- Logs the access
**	- Allows stdin filter option
**	- Trace ouput and error messages
**
**    On Entry,
**	  anchor	    is the node_anchor for the document.
**        full_address      address of the document to be accessed.
**        filter            if YES, treat stdin as HTML.
**
**    On Exit,
**        returns    1     Success in opening document
**                   0     Failure 
**                   -1    Interrupted
**
*/

/* This is exported all the way to gui-documents.c at the moment,
 * to tell mo_load_window_text when to use a redirected URL instead. */
char *use_this_url_instead;

PRIVATE int HTLoadDocument (WWW_CONST char *full_address,
			    HTParentAnchor *anchor,
			    HTFormat format_out,
			    HTStream *sink)
{
    int	status;

    use_this_url_instead = NULL;

    /* We LOVE goto's! 
     *
     * Let's rephrase this..._You_ love goto's...we _abhore_ goto's.  People who
     *   LOVE goto's should be shot.
     */
  try_again:
#ifndef DISABLE_TRACE
    if (www2Trace) {
	fprintf(stderr, "HTAccess: loading document %s\n", full_address);
#ifndef VMS
	fflush(stderr);
#endif /* BSN, or, rather, MG */
    }
#endif

    status = HTLoad(full_address, anchor, format_out, sink);
    
    if (status == HT_LOADED) {
#ifndef DISABLE_TRACE
	if (www2Trace)
	   fprintf(stderr, "HTAccess: `%s' has been accessed.\n", full_address);
#endif
	return 1;
    }

    if ((status == HT_REDIRECTING) && redirecting_url) {
#ifndef DISABLE_TRACE
        if (www2Trace) {
            fprintf(stderr, "HTAccess: '%s' is a redirection URL.\n", 
                    full_address);
            fprintf(stderr, "HTAccess: Redirecting to '%s'\n", redirecting_url);
        }
#endif
	use_this_url_instead = full_address =
		 mo_url_canonicalize_keep_anchor(redirecting_url, full_address);
	free(redirecting_url);
	redirecting_url = NULL;
        goto try_again;
    }

    if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
        if (www2Trace)
            fprintf(stderr, "HTAccess: We were interrupted.\n");
#endif
        return -1;
    }
    
    if (status == HT_NO_DATA) {
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr, "HTAccess: `%s' has been accessed, No data left.\n",
		    full_address);
#endif
	return 0;    
    }
    
    if (status < 0) {		      /* Failure in accessing a document */
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr, "HTAccess: Can't access `%s'\n", full_address);
#endif
	return 0;
    }
 
    /* If you get this, then please find which routine is returning
     * a positive unrecognized error code! */
#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr,
         "HTAccess: socket or file num %d returned by obsolete load routine!\n",
	 status);
#endif
    return 0;
}


/*		Load a document from absolute name
**		---------------
**
**    On Entry,
**        addr     The absolute address of the document to be accessed.
**        filter   if YES, treat document as HTML
**
**    On Exit,
**        returns    1     Success in opening document
**                   0      Failure 
**                   -1      Interrupted
**
*/
PUBLIC int HTLoadAbsolute (WWW_CONST char *addr)
{
    static init = 0;
    static HTAtom *www_present;

    if (!init) {
	www_present = WWW_PRESENT;
	init = 1;
    }

    if (currentURL)
	free(currentURL);
    currentURL = strdup(addr);

    if (MIME_http.last_modified) {
	free(MIME_http.last_modified);
	MIME_http.last_modified = NULL;
    }
    if (MIME_http.expires) {
	free(MIME_http.expires);
	MIME_http.expires = NULL;
    }
    if (MIME_http.refresh) {
	free(MIME_http.refresh);
	MIME_http.refresh = NULL;
    }
    if (MIME_http.charset) {
	free(MIME_http.charset);
	MIME_http.charset = NULL;
    }
    return HTLoadDocument(addr,
     		          HTAnchor_parent(HTAnchor_findAddress(addr)),
       			  HTOutputFormat ? HTOutputFormat : www_present,
			  HTOutputStream);
}


/*		Load a document from absolute name to stream
**		--------------------------------------------
**
**    On Entry,
**        addr     The absolute address of the document to be accessed.
**        sink     if non-NULL, send data down this stream
**
**    On Exit,
**        returns    YES     Success in opening document
**                   NO      Failure 
**
**
*/
PUBLIC BOOL HTLoadToStream (WWW_CONST char *addr, BOOL filter, HTStream *sink)
{
   return HTLoadDocument(addr,
       			 HTAnchor_parent(HTAnchor_findAddress(addr)),
       			 HTOutputFormat ? HTOutputFormat : WWW_PRESENT,
			 sink);
}


/*		Load a document from relative name
**		---------------
**
**    On Entry,
**        relative_name     The relative address of the document
**	  		    to be accessed.
**
**    On Exit,
**        returns    YES     Success in opening document
**                   NO      Failure 
*/
PUBLIC BOOL HTLoadRelative (WWW_CONST char *relative_name, HTParentAnchor *here)
{
    BOOL result;
    char *full_address, *stripped;
    char *mycopy = NULL;
    char *current_address = HTAnchor_address((HTAnchor *) here);

    StrAllocCopy(mycopy, relative_name);

    stripped = HTStrip(mycopy);
    full_address = HTParse(stripped, current_address,
		    PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_PUNCTUATION);
    result = HTLoadAbsolute(full_address);
    free(full_address);
    free(current_address);
    free(mycopy);
    return result;
}

/*		Start load of multiple documents with absolute names
**		---------------
**
**    On Entry,
**        document_list     The list of documents to be accessed.
**
**    On Exit,
**        returns    1      Success in starting
**                   -1     Interrupted
*/
PUBLIC int HTStartMultiLoad (HTBTree *image_loads)
{
    HTBTElement *ele;
    MultiInfo *img;
    int status;
    int count = 0;
    int rv = 1;

    ele = HTBTree_next(image_loads, NULL);

    while (ele && (count < 6) && (multi_count < HTMultiLoadLimit)) {
	img = (MultiInfo *)ele->object;
	ele = HTBTree_next(image_loads, ele);
	if (!img->loaded && !img->filename && !img->killed && !img->failed) {
	    count++;
	    HTMultiLoading = img;
	    force_dump_filename = img->filename = mo_tmpnam(img->url);
	    force_dump_to_file = 1;
	    status = HTLoadAbsolute(img->url);
	    force_dump_to_file = 0;
	    /* -1 = interrupted, 0 = failed */
	    if (status < 1) {
		img->failed = 1;
		free(img->filename);
		img->filename = NULL;
		/* Stream and socket should be closed here. */
		if (status == -1) {
		    rv = -1;
		    break;
		}
		continue;
	    }
	    /* If not loaded completely in first read, add to list */
	    if (!img->loaded) {
		multi_count++;
		img->next = multi_loading;
		multi_loading = img;
	    }
	}
    }
    HTMultiLoading = NULL;

    /* More to do later? */
    if (ele) {
	multi_more = image_loads;
    } else {
	multi_more = NULL;
    }
    return rv;
}

/*		Finishing load one of multiple documents with absolute names
**		---------------
**
**    On Entry,
**        document_list      The list of documents to be accessed.
**
**    On Exit,
**        returns    1     Success in starting load
**		     0	   Failure
**                   -1    Interrupted
*/
PUBLIC int HTMultiLoad (MultiInfo *img)
{
    MultiInfo *next, *old_next;
    fd_set readfds;
    struct timeval timeout;
    int first_loop = 1;
    int status;

    /* More to start? */
    if (multi_more && (multi_count < HTMultiLoadLimit)) {
#ifndef DISABLE_TRACE
	if (www2Trace)
 	    fprintf(stderr, "Calling HTStartMultiLoad from HTMultiLoad\n");
#endif
	if (HTStartMultiLoad(multi_more) == -1)
	    return -1;

	/* May have gotten loaded completely or failed in HTStartMultiLoad */
	if (img->loaded)
	    return 1;
	if (img->failed)
	    return 0;
    }

    /* Check if this one on current loading list */
    next = multi_loading;
    while (next) {
	if (next == img)
	    break;
        next = next->next;
    }

    if (!next) {
	/* Not currently loading, so start loading it. */
	HTMultiLoading = img;
	force_dump_filename = img->filename = mo_tmpnam(img->url);
	force_dump_to_file = 1;
	status = HTLoadAbsolute(img->url);
	force_dump_to_file = 0;
	HTMultiLoading = NULL;
	/* -1 = interrupted, 0 = failed */
	if (status < 1) {
	    img->failed = 1;
	    free(img->filename);
	    img->filename = NULL;
	    /* Stream and socket should be closed here. */
	    return status;
	}
	/* May have got it all in initial read */
	if (img->loaded)
	    return 1;
	img->next = multi_loading;
	multi_loading = img;
	multi_count++;
    } else if (img->loaded) {
	/* Should never happen unless caller didn't check it */
	return 1;
    } else if (img->failed) {
	/* Should never happen unless caller didn't check it */
	return 0;
    }

    /* Remove finished ones from the loading list */
    next = multi_loading;
    multi_loading = NULL;
    while (next) {
	if (!next->loaded && !next->failed) {
	    old_next = next->next;
	    next->next = multi_loading;
	    multi_loading = next;
	    next = old_next;
	} else {
	    next = next->next;
	}
    }

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "multi_count is %d\n", multi_count);
#endif

    /* Loop thru list reading data until this image is done. */
    next = multi_loading;
    while (next) {
	if (!next->filename || next->loaded) {
	    /* Skip it */
	    next = next->next;
	    if (!next)
		next = multi_loading;
	    continue;
	}
#ifdef HAVE_SSL
	if (next->handle && (SSL_pending(next->handle) > 0)) {
	    /* Can skip select() */
	    status = 1;
	} else {
	    status = 0;
	}
	if (!status) {
#else
	{
#endif
	    FD_ZERO(&readfds);
	    FD_SET((unsigned) next->socket, &readfds);
	    timeout.tv_sec = 0;
	    /* Zero timeout on first pass, then 1/10 sec for requested image */
	    if (first_loop || (img != next)) {
		timeout.tv_usec = 0;
	    } else {
		timeout.tv_usec = 100000;
	    }
#if defined(__hpux) || defined(MULTINET) || defined(_DECC_V4_SOURCE) || (defined(__DECC) && !defined(__DECC_VER)) || __DECC_
            status = select((unsigned) next->socket + 1, (int *)&readfds, NULL,
			    NULL, &timeout);
#else
            status = select((unsigned) next->socket + 1, &readfds, NULL,
			    NULL, &timeout);
#endif
	}
        if (status < 0) {
	    /* Error */
	    HTStream *stream = (HTStream *)next->stream;

#ifndef DISABLE_TRACE
	    if (httpTrace)
		fprintf(stderr, "HTMultiLoad: Select errno = %d\n", errno);
#endif
	    next->failed = 1;
	    free(next->filename);
	    next->filename = NULL;
	    /* Treat all errors as interrupts */
	    (*stream->isa->handle_interrupt)(stream);
	    HTClose_HTTP_Socket(next->socket, next->handle);
	    (*stream->isa->free)(stream);
	    multi_count--;
	    if (img == next)
		return 0;
        } else if (status > 0) {
	    HTStream *stream = (HTStream *)next->stream;

	    HTCopyOneRead = 1;
            status = HTCopy(next->socket, stream, next->length,
			    next->handle, -1);
	    HTCopyOneRead = 0;
	    if (status > 0) {
		next->length = status;
	    } else if (!status) {
		/* Finished it */
		force_dump_to_file = 1;
		(*stream->isa->end_document)(stream);
		HTClose_HTTP_Socket(next->socket, next->handle);
		(*stream->isa->free)(stream);
		force_dump_to_file = 0;
		next->loaded = 1;
		multi_count--;
		if (img == next)
		    /* Finished the one we asked for */
		    return 1;
	    } else {
		/* Error */
		next->failed = 1;
		free(next->filename);
		next->filename = NULL;
		/* Treat all errors as interrupts */
		(*stream->isa->handle_interrupt)(stream);
		HTClose_HTTP_Socket(next->socket, next->handle);
	        (*stream->isa->free)(stream);
		multi_count--;
		if (status == -1) {
		    /* Interrupted */
		    return status;
		}
		/* If -2, could be http0 problem?  Need to retry? */
		if (img == next)
		    return 0;
	    }
	}

	next = next->next;
	if (!next) {
	    if (HTCheckActiveIcon(1))
            	return -1;	/* Interrupted */
	    first_loop = 0;	/* Enable non-zero timeout to slow logo down */
	    next = multi_loading;
	}
    }
    return 1;
}

/*		Reset load of multiple documents
**		---------------
**
**    On Entry,
**        document_list     The list of documents being multi loaded.
**
**    On Exit,
**        returns    YES     Success
*/
PUBLIC Boolean HTResetMultiLoad (void)
{
    MultiInfo *next = multi_loading;

    while (next) {
	if (next->filename && !next->loaded && !next->failed) {
	    /* In middle of loading */
	    if (next->stream) {
		HTStream *stream = (HTStream *)next->stream;

	        (*stream->isa->handle_interrupt)(stream);
		HTClose_HTTP_Socket(next->socket, next->handle);
	        (*stream->isa->free)(stream);
	    } else {
	        HTClose_HTTP_Socket(next->socket, next->handle);
	    }
	}
	next = next->next;
    }
    multi_loading = NULL;
    multi_more = NULL;
    multi_count = 0;

    return 1;
}
