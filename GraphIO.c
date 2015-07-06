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
#include <string.h>
#include "Arch.h"
#include "ArchIO.h"
#include "Graph.h"
#include "GraphIO.h"


#define DEFAULT_NAME "unnamed graph"

#define EOLN_MAC         '\r'     
#define EOLN_UNIX        '\n'     
#define STARTING_CHAR    '$'      /*!< Starting char before header */
#define GRAPH_HEADER_STR "&Graph" /*!< Graph header */
#define BUFFER_LENGTH    (GRAPH_MAX_TITLE_LENGTH + 1)    


/* useful file i/o and text macros */

#define WRITE_EOLN(s,a,b)                                                 \
    {                                                                     \
        *s++ = a;                                                         \
        if (b!=0) *s++ = b;                                             \
    }
    
#define READ_CHAR(c,inp,lbl)                                              \
     {                                                                     \
        c = fgetc(inp);                                                   \
        if (c == EOF) goto lbl;                                         \
    }
    
#define TERM_LINE(crlf,in,lbl)                                            \
    {                                                                     \
        if (crlf && fgetc(in) == EOF)  goto lbl;                        \
    }


/*! File format headers. Tokenizes headers to allow further expansion. */

typedef enum file_headers {
    EOF_ERROR      = -1,     /*!< EOF error while reading header. */
    UNKNOWN_HEADER =  0,     /*!< Unknown header. */
    GRAPH_HEADER   =  1      /*!< Graph header. */
} HToken;

struct graph_iterator {
    char**  file_list;
    int     list_length;
    UInt    current_file_info;
    int     current_file_pos;
    int     next_file_pos;
    bool    changedInputFile;
    bool    scanStarted;
    UInt    graphReadCount;
    FILE   *current_input_file;
    char    end_of_line;
    bool    crlf;
    bool    stop;
    bool    gng;
    char   *buffer;
};

static void
resetGraphIterator(GraphIteratorRef i)
{
    
    if (i->current_input_file) fclose(i->current_input_file);
    
    i->file_list              = NULL;
    i->list_length            = 0;
    i->current_input_file     = NULL;
    i->changedInputFile       = false;
    i->graphReadCount         = 0;
    i->current_file_pos       = 0;
    i->current_file_info      = 0;
    i->crlf                   = false;
    i->end_of_line            = 0;
    i->stop                   = false;
    i->gng                    = false;
    i->next_file_pos          = 0;
    i->scanStarted            = false;
    
    
} /* resetGraphIterator */

GraphIteratorRef
allocateGraphIterator(StatusRef s)
{
    GraphIteratorRef i;

    EM(i,         sizeof(struct graph_iterator),   e0);
    EM(i->buffer, sizeof(char) * BUFFER_LENGTH,    e1);
    
    i->current_input_file = NULL;

    return i;
    
e1: free(i);    
e0: *s  = STATUS_NO_MEM;
    return NULL;
    
} /* allocGraphIterator */

GraphIteratorRef
initGraphIterator(GraphIteratorRef i, FILE *f)
{
    if (!i) return NULL;
    
    resetGraphIterator(i);
    i->current_input_file = f;
    i->changedInputFile   = true;
    
    return i;
}

GraphIteratorRef
initGraphIteratorWithFile(GraphIteratorRef i, char *fname)
{
    if (!i) return NULL;
    
    resetGraphIterator(i);
    
    i->file_list   = malloc(sizeof(void*));
    *i->file_list  = fname;
    i->list_length = 1;
    return i;
    
}

GraphIteratorRef
initGraphIteratorWithFiles(GraphIteratorRef i, int cnt, char **fnames)
{    
    if (!i) return NULL;
    
    resetGraphIterator(i);
    
    i->file_list   = fnames;
    i->list_length = cnt;
    
    return i;
}

void
releaseGraphIterator(GraphIteratorRef i)
{
    if (!i) return;
    free(i->buffer);
    free(i);
    
} /* releaseGraphIterator */

static bool
determineEOfLn(FILE *f, int c, char *e, bool *CRLF)
{

    if (EOLN_UNIX == c) {
        *e = c;
        return false;
    } 
    
    if (EOLN_MAC == c) {    
        
        /* check for DOS end of line (CrLF) */
        
        *e = c;
        READ_CHAR(c, f, fail);
        
        if (EOLN_UNIX != c) {
            
            (void) ungetc(c, f);
            return false;
        }
        
        *CRLF  = true;
        return false;
    }
    
    return false;
    
fail:
    
    return true;

}

static bool
startScan(GraphIteratorRef i)
{
    int c;
    
    FILE * in   = i->current_input_file;
    char   e    = i->end_of_line;
    bool   crlf = i->crlf;
    
    if (!e) {

    /* scan for STARTING_CHAR */

        do {
            do {READ_CHAR(c, in, fail);} while(c != STARTING_CHAR);
            READ_CHAR(c, in, fail);
            if (determineEOfLn(in, c, &e, &crlf)) goto fail;
        } while (e != c);

        TERM_LINE(crlf, in, fail);

        i->end_of_line = e;
        i->crlf        = crlf;

        return false;    
    }

    do {
        do { READ_CHAR(c, in, fail);} while(c != STARTING_CHAR);
        READ_CHAR(c, in, fail);
    } while (c != e);
    
    TERM_LINE(crlf, in, fail);
    return false;
    
fail:

    i->current_input_file = NULL;
    return true;
    
} /* start_scan */

static bool
scanHeader(GraphIteratorRef i, HToken *t)
{
    int c;
    
    FILE  *in = i->current_input_file;
    char   e  = i->end_of_line;
    char  *s  = GRAPH_HEADER_STR;    
    
    do {
                           
        READ_CHAR(c, in, fail);
        
        if (c == e) {
            TERM_LINE(i->crlf, in, fail);
            *t = UNKNOWN_HEADER;     
            return false;
        }
        
        if (*s++ != (char)c) {
            *t = UNKNOWN_HEADER;     
            return false;
        }
        
    } while (*s);
    
    READ_CHAR(c, in, fail);  
    
    if (c != e) {
        *t = UNKNOWN_HEADER;     
        return false;
    }
    
    TERM_LINE(i->crlf, in, fail);

    *t = GRAPH_HEADER;   
    return false;
    
fail:

    i->current_input_file = NULL;
    return true;
    
} /* scan_header */

static char *
scanTitle(GraphIteratorRef i)
{
    int   c;

    int   n  = GRAPH_MAX_TITLE_LENGTH;
    FILE *in = i->current_input_file;
    char  e  = i->end_of_line;
    char *s  = i->buffer;

    /* read up to max length of characters */
    
    while (n--) {     
        
        READ_CHAR(c, in, fail);
        if (c == e) break;         
        *s++ = (char)c;

    }                      

     *s = (char) '\0';
               
    /* read up to end of line */
    
     while (c != e) READ_CHAR(c, in, fail); 
     TERM_LINE(i->crlf, in, fail);

    return i->buffer;
    
fail:

    i->current_input_file = NULL;
    return NULL;
    
} /* scan_title */

static bool
parseGraph(GraphIteratorRef i, Graph **graph, StatusRef stat)
{
    SInt        x, u, pts, max;
    VArray      vlist;
    Vertex     *p;

    Graph  *g  = NULL;
    FILE       *in = i->current_input_file;
    char       *s  = scanTitle(i);
    
    if (!s) return true;
    
    vlist.array = NULL;
    
    if (read_signedval(in, &pts)) goto e2;

    /* create and initialize graph */

    g = initGraph(allocateGraph(s, pts, stat));

    CHECK_RETURN_VAL(*stat, true);

    if (read_signedval(in, &x)) goto e2;
    
    if (!x) x = -g->vertex_count;
    
    pts--;
    EM(vlist.array, sizeof(Vertex) * pts, e1);
    
    max = pts;
    
    /* load graph edges */
    
    while (x < 0) {

        u = -x;                         

        if (read_signedval(in, &x)) goto e2;
 
        pts = max;                       
        p   = vlist.array;
        
        /* determine vertices adjacent to u */
        
        while (x > 0 && pts--){           
            
            *p++ = x;
            if (read_signedval(in, &x)) goto e2;
        }                       
        
          vlist.length = max - pts;
        
        *stat = createEdges(g, &vlist, u);
        CHECK_JUMP(*stat, e0);

    }                           

    free(vlist.array);
    
    *graph = g;
    return false;
    
    /* error cleanup */

e2: fclose(in);
    i->current_input_file = NULL;
    goto e0;
    
e1: *stat = STATUS_NO_MEM;
e0: 
    
    if (vlist.array) free(vlist.array);
    if (g) releaseGraph(g);
    return true;

} /* parseGraph */

static void
getGraphNameFromFileName(char *fn, char *buf)
{
    strncpy(buf, fn,  GRAPH_MAX_TITLE_LENGTH - 1);
}

bool
readGnGFile(GraphIteratorRef i, Graph **graph,  StatusRef s)
{
    short int  pts, x, y;
    char       b;
    VArray     va;
    char       buffer[GRAPH_MAX_TITLE_LENGTH];
    char       *fn, *gn;
    
    FILE      *in = i->current_input_file;
    Graph *g  = NULL;
    
    i->current_input_file = NULL;
    if (in != stdin) {
        fn = i->file_list[i->current_file_pos];
        gn = buffer;
        getGraphNameFromFileName(fn, gn);
    } else gn = DEFAULT_NAME;
    
    /* first 2 bytes have been read, next byte is for
       type, must be a 1 for a graph file */
    
    fread(&b, 1, 1, in);
    if (b != 1) return true;
    
    fseek(in, 3, SEEK_CUR);  /* skip over unused status info */
    fread(&pts, 2, 1, in);    /* get vertex count */
    fseek(in, 14, SEEK_CUR); /* skip over window info, 
                                 edge count, subgraph flag, and UI data */
    
    if (ferror(in) || feof(in)) return true;
    
    /* ready to create graph and populate adjacency information */
    
    g = initGraph(allocateGraph(gn, pts, s));
    if (!g) return true;
    
    EM(va.array, sizeof(Vertex) * pts, e1);

    /* read in adjacency info, very similar to text */

    fread(&x, 2, 1, in);
    if (ferror(in) || feof(in)) goto e2;

    while (x < 0) {
        pts = 0;
        x   = -x;
        
        fread(&y, 2, 1, in); 
        if (ferror(in) || feof(in)) goto e2;

        while (y > 0) {
            va.array[pts++]= y;
            fseek(in, 2, SEEK_CUR); /* skip over edge multiplicity info */
            fread(&y, 2, 1, in); 
            if (ferror(in) || feof(in)) goto e2;

        }
        
        va.length = pts;
        CHECK_RETURN_VAL((*s = createEdges(g, &va, x)) , true);
        
        x = y;
    }
    

    free(va.array);
    *graph = g;
    return false;

e2: if (g) releaseGraph(g);
    return true;
    
e1: *s = STATUS_NO_MEM;
    if (g) releaseGraph(g);
    return true;
       
} /* read_gng_file */

static bool
loadNextFile(GraphIteratorRef i)
{
    char *fname;
    FILE *in = NULL;
    
    if (!i->file_list) return false;
    
    do {
        i->current_file_pos = i->next_file_pos++;
        if (i->current_file_pos >= i->list_length) return false;
        fname = i->file_list[i->current_file_pos];
        if (fname[0] == '-') {
            if (fname[1]) continue;
            in = stdin;
        } else in = fopen(fname, "r");
        
        i->end_of_line = 0;
        i->crlf        = false;
        
    } while (!in);
    
    i->scanStarted        = false;
    i->changedInputFile   = true;
    i->current_input_file = in;

    return true;
} /* loadNextFile */

static bool
isGnGFile(GraphIteratorRef i)
{

    short k;
    fread(&k, 2, 1, i->current_input_file);
    if (k == -1) return true;
    rewind(i->current_input_file);
    return false;
}

bool
loadNextGraph(GraphIteratorRef i, Graph **gPtr, StatusRef s)
{
    HToken  t;
loop:

    CHECK_RETURN_VAL(*s, false);

    if (!i->current_input_file && !loadNextFile(i)) return false;
    
    if (!i->scanStarted && isGnGFile(i)) {
        if (readGnGFile(i,gPtr,s)) {
            CHECK_RETURN_VAL(*s, false);
            goto loop;
        }
        i->scanStarted      = true;
        i->changedInputFile = true;
        return true;
    }
    
    i->scanStarted = true;
    
    do {
        /* scan to initial token */
        if (startScan(i) || scanHeader(i, &t)) goto loop;
    } while (t != GRAPH_HEADER);
    
    /* try to parse graph, keep scanning for graph if parse fails */
    
    if (parseGraph(i, gPtr, s)){
        CHECK_RETURN_VAL(*s, false);
        goto loop;
    }
    
    i->changedInputFile = !i->graphReadCount++;
    return true;
    
} /* loadNextGraph */
