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

#ifndef GRAPHIO_H
#define GRAPHIO_H

#include <stdio.h>
#include "Arch.h"
#include "ArchIO.h"
#include "Graph.h"

typedef struct graph_iterator *GraphIteratorRef;  /* opaque type */

GraphIteratorRef allocateGraphIterator(Status *status);
GraphIteratorRef initGraphIterator(GraphIteratorRef i, FILE *file);
GraphIteratorRef initGraphIteratorWithFile(GraphIteratorRef i, char *file);
GraphIteratorRef initGraphIteratorWithFiles(GraphIteratorRef i, 
                                             int c, char **files);
void releaseGraphIterator(GraphIteratorRef i);
bool loadNextGraph(GraphIteratorRef i, Graph **g, Status *status);

#endif /* GRAPHIO_H */

