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


#include "ArchIO.h"
#include "Arch.h"
#include <stdio.h>
#include <stdlib.h>

inline int 
get_usigned_dec_length(const UInt x)
{
    
    if (x<10) return 1;
    if (x<100) return 2;

#if UInt_BIT == 8
    /* 8 bit cut off */ 
    return 3;
#endif

#if UInt_BIT >= 16

    if (x<1000) return 3;
    if (x<10000) return 4;

#if UInt_BIT == 16
    /* 16 bit cut off */
    return 5;
#endif     
    
#if UInt_BIT >= 32
    if (x<100000) return 5;
    if (x<1000000) return 6;
    if (x<10000000) return 7;
    if (x<100000000) return 8;
    if (x<1000000000) return 9;

#if UInt_BIT == 32
    /* 32 bit cut off */
    return 10;
#endif 

#if UInt_BIT >= 64

    if (x<10000000000) return 10;
    if (x<100000000000) return 11;
    if (x<1000000000000) return 12;
    if (x<10000000000000) return 13;
    if (x<100000000000000) return 14;
    if (x<1000000000000000) return 15;
    if (x<10000000000000000) return 16;
    if (x<100000000000000000) return 17;
    if (x<1000000000000000000) return 18;
    if (x<10000000000000000000) return 19;
    
    return 20;

#endif /* 64 bit */
#endif /* 32 bit */
#endif /* 16 bit */    
} /* get_usigned_dec_length */


inline int 
get_usignedbigval_dec_length(const ULongLong x)
{
    
    if (x<10) return 1;
    if (x<100) return 2;

#if ULongLong_BIT == 8
    /* 8 bit cut off */ 
    return 3;
#endif

#if ULongLong_BIT >= 16

    if (x<1000) return 3;
    if (x<10000) return 4;

#if ULongLong_BIT == 16
    /* 16 bit cut off */
    return 5;
#endif     
    
#if ULongLong_BIT >= 32
    if (x<100000) return 5;
    if (x<1000000) return 6;
    if (x<10000000) return 7;
    if (x<100000000) return 8;
    if (x<1000000000) return 9;

#if ULongLong_BIT == 32
    /* 32 bit cut off */
    return 10;
#endif 

#if ULongLong_BIT >= 64

    if (x<10000000000ULL) return 10;
    if (x<100000000000ULL) return 11;
    if (x<1000000000000ULL) return 12;
    if (x<10000000000000ULL) return 13;
    if (x<100000000000000ULL) return 14;
    if (x<1000000000000000ULL) return 15;
    if (x<10000000000000000ULL) return 16;
    if (x<100000000000000000ULL) return 17;
    if (x<1000000000000000000ULL) return 18;
    if (x<10000000000000000000ULL) return 19;
    
    return 20;

#endif /* 64 bit */
#endif /* 32 bit */
#endif /* 16 bit */    
} /* get_usignedbigval_dec_length */



inline int 
get_signed_dec_length(SInt x)
{
    
    if (x<0) x=-x;
    if (x<10) return 1;
    if (x<100) return 2;

#if SInt_BIT == 8
    /* 8 bit cut off */ 
    return 3;
#endif

#if SInt_BIT >= 16

    if (x<1000) return 3;
    if (x<10000) return 4;

#if SInt_BIT == 16
    /* 16 bit cut off */
    return 5;
#endif     
    
#if SInt_BIT >= 32
    if (x<100000) return 5;
    if (x<1000000) return 6;
    if (x<10000000) return 7;
    if (x<100000000) return 8;
    if (x<1000000000) return 9;

#if SInt_BIT == 32
    /* 32 bit cut off */
    return 10;
#endif 

#if SInt_BIT >= 64

    if (x<10000000000) return 10;
    if (x<100000000000) return 11;
    if (x<1000000000000) return 12;
    if (x<10000000000000) return 13;
    if (x<100000000000000) return 14;
    if (x<1000000000000000) return 15;
    if (x<10000000000000000) return 16;
    if (x<100000000000000000) return 17;
    if (x<1000000000000000000) return 18;
    return 19;

#endif /* 64 bit */
#endif /* 32 bit */
#endif /* 16 bit */    
} /* get_signed_dec_length */



/* reading IO */

inline bool
read_signedval(FILE *inp, SInt *xptr)
{
    int c;
    bool neg;
    SInt x;
    
    neg=false;
    do {
        c=fgetc(inp);
        if (c==EOF) return true;
        if (c=='-') {
            c=fgetc(inp);
            if (c==EOF) return true;
            if (c < (int)'0' || c > (int)'9') continue;
            neg=true;
            break;
        }
    } while (c < (int)'0' || c > (int)'9');

    x=0;

    do {
        c-=(int)'0';
        x*=10;
        x+=(SInt)c;
        c=fgetc(inp);
    } while (c >= (int)'0' && c <= (int)'9');

    ungetc(c,inp);
    if (neg) x=-x;
    *xptr=x;

    return false;
} /* read_signedval */


inline bool
read_usignedval(FILE *inp, UInt *xptr)
{
    int c;
    UInt x;
    
    do {
        c=fgetc(inp);
        if (c==EOF) return true;
    } while (c < (int)'0' || c > (int)'9');

    x=0;
    do {
        c-=(int)'0';
        x*=10;
        x+=(UInt)c;
        c=fgetc(inp);
    } while (c >= (int)'0' && c <= (int)'9');
    ungetc(c,inp);
    *xptr=x;
    return false;
} /* read_usignedval */


inline bool
read_usignedbigval(FILE *inp, ULongLong *xptr)
{
    int c;
    ULongLong x;
    
    do {
        c=fgetc(inp);
        if (c==EOF) return true;
    } while (c < (int)'0' || c > (int)'9');

    x=0;
    do {
        c-=(int)'0';
        x*=10;
        x+=(UInt)c;
        c=fgetc(inp);
    } while (c >= (int)'0' && c <= (int)'9');
    ungetc(c,inp);
    *xptr=x;
    return false;
} /* read_usignedbigval */



/* writing IO */

inline int
write_signedval_str(char * buf, SInt x)
{
    int c,l;
    bool neg;
    char *p;
    
    if (x<0) {
        neg=true;
        x=-x;
    } else neg=false;
    
    l=get_signed_dec_length(x);
    if (neg) l++;
    
    p=buf+l;
    do {
        c='0';
        c+=(x%10);
        x/=10;
        (*--p)= (char)c;
    } while (x);
    
    if (neg) *--p='-'; 
    return l;
} /* write_signedval_str */


inline int
write_usignedval_str(char * buf, UInt x)
{
    int c,l;
    char *p;
    
    l=get_usigned_dec_length(x);
    p=buf+l;
    do {
        c='0';
        c+=(x%10);
        x/=10;
        (*--p)= (char)c;
    } while (x);

    return l;
} /* write_usignedval_str */


inline int
write_usignedbigval_str(char * buf, ULongLong x)
{
    int c,l;
    char *p;
    
    l=get_usignedbigval_dec_length(x);
    p=buf+l;
    do {
        c='0';
        c+=(int)(x%10);
        x/=10;
        (*--p)= (char)c;
    } while (x);

    return l;
}

/* right justified writing IO */

inline int
write_rj_usignedbigval_str(char * buf, int len, char pad, ULongLong x)
{
    char *p;
    int l,c;
    
    if (len>MAX_ULongLong_STR_LEN) 
        len = MAX_ULongLong_STR_LEN;
    else {
        l = get_usignedbigval_dec_length(x);
        if (l>len) {
            /* error */
            for (l=0;l<len;l++) buf[l]=RJUST_OVERFLOW_CHAR;
            return len;
        }
    }

    p = buf + len;
    while (x){
        c='0';
        c+=(int)x % 10;
        x /= 10;
        (*--p)=(char)c;
    }

    while (p!=buf) (*--p)=pad;
    return len;
} /* write_rj_usignedbigval_str */


inline int
write_rj_usignedval_str(char * buf, int len, char pad, UInt x)
{
    char *p;
    int l,c;
    
    if (len>MAX_UInt_STR_LEN) 
        len = MAX_UInt_STR_LEN;
    else {
        l = get_usigned_dec_length(x);
        if (l>len) {
            /* error */
            for (l=0;l<len;l++) buf[l]=RJUST_OVERFLOW_CHAR;
            return len;
        }
    }

    p = buf + len;
    while (x){
        c='0';
        c+=(x % 10);
        x /= 10;
        (*--p)=(char)c;
    }

    while (p!=buf) (*--p)=pad;
    return len;
} /* write_rj_usignedval_str */


inline int
write_rj_signedval_str(char * buf, int len, char pad, SInt x)
{
    char *p;
    int l,c;
    bool neg;
    
    neg=false;
    if (x<0) {
        x=-x;
        neg=true;
    }
    
    if (len>MAX_SInt_STR_LEN) 
        len = MAX_SInt_STR_LEN;
    else {
        l = get_signed_dec_length(x);
        if (neg) l++;
        if (l>len) {
            /* error */
            for (l=0;l<len;l++) buf[l]=RJUST_OVERFLOW_CHAR;
            return len;
        }
    }

    p = buf + len;
    while (x){
        c='0';
        c+=(x % 10);
        x /= 10;
        (*--p)=(char)c;
    }
    
    if (neg) (*--p)=(char)'-';
    while (p!=buf) (*--p)=pad;
    return len;
} /* write_rj_signedval_str */
