/* Copyright (C) 2008, 2009 - The VMS Mosaic Project */

#include "../config.h"

#include <string.h>
#include <stdio.h>

#include "HTMLP.h"
#include "HTMLcss.h"

#ifndef DISABLE_TRACE
extern int htmlwTrace;
#endif

#define SHORT_XML_NAMES_ONLY
#include "../libxml2/libxml2_config.h"

#include "../libcroco/libcroco.h"

static void start_document_cb(CRDocHandler *a_handler)
{
#ifndef DISABLE_TRACE
	if (htmlwTrace)
		fprintf(stderr, "CSS document start\n") ;
#endif
}

static void end_document_cb(CRDocHandler *a_handler)
{
#ifndef DISABLE_TRACE
	if (htmlwTrace)
		fprintf(stderr, "CSS document end\n") ;
#endif
}

int HTMLParseCSS(char *sheet)
{
	CRParser *parser = NULL;
	CRDocHandler *sac_handler = NULL;
	gulong len = strlen(sheet);
	int freeit = 0;
	int status = 1;

	/* Add {} if not there already */
	if (*sheet != '{') {
		char *tmp = sheet;
		char *ptr = malloc(len + 3);

		sheet = ptr;
		len += 2;
		*ptr++ = '{';

		for (; *tmp; )
			*ptr++ = *tmp++;

		*ptr++ = '}';
		*ptr = '\0';
		freeit = 1;
#ifndef DISABLE_TRACE
		if (htmlwTrace)
			fprintf(stderr, "%s\n", sheet);
#endif
	}

	/* Setup the libcroco parser */
	if (!(parser = cr_parser_new_from_buf((unsigned char *)sheet,
					      len, CR_ASCII, False))) {
		status = -1;
		goto error;
	}
	/* Setup SAC document handler */
	if (!(sac_handler = cr_doc_handler_new())) {
		cr_parser_destroy(parser);
		status = -1;
		goto error;
	}

	/* Setup callbacks */
	sac_handler->start_document = start_document_cb;
	sac_handler->end_document = end_document_cb;

	/* Register SAC handler */
	cr_parser_set_sac_handler(parser, sac_handler);

	/* Do parse */
	cr_parser_parse(parser);

	/* Destroy parser */
	cr_parser_destroy(parser);

 error:
	if (freeit)
		free(sheet);

	return status;
}
