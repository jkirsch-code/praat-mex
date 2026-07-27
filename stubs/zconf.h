/* zconf.h - zlib configuration for MSVC */
#ifndef ZCONF_H
#define ZCONF_H

#include <stddef.h>

#ifndef z_const
#  define z_const
#endif

#define z_longlong long long

#include <stddef.h>
typedef size_t z_size_t;
#undef z_longlong

#ifndef MAX_MEM_LEVEL
#  define MAX_MEM_LEVEL 9
#endif

#ifndef MAX_WBITS
#  define MAX_WBITS   15
#endif

#ifndef OF
#  define OF(args)  args
#endif

#ifndef FAR
#  define FAR
#endif

#ifndef ZEXTERN
#  define ZEXTERN extern
#endif
#ifndef ZEXPORT
#  define ZEXPORT
#endif
#ifndef ZEXPORTVA
#  define ZEXPORTVA
#endif

#ifndef ZLIB_INTERNAL
#  define ZLIB_INTERNAL
#endif

#if !defined(__MACTYPES__)
typedef unsigned char  Byte;
#endif
typedef unsigned int   uInt;
typedef unsigned long  uLong;

typedef Byte  FAR Bytef;
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;

typedef void const *voidpc;
typedef void FAR   *voidpf;
typedef void       *voidp;

#ifndef Z_U4
#  define Z_U4 unsigned long
#endif

typedef Z_U4 z_crc_t;

#ifndef z_off_t
#  define z_off_t long long
#endif
#define z_off64_t long long

#define STDC
#define Z_HAVE_STDARG_H

#endif /* ZCONF_H */
