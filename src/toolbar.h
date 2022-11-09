/* Copyright (C) 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

/* Toolbar Stuff */

struct toolbar {
    Widget w;
    int gray;
};

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

struct tool {
    char *text;            /* Button text */
    char *long_text;       /* Long button text */
    char *label;           /* Tracker label */
    int action;            /* mo_* for menubar dispatcher */
    Pixmap *image;         /* The icon */
    Pixmap *greyimage;     /* The greyed icon or NULL */
    int toolset;           /* Tool set it belongs too (0=always on) */
    int kioskok;           /* Allowed in kiosk mode */
};

extern void mo_tool_state(struct toolbar *t, int state, int index);

