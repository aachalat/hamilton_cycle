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

#ifndef GRAPH_H
#define GRAPH_H

#include "Arch.h"

#define GRAPH_MAX_POINTS         100000
#define GRAPH_MAX_TITLE_LENGTH   120

typedef SInt Vertex;

/*! Adjacency List. */
typedef struct arc {
    Vertex      target;  /*!< Vertex adjacent to this node. */
    struct arc *next;    /*!< Next node (null terminated list). */
    struct arc *prev;    /*!< Back link (circular list). */
    struct arc *cross;   /*!< Opposite end of edge. */
} Arc;

/*! List of vertices */
typedef struct vertex_list {
    Vertex v;
    struct vertex_list *next;
} VList;

typedef struct vertex_array {
    Vertex     *array;         
    UInt        length;         
} VArray;

/*! Graph Information Structure Adapted from Groups and Graphs. */
typedef struct graph {
    char    *name;           /*!< Graph title.                */
    UInt     vertex_count;   /*!< Number of Vertices.         */
    UInt     edge_count;     /*!< Number of edges.            */
    Arc   **adj_lists;      /*!< Array of adjacency lists.   */
    bool   **adj_matrix;     /*!< Adjacency matrix            */
    UInt    *degree;         /*!< degree list, not always initialized */
} Graph;

/*! status flags for i/o and memory failure and incorrect input,
    ideally only for checking for extraneous circumstances and 
    should not impact normal application running if everything works 
    perfectly. ie all code analyzing and using status information from 
    this structure could be removed and the code will still work if no
    errors occur. 
 
    Generally the status value should always be STATUS_OK in value
    on entry to any procedure that requires the flag, otherwise it should
    fail immediately. */

typedef enum status {
    STATUS_OK                   = 0,
    STATUS_NO_MEM               = 1,
    STATUS_INVALID_NAME         = 2,
    STATUS_TOO_MANY_PTS         = 3,
    STATUS_TIMING_ERROR         = 5,
    STATUS_MISSING_REFERENCE    = 6,
    STATUS_FILE_READ_ERROR      = 7,
    STATUS_STREAM_MISSING_TOKEN = 8,
    STATUS_INVALID_INPUT        = 9
} Status;

typedef Status * StatusRef;

/* the following macros should be used when controlling application flow
   that depend on status values.  this allows for logging to be easily
   injected into code that fails status checks. */

/* macro for quickly checking status and a label to jump to if
   status not ok */

#define CHECK_JUMP(status, fail_label)    \
if ((status) != STATUS_OK) goto fail_label

/* macro for quickly checking status exiting procedure if failed */

#define CHECK_RETURN(status)    \
if ((status) != STATUS_OK) return


/* macro for quickly checking status exiting procedure if failed, 
   returning val */

#define CHECK_RETURN_VAL(status, val)    \
if ((status) != STATUS_OK) return (val)

Graph *allocateGraph(char *name, UInt pts, StatusRef s);
Graph *allocateGraph_with_deg(char *name, UInt pts, StatusRef s);
Graph *initGraph(Graph *g);

VArray *allocateVArray(UInt pts, Status *st);
VArray *initVArray(VArray *va);
void releaseVArray(VArray *va);

void insertArc(Arc **Lx, Arc *nx);
void removeArc(Arc **Lx, Arc *nx);

Status createEdge(Graph *g, Vertex x, Vertex y);
Status createEdges(Graph *g, VArray *l, Vertex x);

void releaseGraph(Graph *g);
void resetAdjmatrix(bool **M, UInt pts);

#endif /* GRAPH_H */

