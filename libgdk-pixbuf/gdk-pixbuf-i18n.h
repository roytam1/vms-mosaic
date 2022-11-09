#ifndef __GDKPIXBUFINTL_H__
#define __GDKPIXBUFINTL_H__

#include "gdk-pixbuf_config.h"
#include "../glib/config.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) dgettext(GETTEXT_PACKAGE,String)
#ifndef VAXC
#define P_(String) dgettext(GETTEXT_PACKAGE "-properties",String)
#else
#define P_(String) dgettext("glib20-properties",String)
#endif
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS is disabled */
#define _(String) (String)
#define P_(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define ngettext(String,Plural,Count) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain) 
#endif

#endif
