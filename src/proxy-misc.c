/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright 1993-1995, Board of Trustees of the University of Illinois     *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/

/* Copyright (C) 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include "mosaic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proxy.h"
#ifdef CCI
#include "../libnut/str-tools.h"
#endif

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

#define BUFLEN 256
#define BLANKS " \t\n"

extern struct Proxy *proxy_list, *noproxy_list;

struct Proxy *ReadProxies(char *filename)
{
	FILE *fp;
	char buf[BUFLEN];
	char *psb;
	struct Proxy *head = NULL;
	struct Proxy *cur = NULL;
	struct Proxy *p;
	struct ProxyDomain *pCurList;
		
	if (!(fp = fopen(filename, "r"))) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "ReadProxies: Failed opening %s\n",
				filename);
#endif
		return NULL;
	}

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "ReadProxies: Opened %s\n", filename);
#endif
	/*
	** Read entries from the proxy list
	**
	** These calloc()s should be checked for returning NULL
	*/
	while (fgets(buf, BUFLEN, fp)) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Read proxy: %s\n", buf);
#endif
		p = (struct Proxy *)calloc(1, sizeof(struct Proxy));
		
		/** calloc zeros them
		p->next = NULL;
		p->prev = NULL;
		p->alive = 0;
		p->list = NULL;
		**/

		/*
		** Read the proxy scheme
		*/
		if (!(psb = strtok(buf, BLANKS)))
			return head;
		p->scheme = strdup(psb);

		/*
		** Read the proxy address
		*/
		if (!(psb = strtok(NULL, BLANKS)))
			return head;
		p->address = strdup(psb);

		/*
		** Read the proxy port
		*/
		if (!(psb = strtok(NULL, BLANKS)))
			return head;
		p->port = strdup(psb);

		/*
		** Read the transport mechanism
		*/
		if (!(psb = strtok(NULL, BLANKS)))
			return head;
		p->transport = strdup(psb);

		p->trans_val = TRANS_HTTP;
#ifdef CCI
		if (!my_strcasecmp(p->transport, "cci"))
			p->trans_val = TRANS_CCI;
#endif
		/*
		** Read the domain
		*/
		if (psb = strtok(NULL, BLANKS)) {
			AddProxyDomain(psb, &p->list);

			pCurList = p->list;
			while (psb = strtok(NULL, BLANKS)) {
				if (*psb == '\\') {
					if (!fgets(buf, BUFLEN, fp))
						return head;
					psb = strtok(buf, BLANKS);
					if (!psb)
						return head;
				}
				if (!AddProxyDomain(psb, &pCurList))
					return head;
			}
		}
		if (!cur) {
			head = cur = p;
		} else {
			p->prev = cur;
			cur->next = p;
			cur = p;
		}
		if (feof(fp))
			break;
	}
	return(head);
}

struct Proxy *ReadNoProxies(char *filename)
{
	FILE *fp;
	char buf[BUFLEN];
	char *psb;
	struct Proxy *head = NULL;
	struct Proxy *cur = NULL;
	struct Proxy *p;
		
	if (!(fp = fopen(filename, "r")))
		return NULL;

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "ReadNoProxies: Opened %s\n", filename);
#endif
	/*
	** Read entries from the no proxy list
	**
	** These calloc()s should be checked for returning NULL
	*/
	while (fgets(buf, BUFLEN, fp)) {
#ifndef DISABLE_TRACE
		if (srcTrace) 
			fprintf(stderr, "Read no proxy: %s\n", buf);
#endif
		p = (struct Proxy *)calloc(1, sizeof(struct Proxy));
		
		/*
		** The proxy protocol, transport, and list
		** are all null for no proxy.
		*/
		/** calloc zeros them
		p->next = NULL;
		p->prev = NULL;
		p->scheme = NULL;
		p->port = NULL;
		p->transport = NULL;
		p->list = NULL;
		**/

		/*
		** Read the proxy address
		*/
		if (!(psb = strtok(buf, BLANKS)))
			return head;
		p->address = strdup(psb);

		/*
		** Read the proxy port
		*/
		if (psb = strtok(NULL, BLANKS))
			p->port = strdup(psb);

		if (!cur) {
			head = p;
			cur = p;
		} else {
			p->prev = cur;
			cur->next = p;
			cur = p;
		}
		if (feof(fp))
			break;
	}
	return(head);
}

struct ProxyDomain *AddProxyDomain(char *sbDomain, struct ProxyDomain **pdList)
{
	struct ProxyDomain *pNewDomain =
		       (struct ProxyDomain *)malloc(sizeof(struct ProxyDomain));

	if (!pNewDomain)
		return NULL;

	pNewDomain->domain = strdup(sbDomain);
	if (!*pdList) {
		*pdList = pNewDomain;
		(*pdList)->next = NULL;
		(*pdList)->prev = NULL;
	} else {
		struct ProxyDomain *p = *pdList;

		while (p->next)
			p = p->next;
		pNewDomain->prev = p;
		pNewDomain->next = NULL;
		p->next = pNewDomain;
	}
	return pNewDomain;
}

void DeleteProxyDomain(struct ProxyDomain *p)
{
	if (!p)
		return;
	if (p->next)
		p->next->prev = p->prev;
	if (p->prev)
		p->prev->next = p->next;

	if (p->domain) {
		free(p->domain);
		p->domain = NULL;
	}
	free(p);
}

/*
 * Returns true if there is at least one fallback proxy for the specified
 * protocol (means more than one proxy server specified).
 *
 * --SWP
 */
int has_fallbacks(char *protocol)
{
	int protocol_len;
	struct Proxy *ptr = proxy_list;

	if (!proxy_list || !protocol || !*protocol)
		return(0);

	protocol_len = strlen(protocol);

	while (ptr) {
		if (ptr->scheme &&
		    !strncmp(ptr->scheme, protocol, protocol_len))
			return(1);
		ptr = ptr->next;
	}
	return(0);
}

struct Proxy *GetNoProxy(char *access, char *site)
{
	struct Proxy *p = noproxy_list;
	char *port = NULL;
	int portnum = -1;

	if (!p || !access || !site)
		return NULL;

	if (port = strchr(site, ':')) {
		*port++ = '\0';
		portnum = atoi(port);
	} else {
		if      (!strcmp(access, "http"))    portnum = 80;
		else if (!strcmp(access, "gopher"))  portnum = 70;
		else if (!strcmp(access, "ftp"))     portnum = 21;
		else if (!strcmp(access, "wais"))    portnum = 210;
	}

	while (p) {
		if (strstr(site, p->address)) {
			if (!p->port) {
				break;
			} else if (atoi(p->port) == portnum) {
				break;
			}
		}
		p = p->next;
	}
	return p;
}

void ClearTempBongedProxies()
{
	struct Proxy *p = proxy_list;

	while (p) {
		if (p->alive == 2)
			p->alive = 0;
		p = p->next;
	}
	return;
}

struct Proxy *GetProxy(char *proxy, char *access, int fMatchEnd)
{
	struct Proxy *p = proxy_list;
	struct ProxyDomain *pd;

	if (!access || !proxy)
		return NULL;

	while (p) {
		if (strcmp(p->scheme, proxy) || p->alive) {
			p = p->next;
			continue;
		}
		/* Found a matching proxy */

		/*
		** If the access list is empty, that's a match on
		** everything.  Bale out here.
		*/
		if (!p->list)
			return p;	
		pd = p->list;
		
		while (pd) {
			char *ptr = strstr(access, pd->domain);

			if (ptr) {
				if (fMatchEnd) {
					/* At the end? */
					if (strlen(ptr) == strlen(pd->domain)) 
						break;
				} else if (ptr == access) {
					/* At beginning? */
					break;
				}
			}
			pd = pd->next;
		}
		if (!pd) {
			p = p->next;
			continue;  /* We didn't match... look for another */
		}
		return p;  /* We found a match on access and proxy */
	}
	return NULL;
}
