#ifndef LIBHTMLW_HTMLFRAME_H
#define LIBHTMLW_HTMLFRAME_H

extern void FramePlace(HTMLWidget hw, MarkInfo *mptr, PhotoComposeContext *pcc);
extern void HTMLDestroyFrames(HTMLWidget hw);
extern Boolean HTMLCreateFrameSet(HTMLWidget hw, MarkInfo **mptr,
	PhotoComposeContext *pcc);
extern void IframeRefresh(HTMLWidget hw, ElemInfo *eptr);

#endif
