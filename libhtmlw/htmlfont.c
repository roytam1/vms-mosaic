/* Copyright (C) 1998, 1999, 2000, 2003, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>

#include "HTMLP.h"
#include "HTMLfont.h"
#include "HTMLmiscdefs.h"
#include "../src/mosaic.h"

extern mo_window *current_win;

#ifndef DISABLE_TRACE
extern int htmlwTrace;
extern int reportBugs;
#endif

static long courier_mr34 = 0;
static long courier_mr24 = 0;
static long courier_mr20l = 0;
static long courier_mr20 = 0;
static long courier_mr18 = 0;
static long courier_mr17 = 0;
static long courier_mr14 = 0;
static long courier_mr12 = 0;
static long courier_mr10 = 0;
static long courier_mr8 = 0;
static long courier_br34 = 0;
static long courier_br24 = 0;
static long courier_br20l = 0;
static long courier_br20 = 0;
static long courier_br18 = 0;
static long courier_br17 = 0;
static long courier_br14 = 0;
static long courier_br12 = 0;
static long courier_br10 = 0;
static long courier_mo34 = 0;
static long courier_mo24 = 0;
static long courier_mo20l = 0;
static long courier_mo20 = 0;
static long courier_mo18 = 0;
static long courier_mo17 = 0;
static long courier_mo14 = 0;
static long courier_mo12 = 0;
static long courier_mo10 = 0;

static int err_count;

void InitFontStack(HTMLWidget hw, PhotoComposeContext *pcc)
{
	FontRec *FontStack = hw->html.fontstack;

	while (FontStack && FontStack->next) {
		FontRec *fptr = FontStack;

		FontStack = FontStack->next;
		free((char *)fptr);
#ifndef DISABLE_TRACE
		if (htmlwTrace || reportBugs) 
			fprintf(stderr, "Popping previous font stack!\n");
#endif
	}
	hw->html.pushfont_count = hw->html.font_save_count = 0;
	if (!FontStack)
		FontStack = (FontRec *)malloc(sizeof(FontRec));
	pcc->cur_font = FontStack->font = hw->html.font;
	pcc->cur_font_size = FontStack->size = hw->html.font_base;
	pcc->cur_font_type = FontStack->type = FONT;
	pcc->cur_font_family = FontStack->family = hw->html.font_family;
	FontStack->color_ch = 0;
	pcc->cur_font_color = FontStack->color = hw->manager.foreground;
	pcc->cur_font_base = hw->html.font_base;
	FontStack->next = NULL;
	hw->html.fontstack = FontStack;
}

FontRec *PushFont(HTMLWidget hw, PhotoComposeContext *pcc)
{
	FontRec *fptr = (FontRec *)malloc(sizeof(FontRec));

	CHECK_OUT_OF_MEM(fptr);
	hw->html.pushfont_count++;
	fptr->font = pcc->cur_font;
	fptr->size = pcc->cur_font_size;
	fptr->type = pcc->cur_font_type;
	fptr->family = pcc->cur_font_family;
	fptr->color = pcc->cur_font_color;
	fptr->color_ch = 0;
	fptr->next = hw->html.fontstack;
	hw->html.fontstack = fptr;
	return(fptr);
}

XFontStruct *PopFont(HTMLWidget hw, PhotoComposeContext *pcc)
{
	FontRec *FontStack = hw->html.fontstack;
	XFontStruct *font;

	/* Don't pop it if at the save limit */
	if (hw->html.font_save_count >= hw->html.pushfont_count) {
#ifndef DISABLE_TRACE
		if (htmlwTrace || reportBugs) 
			fprintf(stderr, "PopFont: at save count\n");
#endif
		return(pcc->cur_font);
	}
	if (FontStack->next) {
		FontRec *fptr = FontStack;

		hw->html.pushfont_count--;
		FontStack = FontStack->next;
		font = fptr->font;
		pcc->cur_font_size = fptr->size;
		pcc->cur_font_type = fptr->type;
		pcc->cur_font_family = fptr->family;
		if (fptr->color_ch)
			pcc->fg = pcc->cur_font_color = fptr->color;
		free((char *)fptr);
		hw->html.fontstack = FontStack;
	} else {
#ifndef DISABLE_TRACE
		if (htmlwTrace || reportBugs) 
			fprintf(stderr, "Popfont: empty font stack!\n");
#endif
		hw->html.pushfont_count = 0;
		font = FontStack->font;
		pcc->cur_font_size = FontStack->size;
		pcc->cur_font_type = FontStack->type;
		pcc->cur_font_family = FontStack->family;
		if (FontStack->color_ch)
			pcc->fg = pcc->cur_font_color = FontStack->color;
	}
	return(font);
}

void PopFontSaved(HTMLWidget hw, PhotoComposeContext *pcc)
{
	if (hw->html.font_save_count != hw->html.pushfont_count) {
		while (hw->html.font_save_count < hw->html.pushfont_count)
			pcc->cur_font = PopFont(hw, pcc);
		SetFontSize(hw, pcc, 0);
	}
}

/* Set the default font family */
void DefaultFontFamily(HTMLWidget hw, PhotoComposeContext *pcc,
		       CurFontFamily family)
{
	FontRec *fptr = hw->html.fontstack;

	/* If nothing pushed, then change current font family */
	if (!fptr->next) {
		pcc->cur_font_family = hw->html.font_family = family;
		SetFontSize(hw, pcc, 0);
	} else {
		/* Else find the initial entry */
		while (fptr->next)
			fptr = fptr->next;
	}
	/* Make it the default */
	fptr->family = family;
}

/* Set the default font color */
void DefaultFontColor(HTMLWidget hw, PhotoComposeContext *pcc,
		      unsigned long color)
{
	FontRec *fptr = hw->html.fontstack;

	/* If nothing pushed, then change current font color */
	if (!fptr->next) {
		pcc->fg = pcc->cur_font_color = color;
	} else {
		/* Else find the initial entry */
		while (fptr->next)
			fptr = fptr->next;
	}
	/* Make it the default */
	fptr->color_ch = 1;
	fptr->color = color;
}

/* Get the default font color */
unsigned long GetDefaultFontColor(HTMLWidget hw)
{
	FontRec *fptr = hw->html.fontstack;

	if (!fptr->next) {
		return(hw->html.fontstack->color);
	} else {
		/* Else find the initial entry */
		while (fptr->next)
			fptr = fptr->next;
	}
	return(fptr->color);
}

static XFontStruct *wrapError(char *name)
{
  char buf[BUFSIZ];
  XFontStruct *font = XLoadQueryFont(dsp, "fixed");

  /* Only do popup display once per groups of fonts */
  if (current_win && !err_count) {
      sprintf(buf, "Could not open font '%s'. Using fixed instead.", name);
      XmxMakeWarningDialog(current_win->base, buf, "Load Font Error");
  } else {
      fprintf(stderr, "Load Font Error\n");
      fprintf(stderr, "Could not open font '%s'.\n", name);
      fprintf(stderr, "     Using fixed instead.\n");
  }
  err_count++;

  return(font);
}

static long wrapFont(char *name)
{
  XFontStruct *font = XLoadQueryFont(dsp, name);

  if (!font)
      font = wrapError(name);
  return((long)font);
}

static long wrapFont2(char *name, char *name2)
{
  XFontStruct *font = XLoadQueryFont(dsp, name);

  if (!font) {
      if (name2)
          font = XLoadQueryFont(dsp, name2);
      if (!font)
	  font = wrapError(name);
  }
  return((long)font);
}

static void SetFixedFont(int size)
{
  switch (size) {
    case 1:
      if (!courier_mr10)
	courier_mr10 = wrapFont("-adobe-courier-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, courier_mr10);
      if (!courier_br10)
	courier_br10 = wrapFont("-adobe-courier-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, courier_br10);
      if (!courier_mo10)
	courier_mo10 = wrapFont("-adobe-courier-medium-o-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo10);
      XmxSetArg(WbNplainFont, courier_mr10);
      XmxSetArg(WbNplainboldFont, courier_br10);
      XmxSetArg(WbNplainitalicFont, courier_mo10);
      if (!courier_mr8)
	courier_mr8 = wrapFont("-adobe-courier-medium-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr8);

      break;
    case 2:
      if (!courier_mr14)
	courier_mr14 = wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, courier_mr14);
      if (!courier_br14)
	courier_br14 = wrapFont("-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, courier_br14);
      if (!courier_mo14)
	courier_mo14 = wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo14);
      if (!courier_mr12)
	courier_mr12 = wrapFont("-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, courier_mr12);
      if (!courier_br12)
	courier_br12 = wrapFont("-adobe-courier-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, courier_br12);
      if (!courier_mo12)
	courier_mo12 = wrapFont("-adobe-courier-medium-o-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo12);
      if (!courier_mr10)
	courier_mr10 = wrapFont("-adobe-courier-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr10);

      break;
    case 3:
      if (!courier_mr17)
	courier_mr17 = wrapFont2("-adobe-courier-medium-r-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, courier_mr17);
      if (!courier_br17)
	courier_br17 = wrapFont2("-adobe-courier-bold-r-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, courier_br17);
      if (!courier_mo17)
	courier_mo17 = wrapFont2("-adobe-courier-medium-o-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo17);
      if (!courier_mr14)
	courier_mr14 = wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, courier_mr14);
      if (!courier_br14)
	courier_br14 = wrapFont("-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, courier_br14);
      if (!courier_mo14)
	courier_mo14 = wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo14);
      if (!courier_mr12)
	courier_mr12 = wrapFont("-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr12);

      break;
    case 4:
      if (!courier_mr18)
	courier_mr18 = wrapFont("-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, courier_mr18);
      if (!courier_br18)
	courier_br18 = wrapFont("-adobe-courier-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, courier_br18);
      if (!courier_mo18)
	courier_mo18 = wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo18);
      if (!courier_mr17)
	courier_mr17 = wrapFont2("-adobe-courier-medium-r-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, courier_mr17);
      if (!courier_br17)
	courier_br17 = wrapFont2("-adobe-courier-bold-r-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, courier_br17);
      if (!courier_mo17)
	courier_mo17 = wrapFont2("-adobe-courier-medium-o-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo17);
      if (!courier_mr14)
	courier_mr14 = wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr14);

      break;
    case 5:
      if (!courier_mr20)
	courier_mr20 = wrapFont2("-adobe-courier-medium-r-normal-*-20-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, courier_mr20);
      if (!courier_br20)
	courier_br20 = wrapFont2("-adobe-courier-bold-r-normal-*-20-*-*-*-*-*-*-*",
				 "-adobe-courier-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, courier_br20);
      if (!courier_mo20)
	courier_mo20 = wrapFont2("-adobe-courier-medium-o-normal-*-20-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo20);
      if (!courier_mr18)
	courier_mr18 = wrapFont("-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, courier_mr18);
      if (!courier_br18)
	courier_br18 = wrapFont("-adobe-courier-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, courier_br18);
      if (!courier_mo18)
	courier_mo18 = wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo18);
      if (!courier_mr17)
	courier_mr17 = wrapFont2("-adobe-courier-medium-r-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr17);

      break;
    case 6:
      if (!courier_mr24)
	courier_mr24 = wrapFont("-adobe-courier-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, courier_mr24);
      if (!courier_br24)
	courier_br24 = wrapFont("-adobe-courier-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, courier_br24);
      if (!courier_mo24)
	courier_mo24 = wrapFont("-adobe-courier-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo24);
      if (!courier_mr20l)
	courier_mr20l = wrapFont2("-adobe-courier-medium-r-normal-*-20-*-*-*-*-*-*-*",
				  "-adobe-courier-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, courier_mr20l);
      if (!courier_br20l)
	courier_br20l = wrapFont2("-adobe-courier-bold-r-normal-*-20-*-*-*-*-*-*-*",
				  "-adobe-courier-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, courier_br20l);
      if (!courier_mo20l)
	courier_mo20l = wrapFont2("-adobe-courier-medium-o-normal-*-20-*-*-*-*-*-*-*",
				  "-adobe-courier-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo20l);
      if (!courier_mr20)
	courier_mr20 = wrapFont2("-adobe-courier-medium-r-normal-*-20-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr20);

      break;
    case 7:
      if (!courier_mr34)
	courier_mr34 = wrapFont2("-adobe-courier-medium-r-normal-*-34-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, courier_mr34);
      if (!courier_br34)
	courier_br34 = wrapFont2("-adobe-courier-bold-r-normal-*-34-*-*-*-*-*-*-*",
				 "-adobe-courier-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, courier_br34);
      if (!courier_mo34)
	courier_mo34 = wrapFont2("-adobe-courier-medium-o-normal-*-34-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo34);
      if (!courier_mr24)
	courier_mr24 = wrapFont("-adobe-courier-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, courier_mr24);
      if (!courier_br24)
	courier_br24 = wrapFont("-adobe-courier-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, courier_br24);
      if (!courier_mo24)
	courier_mo24 = wrapFont("-adobe-courier-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo24);
      if (!courier_mr20l)
	courier_mr20l = wrapFont2("-adobe-courier-medium-r-normal-*-20-*-*-*-*-*-*-*",
				  "-adobe-courier-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr20l);
  }

}

void SetFontSize(HTMLWidget hw, PhotoComposeContext *pcc, int refresh)
{
  Widget w = (Widget)hw;
  int lucida_missing = 0;
  static long times_mr34 = 0;
  static long times_mr24 = 0;
  static long times_mr20 = 0;
  static long times_mr18 = 0;
  static long times_mr17 = 0;
  static long times_mr14 = 0;
  static long times_mr12 = 0;
  static long times_mr10 = 0;
  static long times_mi34 = 0;
  static long times_mi24 = 0;
  static long times_mi20 = 0;
  static long times_mi18 = 0;
  static long times_mi17 = 0;
  static long times_mi14 = 0;
  static long times_mi10 = 0;
  static long times_br34 = 0;
  static long times_br25 = 0;
  static long times_br24 = 0;
  static long times_br20 = 0;
  static long times_br20l = 0;
  static long times_br18 = 0;
  static long times_br17 = 0;
  static long times_br17s = 0;
  static long times_br14 = 0;
  static long times_br12 = 0;
  static long times_br10 = 0;
  static long times_br8 = 0;
  static long times_bi34 = 0;
  static long times_bi24 = 0;
  static long times_bi20 = 0;
  static long times_bi18 = 0;
  static long times_bi17 = 0;
  static long times_bi14 = 0;
  static long times_bi10 = 0;
  static long helvetica_mr34 = 0;
  static long helvetica_mr24 = 0;
  static long helvetica_mr20 = 0;
  static long helvetica_mr18 = 0;
  static long helvetica_mr17 = 0;
  static long helvetica_mr14 = 0;
  static long helvetica_mr12 = 0;
  static long helvetica_mr10 = 0;
  static long helvetica_mi34 = 0;
  static long helvetica_mi24 = 0;
  static long helvetica_mi20 = 0;
  static long helvetica_mi18 = 0;
  static long helvetica_mi17 = 0;
  static long helvetica_mi14 = 0;
  static long helvetica_mi10 = 0;
  static long helvetica_br34 = 0;
  static long helvetica_br25 = 0;
  static long helvetica_br24 = 0;
  static long helvetica_br20 = 0;
  static long helvetica_br20l = 0;
  static long helvetica_br18 = 0;
  static long helvetica_br17 = 0;
  static long helvetica_br17s = 0;
  static long helvetica_br14 = 0;
  static long helvetica_br12 = 0;
  static long helvetica_br10 = 0;
  static long helvetica_br8 = 0;
  static long helvetica_bi34 = 0;
  static long helvetica_bi24 = 0;
  static long helvetica_bi20 = 0;
  static long helvetica_bi18 = 0;
  static long helvetica_bi17 = 0;
  static long helvetica_bi14 = 0;
  static long helvetica_bi10 = 0;
  static long newcentury_mr34 = 0;
  static long newcentury_mr24 = 0;
  static long newcentury_mr20 = 0;
  static long newcentury_mr18 = 0;
  static long newcentury_mr17 = 0;
  static long newcentury_mr14 = 0;
  static long newcentury_mr12 = 0;
  static long newcentury_mr10 = 0;
  static long newcentury_mi34 = 0;
  static long newcentury_mi24 = 0;
  static long newcentury_mi20 = 0;
  static long newcentury_mi18 = 0;
  static long newcentury_mi17 = 0;
  static long newcentury_mi14 = 0;
  static long newcentury_mi10 = 0;
  static long newcentury_br34 = 0;
  static long newcentury_br25 = 0;
  static long newcentury_br24 = 0;
  static long newcentury_br20 = 0;
  static long newcentury_br20l = 0;
  static long newcentury_br18 = 0;
  static long newcentury_br17 = 0;
  static long newcentury_br17s = 0;
  static long newcentury_br14 = 0;
  static long newcentury_br12 = 0;
  static long newcentury_br10 = 0;
  static long newcentury_br8 = 0;
  static long newcentury_bi34 = 0;
  static long newcentury_bi24 = 0;
  static long newcentury_bi20 = 0;
  static long newcentury_bi18 = 0;
  static long newcentury_bi17 = 0;
  static long newcentury_bi14 = 0;
  static long newcentury_bi10 = 0;
  static long lucidabright_mr34 = 0;
  static long lucidabright_mr24 = 0;
  static long lucidabright_mr20 = 0;
  static long lucidabright_mr18 = 0;
  static long lucidabright_mr17 = 0;
  static long lucidabright_mr14 = 0;
  static long lucidabright_mr12 = 0;
  static long lucidabright_mr10 = 0;
  static long lucidabright_mi34 = 0;
  static long lucidabright_mi24 = 0;
  static long lucidabright_mi20 = 0;
  static long lucidabright_mi18 = 0;
  static long lucidabright_mi17 = 0;
  static long lucidabright_mi14 = 0;
  static long lucidabright_mi10 = 0;
  static long lucidabright_br34 = 0;
  static long lucidabright_br25 = 0;
  static long lucidabright_br24 = 0;
  static long lucidabright_br20 = 0;
  static long lucidabright_br20l = 0;
  static long lucidabright_br18 = 0;
  static long lucidabright_br17 = 0;
  static long lucidabright_br17s = 0;
  static long lucidabright_br14 = 0;
  static long lucidabright_br12 = 0;
  static long lucidabright_br10 = 0;
  static long lucidabright_br8 = 0;
  static long lucidabright_bi34 = 0;
  static long lucidabright_bi24 = 0;
  static long lucidabright_bi20 = 0;
  static long lucidabright_bi18 = 0;
  static long lucidabright_bi17 = 0;
  static long lucidabright_bi14 = 0;
  static long lucidabright_bi10 = 0;
  static long lucidatypewriter_mr34 = 0;
  static long lucidatypewriter_mr24 = 0;
  static long lucidatypewriter_mr20l = 0;
  static long lucidatypewriter_mr20 = 0;
  static long lucidatypewriter_mr18 = 0;
  static long lucidatypewriter_mr17 = 0;
  static long lucidatypewriter_mr14 = 0;
  static long lucidatypewriter_mr12 = 0;
  static long lucidatypewriter_mr10 = 0;
  static long lucidatypewriter_br34 = 0;
  static long lucidatypewriter_br24 = 0;
  static long lucidatypewriter_br20l = 0;
  static long lucidatypewriter_br20 = 0;
  static long lucidatypewriter_br18 = 0;
  static long lucidatypewriter_br17 = 0;
  static long lucidatypewriter_br14 = 0;
  static long lucidatypewriter_br12 = 0;
  static long lucidatypewriter_br10 = 0;
  static long symbol_mr34 = 0;
  static long symbol_mr25 = 0;
  static long symbol_mr24 = 0;
  static long symbol_mr20 = 0;
  static long symbol_mr20l = 0;
  static long symbol_mr18 = 0;
  static long symbol_mr17 = 0;
  static long symbol_mr17s = 0;
  static long symbol_mr14 = 0;
  static long symbol_mr12 = 0;
  static long symbol_mr10 = 0;
  static long symbol_mr8 = 0;

  /* If true, then init some values and allow window repaint */
  if (refresh) {
    hw->html.font_base = pcc->cur_font_base;
    hw->html.font_family = pcc->cur_font_family;
  } else {
    hw->html.changing_font = 1;
  }

  err_count = 0;

#ifndef DISABLE_TRACE
  if (htmlwTrace)
    fprintf(stderr, "Setting font size = %d, family = %d\n",
	    pcc->cur_font_size, pcc->cur_font_family);
#endif

  if (pcc->cur_font_family == TIMES) {
   SetFixedFont(pcc->cur_font_size);

   switch (pcc->cur_font_size) {
    case 1:
      if (!times_mr10)
	times_mr10 = wrapFont("-adobe-times-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, times_mr10);
      if (!times_mi10)
	times_mi10 = wrapFont("-adobe-times-medium-i-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, times_mi10);
      if (!times_br10)
	times_br10 = wrapFont("-adobe-times-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, times_br10);
      if (!times_bi10)
	times_bi10 = wrapFont("-adobe-times-bold-i-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, times_bi10);
      if (!times_br18)
	times_br18 = wrapFont("-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, times_br18);
      if (!times_br17)
	times_br17 = wrapFont2("-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, times_br17);
      if (!times_br14)
	times_br14 = wrapFont("-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, times_br14);
      if (!times_br12)
	times_br12 = wrapFont("-adobe-times-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, times_br12);
      XmxSetArg(WbNheader5Font, times_br10);
      if (!times_br8)
	times_br8 = wrapFont("-adobe-times-bold-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, times_br8);
      XmxSetArg(WbNaddressFont, times_mi10);

      break;
    case 2:
      if (!times_mr14)
	times_mr14 = wrapFont("-adobe-times-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, times_mr14);
      if (!times_mi14)
	times_mi14 = wrapFont("-adobe-times-medium-i-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, times_mi14);
      if (!times_br14)
	times_br14 = wrapFont("-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, times_br14);
      if (!times_bi14)
	times_bi14 = wrapFont("-adobe-times-bold-i-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, times_bi14);
      if (!times_br20)
	times_br20 = wrapFont2("-adobe-times-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, times_br20);
      if (!times_br18)
	times_br18 = wrapFont("-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, times_br18);
      XmxSetArg(WbNheader3Font, times_br14);
      if (!times_br12)
	times_br12 = wrapFont("-adobe-times-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, times_br12);
      if (!times_br10)
	times_br10 = wrapFont("-adobe-times-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, times_br10);
      if (!times_br8)
	times_br8 = wrapFont("-adobe-times-bold-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, times_br8);
      XmxSetArg(WbNaddressFont, times_mi14);

      break;
    case 3:
      if (!times_mr17)
	times_mr17 = wrapFont2("-adobe-times-medium-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-times-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, times_mr17);
      if (!times_mi17)
	times_mi17 = wrapFont2("-adobe-times-medium-i-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-times-medium-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, times_mi17);
      if (!times_br17)
	times_br17 = wrapFont2("-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, times_br17);
      if (!times_bi17)
	times_bi17 = wrapFont2("-adobe-times-bold-i-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-times-bold-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, times_bi17);
      if (!times_br24)
	times_br24 = wrapFont("-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, times_br24);
      if (!times_br20)
	times_br20 = wrapFont2("-adobe-times-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, times_br20);
      XmxSetArg(WbNheader3Font, times_br17);
      if (!times_br14)
	times_br14 = wrapFont("-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, times_br14);
      if (!times_br12)
	times_br12 = wrapFont("-adobe-times-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, times_br12);
      if (!times_br10)
	times_br10 = wrapFont("-adobe-times-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, times_br10);
      XmxSetArg(WbNaddressFont, times_mi17);

      break;
    case 4:
      if (!times_mr18)
	times_mr18 = wrapFont("-adobe-times-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, times_mr18);
      if (!times_mi18)
	times_mi18 = wrapFont("-adobe-times-medium-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, times_mi18);
      if (!times_br18)
	times_br18 = wrapFont("-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, times_br18);
      if (!times_bi18)
	times_bi18 = wrapFont("-adobe-times-bold-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, times_bi18);
      if (!times_br24)
	times_br24 = wrapFont("-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, times_br24);
      if (!times_br20l)
	times_br20l = wrapFont2("-adobe-times-bold-r-normal-*-20-*-*-*-*-*-*-*",
			        "-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, times_br20l);
      XmxSetArg(WbNheader3Font, times_br18);
      if (!times_br17)
	times_br17 = wrapFont2("-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, times_br17);
      if (!times_br14)
	times_br14 = wrapFont("-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, times_br14);
      if (!times_br12)
	times_br12 = wrapFont("-adobe-times-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, times_br12);
      XmxSetArg(WbNaddressFont, times_mi18);

      break;
    case 5:
      if (!times_mr20)
	times_mr20 = wrapFont2("-adobe-times-medium-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-times-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, times_mr20);
      if (!times_mi20)
	times_mi20 = wrapFont2("-adobe-times-medium-i-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-times-medium-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, times_mi20);
      if (!times_br20)
	times_br20 = wrapFont2("-adobe-times-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, times_br20);
      if (!times_bi20)
	times_bi20 = wrapFont2("-adobe-times-bold-i-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-times-bold-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, times_bi20);
      if (!times_br25)
	times_br25 = wrapFont2("-adobe-times-bold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, times_br25);
      if (!times_br24)
	times_br24 = wrapFont("-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, times_br24);
      XmxSetArg(WbNheader3Font, times_br20);
      if (!times_br18)
	times_br18 = wrapFont("-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, times_br18);
      if (!times_br17)
	times_br17 = wrapFont2("-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, times_br17);
      if (!times_br14)
	times_br14 = wrapFont("-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, times_br14);
      XmxSetArg(WbNaddressFont, times_mi20);

      break;
    case 6:
      if (!times_mr24)
	times_mr24 = wrapFont("-adobe-times-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, times_mr24);
      if (!times_mi24)
	times_mi24 = wrapFont("-adobe-times-medium-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, times_mi24);
      if (!times_br24)
	times_br24 = wrapFont("-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, times_br24);
      if (!times_bi24)
	times_bi24 = wrapFont("-adobe-times-bold-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, times_bi24);
      if (!times_br34)
	times_br34 = wrapFont2("-adobe-times-bold-r-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, times_br34);
      if (!times_br25)
	times_br25 = wrapFont2("-adobe-times-bold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, times_br25);
      XmxSetArg(WbNheader3Font, times_br24);
      if (!times_br20)
	times_br20 = wrapFont2("-adobe-times-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, times_br20);
      if (!times_br18)
	times_br18 = wrapFont("-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, times_br18);
      if (!times_br17s)
	times_br17s = wrapFont2("-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*",
			        "-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, times_br17s);
      XmxSetArg(WbNaddressFont, times_mi24);

      break;
    case 7:
      if (!times_mr34)
	times_mr34 = wrapFont2("-adobe-times-medium-r-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-times-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, times_mr34);
      if (!times_mi34)
	times_mi34 = wrapFont2("-adobe-times-medium-i-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-times-medium-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, times_mi34);
      if (!times_br34)
	times_br34 = wrapFont2("-adobe-times-bold-r-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, times_br34);
      if (!times_bi34)
	times_bi34 = wrapFont2("-adobe-times-bold-i-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-times-bold-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, times_bi34);
      XmxSetArg(WbNheader1Font, times_br34);
      XmxSetArg(WbNheader2Font, times_br34);
      if (!times_br25)
	times_br25 = wrapFont2("-adobe-times-bold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, times_br25);
      if (!times_br24)
	times_br24 = wrapFont("-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, times_br24);
      if (!times_br20)
	times_br20 = wrapFont2("-adobe-times-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, times_br20);
      if (!times_br18)
	times_br18 = wrapFont("-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, times_br18);
      XmxSetArg(WbNaddressFont, times_mi34);

      break;
   }

  } else if (pcc->cur_font_family == HELVETICA) {
   SetFixedFont(pcc->cur_font_size);

   switch (pcc->cur_font_size) {
    case 1:
      if (!helvetica_mr10)
	helvetica_mr10 = wrapFont("-adobe-helvetica-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, helvetica_mr10);
      if (!helvetica_mi10)
	helvetica_mi10 = wrapFont("-adobe-helvetica-medium-o-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, helvetica_mi10);
      if (!helvetica_br10)
	helvetica_br10 = wrapFont("-adobe-helvetica-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, helvetica_br10);
      if (!helvetica_bi10)
	helvetica_bi10 = wrapFont("-adobe-helvetica-bold-o-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, helvetica_bi10);
      if (!helvetica_br18)
	helvetica_br18 = wrapFont("-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, helvetica_br18);
      if (!helvetica_br17)
	helvetica_br17 = wrapFont2("-adobe-helvetica-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, helvetica_br17);
      if (!helvetica_br14)
	helvetica_br14 = wrapFont("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, helvetica_br14);
      if (!helvetica_br12)
	helvetica_br12 = wrapFont("-adobe-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, helvetica_br12);
      XmxSetArg(WbNheader5Font, helvetica_br10);
      if (!helvetica_br8)
	helvetica_br8 = wrapFont("-adobe-helvetica-bold-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, helvetica_br8);
      XmxSetArg(WbNaddressFont, helvetica_mi10);

      break;
    case 2:
      if (!helvetica_mr14)
	helvetica_mr14 = wrapFont("-adobe-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, helvetica_mr14);
      if (!helvetica_mi14)
	helvetica_mi14 = wrapFont("-adobe-helvetica-medium-o-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, helvetica_mi14);
      if (!helvetica_br14)
	helvetica_br14 = wrapFont("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, helvetica_br14);
      if (!helvetica_bi14)
	helvetica_bi14 = wrapFont("-adobe-helvetica-bold-o-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, helvetica_bi14);
      if (!helvetica_br20)
	helvetica_br20 = wrapFont2("-adobe-helvetica-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, helvetica_br20);
      if (!helvetica_br18)
	helvetica_br18 = wrapFont("-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, helvetica_br18);
      XmxSetArg(WbNheader3Font, helvetica_br14);
      if (!helvetica_br12)
	helvetica_br12 = wrapFont("-adobe-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, helvetica_br12);
      if (!helvetica_br10)
	helvetica_br10 = wrapFont("-adobe-helvetica-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, helvetica_br10);
      if (!helvetica_br8)
	helvetica_br8 = wrapFont("-adobe-helvetica-bold-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, helvetica_br8);
      XmxSetArg(WbNaddressFont, helvetica_mi14);

      break;
    case 3:
      if (!helvetica_mr17)
	helvetica_mr17 = wrapFont2("-adobe-helvetica-medium-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-helvetica-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, helvetica_mr17);
      if (!helvetica_mi17)
	helvetica_mi17 = wrapFont2("-adobe-helvetica-medium-o-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-helvetica-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, helvetica_mi17);
      if (!helvetica_br17)
	helvetica_br17 = wrapFont2("-adobe-helvetica-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, helvetica_br17);
      if (!helvetica_bi17)
	helvetica_bi17 = wrapFont2("-adobe-helvetica-bold-o-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, helvetica_bi17);
      if (!helvetica_br24)
	helvetica_br24 = wrapFont("-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, helvetica_br24);
      if (!helvetica_br20)
	helvetica_br20 = wrapFont2("-adobe-helvetica-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, helvetica_br20);
      XmxSetArg(WbNheader3Font, helvetica_br17);
      if (!helvetica_br14)
	helvetica_br14 = wrapFont("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, helvetica_br14);
      if (!helvetica_br12)
	helvetica_br12 = wrapFont("-adobe-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, helvetica_br12);
      if (!helvetica_br10)
	helvetica_br10 = wrapFont("-adobe-helvetica-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, helvetica_br10);
      XmxSetArg(WbNaddressFont, helvetica_mi17);

      break;
    case 4:
      if (!helvetica_mr18)
	helvetica_mr18 = wrapFont("-adobe-helvetica-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, helvetica_mr18);
      if (!helvetica_mi18)
	helvetica_mi18 = wrapFont("-adobe-helvetica-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, helvetica_mi18);
      if (!helvetica_br18)
	helvetica_br18 = wrapFont("-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, helvetica_br18);
      if (!helvetica_bi18)
	helvetica_bi18 = wrapFont("-adobe-helvetica-bold-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, helvetica_bi18);
      if (!helvetica_br24)
	helvetica_br24 = wrapFont("-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, helvetica_br24);
      if (!helvetica_br20l)
	helvetica_br20l = wrapFont2("-adobe-helvetica-bold-r-normal-*-20-*-*-*-*-*-*-*",
			      "-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, helvetica_br20l);
      XmxSetArg(WbNheader3Font, helvetica_br18);
      if (!helvetica_br17)
	helvetica_br17 = wrapFont2("-adobe-helvetica-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, helvetica_br17);
      if (!helvetica_br14)
	helvetica_br14 = wrapFont("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, helvetica_br14);
      if (!helvetica_br12)
	helvetica_br12 = wrapFont("-adobe-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, helvetica_br12);
      XmxSetArg(WbNaddressFont, helvetica_mi18);

      break;
    case 5:
      if (!helvetica_mr20)
	helvetica_mr20 = wrapFont2("-adobe-helvetica-medium-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-helvetica-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, helvetica_mr20);
      if (!helvetica_mi20)
	helvetica_mi20 = wrapFont2("-adobe-helvetica-medium-o-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-helvetica-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, helvetica_mi20);
      if (!helvetica_br20)
	helvetica_br20 = wrapFont2("-adobe-helvetica-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, helvetica_br20);
      if (!helvetica_bi20)
	helvetica_bi20 = wrapFont2("-adobe-helvetica-bold-o-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, helvetica_bi20);
      if (!helvetica_br25)
	helvetica_br25 = wrapFont2("-adobe-helvetica-bold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, helvetica_br25);
      if (!helvetica_br24)
	helvetica_br24 = wrapFont("-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, helvetica_br24);
      XmxSetArg(WbNheader3Font, helvetica_br20);
      if (!helvetica_br18)
	helvetica_br18 = wrapFont("-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, helvetica_br18);
      if (!helvetica_br17)
	helvetica_br17 = wrapFont2("-adobe-helvetica-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, helvetica_br17);
      if (!helvetica_br14)
	helvetica_br14 = wrapFont("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, helvetica_br14);
      XmxSetArg(WbNaddressFont, helvetica_mi20);

      break;
    case 6:
      if (!helvetica_mr24)
	helvetica_mr24 = wrapFont("-adobe-helvetica-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, helvetica_mr24);
      if (!helvetica_mi24)
	helvetica_mi24 = wrapFont("-adobe-helvetica-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, helvetica_mi24);
      if (!helvetica_br24)
	helvetica_br24 = wrapFont("-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, helvetica_br24);
      if (!helvetica_bi24)
	helvetica_bi24 = wrapFont("-adobe-helvetica-bold-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, helvetica_bi24);
      if (!helvetica_br34)
	helvetica_br34 = wrapFont("-adobe-helvetica-bold-r-normal-*-34-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, helvetica_br34);
      if (!helvetica_br25)
	helvetica_br25 = wrapFont2("-adobe-helvetica-bold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, helvetica_br25);
      XmxSetArg(WbNheader3Font, helvetica_br24);
      if (!helvetica_br20)
	helvetica_br20 = wrapFont2("-adobe-helvetica-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, helvetica_br20);
      if (!helvetica_br18)
	helvetica_br18 = wrapFont("-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, helvetica_br18);
      if (!helvetica_br17s)
	helvetica_br17s = wrapFont2("-adobe-helvetica-bold-r-normal-*-17-*-*-*-*-*-*-*",
			        "-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, helvetica_br17s);
      XmxSetArg(WbNaddressFont, helvetica_mi24);

      break;
    case 7:
      if (!helvetica_mr34)
	helvetica_mr34 = wrapFont2("-adobe-helvetica-medium-r-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-helvetica-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, helvetica_mr34);
      if (!helvetica_mi34)
	helvetica_mi34 = wrapFont2("-adobe-helvetica-medium-o-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-helvetica-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, helvetica_mi34);
      if (!helvetica_br34)
	helvetica_br34 = wrapFont2("-adobe-helvetica-bold-r-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, helvetica_br34);
      if (!helvetica_bi34)
	helvetica_bi34 = wrapFont2("-adobe-helvetica-bold-o-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, helvetica_bi34);
      XmxSetArg(WbNheader1Font, helvetica_br34);
      XmxSetArg(WbNheader2Font, helvetica_br34);
      if (!helvetica_br25)
	helvetica_br25 = wrapFont2("-adobe-helvetica-bold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, helvetica_br25);
      if (!helvetica_br24)
	helvetica_br24 = wrapFont("-adobe-helvetica-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, helvetica_br24);
      if (!helvetica_br20)
	helvetica_br20 = wrapFont2("-adobe-helvetica-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, helvetica_br20);
      if (!helvetica_br18)
	helvetica_br18 = wrapFont("-adobe-helvetica-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, helvetica_br18);
      XmxSetArg(WbNaddressFont, helvetica_mi34);

      break;
   }

  } else if (pcc->cur_font_family == CENTURY) {
   SetFixedFont(pcc->cur_font_size);

   switch (pcc->cur_font_size) {
    case 1:
      if (!newcentury_mr10)
	newcentury_mr10 = wrapFont("-adobe-new century schoolbook-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, newcentury_mr10);
      if (!newcentury_mi10)
	newcentury_mi10 = wrapFont("-adobe-new century schoolbook-medium-i-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, newcentury_mi10);
      if (!newcentury_br10)
	newcentury_br10 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, newcentury_br10);
      if (!newcentury_bi10)
	newcentury_bi10 = wrapFont("-adobe-new century schoolbook-bold-i-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, newcentury_bi10);
      if (!newcentury_br18)
	newcentury_br18 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, newcentury_br18);
      if (!newcentury_br17)
	newcentury_br17 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, newcentury_br17);
      if (!newcentury_br14)
	newcentury_br14 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, newcentury_br14);
      if (!newcentury_br12)
	newcentury_br12 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, newcentury_br12);
      XmxSetArg(WbNheader5Font, newcentury_br10);
      if (!newcentury_br8)
	newcentury_br8 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, newcentury_br8);
      XmxSetArg(WbNaddressFont, newcentury_mi10);

      break;
    case 2:
      if (!newcentury_mr14)
	newcentury_mr14 = wrapFont("-adobe-new century schoolbook-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, newcentury_mr14);
      if (!newcentury_mi14)
	newcentury_mi14 = wrapFont("-adobe-new century schoolbook-medium-i-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, newcentury_mi14);
      if (!newcentury_br14)
	newcentury_br14 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, newcentury_br14);
      if (!newcentury_bi14)
	newcentury_bi14 = wrapFont("-adobe-new century schoolbook-bold-i-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, newcentury_bi14);
      if (!newcentury_br20)
	newcentury_br20 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, newcentury_br20);
      if (!newcentury_br18)
	newcentury_br18 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, newcentury_br18);
      XmxSetArg(WbNheader3Font, newcentury_br14);
      if (!newcentury_br12)
	newcentury_br12 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, newcentury_br12);
      if (!newcentury_br10)
	newcentury_br10 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, newcentury_br10);
      if (!newcentury_br8)
	newcentury_br8 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, newcentury_br8);
      XmxSetArg(WbNaddressFont, newcentury_mi14);

      break;
    case 3:
      if (!newcentury_mr17)
	newcentury_mr17 = wrapFont2("-adobe-new century schoolbook-medium-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, newcentury_mr17);
      if (!newcentury_mi17)
	newcentury_mi17 = wrapFont2("-adobe-new century schoolbook-medium-i-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-medium-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, newcentury_mi17);
      if (!newcentury_br17)
	newcentury_br17 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, newcentury_br17);
      if (!newcentury_bi17)
	newcentury_bi17 = wrapFont2("-adobe-new century schoolbook-bold-i-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, newcentury_bi17);
      if (!newcentury_br24)
	newcentury_br24 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, newcentury_br24);
      if (!newcentury_br20)
	newcentury_br20 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, newcentury_br20);
      XmxSetArg(WbNheader3Font, newcentury_br17);
      if (!newcentury_br14)
	newcentury_br14 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, newcentury_br14);
      if (!newcentury_br12)
	newcentury_br12 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, newcentury_br12);
      if (!newcentury_br10)
	newcentury_br10 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, newcentury_br10);
      XmxSetArg(WbNaddressFont, newcentury_mi17);

      break;
    case 4:
      if (!newcentury_mr18)
	newcentury_mr18 = wrapFont("-adobe-new century schoolbook-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, newcentury_mr18);
      if (!newcentury_mi18)
	newcentury_mi18 = wrapFont("-adobe-new century schoolbook-medium-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, newcentury_mi18);
      if (!newcentury_br18)
	newcentury_br18 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, newcentury_br18);
      if (!newcentury_bi18)
	newcentury_bi18 = wrapFont("-adobe-new century schoolbook-bold-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, newcentury_bi18);
      if (!newcentury_br24)
	newcentury_br24 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, newcentury_br24);
      if (!newcentury_br20l)
	newcentury_br20l = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-20-*-*-*-*-*-*-*",
			      "-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, newcentury_br20l);
      XmxSetArg(WbNheader3Font, newcentury_br18);
      if (!newcentury_br17)
	newcentury_br17 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, newcentury_br17);
      if (!newcentury_br14)
	newcentury_br14 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, newcentury_br14);
      if (!newcentury_br12)
	newcentury_br12 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, newcentury_br12);
      XmxSetArg(WbNaddressFont, newcentury_mi18);

      break;
    case 5:
      if (!newcentury_mr20)
	newcentury_mr20 = wrapFont2("-adobe-new century schoolbook-medium-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, newcentury_mr20);
      if (!newcentury_mi20)
	newcentury_mi20 = wrapFont2("-adobe-new century schoolbook-medium-i-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-medium-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, newcentury_mi20);
      if (!newcentury_br20)
	newcentury_br20 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, newcentury_br20);
      if (!newcentury_bi20)
	newcentury_bi20 = wrapFont2("-adobe-new century schoolbook-bold-i-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, newcentury_bi20);
      if (!newcentury_br25)
	newcentury_br25 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, newcentury_br25);
      if (!newcentury_br24)
	newcentury_br24 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, newcentury_br24);
      XmxSetArg(WbNheader3Font, newcentury_br20);
      if (!newcentury_br18)
	newcentury_br18 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, newcentury_br18);
      if (!newcentury_br17)
	newcentury_br17 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, newcentury_br17);
      if (!newcentury_br14)
	newcentury_br14 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, newcentury_br14);
      XmxSetArg(WbNaddressFont, newcentury_mi20);

      break;
    case 6:
      if (!newcentury_mr24)
	newcentury_mr24 = wrapFont("-adobe-new century schoolbook-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, newcentury_mr24);
      if (!newcentury_mi24)
	newcentury_mi24 = wrapFont("-adobe-new century schoolbook-medium-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, newcentury_mi24);
      if (!newcentury_br24)
	newcentury_br24 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, newcentury_br24);
      if (!newcentury_bi24)
	newcentury_bi24 = wrapFont("-adobe-new century schoolbook-bold-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, newcentury_bi24);
      if (!newcentury_br34)
	newcentury_br34 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-34-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, newcentury_br34);
      if (!newcentury_br25)
	newcentury_br25 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, newcentury_br25);
      XmxSetArg(WbNheader3Font, newcentury_br24);
      if (!newcentury_br20)
	newcentury_br20 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, newcentury_br20);
      if (!newcentury_br18)
	newcentury_br18 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, newcentury_br18);
      if (!newcentury_br17s)
	newcentury_br17s = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-17-*-*-*-*-*-*-*",
			        "-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, newcentury_br17s);
      XmxSetArg(WbNaddressFont, newcentury_mi24);

      break;
    case 7:
      if (!newcentury_mr34)
	newcentury_mr34 = wrapFont2("-adobe-new century schoolbook-medium-r-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, newcentury_mr34);
      if (!newcentury_mi34)
	newcentury_mi34 = wrapFont2("-adobe-new century schoolbook-medium-i-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-medium-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, newcentury_mi34);
      if (!newcentury_br34)
	newcentury_br34 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, newcentury_br34);
      if (!newcentury_bi34)
	newcentury_bi34 = wrapFont2("-adobe-new century schoolbook-bold-i-normal-*-34-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, newcentury_bi34);
      XmxSetArg(WbNheader1Font, newcentury_br34);
      XmxSetArg(WbNheader2Font, newcentury_br34);
      if (!newcentury_br25)
	newcentury_br25 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, newcentury_br25);
      if (!newcentury_br24)
	newcentury_br24 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, newcentury_br24);
      if (!newcentury_br20)
	newcentury_br20 = wrapFont2("-adobe-new century schoolbook-bold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, newcentury_br20);
      if (!newcentury_br18)
	newcentury_br18 = wrapFont("-adobe-new century schoolbook-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, newcentury_br18);
      XmxSetArg(WbNaddressFont, newcentury_mi34);

      break;
   }

  } else if (pcc->cur_font_family == LUCIDA) {
   switch (pcc->cur_font_size) {
    case 1:
      if (!lucidabright_mr10)
	lucidabright_mr10 = wrapFont("-b&h-lucidabright-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, lucidabright_mr10);
      if (!lucidabright_mi10)
	lucidabright_mi10 = wrapFont("-b&h-lucidabright-medium-i-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, lucidabright_mi10);
      if (!lucidabright_br10)
	lucidabright_br10 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, lucidabright_br10);
      if (!lucidabright_bi10)
	lucidabright_bi10 = wrapFont("-b&h-lucidabright-demibold-i-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, lucidabright_bi10);
      if (!lucidatypewriter_mr10)
	lucidatypewriter_mr10 = wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, lucidatypewriter_mr10);
      if (!lucidatypewriter_br10)
	lucidatypewriter_br10 = wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, lucidatypewriter_br10);
      if (!courier_mo10)
	courier_mo10 = wrapFont("-adobe-courier-medium-o-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo10);
      if (!lucidabright_br18)
	lucidabright_br18 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, lucidabright_br18);
      if (!lucidabright_br17)
	lucidabright_br17 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, lucidabright_br17);
      if (!lucidabright_br14)
	lucidabright_br14 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, lucidabright_br14);
      if (!lucidabright_br12)
	lucidabright_br12 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, lucidabright_br12);
      XmxSetArg(WbNheader5Font, lucidabright_br10);
      if (!lucidabright_br8)
	lucidabright_br8 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, lucidabright_br8);
      XmxSetArg(WbNaddressFont, lucidabright_mi10);
      XmxSetArg(WbNplainFont, lucidatypewriter_mr10);
      XmxSetArg(WbNplainboldFont, lucidatypewriter_br10);
      XmxSetArg(WbNplainitalicFont, courier_mo10);
      if (!courier_mr8)
	courier_mr8 = wrapFont("-adobe-courier-medium-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr8);

      break;
    case 2:
      if (!lucidabright_mr14)
	lucidabright_mr14 = wrapFont("-b&h-lucidabright-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, lucidabright_mr14);
      if (!lucidabright_mi14)
	lucidabright_mi14 = wrapFont("-b&h-lucidabright-medium-i-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, lucidabright_mi14);
      if (!lucidabright_br14)
	lucidabright_br14 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, lucidabright_br14);
      if (!lucidabright_bi14)
	lucidabright_bi14 = wrapFont("-b&h-lucidabright-demibold-i-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, lucidabright_bi14);
      if (!lucidatypewriter_mr14)
	lucidatypewriter_mr14 = wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, lucidatypewriter_mr14);
      if (!lucidatypewriter_br14)
	lucidatypewriter_br14 = wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, lucidatypewriter_br14);
      if (!courier_mo14)
	courier_mo14 = wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo14);
      if (!lucidabright_br20)
	lucidabright_br20 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, lucidabright_br20);
      if (!lucidabright_br18)
	lucidabright_br18 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, lucidabright_br18);
      XmxSetArg(WbNheader3Font, lucidabright_br14);
      if (!lucidabright_br12)
	lucidabright_br12 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, lucidabright_br12);
      if (!lucidabright_br10)
	lucidabright_br10 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, lucidabright_br10);
      if (!lucidabright_br8)
	lucidabright_br8 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, lucidabright_br8);
      XmxSetArg(WbNaddressFont, lucidabright_mi14);
      if (!lucidatypewriter_mr12)
	lucidatypewriter_mr12 = wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, lucidatypewriter_mr12);
      if (!lucidatypewriter_br12)
	lucidatypewriter_br12 = wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, lucidatypewriter_br12);
      if (!courier_mo12)
	courier_mo12 = wrapFont("-adobe-courier-medium-o-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo12);
      if (!courier_mr10)
	courier_mr10 = wrapFont("-adobe-courier-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr10);

      break;
    case 3:
      if (!lucidabright_mr17)
	lucidabright_mr17 = wrapFont2("-b&h-lucidabright-medium-r-normal-*-17-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, lucidabright_mr17);
      if (!lucidabright_mi17)
	lucidabright_mi17 = wrapFont2("-b&h-lucidabright-medium-i-normal-*-17-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-medium-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, lucidabright_mi17);
      if (!lucidabright_br17)
	lucidabright_br17 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, lucidabright_br17);
      if (!lucidabright_bi17)
	lucidabright_bi17 = wrapFont2("-b&h-lucidabright-demibold-i-normal-*-17-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, lucidabright_bi17);
      if (!lucidatypewriter_mr17)
	lucidatypewriter_mr17 = wrapFont2("-b&h-lucidatypewriter-medium-r-normal-*-17-*-*-*-*-*-*-*",
				 "-b&h-lucidatypewriter-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, lucidatypewriter_mr17);
      if (!lucidatypewriter_br17)
	lucidatypewriter_br17 = wrapFont2("-b&h-lucidatypewriter-bold-r-normal-*-17-*-*-*-*-*-*-*",
				 "-b&h-lucidatypewriter-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, lucidatypewriter_br17);
      if (!courier_mo17)
	courier_mo17 = wrapFont2("-adobe-courier-medium-o-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo17);
      if (!lucidabright_br24)
	lucidabright_br24 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, lucidabright_br24);
      if (!lucidabright_br20)
	lucidabright_br20 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, lucidabright_br20);
      XmxSetArg(WbNheader3Font, lucidabright_br17);
      if (!lucidabright_br14)
	lucidabright_br14 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, lucidabright_br14);
      if (!lucidabright_br12)
	lucidabright_br12 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, lucidabright_br12);
      if (!lucidabright_br10)
	lucidabright_br10 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, lucidabright_br10);
      XmxSetArg(WbNaddressFont, lucidabright_mi17);
      if (!lucidatypewriter_mr14)
	lucidatypewriter_mr14 = wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, lucidatypewriter_mr14);
      if (!lucidatypewriter_br14)
	lucidatypewriter_br14 = wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, lucidatypewriter_br14);
      if (!courier_mo14)
	courier_mo14 = wrapFont("-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo14);
      if (!courier_mr12)
	courier_mr12 = wrapFont("-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr12);

      break;
    case 4:
      if (!lucidabright_mr18)
	lucidabright_mr18 = wrapFont("-b&h-lucidabright-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, lucidabright_mr18);
      if (!lucidabright_mi18)
	lucidabright_mi18 = wrapFont("-b&h-lucidabright-medium-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, lucidabright_mi18);
      if (!lucidabright_br18)
	lucidabright_br18 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, lucidabright_br18);
      if (!lucidabright_bi18)
	lucidabright_bi18 = wrapFont("-b&h-lucidabright-demibold-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, lucidabright_bi18);
      if (!lucidatypewriter_mr18)
	lucidatypewriter_mr18 = wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, lucidatypewriter_mr18);
      if (!lucidatypewriter_br18)
	lucidatypewriter_br18 = wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, lucidatypewriter_br18);
      if (!courier_mo18)
	courier_mo18 = wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo18);
      if (!lucidabright_br24)
	lucidabright_br24 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, lucidabright_br24);
      if (!lucidabright_br20l)
	lucidabright_br20l = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-20-*-*-*-*-*-*-*",
			      "-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, lucidabright_br20l);
      XmxSetArg(WbNheader3Font, lucidabright_br18);
      if (!lucidabright_br17)
	lucidabright_br17 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, lucidabright_br17);
      if (!lucidabright_br14)
	lucidabright_br14 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, lucidabright_br14);
      if (!lucidabright_br12)
	lucidabright_br12 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, lucidabright_br12);
      XmxSetArg(WbNaddressFont, lucidabright_mi18);
      if (!lucidatypewriter_mr17)
	lucidatypewriter_mr17 = wrapFont2("-b&h-lucidatypewriter-medium-r-normal-*-17-*-*-*-*-*-*-*",
				"-b&h-lucidatypewriter-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, lucidatypewriter_mr17);
      if (!lucidatypewriter_br17)
	lucidatypewriter_br17 = wrapFont2("-b&h-lucidatypewriter-bold-r-normal-*-17-*-*-*-*-*-*-*",
				"-b&h-lucidatypewriter-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, lucidatypewriter_br17);
      if (!courier_mo17)
	courier_mo17 = wrapFont2("-adobe-courier-medium-o-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo17);
      if (!courier_mr14)
	courier_mr14 = wrapFont("-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr14);

      break;
    case 5:
      if (!lucidabright_mr20)
	lucidabright_mr20 = wrapFont2("-b&h-lucidabright-medium-r-normal-*-20-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, lucidabright_mr20);
      if (!lucidabright_mi20)
	lucidabright_mi20 = wrapFont2("-b&h-lucidabright-medium-i-normal-*-20-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-medium-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, lucidabright_mi20);
      if (!lucidabright_br20)
	lucidabright_br20 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, lucidabright_br20);
      if (!lucidabright_bi20)
	lucidabright_bi20 = wrapFont2("-b&h-lucidabright-demibold-i-normal-*-20-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-i-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, lucidabright_bi20);
      if (!lucidatypewriter_mr20)
	lucidatypewriter_mr20 = wrapFont2("-b&h-lucidatypewriter-medium-r-normal-*-20-*-*-*-*-*-*-*",
				 "-b&h-lucidatypewriter-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, lucidatypewriter_mr20);
      if (!lucidatypewriter_br20)
	lucidatypewriter_br20 = wrapFont2("-b&h-lucidatypewriter-bold-r-normal-*-20-*-*-*-*-*-*-*",
				 "-b&h-lucidatypewriter-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, lucidatypewriter_br20);
      if (!courier_mo20)
	courier_mo20 = wrapFont2("-adobe-courier-medium-o-normal-*-20-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo20);
      if (!lucidabright_br25)
	lucidabright_br25 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, lucidabright_br25);
      if (!lucidabright_br24)
	lucidabright_br24 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, lucidabright_br24);
      XmxSetArg(WbNheader3Font, lucidabright_br20);
      if (!lucidabright_br18)
	lucidabright_br18 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, lucidabright_br18);
      if (!lucidabright_br17)
	lucidabright_br17 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-17-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, lucidabright_br17);
      if (!lucidabright_br14)
	lucidabright_br14 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, lucidabright_br14);
      XmxSetArg(WbNaddressFont, lucidabright_mi20);
      if (!lucidatypewriter_mr18)
	lucidatypewriter_mr18 = wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, lucidatypewriter_mr18);
      if (!lucidatypewriter_br18)
	lucidatypewriter_br18 = wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, lucidatypewriter_br18);
      if (!courier_mo18)
	courier_mo18 = wrapFont("-adobe-courier-medium-o-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo18);
      if (!courier_mr17)
	courier_mr17 = wrapFont2("-adobe-courier-medium-r-normal-*-17-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr17);

      break;
    case 6:
      if (!lucidabright_mr24)
	lucidabright_mr24 = wrapFont("-b&h-lucidabright-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, lucidabright_mr24);
      if (!lucidabright_mi24)
	lucidabright_mi24 = wrapFont("-b&h-lucidabright-medium-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, lucidabright_mi24);
      if (!lucidabright_br24)
	lucidabright_br24 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, lucidabright_br24);
      if (!lucidabright_bi24)
	lucidabright_bi24 = wrapFont("-b&h-lucidabright-demibold-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, lucidabright_bi24);
      if (!lucidatypewriter_mr24)
	lucidatypewriter_mr24 = wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, lucidatypewriter_mr24);
      if (!lucidatypewriter_br24)
	lucidatypewriter_br24 = wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, lucidatypewriter_br24);
      if (!courier_mo24)
	courier_mo24 = wrapFont("-adobe-courier-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo24);
      if (!lucidabright_br34)
	lucidabright_br34 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-34-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, lucidabright_br34);
      if (!lucidabright_br25)
	lucidabright_br25 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, lucidabright_br25);
      XmxSetArg(WbNheader3Font, lucidabright_br24);
      if (!lucidabright_br20)
	lucidabright_br20 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, lucidabright_br20);
      if (!lucidabright_br18)
	lucidabright_br18 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, lucidabright_br18);
      if (!lucidabright_br17s)
	lucidabright_br17s = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-17-*-*-*-*-*-*-*",
			        "-b&h-lucidabright-demibold-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, lucidabright_br17s);
      XmxSetArg(WbNaddressFont, lucidabright_mi24);
      if (!lucidatypewriter_mr20l)
	lucidatypewriter_mr20l = wrapFont2("-b&h-lucidatypewriter-medium-r-normal-*-20-*-*-*-*-*-*-*",
				  "-b&h-lucidatypewriter-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, lucidatypewriter_mr20l);
      if (!lucidatypewriter_br20l)
	lucidatypewriter_br20l = wrapFont2("-b&h-lucidatypewriter-bold-r-normal-*-20-*-*-*-*-*-*-*",
				  "-b&h-lucidatypewriter-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, lucidatypewriter_br20l);
      if (!courier_mo20l)
	courier_mo20l = wrapFont2("-adobe-courier-medium-o-normal-*-20-*-*-*-*-*-*-*",
				  "-adobe-courier-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo20l);
      if (!courier_mr20)
	courier_mr20 = wrapFont2("-adobe-courier-medium-r-normal-*-20-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr20);

      break;
    case 7:
      if (!lucidabright_mr34)
	lucidabright_mr34 = wrapFont2("-b&h-lucidabright-medium-r-normal-*-34-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, lucidabright_mr34);
      if (!lucidabright_mi34)
	lucidabright_mi34 = wrapFont2("-b&h-lucidabright-medium-i-normal-*-34-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-medium-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNitalicFont, lucidabright_mi34);
      if (!lucidabright_br34)
	lucidabright_br34 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-34-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldFont, lucidabright_br34);
      if (!lucidabright_bi34)
	lucidabright_bi34 = wrapFont2("-b&h-lucidabright-demibold-i-normal-*-34-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-i-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNboldItalicFont, lucidabright_bi34);
      if (!lucidatypewriter_mr34)
	lucidatypewriter_mr34 = wrapFont2("-b&h-lucidatypewriter-medium-r-normal-*-34-*-*-*-*-*-*-*",
				 "-b&h-lucidatypewriter-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedFont, lucidatypewriter_mr34);
      if (!lucidatypewriter_br34)
	lucidatypewriter_br34 = wrapFont2("-b&h-lucidatypewriter-bold-r-normal-*-34-*-*-*-*-*-*-*",
				 "-b&h-lucidatypewriter-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixedboldFont, lucidatypewriter_br34);
      if (!courier_mo34)
	courier_mo34 = wrapFont2("-adobe-courier-medium-o-normal-*-34-*-*-*-*-*-*-*",
				 "-adobe-courier-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNfixeditalicFont, courier_mo34);
      XmxSetArg(WbNheader1Font, lucidabright_br34);
      XmxSetArg(WbNheader2Font, lucidabright_br34);
      if (!lucidabright_br25)
	lucidabright_br25 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-25-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, lucidabright_br25);
      if (!lucidabright_br24)
	lucidabright_br24 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, lucidabright_br24);
      if (!lucidabright_br20)
	lucidabright_br20 = wrapFont2("-b&h-lucidabright-demibold-r-normal-*-20-*-*-*-*-*-*-*",
			       "-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, lucidabright_br20);
      if (!lucidabright_br18)
	lucidabright_br18 = wrapFont("-b&h-lucidabright-demibold-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, lucidabright_br18);
      XmxSetArg(WbNaddressFont, lucidabright_mi34);
      if (!lucidatypewriter_mr24)
	lucidatypewriter_mr24 = wrapFont("-b&h-lucidatypewriter-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, lucidatypewriter_mr24);
      if (!lucidatypewriter_br24)
	lucidatypewriter_br24 = wrapFont("-b&h-lucidatypewriter-bold-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainboldFont, lucidatypewriter_br24);
      if (!courier_mo24)
	courier_mo24 = wrapFont("-adobe-courier-medium-o-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainitalicFont, courier_mo24);
      if (!courier_mr20l)
	courier_mr20l = wrapFont2("-adobe-courier-medium-r-normal-*-20-*-*-*-*-*-*-*",
				  "-adobe-courier-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, courier_mr20l);

      break;
   }
   if (err_count > 9)
	lucida_missing = 1;

  } else if (pcc->cur_font_family == SYMBOL) {
   switch (pcc->cur_font_size) {
    case 1:
      if (!symbol_mr10)
	symbol_mr10 = wrapFont("-adobe-symbol-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, symbol_mr10);
      XmxSetArg(WbNitalicFont, symbol_mr10);
      XmxSetArg(WbNboldFont, symbol_mr10);
      XmxSetArg(WbNboldItalicFont, symbol_mr10);
      XmxSetArg(WbNfixedFont, symbol_mr10);
      XmxSetArg(WbNfixedboldFont, symbol_mr10);
      XmxSetArg(WbNfixeditalicFont, symbol_mr10);
      if (!symbol_mr18)
	symbol_mr18 = wrapFont("-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, symbol_mr18);
      if (!symbol_mr17)
	symbol_mr17 = wrapFont2("-adobe-symbol-medium-r-normal-*-17-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, symbol_mr17);
      if (!symbol_mr14)
	symbol_mr14 = wrapFont("-adobe-symbol-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, symbol_mr14);
      if (!symbol_mr12)
	symbol_mr12 = wrapFont("-adobe-symbol-medium-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, symbol_mr12);
      XmxSetArg(WbNheader5Font, symbol_mr10);
      if (!symbol_mr8)
	symbol_mr8 = wrapFont("-adobe-symbol-medium-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, symbol_mr8);
      XmxSetArg(WbNaddressFont, symbol_mr10);
      XmxSetArg(WbNplainFont, symbol_mr10);
      XmxSetArg(WbNplainboldFont, symbol_mr10);
      XmxSetArg(WbNplainitalicFont, symbol_mr10);
      XmxSetArg(WbNlistingFont, symbol_mr8);

      break;
    case 2:
      if (!symbol_mr14)
	symbol_mr14 = wrapFont("-adobe-symbol-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, symbol_mr14);
      XmxSetArg(WbNitalicFont, symbol_mr14);
      XmxSetArg(WbNboldFont, symbol_mr14);
      XmxSetArg(WbNboldItalicFont, symbol_mr14);
      XmxSetArg(WbNfixedFont, symbol_mr14);
      XmxSetArg(WbNfixedboldFont, symbol_mr14);
      XmxSetArg(WbNfixeditalicFont, symbol_mr14);
      if (!symbol_mr20)
	symbol_mr20 = wrapFont2("-adobe-symbol-medium-r-normal-*-20-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, symbol_mr20);
      if (!symbol_mr18)
	symbol_mr18 = wrapFont("-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, symbol_mr18);
      XmxSetArg(WbNheader3Font, symbol_mr14);
      if (!symbol_mr12)
	symbol_mr12 = wrapFont("-adobe-symbol-medium-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, symbol_mr12);
      if (!symbol_mr10)
	symbol_mr10 = wrapFont("-adobe-symbol-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, symbol_mr10);
      if (!symbol_mr8)
	symbol_mr8 = wrapFont("-adobe-symbol-medium-r-normal-*-8-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, symbol_mr8);
      XmxSetArg(WbNaddressFont, symbol_mr14);
      XmxSetArg(WbNplainFont, symbol_mr12);
      XmxSetArg(WbNplainboldFont, symbol_mr12);
      XmxSetArg(WbNplainitalicFont, symbol_mr12);
      XmxSetArg(WbNlistingFont, symbol_mr10);

      break;
    case 3:
      if (!symbol_mr17)
	symbol_mr17 = wrapFont2("-adobe-symbol-medium-r-normal-*-17-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, symbol_mr17);
      XmxSetArg(WbNitalicFont, symbol_mr17);
      XmxSetArg(WbNboldFont, symbol_mr17);
      XmxSetArg(WbNboldItalicFont, symbol_mr17);
      XmxSetArg(WbNfixedFont, symbol_mr17);
      XmxSetArg(WbNfixedboldFont, symbol_mr17);
      XmxSetArg(WbNfixeditalicFont, symbol_mr17);
      if (!symbol_mr24)
	symbol_mr24 = wrapFont("-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, symbol_mr24);
      if (!symbol_mr20)
	symbol_mr20 = wrapFont2("-adobe-symbol-medium-r-normal-*-20-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, symbol_mr20);
      XmxSetArg(WbNheader3Font, symbol_mr17);
      if (!symbol_mr14)
	symbol_mr14 = wrapFont("-adobe-symbol-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, symbol_mr14);
      if (!symbol_mr12)
	symbol_mr12 = wrapFont("-adobe-symbol-medium-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, symbol_mr12);
      if (!symbol_mr10)
	symbol_mr10 = wrapFont("-adobe-symbol-medium-r-normal-*-10-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, symbol_mr10);
      XmxSetArg(WbNaddressFont, symbol_mr17);
      XmxSetArg(WbNplainFont, symbol_mr14);
      XmxSetArg(WbNplainboldFont, symbol_mr14);
      XmxSetArg(WbNplainitalicFont, symbol_mr14);
      XmxSetArg(WbNlistingFont, symbol_mr12);

      break;
    case 4:
      if (!symbol_mr18)
	symbol_mr18 = wrapFont("-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, symbol_mr18);
      XmxSetArg(WbNitalicFont, symbol_mr18);
      XmxSetArg(WbNboldFont, symbol_mr18);
      XmxSetArg(WbNboldItalicFont, symbol_mr18);
      XmxSetArg(WbNfixedFont, symbol_mr18);
      XmxSetArg(WbNfixedboldFont, symbol_mr18);
      XmxSetArg(WbNfixeditalicFont, symbol_mr18);
      if (!symbol_mr24)
	symbol_mr24 = wrapFont("-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, symbol_mr24);
      if (!symbol_mr20l)
	symbol_mr20l = wrapFont2("-adobe-symbol-medium-r-normal-*-20-*-*-*-*-*-*-*",
			         "-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, symbol_mr20l);
      XmxSetArg(WbNheader3Font, symbol_mr18);
      if (!symbol_mr17)
	symbol_mr17 = wrapFont2("-adobe-symbol-medium-r-normal-*-17-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, symbol_mr17);
      if (!symbol_mr14)
	symbol_mr14 = wrapFont("-adobe-symbol-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, symbol_mr14);
      if (!symbol_mr12)
	symbol_mr12 = wrapFont("-adobe-symbol-medium-r-normal-*-12-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, symbol_mr12);
      XmxSetArg(WbNaddressFont, symbol_mr18);
      XmxSetArg(WbNplainFont, symbol_mr17);
      XmxSetArg(WbNplainboldFont, symbol_mr17);
      XmxSetArg(WbNplainitalicFont, symbol_mr17);
      XmxSetArg(WbNlistingFont, symbol_mr14);

      break;
    case 5:
      if (!symbol_mr20)
	symbol_mr20 = wrapFont2("-adobe-symbol-medium-r-normal-*-20-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, symbol_mr20);
      XmxSetArg(WbNitalicFont, symbol_mr20);
      XmxSetArg(WbNboldFont, symbol_mr20);
      XmxSetArg(WbNboldItalicFont, symbol_mr20);
      XmxSetArg(WbNfixedFont, symbol_mr20);
      XmxSetArg(WbNfixedboldFont, symbol_mr20);
      XmxSetArg(WbNfixeditalicFont, symbol_mr20);
      if (!symbol_mr25)
	symbol_mr25 = wrapFont2("-adobe-symbol-medium-r-normal-*-25-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, symbol_mr25);
      if (!symbol_mr24)
	symbol_mr24 = wrapFont("-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, symbol_mr24);
      XmxSetArg(WbNheader3Font, symbol_mr20);
      if (!symbol_mr18)
	symbol_mr18 = wrapFont("-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, symbol_mr18);
      if (!symbol_mr17)
	symbol_mr17 = wrapFont2("-adobe-symbol-medium-r-normal-*-17-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, symbol_mr17);
      if (!symbol_mr14)
	symbol_mr14 = wrapFont("-adobe-symbol-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, symbol_mr14);
      XmxSetArg(WbNaddressFont, symbol_mr20);
      XmxSetArg(WbNplainFont, symbol_mr18);
      XmxSetArg(WbNplainboldFont, symbol_mr18);
      XmxSetArg(WbNplainitalicFont, symbol_mr18);
      XmxSetArg(WbNlistingFont, symbol_mr17);

      break;
    case 6:
      if (!symbol_mr24)
	symbol_mr24 = wrapFont("-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, symbol_mr24);
      XmxSetArg(WbNitalicFont, symbol_mr24);
      XmxSetArg(WbNboldFont, symbol_mr24);
      XmxSetArg(WbNboldItalicFont, symbol_mr24);
      XmxSetArg(WbNfixedFont, symbol_mr24);
      XmxSetArg(WbNfixedboldFont, symbol_mr24);
      XmxSetArg(WbNfixeditalicFont, symbol_mr24);
      if (!symbol_mr34)
	symbol_mr34 = wrapFont("-adobe-symbol-medium-r-normal-*-34-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader1Font, symbol_mr34);
      if (!symbol_mr25)
	symbol_mr25 = wrapFont2("-adobe-symbol-medium-r-normal-*-25-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader2Font, symbol_mr25);
      XmxSetArg(WbNheader3Font, symbol_mr24);
      if (!symbol_mr20)
	symbol_mr20 = wrapFont2("-adobe-symbol-medium-r-normal-*-20-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, symbol_mr20);
      if (!symbol_mr18)
	symbol_mr18 = wrapFont("-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, symbol_mr18);
      if (!symbol_mr17s)
	symbol_mr17s = wrapFont2("-adobe-symbol-medium-r-normal-*-17-*-*-*-*-*-*-*",
			         "-adobe-symbol-medium-r-normal-*-14-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, symbol_mr17s);
      XmxSetArg(WbNaddressFont, symbol_mr24);
      if (!symbol_mr20l)
	symbol_mr20l = wrapFont2("-adobe-symbol-medium-r-normal-*-20-*-*-*-*-*-*-*",
				 "-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNplainFont, symbol_mr20l);
      XmxSetArg(WbNplainboldFont, symbol_mr20l);
      XmxSetArg(WbNplainitalicFont, symbol_mr20l);
      XmxSetArg(WbNlistingFont, symbol_mr20);

      break;
    case 7:
      if (!symbol_mr34)
	symbol_mr34 = wrapFont2("-adobe-symbol-medium-r-normal-*-34-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(XtNfont, symbol_mr34);
      XmxSetArg(WbNitalicFont, symbol_mr34);
      XmxSetArg(WbNboldFont, symbol_mr34);
      XmxSetArg(WbNboldItalicFont, symbol_mr34);
      XmxSetArg(WbNfixedFont, symbol_mr34);
      XmxSetArg(WbNfixedboldFont, symbol_mr34);
      XmxSetArg(WbNfixeditalicFont, symbol_mr34);
      XmxSetArg(WbNheader1Font, symbol_mr34);
      XmxSetArg(WbNheader2Font, symbol_mr34);
      if (!symbol_mr25)
	symbol_mr25 = wrapFont2("-adobe-symbol-medium-r-normal-*-25-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader3Font, symbol_mr25);
      if (!symbol_mr24)
	symbol_mr24 = wrapFont("-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader4Font, symbol_mr24);
      if (!symbol_mr20)
	symbol_mr20 = wrapFont2("-adobe-symbol-medium-r-normal-*-20-*-*-*-*-*-*-*",
			        "-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader5Font, symbol_mr20);
      if (!symbol_mr18)
	symbol_mr18 = wrapFont("-adobe-symbol-medium-r-normal-*-18-*-*-*-*-*-*-*");
      XmxSetArg(WbNheader6Font, symbol_mr18);
      XmxSetArg(WbNaddressFont, symbol_mr34);
      XmxSetArg(WbNplainFont, symbol_mr24);
      XmxSetArg(WbNplainboldFont, symbol_mr24);
      XmxSetArg(WbNplainitalicFont, symbol_mr24);
      if (!symbol_mr20l)
	symbol_mr20l = wrapFont2("-adobe-symbol-medium-r-normal-*-20-*-*-*-*-*-*-*",
				 "-adobe-symbol-medium-r-normal-*-24-*-*-*-*-*-*-*");
      XmxSetArg(WbNlistingFont, symbol_mr20l);

      break;
   }
  }

  XmxSetValues(w);

  /* Must do after XmxSetValues */
  if (lucida_missing) {
    static int did_this = 0;

    /* Only display message once */
    if (did_this)
      goto skip_it;
    did_this = 1;

    if (current_win) {
      XmxMakeWarningDialog(current_win->base,
			   "X server does not have Lucida font",
			   "Load Font Error");
    } else {
      fprintf(stderr,
	      "Load Font Error:  X Server does not have Lucida font.\n");
      fprintf(stderr, "                  Using fixed instead.\n");
    }
  }
 skip_it:

  switch (pcc->cur_font_type) {
    case FONT:
      pcc->cur_font = hw->html.font;
      break;
    case ITALIC_FONT:
      pcc->cur_font = hw->html.italic_font;
      break;
    case BOLD_FONT:
      pcc->cur_font = hw->html.bold_font;
      break;
    case BOLDITALIC_FONT:
      pcc->cur_font = hw->html.bolditalic_font;
      break;
    case FIXED_FONT:
      pcc->cur_font = hw->html.fixed_font;
      break;
    case FIXEDBOLD_FONT:
      pcc->cur_font = hw->html.fixedbold_font;
      break;
    case FIXEDITALIC_FONT:
      pcc->cur_font = hw->html.fixeditalic_font;
      break;
    case HEADER1_FONT:
      pcc->cur_font = hw->html.header1_font;
      break;
    case HEADER2_FONT:
      pcc->cur_font = hw->html.header2_font;
      break;
    case HEADER3_FONT:
      pcc->cur_font = hw->html.header3_font;
      break;
    case HEADER4_FONT:
      pcc->cur_font = hw->html.header4_font;
      break;
    case HEADER5_FONT:
      pcc->cur_font = hw->html.header5_font;
      break;
    case HEADER6_FONT:
      pcc->cur_font = hw->html.header6_font;
      break;
    case ADDRESS_FONT:
      pcc->cur_font = hw->html.address_font;
      break;
    case PLAIN_FONT:
      pcc->cur_font = hw->html.plain_font;
      break;
    case PLAINBOLD_FONT:
      pcc->cur_font = hw->html.plainbold_font;
      break;
    case PLAINITALIC_FONT:
      pcc->cur_font = hw->html.plainitalic_font;
      break;
    case LISTING_FONT:
      pcc->cur_font = hw->html.listing_font;
  }
  hw->html.changing_font = 0;
  return;
}
