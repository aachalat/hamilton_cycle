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

#ifndef HAMILTONIANCYCLE_H
#define HAMILTONIANCYCLE_H

#include "Arch.h"
#include "Graph.h"

typedef struct hc_state * HCStateRef; /* opaque type */

HCStateRef allocateHCState(UInt pts, Status *status);
HCStateRef initHCState(HCStateRef s, UInt *d, Arc **adj, VArray *vo);
                        
void releaseHCState(HCStateRef s);
bool firstHamiltonianCycle(HCStateRef s);
bool nextHamiltonianCycle(HCStateRef s);
bool firstHamiltonianCycleWithPruning(HCStateRef s);
bool nextHamiltonianCycleWithPruning(HCStateRef s);

/* v is a 2n+1 sized array initialized to 0's
   returns cycle ptrs where v[x] and v[n+x] point to adjacent vertices
   in cycle to x*/
void getCurrentHamiltonianCycle(HCStateRef s, Vertex *v );

/* e is a 2n sized array initialized to 0's
   returns vertex pairs where each mod(2) pair form an edge x */
void getCurrentHamiltonianCycleEdges(HCStateRef s, Vertex *e);

#endif /* HAMILTONIANCYCLE_H */

