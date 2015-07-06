/* Hamiltonian Cycle Tool.

Copyright (C) 2009 Andrew Chalaturnyk and William Kocay.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#ifndef ARCH_H
#define ARCH_H

#include <limits.h>
#include <stdbool.h>

typedef signed int         SInt;
typedef unsigned int       UInt;
typedef unsigned long long ULongLong;

/* macro to ease error recovering malloc assignments */

#define EM(val, len, e_dest) if (!(val=malloc(len))) goto e_dest

#define SInt_MAX      INT_MAX
#define SInt_MIN      INT_MIN
#define UInt_MAX      UINT_MAX
#define ULongLong_MAX ULLONG_MAX

#if SInt_MAX == 32767
#define SInt_BIT 16
#elif SInt_MAX == 2147483647
#define SInt_BIT 32
#else
#define SInt_BIT 64
#endif

#if UInt_MAX == 65535
#define UInt_BIT 16
#elif UInt_MAX == 4294967295U
#define UInt_BIT 32
#else
#define UInt_BIT 64
#endif

#if UInt_MAX < ULongLong_MAX
#define ULongLong_BIT (2*UInt_BIT)
#else
#define ULongLong_BIT UInt_BIT
#endif


/*! Used in printf format strings for SInt. */
#define c_PF_SInt      "d"  /* hd - short, ld - long, d - int */

/*! Used in printf format strings for UInt. */
#define c_PF_UInt      "u"  /* hu - ushort, lu - ulong, u - uint */

/*! Used in printf format strings for ULongLong. */
#define c_PF_ULongLong  "llu"

#endif /* ARCH_H */
