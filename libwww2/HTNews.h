/*           Network News Transfer protocol module for the WWW library
                                     HTNEWS
                                             
 */
/* History:
**      26 Sep 90       Written TBL in Objective-C
**      29 Nov 91       Downgraded to C, for portable implementation.
**         Mar 96       Moved NewsArt here.  Upgraded back to C from
**		        Objectionable-C
*/

#ifndef HTNEWS_H
#define HTNEWS_H

#ifndef HTACCESS_H
#include "HTAccess.h"
#endif
#ifndef HTANCHOR_H
#include "HTAnchor.h"
#endif
#ifndef NEWSRC_H
#include "../src/newsrc.h"
#endif

extern HTProtocol HTNews;

extern void HTSetNewsHost(WWW_CONST char *value);
extern WWW_CONST char *HTGetNewsHost();

extern char *HTNewsHost;
extern int newsShowAllGroups;
extern int newsShowReadGroups;
extern int ConfigView;
extern int newsGotList;
extern char *NewsGroup;
extern newsgroup_t *NewsGroupS;
extern int newsShowAllArticles;

void HTSetNewsConfig(int, int, int, int, int, int, int, int);
void news_index(char *);
void news_next(char *);
void news_nextt(char *);
void news_prev(char *);
void news_prevt(char *);
void news_status(char *, int *, int *, int *, int *, int *);
extern int NNTPpost(char *from, char *subj, char *ref, char *groups, char *msg);
extern int NNTPgetarthdrs(char *art, char **ref, char **grp, char **subj,
			  char **from);
extern char *NNTPgetquoteline(char *art);

#define NO_CHANGE -1

/* Thread Chain Structure */
typedef struct NEWSART {
    struct NEWSART *prev, *next, *prevt, *nextt;    /* Article List pointers */
    char *FirstRef, *LastRef;                       /* Thread List pointers */
    long num;                                       /* Article Header Info */
    char *ID;
    char *SUBJ;
    char *FROM;
} NewsArt;

extern NewsArt *CurrentArt;

#endif  /* HTNEWS_H */
