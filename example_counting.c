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
#include "Graph.h"
#include "GraphIO.h"
#include "HamiltonianCycle.h"
#include "VertexOrder.h"


#define COPYRIGHT "Copyright (C) 2009 Andrew Chalaturnyk and William Kocay.\n\n"\
"This program is free software: you can redistribute it and/or modify\n"\
"it under the terms of the GNU General Public License as published by\n"\
"the Free Software Foundation, either version 3 of the License, or\n"\
"(at your option) any later version.\n"\
"\n"\
"This program is distributed in the hope that it will be useful,\n"\
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
"GNU General Public License for more details.\n"\
"\n"\
"You should have received a copy of the GNU General Public License\n"\
"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"

static void
print_usage()
{
	puts(COPYRIGHT);
	puts("This program outputs the hamiltonian cycle counts found"
		" for each of the the inputed graphs.\n");
	puts("Usage: hc [-p] files...\n");
    puts("Flags:\n\t-h\thelp\n\t-p\tUse pruning algorithm during search.\n");
    
}

int main(int argc, char ** argv)
{
    Graph *g;
    GraphIteratorRef i;
    HCStateRef hc;
    VArray *vo;
    Status  stat = STATUS_OK;
    StatusRef  s = &stat;
    UInt    pts; 
	ULongLong c;
    UInt    t;
    bool    prune = 0;
    
    if (argc == 1) { 
        print_usage();
        exit(0);
    }
    /* first scan arguments for -h or -p flags */
    for (t = 0; t < argc; t++) {
        if (argv[t][0] == '-' && argv[t][1]){
            if (argv[t][1]=='h'){ 
                print_usage();
                exit(0);
            }
            prune = argv[t][1] == 'p';
        }
    }    

    i = allocateGraphIterator(s);
    initGraphIteratorWithFiles(i,argc - 1 , ++argv);
    
    while ( loadNextGraph(i, &g, s) ){
        
        pts = g->vertex_count;
        vo  = sortVerticesDegreeDesc( 
                    initVArray(allocateVArray(pts, s)), g->degree);
        
        hc = initHCState(allocateHCState(g->vertex_count, s),
                         g->degree, g->adj_lists, vo);
        
		c = 0;
        if (firstHamiltonianCycle(hc))		
			do c++; while (nextHamiltonianCycle(hc));
        printf("%s has %" c_PF_ULongLong " Hamiltonian Cycles.\n", 
                 g->name, c);
        releaseHCState(hc);
        releaseVArray(vo);
        releaseGraph(g);
    }
    
    releaseGraphIterator(i);
    return 0;
}



