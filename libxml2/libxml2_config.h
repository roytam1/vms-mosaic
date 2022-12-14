/* config.h  */

#ifndef LIBXML2_CONFIG_H
#define LIBXML2_CONFIG_H 

#ifndef SHORT_XML_NAMES_ONLY

#define VMS	1

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS	1

#undef PACKAGE
#undef VERSION
#undef HAVE_LIBZ
#undef HAVE_LIBM
#undef HAVE_ISINF
#if defined(_IEEE_FP) && (__CRTL_VER >= 60200000)
# define HAVE_ISNAN 1
#endif
#undef HAVE_LIBHISTORY
#undef HAVE_LIBREADLINE

/* Define if you have the class function.  */
#undef HAVE_CLASS

/* Define if you have the finite function.  */
#undef HAVE_FINITE

/* Define if you have the fp_class function.  */
#define HAVE_FP_CLASS	1

/* Define if you have the fpclass function.  */
#undef HAVE_FPCLASS

/* Define if you have the isnand function.  */
#undef HAVE_ISNAND

/* Define if you have the localtime function.  */
#define HAVE_LOCALTIME	1

/* Define if you have the snprintf function.  */
#undef HAVE_SNPRINTF

/* Define if you have the strdup function.  */
#define HAVE_STRDUP	1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR	1

/* Define if you have the strftime function.  */
#if !defined(VAXC) && !defined(__GNUC__)
#define HAVE_STRFTIME 	1
#else
#undef HAVE_STRFTIME
#endif

/* Define if you have the strndup function.  */
#undef HAVE_STRNDUP

/* Define if you have the <arpa/inet.h> header file.  */
#undef HAVE_ARPA_INET_H

/* Define if you have the <ctype.h> header file.  */
#define HAVE_CTYPE_H	1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H	1

/* Define if you have the <errno.h> header file.  */
#define HAVE_ERRNO_H	1

/* Define if you have the <fcntl.h> header file.  */
#undef HAVE_FCNTL_H

/* Define if you have the <float.h> header file.  */
#define HAVE_FLOAT_H	1

/* Define if you have the <fp_class.h> header file.  */
#define HAVE_FP_CLASS_H	1

/* Define if you have the <ieeefp.h> header file.  */
#undef HAVE_IEEEFP_H

/* Define if you have the <malloc.h> header file.  */
#undef HAVE_MALLOC_H

/* Define if you have the <math.h> header file.  */
#define HAVE_MATH_H	1

/* Define if you have the <nan.h> header file.  */
#undef HAVE_NAN_H

/* Define if you have the <ndir.h> header file.  */
#undef HAVE_NDIR_H

/* Define if you have the <netdb.h> header file.  */
#define HAVE_NETDB_H	1

/* Define if you have the <netinet/in.h> header file.  */
#undef HAVE_NETINET_IN_H

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H	1

/* Define if you have the <stdlib.h> header file.  */
#ifndef HAVE_STDLIB_H
#define HAVE_STDLIB_H	1
#endif

/* Define if you have the <sys/dir.h> header file.  */
#undef HAVE_SYS_DIR_H

/* Define if you have the <sys/mman.h> header file.  */
#undef HAVE_SYS_MMAN_H

/* Define if you have the <sys/ndir.h> header file.  */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <sys/select.h> header file.  */
#undef HAVE_SYS_SELECT_H

/* Define if you have the <sys/socket.h> header file.  */
#undef HAVE_SYS_SOCKET_H

/* Define if you have the <sys/stat.h> header file.  */
#undef HAVE_SYS_STAT_H

/* Define if you have the <sys/time.h> header file.  */
#undef HAVE_SYS_TIME_H

/* Define if you have the <sys/types.h> header file.  */
#undef HAVE_SYS_TYPES_H

/* Define if you have the <time.h> header file.  */
#define HAVE_TIME_H	1

/* Define if you have the <unistd.h> header file.  */
#if defined(VAXC) && !defined(__DECC)
#undef HAVE_UNISTD_H
#else
#define HAVE_UNISTD_H	1
#endif

/* Define if you have the <zlib.h> header file.  */
#define HAVE_ZLIB_H 1

/* Define if you have the inet library (-linet).  */
#undef HAVE_LIBINET

/* Define if you have the nsl library (-lnsl).  */
#undef HAVE_LIBNSL

/* Define if you have the socket library (-lsocket).  */
#undef HAVE_LIBSOCKET

/* Name of package */
#undef PACKAGE

/* Version number of package */
#undef VERSION

/* Up to this point this is just a hard-wired version of 
 * config.h.  After this will be anything else we need 
 * that is VMS-specific.
 */
#endif /* Short names */

/* For best results, compile with /NAMES=(SHORTENED), which requires
 * DEC C 5.7 or later.  For older compilers, the shortened names below
 * are the same ones the mangler generates in C 5.7 and later.  These may
 * work, though there will probably be some conflicts with redefinitions 
 * in globals.h.
 */

/*      0        1         2         3               0        1         2         3
 *      123456789012345678901234567890123456789      1234567890123456789012345678901 
 */
#define __xmlDoValidityCheckingDefaultValue          __xmlDoValidityChecking3qad3pq$
#define __xmlSubstituteEntitiesDefaultValue          __xmlSubstituteEntities0pij13u$
#define trio_locale_set_thousand_separator           trio_locale_set_thousan259ikkk$
#define xmlDoValidityCheckingDefaultValue            xmlDoValidityCheckingDe1bcsei4$
#define xmlParseBalancedChunkMemoryRecover           xmlParseBalancedChunkMe1lu1e86$
#define xmlParseElementChildrenContentDecl           xmlParseElementChildren1mp6pcb$
#define xmlParserInputBufferCreateFilename           xmlParserInputBufferCre36lujn2$
#define xmlRegisterDefaultInputCallbacks             xmlRegisterDefaultInput3vin0cp$
#define xmlRegisterDefaultOutputCallbacks            xmlRegisterDefaultOutpu0q443dd$
#define xmlSubstituteEntitiesDefaultValue            xmlSubstituteEntitiesDe28k2c80$
#define xmlUCSIsAlphabeticPresentationForms          xmlUCSIsAlphabeticPrese2qr24s3$
#define xmlUCSIsArabicPresentationFormsB             xmlUCSIsArabicPresentat1gajvg8$
#define xmlUCSIsArabicPresentationFormsA             xmlUCSIsArabicPresentat3sq1bti$
#define xmlUCSIsCJKCompatibilityIdeographsSupplement xmlUCSIsCJKCompatibilit0or40ki$
#define xmlUCSIsCJKCompatibilityIdeographs           xmlUCSIsCJKCompatibilit2nodmc5$
#define xmlUCSIsCJKSymbolsandPunctuation             xmlUCSIsCJKSymbolsandPu0a3i7ra$
#define xmlUCSIsCJKUnifiedIdeographsExtensionA       xmlUCSIsCJKUnifiedIdeog11ig3fd$
#define xmlUCSIsCJKUnifiedIdeographsExtensionB       xmlUCSIsCJKUnifiedIdeog3d22n2n$
#define xmlUCSIsCombiningDiacriticalMarks            xmlUCSIsCombiningDiacri3tj3nl8$
#define xmlUCSIsCombiningMarksforSymbols             xmlUCSIsCombiningMarksf3ftqd7s$
#define xmlUCSIsEnclosedCJKLettersandMonths          xmlUCSIsEnclosedCJKLett0nq67g4$
#define xmlUCSIsHalfwidthandFullwidthForms           xmlUCSIsHalfwidthandFul047l0a1$
#define xmlUCSIsHighPrivateUseSurrogates             xmlUCSIsHighPrivateUseS071kh83$
#define xmlUCSIsIdeographicDescriptionCharacters     xmlUCSIsIdeographicDesc1rovf8g$
#define xmlUCSIsMathematicalAlphanumericSymbols      xmlUCSIsMathematicalAlp2ag8r44$
#define xmlUCSIsOpticalCharacterRecognition          xmlUCSIsOpticalCharacte1juuh06$
#define xmlUCSIsSuperscriptsandSubscripts            xmlUCSIsSuperscriptsand3fi4eup$
#define xmlUCSIsUnifiedCanadianAboriginalSyllabics   xmlUCSIsUnifiedCanadian0lbvi9b$
#define xmlValidCtxtNormalizeAttributeValue          xmlValidCtxtNormalizeAt0q11n5f$
#define xmlXPathRegisteredVariablesCleanup           xmlXPathRegisteredVaria1uvs4uc$

#define xmlBufferWriteChar xmlBufferWriteChar2

#endif
