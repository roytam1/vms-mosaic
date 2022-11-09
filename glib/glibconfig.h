/* glibconfig.h
 */

#ifndef __G_LIBCONFIG_H__
#define __G_LIBCONFIG_H__

#include "gmacros.h"
#include <limits.h>
#include <float.h>

G_BEGIN_DECLS

#define G_MINFLOAT	FLT_MIN
#define G_MAXFLOAT	FLT_MAX
#define G_MINDOUBLE	DBL_MIN
#define G_MAXDOUBLE	DBL_MAX
#define G_MINSHORT	SHRT_MIN
#define G_MAXSHORT	SHRT_MAX
#define G_MAXUSHORT	USHRT_MAX
#define G_MININT	INT_MIN
#define G_MAXINT	INT_MAX
#define G_MAXUINT	UINT_MAX
#define G_MINLONG	LONG_MIN
#define G_MAXLONG	LONG_MAX
#define G_MAXULONG	ULONG_MAX

#ifdef VAXC
/* Doesn't have this keyword */
#define signed
#endif

typedef signed char gint8;
typedef unsigned char guint8;
typedef signed short gint16;
typedef unsigned short guint16;
#define G_GINT16_MODIFIER "h"
#define G_GINT16_FORMAT "hi"
#define G_GUINT16_FORMAT "hu"
typedef signed int gint32;
typedef unsigned int guint32;
#define G_GINT32_MODIFIER ""
#define G_GINT32_FORMAT "i"
#define G_GUINT32_FORMAT "u"
#define G_HAVE_GINT64 1          /* deprecated, always true */

#ifndef VAX
G_GNUC_EXTENSION typedef signed long long gint64;
G_GNUC_EXTENSION typedef unsigned long long guint64;
#else
G_GNUC_EXTENSION typedef signed long gint64;
G_GNUC_EXTENSION typedef unsigned long guint64;
#endif

#define G_GINT64_CONSTANT(val)	(G_GNUC_EXTENSION (val##LL))
#define G_GINT64_MODIFIER "I64"
#define G_GINT64_FORMAT "li"
#define G_GUINT64_FORMAT "llu"

#define GLIB_SIZEOF_VOID_P 4
#define GLIB_SIZEOF_LONG   4
#define GLIB_SIZEOF_SIZE_T 4

typedef signed int gssize;
typedef unsigned int gsize;
#define G_GSIZE_MODIFIER ""
#define G_GSSIZE_FORMAT "i"
#define G_GSIZE_FORMAT "u"

#define G_MAXSIZE	G_MAXUINT

#define GPOINTER_TO_INT(p)	((gint)   (p))
#define GPOINTER_TO_UINT(p)	((guint)  (p))

#define GINT_TO_POINTER(i)	((gpointer)  (i))
#define GUINT_TO_POINTER(u)	((gpointer)  (u))

#ifdef NeXT /* @#%@! NeXTStep */
# define g_ATEXIT(proc)	(!atexit (proc))
#else
# define g_ATEXIT(proc)	(atexit (proc))
#endif

#define g_memmove(d,s,n) G_STMT_START { memmove ((d), (s), (n)); } G_STMT_END

#define GLIB_MAJOR_VERSION 2
#define GLIB_MINOR_VERSION 6
#define GLIB_MICRO_VERSION 6

#define G_OS_UNIX

/* #undef G_VA_COPY va_copy */

#ifdef	__cplusplus
#define	G_HAVE_INLINE	1
#elif !defined(VAXC)
#define G_HAVE_INLINE 1
#define G_HAVE___INLINE 1
#define G_HAVE___INLINE__ 1
#endif

#if !defined(VAXC) && !defined(__GNUC__)
#define G_HAVE_ISO_VARARGS 1
#endif

/* gcc-2.95.x supports both gnu style and ISO varargs, but if -ansi
 * is passed ISO vararg support is turned off, and there is no work
 * around to turn it on, so we unconditionally turn it off.
 */
#if __GNUC__ == 2 && __GNUC_MINOR__ == 95
#  undef G_HAVE_ISO_VARARGS
#endif

#ifndef VAXC
#define G_HAVE_GNUC_VARARGS 1
#endif

#define G_HAVE_GROWING_STACK 0


#define G_GNUC_INTERNAL

#ifndef VAXC
#define G_THREADS_ENABLED
#endif

typedef struct _GMutex* GStaticMutex;
#define G_STATIC_MUTEX_INIT NULL
#define g_static_mutex_get_mutex(mutex) \
  (g_static_mutex_get_mutex_impl_shortcut (mutex))
/* This represents a system thread as used by the implementation. An
 * alien implementaion, as loaded by g_thread_init can only count on
 * "sizeof (gpointer)" bytes to store their info. We however need more
 * for some of our native implementations. */
typedef union _GSystemThread GSystemThread;
union _GSystemThread
{
  char   data[8];
  double dummy_double;
  void  *dummy_pointer;
  long   dummy_long;
};

#define GINT16_TO_LE(val)	((gint16) (val))
#define GUINT16_TO_LE(val)	((guint16) (val))
#define GINT16_TO_BE(val)	((gint16) GUINT16_SWAP_LE_BE (val))
#define GUINT16_TO_BE(val)	(GUINT16_SWAP_LE_BE (val))
#define GINT32_TO_LE(val)	((gint32) (val))
#define GUINT32_TO_LE(val)	((guint32) (val))
#define GINT32_TO_BE(val)	((gint32) GUINT32_SWAP_LE_BE (val))
#define GUINT32_TO_BE(val)	(GUINT32_SWAP_LE_BE (val))
#define GINT64_TO_LE(val)	((gint64) (val))
#define GUINT64_TO_LE(val)	((guint64) (val))
#define GINT64_TO_BE(val)	((gint64) GUINT64_SWAP_LE_BE (val))
#define GUINT64_TO_BE(val)	(GUINT64_SWAP_LE_BE (val))
#define GLONG_TO_LE(val)	((glong) GINT32_TO_LE (val))
#define GULONG_TO_LE(val)	((gulong) GUINT32_TO_LE (val))
#define GLONG_TO_BE(val)	((glong) GINT32_TO_BE (val))
#define GULONG_TO_BE(val)	((gulong) GUINT32_TO_BE (val))
#define GINT_TO_LE(val)		((gint) GINT32_TO_LE (val))
#define GUINT_TO_LE(val)	((guint) GUINT32_TO_LE (val))
#define GINT_TO_BE(val)		((gint) GINT32_TO_BE (val))
#define GUINT_TO_BE(val)	((guint) GUINT32_TO_BE (val))
#define G_BYTE_ORDER G_LITTLE_ENDIAN

#define GLIB_SYSDEF_POLLIN =1
#define GLIB_SYSDEF_POLLOUT =4
#define GLIB_SYSDEF_POLLPRI =2
#define GLIB_SYSDEF_POLLHUP =16
#define GLIB_SYSDEF_POLLERR =8
#define GLIB_SYSDEF_POLLNVAL =32

#define G_MODULE_SUFFIX "exe"

typedef void * GPid;

/* Too long */
#define g_async_queue_timed_pop_unlocked g_async_queue_time_pop_unlocked
#define g_atomic_int_compare_and_exchange g_atomic_int_compare_and_exchan
#define g_atomic_pointer_compare_and_exchange g_atom_point_compare_and_exchan
#define g_key_file_get_locale_string_list g_key_fil_get_local_string_list
#define g_key_file_set_locale_string_list g_key_fil_set_local_string_list
#define g_main_context_find_source_by_id g_main_context_find_sourc_by_id
#define g_main_context_find_source_by_user_data g_main_cont_find_sour_by_use_da
#define g_main_context_find_source_by_funcs_user_data g_main_con_fin_sou_by_fun_us_da
#define g_source_remove_by_funcs_user_data g_source_remov_by_funcs_user_da
#define g_markup_parse_context_end_parse g_markup_parse_context_end_pars
#define g_markup_parse_context_get_element g_markup_parse_context_get_elem
#define g_markup_parse_context_get_position g_markup_parse_context_get_posi
#define g_option_context_set_help_enabled g_option_context_set_help_enabl
#define g_option_context_get_help_enabled g_option_context_get_help_enabl
#define g_option_context_set_ignore_unknown_options g_option_cont_set_ignor_unk_opt
#define g_option_context_get_ignore_unknown_options g_option_cont_get_ignor_unk_opt
#define g_option_context_add_main_entries g_option_context_add_main_entri
#define g_option_group_set_translate_func g_option_group_set_transla_func
#define g_option_group_set_translation_domain g_option_group_set_trans_domain
#define g_thread_pool_set_max_unused_threads g_thread_pool_set_max_unuse_thr
#define g_thread_pool_get_max_unused_threads g_thread_pool_get_max_unuse_thr
#define g_thread_pool_get_num_unused_threads g_thread_pool_get_num_unuse_thr
#define g_thread_pool_stop_unused_threads g_thread_pool_stop_unuse_thread
#define g_unicode_canonical_decomposition g_unicode_canonical_decompositi
#define g_thread_init_with_errorcheck_mutexes g_thread_init_with_errorcheck_m
G_END_DECLS

#endif /* GLIBCONFIG_H */
