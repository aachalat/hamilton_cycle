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
#include <stdio.h>

#include "Arch.h"
#include "Graph.h"
#include "DFSAlgorithms.h"
#include "HamiltonianCycle.h"

/* NOTES ON TERMS USED IN COMMENTS:  

segments and virtual edges: 
---------------------------
When a vertex degree count has been reduced to zero, it is now within
a *segment*.  A segment is a path sub-section of the current, and
yet to be found, potential Hamilton cycle. Segment endpoints are linked via 
a *virtual edge*. A virtual edge is stored by the virtualEdges array in the 
HCStateRef data structure.  Segments do not overlap each other.  

consistant state:
-----------------
A graph is considered to be in a *consistant* state during the execution of 
the algorithm iff all vertices remaining in the graph are of at least
degree 3. */

typedef struct hc_flags {
    bool isHamiltonian;
    bool isHamiltonCycle;
} HCFlags;


typedef enum {
    HC_ENDPOINT      = 1,
    HC_ANCHOR_POINT  = 2,
    HC_ANCHOR_EXTEND = 4,
    HC_FLIP_SOURCE   = 8,
    HC_ANCHOR_TYPE1  = 16,
    HC_FORCED_DEG2   = 64,
    HC_FORCED        = 128,
    HC_PRUNE_ONCE    = 512,
    HC_PRUNE_TEST    = 1024,
    HC_HAMILTONIAN   = 2048,
    HC_TERMINATE     = 4096
} HCArcStatus;


typedef struct hc_tape {
    HCArcStatus  status;
    Arc        *arc;
} HCTape;


struct hc_state {
    HCFlags     flags;
    HCDFSRef    dfs;          
    Arc      **adjList;       
    UInt        vertexCount;   
    HCTape     *pos;
    HCTape     *origin;
    UInt       *degree;        
    Vertex     *virtualEdge; 
    Vertex     *vertexOrder;  
    Arc       *removedEdges;
    Arc      **removedEdgesStack;
    Vertex     *deg2Stack;
};

/*! Removes the bit flags that indicate an endpoint of a segment.
Will restore the incoming arc 'a' back to the graph if nesseary.
Called only from extendSegments. */

static inline void
fixInArc(Arc **L, Arc *a, Vertex x, UInt *kPtr)
{
    UInt k = *kPtr;

    if (k & (HC_ENDPOINT)) {
        insertArc(L + x, a);
        k &= ~HC_ENDPOINT;
        *kPtr = k;
    }
} /* fixInArc */

/*! Remove all but one of the incoming arcs of the source vertex of arc 'a'.
The initial incoming arc (a->cross) is not removed.  This
is function is only called by extendSegments and is declared only
to allow more clarity in extendSegments for arc status attributes. */

static inline bool
removeForcedD2InArcs(Arc **L, Arc *a, UInt *d, Vertex **d2Ptr)
{
    Vertex  y;
    UInt    dy;
    
    Vertex *d2 = *d2Ptr;
    Arc   *p  = a->prev;
            
    do {
    
        y  = p->target;
        dy = d[y];
        
        if (dy == 2) break;
        if (--dy == 2) *(++d2) = y;
        
        d[y] = dy;
        removeArc(L + y, p->cross);
        p = p->prev;
        
    } while (a != p);
    
    if (a != p) {
    
        /* deg 1 vertex encountered */
        /* restore from a to n before exiting */
        
        a = a->prev;
        while (a != p){
            y = a->target;
            d[y]++;
            insertArc(L + y, a->cross);
            a = a->prev;
        }
        
        return true;
    }
    
    *d2Ptr = d2;
    return false;
} /* removeForcedD2InArcs */

/*! Extend all segments for this state of the graph.

The extention of the segment is attempted from both sides of the segment
until no further extention possible with current graph state.

This procedure follows the initial vector provided by the arc 'a' and 
follows the path until no degree 2 vertex is encountered or until a cycle
is created by hitting vertex 'z' (the otherside of the segment).

When the extention in current direction provided by arc 'a' can go no further,
the endpoint 'z' may have been reduced to degree 2.  In this case the 
new endpoint and z are swapped, a new initial arc is chosen at z and 
the process is repeated until search terminated or until no further 
extention possible from ether side of the segment.

Once a segment has been completely extended to its maximal length within
it's local domain, the process is repeated on a new segment
until the degree 2 stack 'd2' is reduced to 0.

Procedure returns true if current state of graph terminates search, ie. when
a vertex is reduced to degree of 1 or a cycle is forced. 

A cycle can only be forced when the current segment encounters its other
endpoint while trying to extend.

*/

static inline bool
extendSegments(HCStateRef s, Arc *a, Vertex z, UInt k, Vertex *d2)
{
    Vertex       ex, x;
    Arc        *c;                /* cross arc directed at current endpoint */

    UInt        *d  = s->degree;    
    Vertex      *e  = s->virtualEdge;   
    Arc       **L  = s->adjList; 
    
    HCTape  *hz = NULL;            /* tape position for other endpoint */
    HCTape  *hx = s->pos;    /* tape position for current endpoint */

extend_segment:

    /* attempt to traverse arc a */
    
    c = a->cross;
    x = a->target;
    
    
    /* store cross in tape so that in negative direction arc target always
       points at a vertex to be restored */

    hx++;
    hx->arc = c;

    if (x == z) {
    
        /* a cycle is forced */

        if (hz) fixInArc(L, hz->arc, z, &hz->status);

        /* restore focal point, no longer nessary to maintain tape position 
           at this point */

        d[c->target] = 2;         
        s->pos       = hx - 1;
        
        /* determine if cycle is a Hamilton cycle */
        
        hx++;
        s->flags.isHamiltonCycle = hx->status == HC_HAMILTONIAN;
        
        return false;
    }
    
    if ((ex = e[x])){
        
        /* arc has collided with a virtual edge */
    
        /* attempt remove the target of arc (x) from graph and continue
           to extend the segment. */
           
        if (d[x] > 2) { 
            if (removeForcedD2InArcs(L, c, d, &d2)){
                
                /* ensure that segment endpoint info removed and
                   that source of arc a is placed back on graph */
                   
                if (hz) fixInArc(L, hz->arc, z, &hz->status);
                hx->status = k;
                s->pos     = hx;
                return false;
            }
            
            k |= HC_FORCED_DEG2;
        }

        hx->status = k | HC_FORCED;
        d[x]       = 0;
        
        
        
        if (d[ex] != 2) {
        
            /* the other side of of virtual edge will not allow 
               segment to continue in current direction */
        
            if (d[z] != 2) {
                x = ex;
                goto finish_segment;
            } 
            
            /* otherside of segment is to be forced onto the cycle, 
               grow segment in the x->z direction */
            
            /* Two possiblities on what arc to extend at z. 
            
            The first is that z was a degree 2 vertex not on a virtual edge. 
            The    L[z] arc is the arc first used to extend segment towards current
            x. Since L[z] is of degree 2 we know that the previous arc is
            the other arc pointing out of z.  
            
            The second case is that L[z] is already on a virtual edge. In
            this case L[z] will only have one arc in its list and
            L[z]->prev == L[z].  
            
            By choosing L[z]->prev in both cases an expensive branch is avoided. 
            
            The same reasoning applies whenever L[z]->prev is chosen when
            switching endpoints */
            
            a    = L[z]->prev;
            d[z] = 0;
            
            if (hz) fixInArc(L, hz->arc, z, &hz->status);

            z    = ex;
            hz   = hx;
            k    = 0;
            
            goto extend_segment;
        }
        
        /* continue growing segment in the current direction */
        
        k     = 0;
        d[ex] = 0;
        a     = L[ex];
        
        goto extend_segment;
    } 
    
    
    if (d[x] == 2) {
    
        /* x is degree 2, add arc and continue in same direction */
        hx->status = k;
        k          = 0;
        d[x]       = 0;
        
        a          = c->prev;
        goto extend_segment;
    }
    
    /* Segment can not be extended further in current direction, 
       remove arc pointing into segment. */
       
    hx->status = k | HC_ENDPOINT;
    removeArc(L + x, c);
    
    if (d[z] != 2) goto finish_segment;

    a = L[z]->prev;

    if (hz) fixInArc(L, hz->arc, z, &hz->status);

    d[z] = 0;
    
    z    = x;
    hz   = hx;
    k    = 0;
    
    goto extend_segment;
    
finish_segment:
    
    /* A multigraph may have been created. Segment end points x & z may be
       physically adjacent along with their new virtual edge joining them.
       Remove actual edge to take away the multigraph status.   */
       
    if (d[z] < d[x]) {
        a = L[z];
        while (a && a->target != x) a = a->next;
    } else {
        a = L[x];
        while (a && a->target != z) a = a->next;
    }
    
    if (a) {
    
        c = a->cross;
        removeArc(L + a->target, c);
        removeArc(L + c->target, a);
        a->next = s->removedEdges;
        s->removedEdges = a;
        
        if (--d[x] == 2) {
            
            d[z]--;
            d[x] = 0;
            
            a = L[x]->prev;
            fixInArc(L, hx->arc, x, &hx->status);
            k = 0;
            goto extend_segment;
        }
        
        if (--d[z] == 2) {
            a    = L[z]->prev;
            if (hz) fixInArc(L, hz->arc, z, &hz->status);
            d[z] = 0;
            
            hz   = hx;
            z    = x;
            k    = 0;
            goto extend_segment;
        }
        
    } 
    
    /* a new virtual edge has been created that is consistant within
       its local area of the graph (its endpoints) */
    
    e[z] = x;
    e[x] = z;
    
    /* check if any more segments need to be grown */
    
    do x = *d2--; while (x && !d[x]);
    
    if (x) {
    
        /* there is a degree 2 vertex still on the stack,
           create a new segment or enlarge an existing one */

        if ((z = e[x])) {
            d[x] = 0;  /* enlarge a segment */
        } else  z = x;                 /* starts a new segment */
        
        a  = L[x];
        hz = NULL;
        k  = 0;
        
        goto extend_segment;
    }

    s->pos = hx;
    return true;

} /* extendSegments */



static inline UInt
restoreInArcsWithCount(Arc **L, Arc *a, UInt *d) {

    Vertex  v;
    
    UInt    c = 0;
    Arc   *p = a->prev;
    
    while (p != a) {
        v = p->target;
        insertArc(L + v, p->cross);
        d[v]++; 
        p = p->prev;
        c++;
    }
    
    return c;
} /* restoreInArcsWithCount */



static inline void
restoreEdges(Arc **L, Arc *a, UInt *d)
{
    Vertex  u, v;
    Arc   *n;

    while (a) {
        n = a->next;
        u = a->target;
        v = a->cross->target;
        insertArc(L + u, a->cross);
        insertArc(L + v, a);
        d[u]++;
        d[v]++;
        a = n;
    }
} /* restoreEdges */



static inline void
unrollArc(Arc **L, Vertex *e, UInt *d, Arc *a, UInt k)
{
    Vertex x;
    
    /* Restore arc: y<---a----x  */

    if (k & HC_ENDPOINT) {
        x = a->cross->target;
        insertArc(L + x, a);
        e[x] = 0;
    } else if (k & HC_FORCED) {
        x    = a->cross->target;
        e[e[x]] = x;
        d[x]    = (k & HC_FORCED_DEG2)? 
                      restoreInArcsWithCount(L, a, d) + 2 : 2;
    }
    
} /* unrollArc */



static inline Vertex *
removeInArcs(Arc **L, Arc *a, UInt *d, Vertex *d2)
{
    Vertex  x;
    Arc   *p = a->prev;
    
    while (p != a) {
        x = p->target;
        if (--d[x] == 2) *(++d2) = x;
        removeArc(L + x, p->cross);
        p = p->prev;
    }
    
    return d2;
}


/*! Extend or create a segment and remove the vertex x from the graph 
   by marking one or two arcs extending out of it as pivot arc(s). The
   call to extendAnchor must only be done while the graph is in 
   a consistant state. */

static inline bool
extendAnchor(HCStateRef s, Arc **L, Vertex *e, UInt *d, Vertex x)
{
    Vertex   y, ex;
    Arc    *a;
    UInt     k;

    Vertex  *d2 = s->deg2Stack;
    
    if ((ex = e[x])){
    
    /* case 1: source vertex of current arc is already on a virtual edge 
        and must be forced onto the potential hamilton cycle */
    
        *s->removedEdgesStack++ = s->removedEdges ;
        s->removedEdges         = NULL;
        
        a    = L[x];
        k    = HC_ANCHOR_POINT | HC_ANCHOR_EXTEND;
        d[x] = 0;
        return extendSegments(s, a, ex, k, removeInArcs(L, a, d, d2));
        
    } 
    
    a = L[x];
    y = a->target;
    
    if ((ex = e[y])) {
    
        /* case 2: same as first case but origin flipped */
        
        k = HC_ANCHOR_POINT | HC_FLIP_SOURCE | HC_ANCHOR_EXTEND;
        a = a->cross;
        
        *s->removedEdgesStack++ = s->removedEdges ;
        s->removedEdges         = NULL;
        
        d[y] = 0;
        if (!extendSegments(s, a, ex, k, removeInArcs(L, a, d, d2)))
            return false;
        
        if (!d[x]) return true;
        
        *s->removedEdgesStack++ = s->removedEdges ;
        s->removedEdges         = NULL;
        
        a    = L[x];
        d[x] = 0;
        k    = HC_ANCHOR_POINT | HC_ANCHOR_EXTEND;
        return extendSegments(s, a, e[x], k, removeInArcs(L, a, d, d2));
        
    }
    
    /* case 3: neither the source or the target of the arc is on a virtual
       edge, simple join them by a virtual edge and remove the
       two arcs joining them. */
    *s->removedEdgesStack++ = s->removedEdges ;
    
    removeArc(L + y, a->cross);
    removeArc(L + x, a);
    
    e[x] = y;
    e[y] = x;
    
    s->pos++;
    s->pos->arc    = a->cross;
    s->pos->status = HC_ANCHOR_POINT;
    
    *s->removedEdgesStack++ = NULL;
    s->removedEdges         = NULL;

    a    = L[x];
    d[x] = 0;
    k    = HC_ANCHOR_POINT | HC_ANCHOR_EXTEND;
    
    return extendSegments(s, a, y, k, removeInArcs(L, a, d, d2));    
    
} /* extendAnchor */


/*! Process required edges, and insert any initial segment focal points 
(degree 2 vertices).  Returns true only if runTurningMachine can
be entered.  This could be due to an unreported hamilton cycle, or a 
halting conditon may have occured (degree < 2 and not in a segment)  */

static Vertex
primeTape(HCStateRef s)
{
    UInt     dx;
    Vertex   ex;
    
    Vertex   x  = s->vertexCount + 1;
    Arc   **L  = s->adjList;
    Vertex  *e  = s->virtualEdge;
    UInt    *d  = s->degree;
    Vertex  *nv = s->vertexOrder;
    Vertex  *d2 = s->deg2Stack;
    
    /* check for any degree 2 vertices, or stop condition */
    
    while (--x){
        dx = d[x];
        if (dx < 2) return false;
        if (dx == 2) *(++d2) = x;
    }

    /* force any degree 2 vertices onto segments */
    if ((x = *d2)) {

        ex = e[x];
        
        if (ex) {
            d[x] = 0;
        } else ex = x;
        
        if (!extendSegments(s, L[x], ex, 0, --d2))
            return !s->flags.isHamiltonCycle;     
    } 
    
    /* repeatedly place pivot vertices until stop condition reached */
    x = 0;
    do {
        do x = nv[x]; while (!d[x]);
    } while (extendAnchor(s, L, e, d, x));
    
    return !s->flags.isHamiltonCycle;
    
} /* primeTape */


static inline HCTape *
unwindSearchEdge(Arc **L, Vertex *e, UInt *d, HCTape *hx)
{
    
    UInt         k = hx->status;
    Vertex       x;
    Arc        *a;
    
    /* roll back graph state to closest pivot point */
    while(!(k & (HC_ANCHOR_POINT | HC_TERMINATE))) {
    
        a = hx->arc;
        x = a->target;
        
        /**** NEGATIVE DIRECTION *****/
    
        /* restore vertex */

        unrollArc(L, e, d, a, k);
        
        d[x]    = 2;
        e[e[x]] = x;
        
        /* move tape head to the left and restart loop */
        hx--;
        k = hx->status;
    }
    
    return hx;
    
} /* unwindSearchEdge */



static inline Vertex
rotateAnchorPoint(HCStateRef s, Arc **L, Vertex *e, UInt *d,
 HCTape *hx, Vertex **d2Ptr)
{
    Vertex  *d2 = s->deg2Stack;
    Arc    *a  = hx->arc;
    UInt     k  = hx->status;
    Vertex   x  = a->target;
    Arc    *c  = a->cross;
    Vertex   y  = c->target;

    if (k & HC_ANCHOR_EXTEND) {
        unrollArc(L, e, d, a, k);
        
        e[e[x]] = x;
        d[x]    = 2 + restoreInArcsWithCount(L, c, d);
        
        removeArc(L + x, c);
        removeArc(L + y, a);

    } else {

        e[x] = 0;
        e[y] = 0;
    }
    
    /* Restore edges removed during previous branch */
    
    restoreEdges(L, s->removedEdges, d);
    
    /* Remove the edge current pivot point represents and give it 
    to the closest pivot point to the left to restore.  This indicates
    that no more search possible with the edge in question. (or at least
    until pivot point to the left encountered.) */
    
    a->next         = *--s->removedEdgesStack;
    s->removedEdges = a;

    if (--d[y] == 2) *(++d2) = y;
    if (--d[x] == 2) *(++d2) = x;
    
    *d2Ptr = d2;
    
    s->pos = hx - 1;
    
    if (k & HC_FLIP_SOURCE) return y;
    return x;
        
} /* rotateAnchorPoint */



static inline void
restoreAnchorPoint(HCStateRef s, Arc **L, Vertex *e, UInt *d,HCTape *hx)
{
    UInt    k = hx->status;
    Arc   *a = hx->arc;
    Arc   *c = a->cross;
    Vertex  x = a->target;
    Vertex  y = c->target;

    if (k & HC_ANCHOR_EXTEND) {

        unrollArc(L, e, d, a, k);

        e[e[x]] = x;
        d[x]    = 2 + restoreInArcsWithCount(L, c, d);
        
    } else {
        
        e[x] = 0;
        e[y] = 0;
        
        insertArc(L + x, c);
        insertArc(L + y, a);
        
    }

    /* Restore edges removed during previous branch */

    restoreEdges(L, s->removedEdges, d);
    s->removedEdges  = *--s->removedEdgesStack;
    
} /* restoreAnchorPoint */



static inline Vertex
ensureConsistent(HCStateRef s, Arc **L, Vertex *e, UInt *d, Vertex *d2, 
Vertex x, Vertex *nv)
{
    Vertex ey;
    Vertex y  = *d2;
    
    /* no vertices have been forced, return x as next pivot point */
    
    if (!y) return x;
    
    /* forced vertices, ensure graph is consistant */
    
    if ((ey = e[y])) {
        d[y] = 0;
    }
    else ey = y;
    
    if (!extendSegments(s, L[y], ey, 0, --d2)) return 0;
    
    /* x may have been absorbed by a segment, ensure return of next
    available pivot */
    
    if (!d[x]) {
        do x = nv[x]; while (!d[x]);
    }
    
    return x;
}



/* return true if unwound all the way back to initial graph state */

static HCTape*
pruneSearchSpace(HCStateRef s, SInt c)
{
    HCTape  *hx;
    
    UInt    *d  = s->degree;    
    Vertex  *e  = s->virtualEdge;
    Arc   **L  = s->adjList;

    HCTape  *stop = hx = s->pos;
    UInt     k    = stop->status;
    
    while (!(k & HC_TERMINATE) && (c > 0)) {
        if (k & HC_ANCHOR_TYPE1) c--;
        if (k & HC_ANCHOR_POINT) c--;
        if (k & HC_FORCED_DEG2)  c--;
    
        stop--;
        k = stop->status;
    } 
    
    stop++;
    
    hx = unwindSearchEdge(L, e, d, hx);
    while (hx > stop) {
        restoreAnchorPoint(s, L, e, d, hx);
        hx = unwindSearchEdge(L, e, d, hx - 1);
    }

    return hx;
}





/*! Execute the Hamilton cycle Turing Machine until a stop condition
encounted.  Returns true only if a Hamilton Cycle encountered. 

Procedure can be entered only under the following conditions:

1/ The tape has been primed (initialized).

2/ A Hamilton Cycle has been found and reported.

Condition 2 may be due to priming or running the Turing Machine.

Machine Description:
--------------------

The tape is stored by an array of type HCCyleArc.  The current read/write head
position is indicated by hx (passed by s->cycleArc). (The array's 0th index
is held by s->origin).

Two values are stored at each position of the tape, an arc and a status value.
The arc represents an edge on the potential Hamilton cycle. 
The status value is a bit field set from values of type HCArcStatus.

The initial direction of the machine is always negative (to the left).

The machine decides what to do at each position in the negative direction
by reading the hx->status value.  

This negative direction is for restoring graph attributes to a previous state.  
These can be arcs, edges, degree counts and virtual edge values.

Machine procedes in negative direction until an anchor point has been
hit or until the TERMINATE value read.  A TERMINATE value indicates
that the machine should stop and that no further search for a Hamilton Cycle
can be done (Search space is exhausted). In this case the machine (procedure)
returns false.

If a pivot point is hit the machine can proceed in the positive direction, 
(to the right) once the pivot point has been processed.

While running in the positive direction new edges (arcs) are added to the
potential hamilton cycle as well as status values indicating what to do 
when running in the negative direction.  

This positive direction is handled by the extendAnchor and
exendSegments procedures.  They both add arcs to the hamilton cycle
and increment the read/write head by one each time this is done.

The positive direction stops when one of two conditions occur:

1/ A cycle is encounted in the graph.
2/ A vertex has been reduced to a degree of 1.
           (see extendSegments for more details on both)
           
If a cycle is encountered the s->flags.isHamiltonCycle is set to true
only if the read head of the Turing Machine reads a HAMILTONIAN value
off the position one to the right of the position a cycle is found.

If this flag is set to true, the machine stops and returns true.
This true value holds multiple meanings:

1/ A Hamilton cycle found.
2/ Evaluation of search space is still not completed.
3/ The machine is in a state that can be re-entered. 

If no stop condition is hit, then the machine proceeds in the negative
direction and search continues.

*/
static bool
runTuringMachineWithPruning(HCStateRef s) 
{
    Vertex  *d2, x1, x, v;
    SInt     c;
    
    HCTape  *low   = s->origin;
    HCTape  *high  = low;
    bool     prune = false;

    UInt    *d  = s->degree;    
    Vertex  *e  = s->virtualEdge;
    Vertex  *nv = s->vertexOrder;
    Arc   **L  = s->adjList; 
    HCTape  *hx = unwindSearchEdge(L, e, d, s->pos); 

    s->flags.isHamiltonCycle = false;

    while (!(hx->status & HC_TERMINATE)) {
    
        x1 = rotateAnchorPoint(s, L, e, d, hx, &d2);
        x  = ensureConsistent(s, L, e, d, d2, x1, nv);
        
        if (x){

            if (prune) { 
                if (x!=x1) hx->status |= HC_ANCHOR_TYPE1;
                high = low;
                c    = 1;
                v    = x;
                while ((v = nv[v])) if (d[v]) c++;
                
                if (getComponentDiff(s->dfs,L,e,d,nv,x,&c, x == x1)){
                    hx = pruneSearchSpace(s, c);
                    continue;
                }
                prune = false;
            }

            while (extendAnchor(s, L, e, d, x)){
                do x = nv[x]; while (!d[x]);
            }
        }
        
        if (s->flags.isHamiltonCycle) return true;
        hx   = unwindSearchEdge(L, e, d, s->pos); 
    
        if (hx > high) high = hx;
        else prune = hx < high;
    }
    
    s->pos = hx;
    return false; 
    
} /* runTuringMachineWithPruning */



static bool
runTuringMachine(HCStateRef s) 
{
    Vertex  *d2, x;
    
    UInt    *d  = s->degree;    
    Vertex  *e  = s->virtualEdge;
    Vertex  *nv = s->vertexOrder;
    Arc   **L  = s->adjList; 
    HCTape  *hx = unwindSearchEdge(L, e, d, s->pos); 
    s->flags.isHamiltonCycle = false;

    while (!(hx->status & HC_TERMINATE)) {
    
        x  = rotateAnchorPoint(s, L, e, d, hx, &d2);
        x  = ensureConsistent(s, L, e, d, d2, x, nv);
        if (x){
            while (extendAnchor(s, L, e, d, x)){
                do x = nv[x]; while (!d[x]);
            }
        }
        
        if (s->flags.isHamiltonCycle) return true;
        hx  = unwindSearchEdge(L, e, d, s->pos); 
    }
    
    s->pos = hx;
    return false; 
    
} /* runTuringMachine */



static void
restoreGraph(HCStateRef s)
{

    UInt        *d  = s->degree;    
    Vertex      *e  = s->virtualEdge;
    Arc       **L  = s->adjList; 
    HCTape  *hx = unwindSearchEdge(L, e, d, s->pos);

    while (!(hx->status & HC_TERMINATE)) {
        restoreAnchorPoint(s, L, e, d, hx);
        hx = unwindSearchEdge(L, e, d, hx - 1);
    }
    
    restoreEdges(L, s->removedEdges, d);
        
} /* restoreGraph */



static void
resetStateAndRestoreGraph(HCStateRef s)
{
    
    restoreGraph(s);
    
    s->flags.isHamiltonian   = false;
    s->flags.isHamiltonCycle = false;

    /* wiped virtual edges and initialize tape */
    
    memset(s->virtualEdge, 0, sizeof(Vertex) * (s->vertexCount + 1));
    memset(s->origin, 0, (s->vertexCount + 2) * sizeof(HCTape));
    
    s->pos = s->origin;
    s->pos[s->vertexCount+1].status = HC_HAMILTONIAN;
    s->pos->status                  = HC_TERMINATE;

    s->removedEdges       = NULL;
    *s->removedEdgesStack = NULL;
    
} /* resetStateAndRestoreGraph */



/*! Find the next hamilton cycle in the search space. Returns false 
 when no more Hamilton cycles found. */

bool
nextHamiltonianCycle(HCStateRef s) {

    return runTuringMachine(s);
    
} /* nextHamiltonCycle */



/*! Find the first Hamilton cycle in the search space. Returns false when no
Hamilton cycle found. */

bool
firstHamiltonianCycle(HCStateRef s)
{
    resetStateAndRestoreGraph(s);
    
    if (primeTape(s) 
         && !runTuringMachine(s)) return false;
    
    s->flags.isHamiltonian = s->flags.isHamiltonCycle;
    
    return s->flags.isHamiltonCycle;
    
} /* firstHamiltonCycle */

/*! Find the next hamilton cycle in the search space. Returns false 
 when no more Hamilton cycles found. */

bool
nextHamiltonianCycleWithPruning(HCStateRef s) {

    return runTuringMachineWithPruning(s);
    
} /* nextHamiltonCycle */



/*! Find the first Hamilton cycle in the search space. Returns false when no
Hamilton cycle found. */

bool
firstHamiltonianCycleWithPruning(HCStateRef s)
{
    resetStateAndRestoreGraph(s);
    
    if (primeTape(s) 
         && !runTuringMachineWithPruning(s)) return false;
    
    s->flags.isHamiltonian = s->flags.isHamiltonCycle;
    
    return s->flags.isHamiltonCycle;
    
} /* firstHamiltonCycle */



HCStateRef
allocateHCState(UInt points, StatusRef status)
{
    HCStateRef s;
    UInt       n = 1 + points;
    
    CHECK_RETURN_VAL(*status, NULL);

    EM(s,                     sizeof(struct hc_state),           e0);
    EM(s->virtualEdge,        n * sizeof(Vertex),              e1);
    EM(s->deg2Stack,          n * sizeof(Vertex),              e2);
    EM(s->removedEdgesStack,  n * sizeof(void *),              e3);
    EM(s->pos,                (points + 2) * sizeof(HCTape),     e4);
    EM(s->vertexOrder,        n * sizeof(Vertex),              e5);
    
    s->dfs         = allocateDFS(points);
    s->vertexCount = points;
    
    if (!s->dfs) goto e6;

    return s;

e6: free(s->vertexOrder);
e5: free(s->pos);
e4: free(s->removedEdgesStack);
e3: free(s->deg2Stack);
e2: free(s->virtualEdge);
e1:    free(s);
    
e0:    *status = STATUS_NO_MEM;
    return NULL;
    
} /* allocHCState */


HCStateRef
initHCState(HCStateRef s, UInt *d, Arc **adj, VArray *vo)
{

    Vertex *nv, *vl, x;

    s->adjList            = adj;
    s->degree             = d;
    s->origin             = s->pos;
    *s->deg2Stack         = 0;
    s->removedEdges       = NULL;
    *s->removedEdgesStack = NULL;
    
    s->pos[s->vertexCount+1].status = HC_HAMILTONIAN;
    s->pos->status                  = HC_TERMINATE;
    
    /* set up vertex order */
    
    nv  = s->vertexOrder;
    vl  = vo->array;
    *nv = *vo->array;
    for (x = 1 ; x < (Vertex)s->vertexCount ; x++) nv[vl[x-1]] = vl[x];
    
    /* mark the end of the list, allows for a vertex remaining count */
    nv[ vl[x-1] ] = 0;        
    
    return s;
} /* initHCState */


void
releaseHCState(HCStateRef s)
{
    if (!s) return;
    
    restoreGraph(s);
    releaseDFS(s->dfs);
    
    free(s->vertexOrder);
    free(s->origin);
    free(s->removedEdgesStack);
    free(s->deg2Stack);
    free(s->virtualEdge);
    free(s);
    
} /* releaseHCState */


/* v is a 2n+1 sized array initialized to 0's
 returns cycle ptrs where v[x] and v[n+x] point to adjacent vertices
 in cycle to x*/
void
getCurrentHamiltonianCycle(HCStateRef s, Vertex *v )
{
    HCTape *tp  = s->origin + 1;
    UInt    pts = s->vertexCount;
    Vertex *vn  = v + pts;
    Vertex  x, y;

    while (pts--) {
        x = tp->arc->target;
        y = tp->arc->cross->target;
        
        if (v[x]) vn[x] = y;
        else v[x] = y;
        
        if (v[y]) vn[y] = x;
        else v[y] = x;
        
        tp++;
    }
}


/* e is a 2n sized array
 returns vertex pairs where each mod(2) pair form an edge x */
void
getCurrentHamiltonianCycleEdges(HCStateRef s, Vertex *e)
{
    Vertex *ep = e + 2 * s->vertexCount;
    HCTape *tp = s->origin + 1;
    
    while (ep != e){
        *(--ep) = tp->arc->target;
        *(--ep) = tp->arc->cross->target;
        tp++;
    }    
}
