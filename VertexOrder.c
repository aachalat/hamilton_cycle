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



#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "Arch.h"
#include "Graph.h"
#include "VertexOrder.h"


static int
vertex_compare_desc(void *deg, const void *a, const void *b)
{
    UInt  *d  = (UInt*)deg;
    UInt   da = d[*((Vertex*)a)];
    UInt   db = d[*((Vertex*)b)];
    
    if (da > db ) return -1;
    if (db == da) return 0;
    return 1;
    
} /* vertex_compare */


static int
vertex_compare_asc(void *deg, const void *a, const void *b)
{
    UInt  *d  = (UInt*)deg;
    UInt   da = d[*((Vertex*)a)];
    UInt   db = d[*((Vertex*)b)];
    
    if (da > db ) return 1;
    if (db == da) return 0;
    return -1;
    
} /* vertex_compare */

/* shell sort implementation.

 Used instead of qsort (as it does not have consistent interface across 
 platforms). 
 
 Code adapted from source listed on wikipedia entry for shell sort, 
 
 Function Declaration matches that of qsort_r

*/

static void
shell_sort(void *a, int l, int s, void *t,
 int (*comp)(void *, const void *, const void *))
{

    ptrdiff_t step;
    
    char *ai, *aj, *an;    /* block ptrs wrt byte array */
    char p[32];            /* handles up to 256 bit types */
    
    void *v   = &p;
    int   d   = l >> 1;    /* step distance in terms of orig array */
    char *ae  = (char*)a + (l*s);
    
    /* list now treated as byte array with s length blocks */

    while (d > 0){
    
        step = d * s;
        an   = ai = (char*)a;
        ai  += step;
        aj   = ai;
        
        while (ai < ae) {
            memcpy(v, ai, s);
            while ((aj != (char *)a) && (comp(t, an, v) > 0)) {
                memcpy(aj, an, s);
                aj  = an;
                an -= step;
            }
            memcpy(aj, v, s);
            an  = ai;
            ai += step;
            aj  = ai;
        }
        
        d = (d != 2)? (int)(d / 2.2) : 1;
    }
}


VArray *
sortVerticesDegreeAsc(VArray *va, UInt *d)
{
    shell_sort(va->array, va->length, 
                sizeof(Vertex), d, &vertex_compare_asc);
    return va;
}


VArray *
sortVerticesDegreeDesc(VArray *va, UInt *d)
{
    shell_sort(va->array, va->length, 
                sizeof(Vertex), d, &vertex_compare_desc);
    return va;
}
