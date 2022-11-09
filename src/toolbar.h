/* Copyright (C) 2004, 2005, 2006, 2007, 2008 - The VMS Mosaic Project */

/* Toolbar Stuff */

typedef struct mo_toolbar_rec {
    Widget w;
    int gray;
} mo_toolbar;

#define BTN_PREV 0
#define BTN_NEXT 1
#define BTN_STOP 14
#define BTN_PTHR 16
#define BTN_PART 17
#define BTN_NART 18
#define BTN_NTHR 19
#define BTN_POST 20
#define BTN_FOLLOW 21
/* Array size */
#define BTN_COUNT 25

typedef struct mo_tool_rec {
    char *text;            /* Button text */
    char *long_text;       /* Long button text */
    char *label;           /* Tracker label */
    int action;            /* mo_* for menubar dispatcher */
    Pixmap *image;         /* The icon */
    Pixmap *greyimage;     /* The greyed icon or NULL */
    int toolset;           /* Tool set it belongs too (0=always on) */
    int kioskok;           /* Allowed in kiosk mode */
} mo_tool;

