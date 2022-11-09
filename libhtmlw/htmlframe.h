/* Copyright (C) 2005 - The VMS Mosaic Project */

#ifndef LIBHTMLW_HTMLFRAME_H
#define LIBHTMLW_HTMLFRAME_H

extern void FramePlace(HTMLWidget hw, MarkInfo *mptr, PhotoComposeContext *pcc);
extern void HTMLDestroyFrames(HTMLWidget hw);
extern Boolean HTMLCreateFrameSet(HTMLWidget hw, MarkInfo **mptr,
	PhotoComposeContext *pcc);
extern void IframeRefresh(ElemInfo *eptr);

#endif
