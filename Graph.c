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



#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "Arch.h"
#include "Graph.h"

/* remove arc from adjacency list */
inline void 
removeArc(Arc **Lx, Arc *nx)
{
    Arc *n = nx->next;
    Arc *p = nx->prev;

    if (*Lx == nx) {
        *Lx = n;
        if (n) n->prev  = p;
    } else {
        p->next = n;
        if (n) n->prev    = p;
        else (*Lx)->prev  = p;
    }
} /* remove_arc */

/* add arc to top of adjacency list */

inline void
insertArc(Arc **Lx, Arc *nx)
{
    Arc *ny = *Lx;
    
    *Lx = nx;
    
    if (ny) {
        
        nx->prev = ny->prev;
        nx->next = ny;
        ny->prev = nx;
        
    } else {
        
        nx->next = NULL;
        nx->prev = nx;
        
    }
    
} /* insert_arc */

Graph *
allocateGraph(char *name, UInt pts, Status *status)
{
    Graph   *g;
    bool       **Mx;

    UInt         c = pts;
    size_t       l = strlen(name) + 1;
    size_t       n = (pts + 1) * sizeof(void*);

    /* validate graph arguments */

    if (l > GRAPH_MAX_TITLE_LENGTH) {
        *status = STATUS_INVALID_NAME;
        return NULL;
    }

    if (pts > GRAPH_MAX_POINTS) {
        *status = STATUS_TOO_MANY_PTS;
        return NULL;
    }

    /* allocate memory for graph structure */

    l *= sizeof(char);

    EM(g,             sizeof(Graph),          e0);
    EM(g->name,       l,                      e1);
    EM(g->adj_lists,  n,                      e2);
    EM(g->adj_matrix, n ,                     e3);
    EM(g->degree, (pts+1) * sizeof(UInt),     e4);

    /* allocate adjacency matrix */

    n  = (pts + 1) * sizeof(bool);
    Mx = g->adj_matrix;

    do if ((*(++Mx) = malloc(n)) == NULL) {

        /* recover already allocated memory */
        while (c++ < pts ) free(*(--Mx));
        goto e5;

    } while (--c);

    g->vertex_count = pts;
    memcpy(g->name, name, l);

    *status = STATUS_OK;
    return g;
    
    /* error recovery ladder */

e5: free(g->degree);
e4: free(g->adj_matrix); 
e3: free(g->adj_lists);
e2: free(g->name);
e1: free(g);
e0: *status = STATUS_NO_MEM;
    
    return NULL;

} /* allocateGraph */

Graph *
initGraph(Graph *g)
{

    if (!g) return NULL;
    g->edge_count = 0;
    resetAdjmatrix(g->adj_matrix, g->vertex_count);
    memset(g->adj_lists, 0, (g->vertex_count + 1) * sizeof(void*));
    memset(g->degree, 0, (g->vertex_count + 1) * sizeof(UInt));

    return g;
} /* init_graph */

Status 
createEdge(Graph *g, Vertex x, Vertex y)
{
    Arc    *n1, *n2;
    
    Arc   **L = g->adj_lists;
    bool   **M = g->adj_matrix;
        
    /* add each edge u->v to graph */
    
    if (M[x][y]) return STATUS_OK;
    
    EM(n1, sizeof(Arc), e0);
    EM(n2, sizeof(Arc), e1);
    
    n1->target = y;
    n1->cross  = n2;
    
    n2->target = x;
    n2->cross  = n1;
    
    /* insert new edges */
    
    insertArc(L + y, n2);
    insertArc(L + x, n1);
    
    M[x][y] = M[y][x] = true;
    
    g->edge_count++;
    
    if (g->degree) {
        g->degree[x]++;
        g->degree[y]++;
    }
    
    return STATUS_OK;
    
e1: free(n1);
e0:    return STATUS_NO_MEM;
    
} /* create_edges */


Status 
createEdges(Graph *g, VArray *l, Vertex u)
{
    Status   s;

    UInt     p = l->length;
    Vertex  *a = l->array;

    while (p--) {
        s = createEdge(g, u, *a++);    
        if (s != STATUS_OK) return s;
    }

    return STATUS_OK;
    
} /* create_edges */


void
releaseGraph(Graph *g)
{
    Arc     *nx, *t, **L;
    bool    **M;
    Vertex    x;
    
    M = g->adj_matrix;
    L = g->adj_lists;
    x = g->vertex_count;
    
    if (x) {
        
        while (x) {
            
            nx = L[x];
            free(M[x]);
            
            while (nx) {
                t = nx->next;
                free(nx);
                nx = t;
            }
            
            x--;
        }

        free(g->adj_matrix);
        free(g->adj_lists );
    }

    if (g->name) free(g->name);
    if (g->degree) free(g->degree);
    free(g);

} /* release_graph */



/*! Clear adjacency matrix. */

void 
resetAdjmatrix(bool **M, UInt pts)
{
    UInt l = pts;
    
    l++;
    l *= sizeof(bool);
    
    while(pts--) memset(*++M , false, l);

} /* reset_adjmatrix */

VArray *
allocateVArray(UInt pts, Status *st)
{
    VArray  *va;
    
    EM(va,         sizeof(VArray),       e0);
    EM(va->array,  sizeof(Vertex) * pts, e1);
    
    va->length = pts;
    
    return va;
    
e1:    free(va);
e0:    *st = STATUS_NO_MEM;
    
    return NULL;
}


VArray *
initVArray(VArray *va)
{
    UInt     l = va->length;
    Vertex  *p = va->array + l;
    
    if (!va) return va;
    
    /* init list to be each vertex in graph, 1, 2 ... n-1, n */
    p = va->array + l;
    while (l) *(--p) = l--;
    
    return va;
}

void
releaseVArray(VArray *va)
{
    free(va->array);
    free(va);
}


