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

#ifndef _DFSALGORITHMS_H_
#define _DFSALGORITHMS_H_

#include "Arch.h"
#include "Graph.h"

/*! data structure storing dfs state for calls to dfs_ccb */

typedef struct dfs_compbipt * HCDFSRef;

HCDFSRef allocateDFS(UInt pts);
void releaseDFS(HCDFSRef dfs);

void initDfSeparatingSet(HCDFSRef dfs);

UInt
dfSeparatingSet(HCDFSRef dfs, Arc **L, Vertex *e, Vertex x, UInt pos, 
UInt *diff, bool xInSep, bool *hasCutPoint);

/* return true if cutpoint found or if unequal bipartition found,
on input c is the number of vertices in current graph state,
on completion c is number of components more than number of cutpoints, or
0 if equal or less, c is only valid if true returned */

bool
getComponentDiff(HCDFSRef dfs, Arc **L, Vertex *e, UInt *d, Vertex *nv, 
Vertex x, SInt *c, bool inSepSet);


#endif /* _DFSALGORITHMS_H_ */
