/* Using AltiVec bitslice. */
#cmakedefine DVBCSA_USE_ALTIVEC 1

/* Using MMX bitslice. */
#cmakedefine DVBCSA_USE_MMX 1

/* Using SSE2 bitslice. */
#cmakedefine DVBCSA_USE_SSE 1

/* Using 32 bits integer bitslice. */
#cmakedefine DVBCSA_USE_UINT32 1

/* Using 64 bits integer bitslice. */
#cmakedefine DVBCSA_USE_UINT64 1

/* Define to 1 if you have the <assert.h> header file. */
#cmakedefine HAVE_ASSERT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* _mm_malloc is available */
#cmakedefine HAVE_MM_MALLOC 1

/* posix_memalign is available */
#cmakedefine HAVE_POSIX_MEMALIGN 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG ${SIZEOF_LONG}

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif
