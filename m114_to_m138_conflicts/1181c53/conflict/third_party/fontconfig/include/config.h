/*
 * Autogenerated by the Meson build system.
 * Do not edit, your changes will be lost.
 */

#pragma once

<<<<<<< HEAD
=======
#include "build/build_config.h"

/* The normal alignment of `double', in bytes. */
>>>>>>> 1181c533483 (Build base_unittests hermetically. (#4935))
#define ALIGNOF_DOUBLE 8

#define ALIGNOF_VOID_P 8

#define CONFIGDIR "/etc/fonts/conf.d"

#define FC_CACHEDIR "/var/cache/fontconfig"

#define FC_DEFAULT_FONTS "\t<dir>/usr/share/fonts</dir>\n\t<dir>/usr/local/share/fonts</dir>\n"

#define FC_FONTPATH ""

/* The type of gperf "len" parameter */
#define FC_GPERF_SIZE_T size_t

#define FC_TEMPLATEDIR "/usr/share/fontconfig/conf.avail"

#define FLEXIBLE_ARRAY_MEMBER

#define FONTCONFIG_PATH "/etc/fonts"

#define GETTEXT_PACKAGE "fontconfig"

#define HAVE_DIRENT_H 1

#define HAVE_FCNTL_H 1

#define HAVE_FSTATFS 1

#define HAVE_FSTATVFS 1

#define HAVE_FT_DONE_MM_VAR 1

#define HAVE_FT_GET_BDF_PROPERTY 1

#define HAVE_FT_GET_PS_FONT_INFO 1

#define HAVE_FT_GET_X11_FONT_FORMAT 1

#define HAVE_FT_HAS_PS_GLYPH_NAMES 1

#define HAVE_GETOPT 1

#define HAVE_GETOPT_LONG 1

#define HAVE_INTEL_ATOMIC_PRIMITIVES 1

#define HAVE_LINK 1

#define HAVE_LRAND48 1

#define HAVE_LSTAT 1

#define HAVE_MKDTEMP 1

#define HAVE_MKOSTEMP 1

#define HAVE_MKSTEMP 1

#define HAVE_MMAP 1

#define HAVE_POSIX_FADVISE 1

#define HAVE_PTHREAD 1

#define HAVE_RAND 1

#define HAVE_RANDOM 1

<<<<<<< HEAD
=======
#if BUILDFLAG(ENABLE_COBALT_HERMETIC_HACKS)
/* TODO: (cobalt b/398295440) Add `random_r' support to Evergreen. */
#define HAVE_RANDOM_R 0
#else
/* Define to 1 if you have the `random_r' function. */
>>>>>>> 1181c533483 (Build base_unittests hermetically. (#4935))
#define HAVE_RANDOM_R 1
#endif

#define HAVE_RAND_R 1

#define HAVE_READLINK 1

#define HAVE_STDATOMIC_PRIMITIVES 1

#define HAVE_STDLIB_H 1

#define HAVE_STRING_H 1

#define HAVE_STRUCT_DIRENT_D_TYPE 1

#define HAVE_SYS_MOUNT_H 1

#define HAVE_SYS_PARAM_H 1

#define HAVE_SYS_STATFS_H 1

#define HAVE_SYS_STATVFS_H 1

#define HAVE_SYS_VFS_H 1

#define HAVE_UNISTD_H 1

#define HAVE_VPRINTF 1

#define SIZEOF_VOID_P 8

#define USE_ICONV 0

#define _GNU_SOURCE 1

#define ENABLE_LIBXML2 1
