/* Copyright (C) 2006 - The VMS Mosaic Project */

#ifndef LIBHTMLW_HTMLWIDGETS_H
#define LIBHTMLW_HTMLWIDGETS_H

#define W_TEXTFIELD     0
#define W_CHECKBOX      1
#define W_RADIOBOX      2
#define W_PUSHBUTTON    3
#define W_PASSWORD      4
#define W_OPTIONMENU    5
#define W_TEXTAREA      6
#define W_LIST          7
#define W_HIDDEN        9

extern void traversal_forward(Widget w, XEvent *event,
                              String *params, Cardinal *num_params);
extern void traversal_back(Widget w, XEvent *event,
                           String *params, Cardinal *num_params);
extern void traversal_current(Widget w, XEvent *event,
                              String *params, Cardinal *num_params);
extern void traversal_end(Widget w, XEvent *event,
                          String *params, Cardinal *num_params);

#endif
