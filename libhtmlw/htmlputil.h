/* Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project */

#ifndef LIBHTMLW_HTMLP_UTIL_H
#define LIBHTMLW_HTMLP_UTIL_H

extern MarkInfo         *HTMLParse(HTMLWidget hw, char *str, char *charset);
extern ElemInfo 	*GetElemRec();
extern MarkInfo 	*GetMarkRec();
extern ImageInfo 	*GetImageRec();
extern char		*GetMarkText(char *text);
extern void 		FreeMarkRec(MarkInfo *mark);
extern void 		FreeMarkUpList(MarkInfo *List);
extern void 		FreeLineList(ElemInfo *list, HTMLWidget hw);
extern void 		FreeMapList(MapInfo *map);
extern void 		FreeImageInfo(ImageInfo *picd, HTMLWidget hw);
extern void		FreeMarkText(char *text);
extern int 		ElementLessThan(ElemInfo *start, ElemInfo *end,
					int start_pos, int end_pos);
extern int 		SwapElements(ElemInfo *start, ElemInfo *end,
				     int start_pos, int end_pos);
extern void 		PartOfTextPlace(HTMLWidget hw, MarkInfo *mptr,
					PhotoComposeContext *pcc);
extern void 		PartOfPreTextPlace(HTMLWidget hw, MarkInfo *mptr,
					   PhotoComposeContext *pcc);
extern void 		LinefeedPlace(HTMLWidget hw, PhotoComposeContext *pcc);
extern void 		ConditionalLineFeed(HTMLWidget hw, int state,
					    PhotoComposeContext *pcc);
extern void 		HRulePlace(HTMLWidget hw, MarkInfo *mptr, 
				   PhotoComposeContext *pcc);
extern void 		BulletPlace(HTMLWidget hw, PhotoComposeContext *pcc,
				    int list);
extern void		ListNumberPlace(HTMLWidget hw, PhotoComposeContext *pcc,
					int val, char type);
extern Dimension	HbarHeight(HTMLWidget hw);

extern ElemInfo  	*CreateElement(HTMLWidget hw, int type, XFontStruct *fp,
        			       int x, int y, int width, int height,
				       int baseline, PhotoComposeContext *pcc);
extern void 		AdjustBaseLine(ElemInfo *eptr,
				       PhotoComposeContext *pcc);
extern int		FormatAll(HTMLWidget hw, int *Fwidth);
extern void 		FormatChunk(HTMLWidget hw, MarkInfo *start_mark,
        			    MarkInfo *end_mark,
				    PhotoComposeContext *pcc);
extern ElemInfo 	*LocateElement(HTMLWidget hw, int x, int y, int *pos);
extern char  		*ParseTextToString(ElemInfo *elist,
					   ElemInfo *startp, ElemInfo *endp,
        				   int start_pos, int end_pos,
        				   int space_width, int lmargin);
extern char  		*ParseTextToPrettyString(HTMLWidget hw, ElemInfo *elist,
						 ElemInfo *startp,
						 ElemInfo *endp,
        					 int start_pos, int end_pos,
        					 int space_width, int lmargin);
extern int 		DocumentWidth(HTMLWidget hw, MarkInfo *list);
extern void 		TextRefresh(HTMLWidget hw, ElemInfo *eptr,
        			    int start_pos, int end_pos,
				    Boolean background);
extern void 		ImageRefresh(HTMLWidget hw, ElemInfo *eptr,
				     ImageInfo *iptr);
extern void 		BulletRefresh(HTMLWidget hw, ElemInfo *eptr);
extern void 		HRuleRefresh(HTMLWidget hw, ElemInfo *eptr);

extern void 		FreeColors(HTMLWidget hw, Colormap colormap);
extern int 		FindColor(HTMLWidget hw, Colormap colormap,
				  XColor *colr);
extern Pixmap 		InfoToImage(HTMLWidget hw, ImageInfo *img_info,
				    int clip, ElemInfo *eptr);
extern void 		ImagePlace(HTMLWidget hw, MarkInfo *mptr, 
				   PhotoComposeContext *pcc);
extern void 		HtmlGetImage(HTMLWidget hw, ImageInfo *picd,
				     PhotoComposeContext *pcc, int force_load);
extern void 		ImageSubmitForm(FormInfo *fptr, XEvent *event,
					char *name, int x, int y);
extern void 		HideWidgets(HTMLWidget hw);
extern char  		*ComposeCommaList(char **list, int cnt);
extern void		FreeCommaList(char **list, int cnt);
extern void		WidgetPlace(HTMLWidget hw, MarkInfo *mptr,
				    PhotoComposeContext *pcc);
extern WidgetInfo 	*MakeWidget(HTMLWidget hw, char *text,
				    PhotoComposeContext *pcc, int id);
extern void 		AddNewForm(HTMLWidget hw, FormInfo *fptr);
extern void		WidgetRefresh(HTMLWidget hw, ElemInfo *eptr);

extern String 		ParseTextToPSString(HTMLWidget hw, ElemInfo *el,
                           		    ElemInfo *startp, ElemInfo *endp,
                           		    int start_pos, int end_pos,
					    int space_width, int lmargin,
					    int fontfamily, char *url,
					    char *time_str);
extern void		hw_do_bg(HTMLWidget hw, char *bgname,
				 PhotoComposeContext *pcc);
extern void		hw_do_color(HTMLWidget hw, char *att, char *cname,
				    PhotoComposeContext *pcc);
extern void 		TablePlace(HTMLWidget hw, MarkInfo **mptr, 
				   PhotoComposeContext *pcc);
extern void 		TableRefresh(HTMLWidget hw, ElemInfo *eptr);
extern void		_FreeTableStruct(TableInfo *t);
extern ElemInfo 	*CellRefresh(HTMLWidget hw, ElemInfo *eptr);

extern void 		AppletPlace(HTMLWidget hw, MarkInfo **mptr, 
				    PhotoComposeContext *pcc, Boolean save);
extern void 		AppletRefresh(HTMLWidget hw, ElemInfo *eptr);
extern void		_FreeAppletStruct(AppletInfo *ats);

extern void 		ProgressiveDisplay(HTMLWidget hw, ElemInfo *eptr,
					   PhotoComposeContext *pcc);
extern void 		ViewClearAndRefresh(HTMLWidget hw);
extern void		ScrollWidgets(HTMLWidget hw);
extern AreaInfo 	*GetMapArea(MapInfo *map, int x, int y);
extern MapInfo		*FindMap(HTMLWidget hw, char *mapname);
extern void		CreateAnchorElement(HTMLWidget hw, MarkInfo *mark,
					    PhotoComposeContext *pcc);
extern ElemInfo 	*RefreshElement(HTMLWidget hw, ElemInfo *eptr);
extern void		RefreshURL(XtPointer cld, XtIntervalId *id);
extern void 		CBResetForm(Widget w, XtPointer client_data,
				    XtPointer call_data);

#endif
