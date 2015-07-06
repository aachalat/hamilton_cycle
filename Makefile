
CC     =  gcc
CFLAGS =  -Winline -Wall -pipe 

debug: CFLAGS = -g -Winline -Wall -pipe 
debug: all

release: CFLAGS = -Wall -pipe -fast 
release: all

ArchIO.o: ArchIO.c ArchIO.h Arch.h

Graph.o: Graph.c Arch.h Graph.h

GraphIO.o: ArchIO.o Graph.o GraphIO.c GraphIO.h

VertexOrder.o: Graph.o VertexOrder.h VertexOrder.c

DFSAlgorithms.o: Graph.o DFSAlgorithms.c DFSAlgorithms.h

HamiltonianCycle.o: Graph.o DFSAlgorithms.o HamiltonianCycle.c \
                    HamiltonianCycle.h 

graph_algs = Graph.o GraphIO.o DFSAlgorithms.o ArchIO.o VertexOrder.o HamiltonianCycle.o

# Unix command line utililty

all: $(graph_algs) hc hc_count hc_list_cycles

hc: $(graph_algs) example.c
		$(CC) $(CFLAGS) -o $@ $(graph_algs) example.c
		
hc_count: $(graph_algs) example_counting.c
		$(CC) $(CFLAGS) -o $@ $(graph_algs) example_counting.c
		
hc_list_cycles: $(graph_algs) example_listing.c
		$(CC) $(CFLAGS) -o $@ $(graph_algs) example_listing.c

clean:
	rm -f *.o hc hc_count hc_list_cycles 
