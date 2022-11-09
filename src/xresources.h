/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright (C) 1993, Board of Trustees of the University of Illinois      *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/

/* Copyright (C) 2003, 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

/* This document should be included in whatever source document
 * sets up the Intrinsics.  It is in a separate file so it doesn't
 * clutter up that file.  So sue me.
 */
#ifndef __MOSAIC_XRESOURCES_H__
#define __MOSAIC_XRESOURCES_H__

/* ----------------------------- X Resources ------------------------------ */
#define offset(x) XtOffset(AppDataPtr, x)

static XtResource resources[] = {

  { "appDefaultsVersion", "AppDefaultsVersion", XtRInt, sizeof(int),
    offset(app_defaults_version), XtRString, "1" },

  /* Default font choice from Options menu choices */
  { "defaultFontChoice", "DefaultFontChoice", XtRString, sizeof(char *),
    offset(default_font_choice), XtRString, "TimesRegular" },

  { "kiosk", "Kiosk", XtRBoolean, sizeof(Boolean),
    offset(kiosk), XtRString, "False" },

  { "kioskPrint", "KioskPrint", XtRBoolean, sizeof(Boolean), 
    offset(kioskPrint), XtRString, "False" },

  { "kioskNoExit", "KioskNoExit", XtRBoolean, sizeof(Boolean),
    offset(kioskNoExit), XtRString, "False" },

  /* Whether Mosaic reads and writes global history from 
   * ~/.mosaic-global-history
   * and thus provides persistent history tracking. */
  { "useGlobalHistory", "UseGlobalHistory", XtRBoolean, sizeof(Boolean),
    offset(use_global_history), XtRString, "True" },

  /* Whether titles will be displayed wherever URL's are normally displayed. */
  { "displayURLsNotTitles", "DisplayURLsNotTitles", XtRBoolean, sizeof(Boolean),
    offset(display_urls_not_titles), XtRString, "False" },

  /* Default width for a Document View window.  This will change as windows
   * are cloned. */
  { "defaultWidth", "DefaultWidth", XtRInt, sizeof(int),
    offset(default_width), XtRString, "740" },

  /* Default height for a Document View window. */
  { "defaultHeight", "DefaultHeight", XtRInt, sizeof(int),
    offset(default_height), XtRString, "840" },

  /* Startup document. */
  { "homeDocument", "HomeDocument", XtRString, sizeof(char *),
    offset(home_document), XtRString, HOME_PAGE_DEFAULT },

  { "confirmExit", "ConfirmExit", XtRBoolean, sizeof(Boolean),
    offset(confirm_exit), XtRString, "True" },

  /* THIS USED TO BE mailCommand BUT IS NOW sendmailCommand. */
  { "sendmailCommand", "SendmailCommand", XtRString, sizeof(char *),
#ifdef __bsdi__
#ifndef VMS
    offset(sendmail_command), XtRString, "/usr/sbin/sendmail -t" },
#else
    offset(sendmail_command), XtRString, "Mail$$" },
#endif /* VMS, BSN */
#else
    offset(sendmail_command), XtRString, "/usr/lib/sendmail -t" },
#endif

  /* Ignore this.  Stealth feature. */
  { "mailFilterCommand", "MailFilterCommand", XtRString, sizeof(char *),
    offset(mail_filter_command), XtRString, NULL },

  { "printCommand", "PrintCommand", XtRString, sizeof(char *),
#if defined(VMS)     /* VMS, BSN, moved by PGE (put above __alpha) */
    offset(print_command), XtRString, PRINT_DEFAULT },
#else
    offset(print_command), XtRString, "lpr" },
#endif

  { "cciPort", "CCIPort", XtRInt, sizeof(int),
    offset(cciPort), XtRString, "0" },

  { "maxNumCCIConnect", "MaxNumCCIConnect", XtRInt, sizeof(int),
    offset(max_num_of_cci_connections), XtRString, "0" },

  { "loadLocalFile", "LoadLocalFile", XtRInt, sizeof(int),
    offset(load_local_file), XtRString, "1"},

  { "editCommand", "EditCommand", XtRString, sizeof(char *),
#ifndef VMS
    offset(edit_command), XtRString, NULL },
#else
    offset(edit_command), XtRString, EDITOR_DEFAULT },
#endif

  { "editCommandUseXterm", "EditCommandUseXterm", XtRBoolean, sizeof(Boolean),
    offset(edit_command_use_xterm), XtRString, "True" },

  { "xtermCommand", "XtermCommand", XtRString, sizeof(char *),
#ifdef _AIX
    offset(xterm_command), XtRString, "aixterm -v" },
#else
#ifndef VMS
    offset(xterm_command), XtRString, "xterm" },
#else
    offset(xterm_command), XtRString, 
    "Create/Term/NoDetach/Insert/Wait/Window=(Init=Window,Title=\"Mosaic DECterm\")"
  },
#endif
#endif

  { "globalHistoryFile", "GlobalHistoryFile", XtRString, sizeof(char *),
#ifndef VMS
    offset(global_history_file), XtRString, ".mosaic-global-history" },
#else
    offset(global_history_file), XtRString, "mosaic.global-history" },
#endif /* VMS, BSN */

  { "historyFile", "HistoryFile", XtRString, sizeof(char *),
#ifndef VMS
    offset(history_file), XtRString, ".mosaic-x-history" },
#else
    offset(history_file), XtRString, "mosaic.x-history" },
#endif

  { "defaultHotlistFile", "DefaultHotlistFile", XtRString, sizeof(char *),
#ifndef VMS
    offset(default_hotlist_file), XtRString, ".mosaic-hotlist-default" },
#else
    offset(default_hotlist_file), XtRString, "mosaic-hotlist-default" },
#endif /* VMS, BSN */

  { "defaultHotFile", "DefaultHotFile", XtRString, sizeof(char *),
#ifndef VMS
    offset(default_hot_file), XtRString, ".mosaic-hot.html" },
#else
    offset(default_hot_file), XtRString, "mosaic-hot.html" },
#endif /* VMS, PGE */

  { "addHotlistAddsRBM", "AddHotlistAddsRBM", XtRBoolean, sizeof(Boolean),
    offset(addHotlistAddsRBM), XtRString, "True" },

  { "addRBMAddsRBM", "AddRBMAddsRBM", XtRBoolean, sizeof(Boolean),
    offset(addRBMAddsRBM), XtRString, "True" },

  { "personalAnnotationDirectory", "PersonalAnnotationDirectory", XtRString, 
    sizeof(char *), offset(private_annotation_directory), XtRString,
#ifndef VMS
    ".mosaic-personal-annotations" },
#else
    "mosaic-annotations" },
#endif /* VMS, BSN */

  /* Whether selections should be fancy, by default. */
  { "fancySelections", "FancySelections", XtRBoolean, sizeof(Boolean),
    offset(default_fancy_selections), XtRString, "False" },

  { "defaultAuthorName", "DefaultAuthorName", XtRString, sizeof(char *),
    offset(default_author_name), XtRString, NULL },

  { "defaultAuthorEmail", "DefaultAuthorEmail", XtRString, sizeof(char *),
    offset(default_author_email), XtRString, NULL },

  { "signature", "Signature", XtRString, sizeof(char *),
    offset(signature), XtRString, NULL },

  { "annotationsOnTop", "AnnotationsOnTop", XtRBoolean, sizeof(Boolean),
    offset(annotations_on_top), XtRString, "False" },

  { "colorsPerInlinedImage", "ColorsPerInlinedImage", XtRInt, sizeof(int),
    offset(colors_per_inlined_image), XtRString, "50" },

  { "trackVisitedAnchors", "TrackVisitedAnchors", XtRBoolean, sizeof(Boolean),
    offset(track_visited_anchors), XtRString, "True" },

  { "uncompressCommand", "UncompressCommand", XtRString, sizeof(char *), 
#ifndef VMS
    offset(uncompress_command), XtRString, "uncompress" },
#else
    offset(uncompress_command), XtRString, "gzip -dfn" },
#endif /* Use gunzip for uncompress on VMS, BSN */

  { "gunzipCommand", "GunzipCommand", XtRString, 
#ifndef VMS
    sizeof(char *), offset(gunzip_command), XtRString, "gunzip -f -n" },
#else
    sizeof(char *), offset(gunzip_command), XtRString, "gzip -dfn" },
#endif  /* Use gzip in place of gunzip on VMS, PGE */

  { "recordCommandLocation", "RecordCommandLocation", XtRString, 
    sizeof(char *), offset(record_command_location), XtRString,
#if defined(__hpux)
    "/usr/audio/bin/srecorder" },
#else
#if defined(__sgi)
    "/usr/sbin/recordaiff" },
#else
#if defined (sun)
    "/usr/demo/SOUND/record" },
#else
    "/bin/true" },
#endif
#endif
#endif

  { "recordCommand", "RecordCommand", XtRString, sizeof(char *),
    offset(record_command), XtRString,
#ifdef __hpux
    "srecorder -au" },
#else
#if defined(__sgi)
    "recordaiff -n 1 -s 8 -r 8000" },
#else
#if defined (sun)
    "record" },
#else
    "true" },
#endif
#endif
#endif

  { "gethostbynameIsEvil", "GethostbynameIsEvil", XtRBoolean, sizeof(Boolean),
    offset(gethostbyname_is_evil), XtRString, "False" },

  { "autoPlaceWindows", "AutoPlaceWindows", XtRBoolean, sizeof(Boolean),
    offset(auto_place_windows), XtRString, "True" },

  { "initialWindowIconic", "InitialWindowIconic", XtRBoolean, sizeof(Boolean),
    offset(initial_window_iconic), XtRString, "False" },

  { "tmpDirectory", "TmpDirectory", XtRString, sizeof(char *),
    offset(tmp_directory), XtRString, NULL },

  { "annotationServer", "AnnotationServer", XtRString, sizeof(char *),
    offset(annotation_server), XtRString, NULL },

  { "catchPriorAndNext", "CatchPriorAndNext", XtRBoolean, sizeof(Boolean),
    offset(catch_prior_and_next), XtRString, "True" },

  { "fullHostname", "FullHostname", XtRString, sizeof(char *),
    offset(full_hostname), XtRString, NULL },

  { "reverseInlinedBitmapColors", "ReverseInlinedBitmapColors", XtRBoolean,
    sizeof(Boolean),
    offset(reverse_inlined_bitmap_colors), XtRString, "False" },

  { "confirmDeleteAnnotation", "ConfirmDeleteAnnotation", XtRBoolean,
    sizeof(Boolean), offset(confirm_delete_annotation), XtRString, "True" },

  { "tweakGopherTypes", "TweakGopherTypes", XtRBoolean, sizeof(Boolean),
    offset(tweak_gopher_types), XtRString, "True" },

  { "guiLayout", "GuiLayout", XtRString, sizeof(char *),
    offset(gui_layout), XtRString, NULL },

  /* --- New in 2.0 --- */
  { "trackPointerMotion", "TrackPointerMotion", XtRBoolean, sizeof(Boolean),
    offset(track_pointer_motion), XtRString, "True" },

  { "trackFullURLs", "TrackFullURLs", XtRBoolean, sizeof(Boolean),
    offset(track_full_url_names), XtRString, "True" },

  { "docsDirectory", "DocsDirectory", XtRString, sizeof(char *),
    offset(docs_directory), XtRString, NULL },

  { "documentsMenuSpecfile", "DocumentsMenuSpecfile", XtRString, sizeof(char *),
    offset(documents_menu_specfile), XtRString, 
#ifndef VMS
    "/usr/local/lib/mosaic/documents.menu" },
#else
    "mosaic_dir:documents.menu" },
#endif /* VMS, BSN */

  { "reloadReloadsImages", "ReloadReloadsImages", XtRBoolean, sizeof(Boolean),
    offset(reload_reloads_images), XtRString, "False" },

  { "reloadPragmaNoCache", "ReloadPragmaNoCache", XtRBoolean, sizeof(Boolean),
    offset(reload_pragma_no_cache), XtRString, "False" },

  { "simpleInterface", "SimpleInterface", XtRBoolean, sizeof(Boolean),
    offset(simple_interface), XtRString, "False" },

  { "maxWaisResponses", "MaxWaisResponses", XtRInt, sizeof(int),
    offset(max_wais_responses), XtRString, "200" },

  { "delayImageLoads", "DelayImageLoads", XtRBoolean, sizeof(Boolean),
    offset(delay_image_loads), XtRString, "False" },

  { "enableTables", "EnableTables", XtRBoolean, sizeof(Boolean),
    offset(enable_tables), XtRString, "True" },

  { "disableMiddleButton", "DisableMiddleButton", XtRBoolean, sizeof(Boolean),
    offset(disableMiddleButton), XtRString, "False" },

  { "useDefaultExtensionMap", "UseDefaultExtensionMap", XtRBoolean,
    sizeof(Boolean), offset(use_default_extension_map), XtRString, "True" },

  { "globalExtensionMap", "GlobalExtensionMap", XtRString, sizeof(char *),
    offset(global_extension_map), XtRString, GLOBAL_EXTENSION_MAP },

  { "personalExtensionMap", "PersonalExtensionMap", XtRString, sizeof(char *),
#ifndef VMS
    offset(personal_extension_map), XtRString, ".mime.types" },
#else
    offset(personal_extension_map), XtRString, "mime.types" },
#endif /* VMS, BSN */

  { "useDefaultTypeMap", "UseDefaultTypeMap", XtRBoolean, sizeof(Boolean),
    offset(use_default_type_map), XtRString, "True" },

  { "globalTypeMap", "GlobalTypeMap", XtRString, sizeof(char *),
    offset(global_type_map), XtRString, GLOBAL_TYPE_MAP },

  { "personalTypeMap", "PersonalTypeMap", XtRString, sizeof(char *),
#ifndef VMS
    offset(personal_type_map), XtRString, ".mailcap" },
#else
    offset(personal_type_map), XtRString, "mailcap." },
#endif /* VMS, BSN */

  { "twirlingTransferIcon", "TwirlingTransferIcon", XtRBoolean, sizeof(Boolean),
    offset(twirling_transfer_icon), XtRString, "True" },

  { "twirlIncrement", "TwirlIncrement", XtRInt, sizeof(int),
    offset(twirl_increment), XtRString, "4096" },

  { "securityIcon", "securityIcon", XtRBoolean, sizeof(Boolean),
    offset(securityIcon), XtRString, "True" },

  { "imageCacheSize", "ImageCacheSize", XtRInt, sizeof(int),
    offset(image_cache_size), XtRString, "2048" },

  { "protectMeFromMyself", "ProtectMeFromMyself", XtRBoolean, sizeof(Boolean),
    offset(protect_me_from_myself), XtRString, "False" },

  { "printMode", "PrintMode", XtRString, sizeof(char *),
    offset(print_mode), XtRString, "plain" },

  { "mailMode", "MailMode", XtRString, sizeof(char *),
    offset(mail_mode), XtRString, "plain" },

  { "saveMode", "SaveMode", XtRString, sizeof(char *),
    offset(save_mode), XtRString, "plain" },

  { "printBanners", "PrintBanners", XtRBoolean, sizeof(Boolean),
    offset(print_banners), XtRString, "True" },

  { "printFootnotes", "PrintFootnotes", XtRBoolean, sizeof(Boolean),
    offset(print_footnotes), XtRString, "True" },

  { "printPaperSizeUS", "PrintPaperSizeUS", XtRBoolean, sizeof(Boolean),
    offset(print_us), XtRString, "True" },

  { "useAFSKlog", "UseAFSKlog", XtRBoolean, sizeof(Boolean),
    offset(useAFSKlog), XtRString, "False" },

#ifdef __sgi
  { "debuggingMalloc", "DebuggingMalloc", XtRBoolean, sizeof(Boolean),
    offset(debugging_malloc), XtRString, "False" },
#endif

  /* New in 2.7 */
  { "clipping", "Clipping", XtRBoolean, sizeof(Boolean),
    offset(clipping), XtRString, "True" },

  { "maxClipTransitions", "MaxClipTransitions", XtRInt, sizeof(int),
    offset(max_clip_transitions), XtRString, "-1" },

  { "useLongTextNames", "UseLongTextNames", XtRBoolean, sizeof(Boolean),
    offset(long_text_names), XtRString, "False" },

  { "toolbarLayout", "ToolbarLayout", XtRString, sizeof(char *),
    offset(toolbar_layout), XtRString, NULL },

  { "installColormap", "InstallColormap", XtRBoolean, sizeof(Boolean),
    offset(instamap), XtRString, "False" },

  { "splashScreen", "SplashScreen", XtRBoolean, sizeof(Boolean),
    offset(splashScreen), XtRString, "True" },

  { "imageViewInternal", "ImageViewInternal", XtRBoolean, sizeof(Boolean),
    offset(imageViewInternal), XtRString, "True" },

  { "urlExpired", "UrlExpired", XtRInt, sizeof(int),
    offset(urlExpired), XtRString, "30" },

  { "httpTrace", "HttpTrace", XtRBoolean, sizeof(Boolean),
    offset(httpTrace), XtRString, "False" },

  { "www2Trace", "Www2Trace", XtRBoolean, sizeof(Boolean),
    offset(www2Trace), XtRString, "False" },

  { "htmlwTrace", "HtmlwTrace", XtRBoolean, sizeof(Boolean),
    offset(htmlwTrace), XtRString, "False" },

  { "cciTrace", "CciTrace", XtRBoolean, sizeof(Boolean),
    offset(cciTrace), XtRString, "False" },

  { "srcTrace", "SrcTrace", XtRBoolean, sizeof(Boolean),
    offset(srcTrace), XtRString, "False" },

  { "cacheTrace", "CacheTrace", XtRBoolean, sizeof(Boolean),
    offset(cacheTrace), XtRString, "False" },

  { "nutTrace", "NutTrace", XtRBoolean, sizeof(Boolean),
    offset(nutTrace), XtRString, "False" },

  { "tableTrace", "TableTrace", XtRBoolean, sizeof(Boolean),
    offset(tableTrace), XtRString, "False" },

  { "animateBusyIcon", "AnimateBusyIcon", XtRBoolean, sizeof(Boolean),
    offset(animateBusyIcon), XtRString, "True" },

  { "sendReferer", "SendReferer", XtRBoolean, sizeof(Boolean),
    offset(sendReferer), XtRString, "True" },

  { "sendAgent", "SendAgent", XtRBoolean, sizeof(Boolean),
    offset(sendAgent), XtRString, "True" },

  { "expandUrls", "ExpandUrls", XtRBoolean, sizeof(Boolean),
    offset(expandUrls), XtRString, "True" },

  { "expandUrlsWithName", "expandUrlsWithName", XtRBoolean, sizeof(Boolean),
    offset(expandUrlsWithName), XtRString, "True" },

  { "defaultProtocol", "DefaultProtocol", XtRString, sizeof(char *),
    offset(defaultProtocol), XtRString, "http" },

  { "meterForeground", "MeterForeground", XtRString, sizeof(char *),
    offset(meterForeground), XtRString, "#FFFF00000000" },

  { "meterBackground", "MeterBackground", XtRString, sizeof(char *),
    offset(meterBackground), XtRString, "#333366666666" },

  { "meterFontForeground", "MeterFontForeground", XtRString, sizeof(char *),
    offset(meterFontForeground), XtRString, "#FFFFFFFFFFFF" },

  { "meterFontBackground", "MeterFontBackground", XtRString, sizeof(char *),
    offset(meterFontBackground), XtRString, "#000000000000" },

  { "meter", "Meter", XtRBoolean, sizeof(Boolean),
    offset(use_meter), XtRString, "True" },

  { "backupDataFiles", "BackupDataFiles", XtRBoolean, sizeof(Boolean),
    offset(backup_files), XtRString, "True" },

  { "iconPixBasename", "IconPixBasename", XtRString, sizeof(char *),
    offset(pix_basename), XtRString, "default" },

  { "iconPixCount", "IconPixCount", XtRInt, sizeof(int),
    offset(pix_count), XtRString, "0" },

  { "acceptLanguage", "AcceptLanguage", XtRString, sizeof(char *),
    offset(acceptlanguage_str), XtRString, NULL },

  { "ftpTimeoutVal", "FtpTimeoutVal", XtRInt, sizeof(int),
    offset(ftp_timeout_val), XtRString, "90" },

  { "ftpRedial", "FtpRedial", XtRInt, sizeof(int),
    offset(ftpRedial), XtRString, "10" },

  { "ftpRedialSleep", "FtpRedialSleep", XtRInt, sizeof(int),
    offset(ftpRedialSleep), XtRString, "3" },

  { "ftpFilenameLength", "FtpFilenameLength", XtRInt, sizeof(int),
    offset(ftpFilenameLength), XtRString, "26" },

  { "ftpEllipsisLength", "FtpEllipsisLength", XtRInt, sizeof(int),
    offset(ftpEllipsisLength), XtRString, "3" },

  { "ftpEllipsisMode", "FtpEllipsisMode", XtRInt, sizeof(int),
    offset(ftpEllipsisMode), XtRString, "2" },

  { "titleIsWindowTitle", "TitleIsWindowTitle", XtRBoolean, sizeof(Boolean),
    offset(titleIsWindowTitle), XtRString, "True" },

  { "proxySpecfile", "ProxySpecfile", XtRString, sizeof(char *),
    offset(proxy_specfile), XtRString, 
#ifndef VMS
    "/usr/local/lib/mosaic/proxy" },
#else
    "mosaic_dir:proxy" },
#endif /* VMS, GEC */

  { "noproxySpecfile", "NoproxySpecfile", XtRString, sizeof(char *),
    offset(noproxy_specfile), XtRString,
#ifndef VMS
    "/usr/local/lib/mosaic/no_proxy" },
#else
    "mosaic_dir:no_proxy" },
#endif /* VMS, GEC */
  
  { "useScreenGamma", "UseScreenGamma", XtRBoolean, sizeof(Boolean),
    offset(useScreenGamma), XtRString, "False" },

  { "prefixVMSMail", "PrefixVMSMail", XtRString, sizeof(char *),
    offset(vms_mail_prefix), XtRString, MAIL_PREFIX_DEFAULT },

  { "backupFileVersions", "BackupFileVersions", XtRInt, sizeof(int),
    offset(backupFileVersions), XtRString, "1" },

  { "screenGamma", "ScreenGamma", XtRFloat, sizeof(float),
    offset(screen_gamma), XtRString, "2.2" },

  { "popupCascadeMappingDelay", "PopupCascadeMappingDelay", XtRInt, sizeof(int),
    offset(popupCascadeMappingDelay), XtRString, "500" },

  { "frameSupport", "FrameSupport", XtRBoolean, sizeof(Boolean),
    offset(frame_support), XtRString, "True" },
  
  /* New news stuff in 2.7b4 */
  { "newsUseThreadView", "NewsUseThreadView", XtRBoolean, sizeof(Boolean),
    offset(newsConfigView), XtRString, "True" },
  
  { "newsNoThreadJumping", "NewsNoThreadJumping", XtRBoolean, sizeof(Boolean),
    offset(newsNoThreadJumping), XtRString, "True" },

  { "newsShowAllGroups", "NewsShowAllGroups", XtRBoolean, sizeof(Boolean),
    offset(newsShowAllGroups), XtRString, "False" },

  { "newsShowReadGroups", "NewsShowReadGroups", XtRBoolean, sizeof(Boolean),
    offset(newsShowReadGroups), XtRString, "False" },

  { "newsShowAllArticles", "NewsShowAllArticles", XtRBoolean, sizeof(Boolean),
    offset(newsShowAllArticles), XtRString, "True" },

  { "newsUseBackgroundFlush", "NewsUseBackgroundFlush", XtRBoolean,
    sizeof(Boolean), offset(newsUseBackgroundFlush), XtRString, "True" },

  { "newsBackgroundFlushTime", "NewsBackgroundFlushTime", XtRInt, sizeof(int),
    offset(newsBackgroundFlushTime), XtRString, "300" },

  /* New in 2.7b5 */
  { "newsPrevIsPrevUnread", "NewsPrevIsPrevUnread", XtRBoolean, sizeof(Boolean),
    offset(newsPrevIsUnread), XtRString, "False" },

  { "newsNextIsNextUnread", "NewsNextIsNextUnread", XtRBoolean, sizeof(Boolean),
    offset(newsNextIsUnread), XtRString, "True" },

  { "newsNewsrcPrefix", "NewsNewsrcPrefix", XtRString, sizeof(char *),
#ifndef VMS
    offset(newsNewsrcPrefix), XtRString, ".newsrc" },
#else
    offset(newsNewsrcPrefix), XtRString, "news.rc" },
#endif

  { "newsUseNewsrc", "NewsUseNewsrc", XtRBoolean, sizeof(Boolean),
    offset(newsUseNewsrc), XtRString, "True" },

  { "newsSubjectWidth", "NewsSubjectWidth", XtRInt, sizeof(int),
    offset(newsSubjectWidth), XtRString, "38" },

  { "newsAuthorWidth", "NewsAuthorWidth", XtRInt, sizeof(int),
    offset(newsAuthorWidth), XtRString, "30" },

  { "focusFollowsMouse", "FocusFollowsMouse", XtRBoolean, sizeof(Boolean),
    offset(focusFollowsMouse), XtRString, "False" },

  { "sessionHistoryOnRBM", "SessionHistoryOnRBM", XtRBoolean, sizeof(Boolean),
    offset(sessionHistoryOnRBM), XtRString, "True" },
  
  { "numberOfItemsInRBMHistory", "NumberOfItemsInRBMHistory", XtRInt,
    sizeof(int), offset(numberOfItemsInRBMHistory), XtRString, "12" },

  { "hotlistOnRBM", "HotlistOnRBM", XtRBoolean, sizeof(Boolean),
    offset(hotlistOnRBM), XtRString, "True" },

  { "newsUseShortNewsrc", "NewsUseShortNewsrc", XtRBoolean, sizeof(Boolean),
    offset(newsUseShortNewsrc), XtRString, "False" },

  /* New in 2.7b6 */
  { "usePreferences", "UsePreferences", XtRBoolean, sizeof(Boolean),
    offset(use_preferences), XtRString, "True" },

  { "bodyColors", "BodyColors", XtRBoolean, sizeof(Boolean),
    offset(bodyColors), XtRString, "True" },

  { "bodyImages", "BodyImages", XtRBoolean, sizeof(Boolean),
    offset(bodyImages), XtRString, "True" },

  { "defaultUnderlines", "DefaultUnderlines", XtRString, sizeof(char *),
    offset(defaultUnderlines), XtRString, "Default" },

  { "ftpBinaryMode", "FtpBinaryMode", XtRBoolean, sizeof(Boolean),
    offset(ftp_binary_mode), XtRString, "True" },

  { "kioskProtocols", "KioskProtocols", XtRString, sizeof(char *),
    offset(kioskProtocols), XtRString, NULL },

  /* New in 3.0 */
  { "fontColors", "FontColors", XtRBoolean, sizeof(Boolean),
    offset(fontColors), XtRString, "True" },

  { "progressiveDisplay", "ProgressiveDisplay", XtRBoolean, sizeof(Boolean),
    offset(progressive_display), XtRString, "True" },

  { "fontSizes", "FontSizes", XtRBoolean, sizeof(Boolean),
    offset(fontSizes), XtRString, "True" },

  { "fontBaseSize", "FontBaseSize", XtRInt, sizeof(int),
    offset(fontBaseSize), XtRString, "3" },

  { "trackTargetAnchors", "TrackTargetAnchors", XtRBoolean, sizeof(Boolean),
    offset(track_target_anchors), XtRString, "True" },

  { "debugMenu", "DebugMenu", XtRBoolean, sizeof(Boolean),
    offset(debug_menu), XtRString, "False" },

  { "reportBugs", "ReportBugs", XtRBoolean, sizeof(Boolean),
    offset(reportBugs), XtRString, "False" },

  { "imageAnimation", "ImageAnimation", XtRBoolean, sizeof(Boolean),
    offset(image_animation), XtRString, "True" },

  { "minAnimationDelay", "MinAnimationDelay", XtRInt, sizeof(int),
    offset(min_animation_delay), XtRString, "1" },

  { "refreshTrace", "RefreshTrace", XtRBoolean, sizeof(Boolean),
    offset(refreshTrace), XtRString, "False" },

  /* New in 3.1 */
  { "refreshURL", "RefreshURL", XtRBoolean, sizeof(Boolean),
    offset(refresh_URL), XtRString, "True" },

  /* New in 3.2 */
  { "browserSafeColors", "BrowserSafeColors", XtRBoolean, sizeof(Boolean),
    offset(browser_safe_colors), XtRString, "True" },

  /* New in 3.4 */
  { "blinkingText", "BlinkingText", XtRBoolean, sizeof(Boolean),
    offset(blinking_text), XtRString, "True" },

  { "blinkTime", "BlinkTime", XtRInt, sizeof(int),
    offset(blink_time), XtRString, "500" },

  { "cookies", "Cookies", XtRBoolean, sizeof(Boolean),
    offset(cookies), XtRString, "True" },

  { "acceptAllCookies", "AcceptAllCookies", XtRBoolean, sizeof(Boolean),
    offset(accept_all_cookies), XtRString, "True" },

  /* New in 3.5-1 */
  { "maxPixmapHeight", "MaxPixmapHeight", XtRInt, sizeof(int),
    offset(maxPixmapHeight), XtRString, "0" },

  { "maxPixmapWidth", "MaxPixmapWidth", XtRInt, sizeof(int),
    offset(maxPixmapWidth), XtRString, "0" },

  /* New in 3.6 */
  { "useCookieFile", "UseCookieFile", XtRBoolean, sizeof(Boolean),
    offset(use_cookie_file), XtRString, "True" },

  { "cookieFile", "CookieFile", XtRString, sizeof(char *),
    offset(cookie_file), XtRString, "mosaic.cookies" },

  /* New in 3.6-2 */
  { "imageDelayFile", "ImageDelayFile", XtRString, sizeof(char *),
#ifndef VMS
    offset(imagedelay_file), XtRString, ".mosaic-imageselect-sites" },
#else
    offset(imagedelay_file), XtRString, "mosaic-imageselect-sites." },
#endif

  /* New in 3.7 */
  { "browserSafeColors_if_Truecolor", "BrowserSafeColors_if_Truecolor",
    XtRBoolean, sizeof(Boolean),
    offset(BSColors_if_Truecolor), XtRString, "False" },

  { "hotkeys", "Hotkeys", XtRBoolean, sizeof(Boolean),
    offset(hotkeys), XtRString, "True" },

  { "invalidCookiePrompt", "InvalidCookiePrompt", XtRBoolean, sizeof(Boolean),
    offset(invalid_cookie_prompt), XtRString, "False" },

  { "maxCookies", "MaxCookies", XtRInt, sizeof(int),
    offset(max_cookies), XtRString, "500" },

  { "cookieDomainLimit", "CookieDomainLimit", XtRInt, sizeof(int),
    offset(cookie_domain_limit), XtRString, "50" },

  /* New in 3.8 */
  { "permFile", "PermFile", XtRString, sizeof(char *),
    offset(perm_file), XtRString, "mosaic.hostperm" },

  { "formButtonBackground", "FormButtonBackground", XtRString, sizeof(char *),
    offset(form_button_background), XtRString, "#BFBFBFBFBFBF" },

  /* New in 3.9 */
  { "verifySSLCertificates", "VerifySSLCertificates", XtRBoolean,
    sizeof(Boolean), offset(verify_ssl_certificates), XtRString, "True" },

  { "hotlistMenuHeight", "HotlistMenuHeight", XtRInt, sizeof(int),
    offset(hotlist_menu_height), XtRString, "502" },

  { "hotlistMenuWidth", "HotlistMenuWidth", XtRInt, sizeof(int),
    offset(hotlist_menu_width), XtRString, "475" },

  { "markupMemoryPreallocation", "MarkupMemoryPreallocation", XtRInt,
    sizeof(int), offset(markup_memory_preallocation), XtRString, "2048" },

  { "elementMemoryPreallocation", "ElementMemoryPreallocation", XtRInt,
    sizeof(int), offset(element_memory_preallocation), XtRString, "1280" },

  { "cookieTrace", "CookieTrace", XtRBoolean, sizeof(Boolean),
    offset(cookieTrace), XtRString, "False" },

  /* New in 4.0 */
  { "clueHelp", "ClueHelp", XtRBoolean, sizeof(Boolean),
    offset(clue_help), XtRString, "True" },

  { "clueForeground", "ClueForeground", XtRString, sizeof(char *),
    offset(clueForeground), XtRString, "#292925256F6F" },

  { "clueBackground", "ClueBackground", XtRString, sizeof(char *),
    offset(clueBackground), XtRString, "#BFBFBFBFBFBF" },

  { "clueDelay", "ClueDelay", XtRInt, sizeof(int),
    offset(clueDelay), XtRString, "750" },

  { "clueDownDelay", "ClueDownDelay", XtRInt, sizeof(int),
    offset(clueDownDelay), XtRString, "4000" },

  { "clueFont", "ClueFont", XtRString, sizeof(char *),
    offset(clueFont), XtRString, "New Century" },

  { "clueOval", "ClueOval", XtRBoolean, sizeof(Boolean),
    offset(clue_oval), XtRString, "False" },

  { "clueRounded", "ClueRounded", XtRBoolean, sizeof(Boolean),
    offset(clue_rounded), XtRString, "True" },

  { "printDuplex", "PrintDuplex", XtRBoolean, sizeof(Boolean),
    offset(print_duplex), XtRString, "True" },

  { "menubarTearoff", "MenubarTearoff", XtRBoolean, sizeof(Boolean),
    offset(menubar_tearoff), XtRString, "True" },

  /* New in 4.1 */
  { "tiffErrorMessages", "TiffErrorMessages", XtRBoolean, sizeof(Boolean),
    offset(tiff_error_messages), XtRString, "True" },

  { "pngErrorMessages", "PngErrorMessages", XtRBoolean, sizeof(Boolean),
    offset(png_error_messages), XtRString, "True" },

  { "jpegErrorMessages", "JpegErrorMessages", XtRBoolean, sizeof(Boolean),
    offset(jpeg_error_messages), XtRString, "True" },

  /* New in 4.2 */
  { "detachToolbar", "DetachToolbar", XtRBoolean, sizeof(Boolean),
    offset(detached_toolbar), XtRString, "False" },

  { "verticalDetachToolbar", "VerticalDetachToolbar", XtRBoolean,
    sizeof(Boolean), offset(detached_toolbar_vertical), XtRString, "False" },

  { "textToolbar", "TextToolbar", XtRBoolean, sizeof(Boolean),
    offset(text_toolbar), XtRString, "False" },

  { "encryptionIcon", "EncryptionIcon", XtRBoolean, sizeof(Boolean),
    offset(encryption_icon), XtRString, "True" },

  { "presentationModeOnRBM", "PresentationModeOnRBM", XtRBoolean,
    sizeof(Boolean), offset(presentationModeOnRBM), XtRString, "True" },

  { "multipleImageLoad", "MultipleImageLoad", XtRBoolean, sizeof(Boolean),
    offset(multiple_image_load), XtRString, "True" },

  { "multipleImageLimit", "MultipleImageLimit", XtRInt, sizeof(int),
    offset(multiple_image_limit), XtRString, "12" },

  { "jpeg2000ErrorMessages", "Jpeg2000ErrorMessages", XtRBoolean,
    sizeof(Boolean), offset(jpeg2000_error_messages), XtRString, "True" }
};

#undef offset

static XrmOptionDescRec options[] = {
  {"-fn",     "*fontList",            XrmoptionSepArg, NULL},
  {"-ft",     "*XmText*fontList",     XrmoptionSepArg, NULL},
  {"-fm",     "*menubar*fontList",    XrmoptionSepArg, NULL},
  {"-home",   "*homeDocument",        XrmoptionSepArg, NULL},
  {"-ngh",    "*useGlobalHistory",    XrmoptionNoArg,  "False"},
  /* Let Xt strip out -mono from stuff it considers interesting. */
  {"-mono",   "*nothingUseful",       XrmoptionNoArg,  "True"},
  {"-color",  "*nothingUseful",       XrmoptionNoArg,  "True"},
  {"-ghbnie", "*gethostbynameIsEvil", XrmoptionNoArg,  "True"},
  {"-iconic", "*initialWindowIconic", XrmoptionNoArg,  "True"},
  {"-i",      "*initialWindowIconic", XrmoptionNoArg,  "True"},
  /* New in 1.1 */
  /* -nd isn't documented since defaults in the widget still take effect,
   * so the benefits of using it are kinda iffy (as if they weren't 
   * anyway)... */
  {"-nd",     "*nothingUseful",       XrmoptionNoArg,  "True"},
  {"-tmpdir", "*tmpDirectory",        XrmoptionSepArg, NULL},
  {"-dil",    "*delayImageLoads",     XrmoptionNoArg,  "True"},
  {"-ics",    "*imageCacheSize",      XrmoptionSepArg, NULL},
  {"-protect","*protectMeFromMyself", XrmoptionNoArg,  "True"},
  {"-kraut",  "*mailFilterCommand",   XrmoptionNoArg,  "kraut"},
#ifdef __sgi
  {"-dm",     "*debuggingMalloc",     XrmoptionNoArg,  "True"},
#endif
  {"-kiosk",  "*kiosk",               XrmoptionNoArg,  "True"},
  {"-kioskPrint",  "*kioskPrint",     XrmoptionNoArg,  "True"},
  {"-kioskNoExit",  "*kioskNoExit",   XrmoptionNoArg,  "True"},
  {"-cciPort",  "*cciPort",   	      XrmoptionSepArg,  "0"},
  {"-maxNumCCIConnect",  "*maxNumCCIConnect",  XrmoptionSepArg,  "0"},
  {"-install",  "*nothingUseful",     XrmoptionNoArg,  "True"},
};

static String color_resources[] = {
  "*AppDefaultsVersion:		0",
  "*XmLabel*fontList:   		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*XmLabelGadget*fontList:		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*XmScale*fontList:   		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*XmBulletinBoard*labelFontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*optionmenu.XmLabelGadget*fontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*XmPushButton*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmPushButtonGadget*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmToggleButton*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmToggleButtonGadget*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*optionmenu*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmIconGadget*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmBulletinBoard*buttonFontList:	-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*menubar*fontList:   		-*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmPushButton*fontList:  -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmLabelGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmPushButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmCascadeButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmCascadeButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmCascadeButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmCascadeButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmToggleButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmToggleButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*pulldownmenu*fontList:	-*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmList*fontList:	  -*-helvetica-medium-r-normal-*-14-*-iso8859-1",
#ifndef VMS   /* PGE, Use multiple font names */
  "*XmText.fontList:      -*-lucidatypewriter-medium-r-normal-*-14-*-iso8859-1",
  "*XmTextField.fontList: -*-lucidatypewriter-medium-r-normal-*-14-*-iso8859-1",
#else
  "*XmText.fontList:      -*-lucidatypewriter-medium-r-normal-*-14-*-iso8859-1/-*-fixed-medium-r-normal-*-14-*-iso8859-1",
  "*XmTextField.fontList: -*-lucidatypewriter-medium-r-normal-*-14-*-iso8859-1/-*-fixed-medium-r-normal-*-14-*-iso8859-1",
#endif /* Old DEC-stuff has no lucida fonts */
  
  "*optionmenu*marginHeight: 	0",
  "*optionmenu*marginTop:	5",
  "*optionmenu*marginBottom: 	5",
  "*optionmenu*marginWidth: 	5",
  "*pulldownmenu*XmPushButton*marginHeight:	1",
  "*pulldownmenu*XmPushButton*marginWidth:	1",
  "*pulldownmenu*XmPushButton*marginLeft:	3",
  "*pulldownmenu*XmPushButton*marginRight:	3",
  "*XmList*listMarginWidth:     3",
  "*menubar*marginHeight: 	1",
  "*menubar.marginHeight: 	0",
  "*menubar*marginLeft:  	1",
  "*menubar.spacing:  		7",
  "*XmMenuShell*marginLeft:  	3",
  "*XmMenuShell*marginRight:  	4",
  "*XmMenuShell*XmToggleButton*spacing: 	    2",
  "*XmMenuShell*XmToggleButton*marginHeight:        0",
  "*XmMenuShell*XmToggleButton*indicatorSize:       12",
  "*XmMenuShell*XmToggleButtonGadget*spacing: 	    2",
  "*XmMenuShell*XmToggleButtonGadget*marginHeight:  0",
  "*XmMenuShell*XmToggleButtonGadget*indicatorSize: 12",
  "*XmMenuShell*XmLabelGadget*marginHeight:	    4",
  "*XmToggleButtonGadget*spacing: 	4",
  "*XmToggleButton*spacing:		4",
  "*XmScrolledWindow*spacing:		0",
  "*XmScrollBar*width: 		        18",
  "*XmScrollBar*height: 		18",
  "*Hbar*height:                        22",
  "*Vbar*width:                         22",
  "*XmScale*scaleHeight: 		20",
  "*XmText*marginHeight:		4",
  "*fsb*XmText*width:                   420",
  "*fsb*XmTextField*width:              420",
  "*fillOnSelect:			True",
  "*visibleWhenOff:		        True",
  "*XmText*highlightThickness:		0",
  "*XmTextField*highlightThickness:	0",
  "*XmPushButton*highlightThickness:	0",
  "*XmScrollBar*highlightThickness:     0",
  "*highlightThickness:	                0",
  /* "*geometry:                           +400+200", */
  /* "*keyboardFocusPolicy:                pointer",*/
  
  "*TitleFont:      -adobe-times-bold-r-normal-*-24-*-*-*-*-*-iso8859-1",
#ifndef VMS   /* PGE, Use multiple font names */
  "*Font:           -adobe-times-medium-r-normal-*-17-*-*-*-*-*-iso8859-1",
  "*ItalicFont:     -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso8859-1",
  "*BoldFont:       -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso8859-1",
  "*BoldItalicFont: -adobe-times-bold-i-normal-*-17-*-*-*-*-*-iso8859-1",
  "*FixedFont:      -adobe-courier-medium-r-normal-*-17-*-*-*-*-*-iso8859-1",
#else
  "*Font: -adobe-times-medium-r-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-medium-r-normal-*-18-*-*-*-*-*-iso8859-1",
  "*ItalicFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-medium-i-normal-*-18-*-*-*-*-*-iso8859-1",
  "*BoldFont: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-bold-r-normal-*-18-*-*-*-*-*-iso8859-1",
  "*BoldItalicFont: -adobe-times-bold-i-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-bold-i-normal-*-18-*-*-*-*-*-iso8859-1",
  "*FixedFont: -adobe-courier-medium-r-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-iso8859-1",
#endif /* 17 pt fonts, BSN */
  "*Header1Font: -adobe-times-bold-r-normal-*-24-*-*-*-*-*-iso8859-1",
  "*Header2Font: -adobe-times-bold-r-normal-*-18-*-*-*-*-*-iso8859-1",
#ifndef VMS   /* PGE, Use multiple font names */
  "*Header3Font: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso8859-1",
#else
  "*Header3Font: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-bold-r-normal-*-18-*-*-*-*-*-iso8859-1",
#endif /* 17 pt fonts, BSN */
  "*Header4Font: -adobe-times-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*Header5Font: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso8859-1",
  "*Header6Font: -adobe-times-bold-r-normal-*-10-*-*-*-*-*-iso8859-1",
#ifndef VMS   /* PGE, Use multiple font names */
  "*AddressFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso8859-1",
#else
  "*AddressFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-medium-i-normal-*-18-*-*-*-*-*-iso8859-1",
#endif /* 17 pt fonts, BSN */
  "*PlainFont:   -adobe-courier-medium-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*ListingFont: -adobe-courier-medium-r-normal-*-12-*-*-*-*-*-iso8859-1",
  "*MeterFont:   -adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*",
  "*ToolbarFont: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso8859-1",
  "*AnchorUnderlines:                   1",
  "*VisitedAnchorUnderlines:            1",
  "*DashVisitedAnchorUnderlines:        True",
  "*VerticalScrollOnRight:              True",
#ifdef VMS
  "*CatchPriorAndNext:                  True",
#endif /* VMS, BSN */

  "*Foreground:			 	#000000000000",
  "*XmScrollBar*Foreground:             #bfbfbfbfbfbf",
  "*XmLabel*Foreground:                 #292925256f6f",
  "*XmMenuShell*XmToggleButton*Foreground: #000000000000",
  "*XmToggleButton*Foreground:          #292925256f6f",
  "*XmMenuShell*XmPushButton*Foreground: #000000000000",
  "*XmPushButton*Foreground:            #000000003333",
  "*logo*Foreground:                    #292925256f6f",
  "*searchindex_button*Foreground:      #292925256f6f",

  "*Background:                         #bfbfbfbfbfbf",
  "*bodyBG:                             #ffffffffffff",

  "*XmList*Background:     		#bfbfbfbfbfbf",
  "*XmText*Background:	 	        #bfbfbfbfbfbf",
  "*XmSelectionBox*Background:	 	#bfbfbfbfbfbf",
  "*XmMessageBox*Background:	 	#bfbfbfbfbfbf",
  "*XmTextField*Background: 		#999999999999",

  "*TopShadowColor:                     #e7e7e7e7e7e7",
  "*XmList*TopShadowColor:              #e7e7e7e7e7e7",
  "*XmText*TopShadowColor:              #e7e7e7e7e7e7",
  "*XmSelectionBox*TopShadowColor:      #e7e7e7e7e7e7",
  "*XmMessageBox*TopShadowColor:        #e7e7e7e7e7e7",
  
  "*TroughColor:                        #666666666666",
  "*SelectColor:			#ffffffff0000",
  "*HighlightColor:		 	#bfbfbfbfbfbf",

  /* Remember to update this in the app-defaults file. */
  "*visitedAnchorColor:                 #333300009999",
  "*anchorColor:                        #00000000cccc",
  "*activeAnchorFG:                     #ffff00000000",
  "*activeAnchorBG:                     #bfbfbfbfbfbf",

  /* Disable Motif Drag-N-Drop */
  "*dragInitiatorProtocolStyle:		XmDRAG_NONE",
  "*dragReceiverProtocolStyle:		XmDRAG_NONE",
 
  NULL
};

static String mono_resources[] = {
  "*AppDefaultsVersion:		0",
  "*XmLabel*fontList:   		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*XmLabelGadget*fontList:		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*XmScale*fontList:   		-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*XmBulletinBoard*labelFontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*optionmenu.XmLabelGadget*fontList:	-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*XmPushButton*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmPushButtonGadget*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmToggleButton*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmToggleButtonGadget*fontList:	-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*optionmenu*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmIconGadget*fontList:		-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*XmBulletinBoard*buttonFontList: 	-*-helvetica-medium-r-normal-*-14-*-iso8859-1",
  "*menubar*fontList:   		-*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmPushButton*fontList:  -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmLabelGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmPushButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmCascadeButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmCascadeButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmCascadeButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmCascadeButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmToggleButton*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmMenuShell*XmToggleButtonGadget*fontList: -*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*pulldownmenu*fontList:	-*-helvetica-bold-o-normal-*-14-*-iso8859-1",
  "*XmList*fontList:	  -*-helvetica-medium-r-normal-*-14-*-iso8859-1",
#ifndef VMS   /* PGE, Use multiple font names */
  "*XmText.fontList:      -*-lucidatypewriter-medium-r-normal-*-14-*-iso8859-1",
  "*XmTextField.fontList: -*-lucidatypewriter-medium-r-normal-*-14-*-iso8859-1",
#else
  "*XmText.fontList:      -*-lucidatypewriter-medium-r-normal-*-14-*-iso8859-1/-*-fixed-medium-r-normal-*-14-*-iso8859-1",
  "*XmTextField.fontList: -*-lucidatypewriter-medium-r-normal-*-14-*-iso8859-1/-*-fixed-medium-r-normal-*-14-*-iso8859-1",
#endif /* Old DEC-stuff has no lucida fonts */

  "*optionmenu*marginHeight: 	0",
  "*optionmenu*marginTop: 	5",
  "*optionmenu*marginBottom: 	5",
  "*optionmenu*marginWidth: 	5",
  "*pulldownmenu*XmPushButton*marginHeight:	1",
  "*pulldownmenu*XmPushButton*marginWidth:	1",
  "*pulldownmenu*XmPushButton*marginLeft:	3",
  "*pulldownmenu*XmPushButton*marginRight:	3",
  "*XmList*listMarginWidth:     3",
  "*menubar*marginHeight: 	1",
  "*menubar.marginHeight: 	0",
  "*menubar*marginLeft:  	1",
  "*menubar.spacing:  		7",
  "*XmMenuShell*marginLeft:  	3",
  "*XmMenuShell*marginRight:  	4",
  "*XmMenuShell*XmToggleButton*spacing: 	    2",
  "*XmMenuShell*XmToggleButton*marginHeight:        0",
  "*XmMenuShell*XmToggleButton*indicatorSize:       12",
  "*XmMenuShell*XmToggleButtonGadget*spacing: 	    2",
  "*XmMenuShell*XmToggleButtonGadget*marginHeight:  0",
  "*XmMenuShell*XmToggleButtonGadget*indicatorSize: 12",
  "*XmMenuShell*XmLabelGadget*marginHeight:         4",
  "*XmToggleButtonGadget*spacing: 	4",
  "*XmToggleButton*spacing: 		4",
  "*XmScrolledWindow*spacing:		0",
  "*XmScrollBar*width: 		        18",
  "*XmScrollBar*height: 		18",
  "*Hbar*height:                        22",
  "*Vbar*width:                         22",
  "*XmScale*scaleHeight: 		20",
  "*XmText*marginHeight:		4",
  "*fsb*XmText*width:                   420",
  "*fsb*XmTextField*width:              420",
  "*fillOnSelect:			True",
  "*visibleWhenOff:		        True",
  "*XmText*highlightThickness:		0",
  "*XmTextField*highlightThickness:	0",
  "*XmPushButton*highlightThickness:	0",
  "*XmScrollBar*highlightThickness:     0",
  "*highlightThickness:	                0",
  /* "*geometry:                           +400+200", */
  /* "*keyboardFocusPolicy:                pointer", */

  "*TitleFont: -adobe-times-bold-r-normal-*-24-*-*-*-*-*-iso8859-1",
#ifndef VMS   /* PGE, Use multiple font names */
  "*Font: -adobe-times-medium-r-normal-*-17-*-*-*-*-*-iso8859-1",
  "*ItalicFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso8859-1",
  "*BoldFont: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso8859-1",
  "*BoldItalicFont: -adobe-times-bold-i-normal-*-17-*-*-*-*-*-iso8859-1",
  "*FixedFont: -adobe-courier-medium-r-normal-*-17-*-*-*-*-*-iso8859-1",
#else
  "*Font: -adobe-times-medium-r-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-medium-r-normal-*-18-*-*-*-*-*-iso8859-1",
  "*ItalicFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-medium-i-normal-*-18-*-*-*-*-*-iso8859-1",
  "*BoldFont: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-bold-r-normal-*-18-*-*-*-*-*-iso8859-1",
  "*BoldItalicFont: -adobe-times-bold-i-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-bold-i-normal-*-18-*-*-*-*-*-iso8859-1",
  "*FixedFont: -adobe-courier-medium-r-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-courier-medium-r-normal-*-18-*-*-*-*-*-iso8859-1",
#endif /* 17 pt fonts, BSN */
  "*Header1Font: -adobe-times-bold-r-normal-*-24-*-*-*-*-*-iso8859-1",
  "*Header2Font: -adobe-times-bold-r-normal-*-18-*-*-*-*-*-iso8859-1",
#ifndef VMS   /* PGE, Use multiple font names */
  "*Header3Font: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso8859-1",
#else
  "*Header3Font: -adobe-times-bold-r-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-bold-r-normal-*-18-*-*-*-*-*-iso8859-1",
#endif /* 17 pt fonts, GEC */
  "*Header4Font: -adobe-times-bold-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*Header5Font: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso8859-1",
  "*Header6Font: -adobe-times-bold-r-normal-*-10-*-*-*-*-*-iso8859-1",
#ifndef VMS   /* PGE, Use multiple font names */
  "*AddressFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso8859-1",
#else
  "*AddressFont: -adobe-times-medium-i-normal-*-17-*-*-*-*-*-iso8859-1/-adobe-times-medium-i-normal-*-18-*-*-*-*-*-iso8859-1",
#endif /* 17 pt fonts, GEC */
  "*PlainFont:   -adobe-courier-medium-r-normal-*-14-*-*-*-*-*-iso8859-1",
  "*ListingFont: -adobe-courier-medium-r-normal-*-12-*-*-*-*-*-iso8859-1",
  "*MeterFont:   -adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*",
  "*ToolbarFont: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso8859-1",

  "*Foreground:                         black",
  "*Background:                         white",
  "*bodyBG:                             white",
  "*TopShadowColor:                     black",
  "*BottomShadowColor:                  black",
  "*anchorColor:                        black",
  "*visitedAnchorColor:                 black",
  "*activeAnchorFG:                     black",
  "*activeAnchorBG:                     white",
  "*TroughColor:                        black",
  "*SelectColor:                        black",
  "*AnchorUnderlines:                   1",
  "*VisitedAnchorUnderlines:            1",
  "*DashVisitedAnchorUnderlines:        True",
  "*VerticalScrollOnRight:              True",
  /* Disable Motif Drag-N-Drop */
  "*dragInitiatorProtocolStyle:		XmDRAG_NONE",
  "*dragReceiverProtocolStyle:		XmDRAG_NONE",
#ifdef VMS
  "*CatchPriorAndNext:                  True",
#endif /* VMS, BSN */
  NULL
};

#endif  /* __MOSAIC_XRESOURCES_H__ */
