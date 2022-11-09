/* This file is Copyright (C) 1996 - G.Dauphin
 * See the file "license.mMosaic" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* Copyright (C) 2004, 2005, 2006 - The VMS Mosaic Project */

extern	void EndForm(HTMLWidget hw, PhotoComposeContext *pcc);
extern	void BeginForm(HTMLWidget hw, MarkInfo **mptr,
        	       PhotoComposeContext *pcc);

extern	void FormButtonEnd(HTMLWidget hw, MarkInfo **mptr,
        		   PhotoComposeContext *pcc);
extern	void FormButtonBegin(HTMLWidget hw, MarkInfo **mptr,
        		     PhotoComposeContext *pcc);

extern	void FormInputField(HTMLWidget hw, MarkInfo **mptr,
        PhotoComposeContext *pcc);

extern	void FormTextAreaBegin(MarkInfo **mptr, PhotoComposeContext *pcc);
extern	void FormTextAreaEnd(HTMLWidget hw, MarkInfo **mptr,
        		     PhotoComposeContext *pcc);
extern	void FormSelectOptionField(MarkInfo **mptr, PhotoComposeContext *pcc);
extern	void FormSelectOptgroup(MarkInfo **mptr, PhotoComposeContext *pcc);
extern	void FormSelectBegin(HTMLWidget hw, MarkInfo **mptr,
        		     PhotoComposeContext *pcc);
extern	void FormSelectEnd(HTMLWidget hw, PhotoComposeContext *pcc);

