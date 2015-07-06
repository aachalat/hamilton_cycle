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
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "Arch.h"
#include "Graph.h"
#include "DFSAlgorithms.h"

struct dfs_compbipt {
    
    /*! Array indexed by vertices.  Indicates traversal number of each
        indexed vertex. */
        
    UInt   *visit;    
    
    /*! Array indexed by vertices.  Indictates lowest traversal value 
        encounterd by the indexed vertex during the dfs. */
    
    UInt   *low;      
    
    /*! Array indexed by vertices. A non-zero value at a vertex's index
        indicates that said vertex is a cutpoint.  The value indicates
        the number of potential components surrounding the indexed vertex. */
    
    UInt   *components;
    UInt   *branches;
    /*! Array indexed by vertices. Contains bipartite set membership for
        indexed vertices. */
    SInt   *colour;
    
    /*! Array indexed by vertices. Contains node pointer of a vertex's arc 
        pointing to the return vertex for the non-recursive dfs algorithm.
        The dfs algorithm in dfs_ccb uses the circularly linked prev
        value of a node to traverse the target graph. The value stored
        by the stop array is used to stop iteration over a vertex and
        ascend back up the search tree. */
    Arc    **iterator;
    Vertex   *previous;
    UInt      stateSize; /* Number of vertices structure allocated with */ 
    
};


HCDFSRef
allocateDFS(UInt pts)
{
   
    HCDFSRef dfs;
    
    UInt     ilen = sizeof(UInt);
    UInt     slen = sizeof(SInt);
    UInt     plen = sizeof(void*);
    UInt     vlen = sizeof(Vertex);
    
    pts++;
    ilen *= pts;
    slen *= pts;
    plen *= pts;
    vlen *= pts;
    
    EM(dfs,           sizeof(struct dfs_compbipt), e0);
    EM(dfs->visit,    ilen,                        e1);
    EM(dfs->branches, ilen,                        e2);
    EM(dfs->low,      ilen,                        e3);
    EM(dfs->colour,   slen,                        e4);
    EM(dfs->previous, vlen,                        e5);
    EM(dfs->iterator, plen,                        e6);
    EM(dfs->components, ilen, e7);
    
    dfs->stateSize = pts;
    return dfs;

e7: free(dfs->iterator);
e6:    free(dfs->previous);
e5:    free(dfs->colour);
e4:    free(dfs->low);
e3:    free(dfs->branches);
e2:    free(dfs->visit);
e1:    free(dfs);

e0:    return NULL;

} /* allocDFSCutBipart */


void releaseDFS(HCDFSRef dfs)
{

    if (!dfs) return;
    
    free(dfs->components);
    free(dfs->iterator);
    free(dfs->previous);
    free(dfs->colour);
    free(dfs->low);
    free(dfs->branches);
    free(dfs->visit);
    free(dfs);
    
} /* free_dfs_compbipt */


void
initDfSeparatingSet(HCDFSRef dfs)
{
    memset(dfs->visit, 0, dfs->stateSize * sizeof(UInt));
} /* initDfSeparatingSet */


UInt
dfSeparatingSet(HCDFSRef dfs, Arc **L, Vertex *e, Vertex x, UInt pos, 
     UInt *diff, bool xInSep, bool *hasCutPoint)
{
    UInt     lx, ly, bx, cmx;
    SInt     cx;
    Vertex   px;
    
    Vertex  *p   = dfs->previous;      
    Arc   **n   = dfs->iterator;      
    UInt    *b   = dfs->branches;
    SInt    *c   = dfs->colour;
    UInt    *l   = dfs->low;
    UInt    *v   = dfs->visit;
    bool     bp  = true;
    Vertex   y   = x;
    Arc    *a   = NULL;
    UInt     vy  = 0;   /* typically visit order of v[y] */
    UInt     cd  = 0;   /* optimized components - cutpoints */
    UInt     cp  = 0;   /* cutpoints */
    UInt    *cm  = dfs->components;
    SInt     bd  = 0;   /* bipartite difference */
    
    b[0] = 0;
    c[0] = 1;
    x    = 0;
    lx   = 0;
    
    for (;;) {
        while (!vy) {
            /* descend into y */
            n[x] = a;
            p[y] = x;

            if (bp) {
                /* bipartite so far, see if y can be added to a set */
                cx   = -c[x];
                c[y] = cx;
                bd  += cx;
            }

            x     = y;
            a     = L[x];
            b[x]  = 0;
            cm[x] = 0;
            lx    = ++pos;
            v[x]  = lx;
            l[x]  = lx;
            y     = e[x];

            if (!y) { 
                y = a->target;
                a = a->next;
            }
            
            vy = v[y];
        }

        /* ensure that y iterates to unvisited vertex */
        px = p[x];
        cx = c[x];
        do {

            if (px != y) {
                if (lx > vy) lx = vy;
                if (bp)      bp = cx != c[y];
            }

            if (!a) {
                y = 0;
                break;
            }
            
            y  = a->target;
            a  = a->next;
            vy = v[y];
            
        } while (vy);
        
        l[x] = lx;
        if (y) continue;

        /* ascend out of x  */
        do {
            y   = px;
            cmx = cm[x];
            bx  = b[x];
            
            if (!y) goto done;
            
            ly  = l[y];
            vy  = v[y];
            
            if (bx) { 
                /* update components - cut points */
                cp++;
                if (cmx>1) cd += (cmx - 1);
            }
            
            if (lx == vy) cm[y]++;
            else if (lx > vy && cmx == 1) cm[y]++;
        
            /* check for branch */
            if (lx >= vy) b[y]++;
            else if (ly > lx) l[y] = ly = lx;
            
            x  = y;
            lx = ly;
            a  = n[x];
            px = p[x];
        } while (!a);
        
        y  = a->target;
        a  = a->next;
        vy = v[y];
        
    }  

done:

    /* update components - cut points */
    if (bx > 1)  cp++;
    if (cmx > 1) cd += (cmx - 1);
    
    /* return maximum separating set found and ensure x is counted in
       separating set if required */
    
    *hasCutPoint = cp > 0;
    
    if (!cp && !bp) {
        *diff = 0;
        return pos;
    }
    
    if (bp) {
        if (xInSep && bd < 0) bd += 2; 
        if (bd < 0) bd = -bd;
    } else bd = 0;
    
    *diff = ((UInt) bd) > cd ? (UInt)bd : cd;

    return pos;
    
} /* dfSeparatingSet */



bool
getComponentDiff(HCDFSRef dfs, Arc **L, Vertex *e, UInt *d, Vertex *nv, 
Vertex x, SInt *c, bool inSepSet)
{
    UInt *v;
    
    UInt pts   = *c;
    UInt p     = 0;
    UInt diff  = 0;
    UInt tdiff = 0;
    bool cp    = false;
    
    initDfSeparatingSet(dfs);
    
    p = dfSeparatingSet(dfs, L, e, x, p, &diff, inSepSet, &cp);
    
    if (p < pts) {
        v     = dfs->visit;
        tdiff = 0;
        while (v[x] || !d[x]) x = nv[x];
        while ((p = dfSeparatingSet(dfs,L,e,x,p,&tdiff,false,&cp)) < pts) {
            diff += (tdiff>0)?tdiff:1;
            tdiff = 0;
            while (v[x] || !d[x]) x = nv[x];
        } 
        
        diff += (tdiff>0)?tdiff:1;
        cp    = true;
    }
    
    *c = diff;
    return cp || diff > 0;
} /* getComponentDiff */

