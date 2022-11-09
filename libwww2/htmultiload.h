/* Copyright (C) 2007 - The VMS Mosaic Project */

#ifndef __HTMULTILOAD_H__
#define __HTMULTILOAD_H__

#ifndef HTBTREE_H
#include "HTBTree.h"
#endif

typedef struct multi_rec {
        char *url;
        char *referer;
        void *cached;
        int killed;
        int loaded;
        int failed;
        char *filename;
        void *stream;		/* HTStream */
	int socket;
	void *handle;		/* SSL Handle */
	int length;		/* Loaded length */
	struct multi_rec *next;
} MultiInfo;

extern int HTStartMultiLoad(HTBTree *image_loads);
extern Boolean HTResetMultiLoad(void);
extern int HTMultiLoad(MultiInfo *multi);

#endif
