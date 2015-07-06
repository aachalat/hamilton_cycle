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


#ifndef ARCHIO_H
#define ARCHIO_H

#include <stdio.h>
#include "Arch.h"

#if SInt_BIT==8
#define MAX_SInt_STR_LEN 4
#elif SInt_BIT==16
#define MAX_SInt_STR_LEN 6
#elif SInt_BIT==32
#define MAX_SInt_STR_LEN 11
#else /* 64 bit */
#define MAX_SInt_STR_LEN 20
#endif

#if UInt_BIT==8
#define MAX_UInt_STR_LEN 3
#elif UInt_BIT==16
#define MAX_UInt_STR_LEN 5
#elif UInt_BIT==32
#define MAX_UInt_STR_LEN 10
#else /* 64 bit */
#define MAX_UInt_STR_LEN 20
#endif

#if ULongLong_BIT==32
#define MAX_ULongLong_STR_LEN 10
#else /* 64 bit */
#define MAX_ULongLong_STR_LEN 20
#endif

#define RJUST_OVERFLOW_CHAR 'X'

int get_usigned_dec_length(const UInt x);
int get_usignedbigval_dec_length(const ULongLong x);
int get_signed_dec_length(SInt x);

bool read_signedval(FILE *inp, SInt *xptr);
bool read_usignedval(FILE *inp, UInt *xptr);
bool read_usignedbigval(FILE *inp, ULongLong *xptr);

int write_signedval_str(char * buf, SInt x);
int write_usignedval_str(char * buf, UInt x);
int write_usignedbigval_str(char * buf, ULongLong x);

int write_rj_signedval_str(char * buf, int len, char pad,
  SInt x);
int write_rj_usignedval_str(char * buf, int len, char pad,
  UInt x);
int write_rj_usignedbigval_str(char * buf, int len, char pad,
  ULongLong x);
  
#endif /* ARCHIO_H */
