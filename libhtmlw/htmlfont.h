/* This file is Copyright (C) 2003 - The VMS Mosaic Project */
#ifndef HTML_FONT_H
#define HTML_FONT_H

extern void SetFontSize(HTMLWidget hw, PhotoComposeContext *pcc, int refresh);
extern FontRec *PushFont(HTMLWidget hw, PhotoComposeContext *pcc);
extern XFontStruct *PopFont(HTMLWidget hw, PhotoComposeContext *pcc);
extern void InitFontStack(HTMLWidget hw, PhotoComposeContext *pcc);
extern void DefaultFontFamily(HTMLWidget hw, PhotoComposeContext *pcc,
	CurFontFamily family);
extern void DefaultFontColor(HTMLWidget hw, PhotoComposeContext *pcc,
	unsigned long color);
extern unsigned long GetDefaultFontColor(HTMLWidget hw);
extern void PopFontSaved(HTMLWidget hw, PhotoComposeContext *pcc);

#endif /* HTML_FONT_H */
